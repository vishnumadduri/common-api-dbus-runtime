// Copyright (C) 2013-2015 Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <algorithm>
#include <cassert>
#include <chrono>
#define BOOST_THREAD_PROVIDES_FUTURE
#include <boost/thread/future.hpp>
#include <sstream>
#include <thread>

#include <CommonAPI/Logger.hpp>
#include <CommonAPI/DBus/DBusConnection.hpp>
#include <CommonAPI/DBus/DBusInputStream.hpp>
#include <CommonAPI/DBus/DBusProxy.hpp>
#include <CommonAPI/DBus/DBusAddressTranslator.hpp>


namespace CommonAPI {
namespace DBus {

DBusConnectionStatusEvent::DBusConnectionStatusEvent(DBusConnection* dbusConnection):
                dbusConnection_(dbusConnection) {
}

void DBusConnectionStatusEvent::onListenerAdded(const Listener& listener, const Subscription subscription) {
    if (dbusConnection_->isConnected())
        listener(AvailabilityStatus::AVAILABLE);
}


const DBusObjectPathVTable* DBusConnection::getDBusObjectPathVTable() {
    static const DBusObjectPathVTable libdbusObjectPathVTable = {
                    NULL, // no need to handle unregister callbacks
                    &DBusConnection::onLibdbusObjectPathMessageThunk
    };
    return &libdbusObjectPathVTable;
}

//std::bind used to start the dispatch thread holds one reference, and the selfReference
//created within the thread is the second. If only those two remain, no one but the
//dispatch thread references the connection, which therefore can be finished.
const int32_t ownUseCount = 2;

void DBusConnection::dispatch() {
	std::shared_ptr<DBusConnection> selfReference = this->shared_from_this();
	while (!stopDispatching_ && readWriteDispatch(10) && selfReference.use_count() > ownUseCount) {
		if (pauseDispatching_) {
			dispatchSuspendLock_.lock();
			dispatchSuspendLock_.unlock();
		}
	}
}

bool DBusConnection::readWriteDispatch(int _timeout) {
    if(isConnected()) {
        return 0 != dbus_connection_read_write_dispatch(connection_, _timeout);
    }
    return false;
}

void DBusConnection::suspendDispatching() const {
    dispatchSuspendLock_.lock();
    pauseDispatching_ = true;
}

void DBusConnection::resumeDispatching() const {
    pauseDispatching_ = false;
    dispatchSuspendLock_.unlock();
}

DBusConnection::DBusConnection(DBusType_t busType) :
                dispatchThread_(NULL),
                stopDispatching_(false),
                dispatchSource_(),
                watchContext_(NULL),
                pauseDispatching_(false),
                connection_(NULL),
                busType_(busType),
                dbusConnectionStatusEvent_(this),
                libdbusSignalMatchRulesCount_(0),
                dbusObjectMessageHandler_(),
                connectionNameCount_(),
                enforcerThread_(NULL),
                enforcerThreadCancelled_(false) {

    dbus_threads_init_default();
}

DBusConnection::DBusConnection(::DBusConnection *_connection) :
                dispatchThread_(NULL),
                stopDispatching_(false),
                dispatchSource_(),
                watchContext_(NULL),
                pauseDispatching_(false),
                connection_(_connection),
                busType_(DBusType_t::WRAPPED),
                dbusConnectionStatusEvent_(this),
                libdbusSignalMatchRulesCount_(0),
                dbusObjectMessageHandler_(),
                connectionNameCount_(),
                enforcerThread_(NULL),
                enforcerThreadCancelled_(false) {

    dbus_threads_init_default();
}

void DBusConnection::setObjectPathMessageHandler(DBusObjectPathMessageHandler handler) {
    dbusObjectMessageHandler_ = handler;
}

bool DBusConnection::isObjectPathMessageHandlerSet() {
    return dbusObjectMessageHandler_.operator bool();
}

DBusConnection::~DBusConnection() {
    if (auto lockedContext = mainLoopContext_.lock()) {
        dbus_connection_set_watch_functions(connection_, NULL, NULL, NULL, NULL, NULL);
        dbus_connection_set_timeout_functions(connection_, NULL, NULL, NULL, NULL, NULL);

        lockedContext->deregisterDispatchSource(dispatchSource_);
        delete watchContext_;
        delete dispatchSource_;
    }

    // ensure, the registry survives until disconnecting is done...
    //std::shared_ptr<DBusServiceRegistry> itsRegistry = DBusServiceRegistry::get(shared_from_this());
       disconnect();

    //Assert that the enforcerThread_ is in a position to finish itself correctly even after destruction
    //of the DBusConnection. Also assert all resources are cleaned up.
    auto it = timeoutMap_.begin();
    while (it != timeoutMap_.end()) {
        DBusPendingCall* libdbusPendingCall = it->first;

        if (!dbus_pending_call_get_completed(libdbusPendingCall)) {
            dbus_pending_call_cancel(libdbusPendingCall);
            DBusMessageReplyAsyncHandler* asyncHandler = std::get<1>(it->second);
            DBusMessage& dbusMessageCall = std::get<2>(it->second);

            asyncHandler->onDBusMessageReply(CallStatus::REMOTE_ERROR, dbusMessageCall.createMethodError(DBUS_ERROR_TIMEOUT));
            delete asyncHandler;

        }
        it = timeoutMap_.erase(it);
        dbus_pending_call_unref(libdbusPendingCall);
    }
}


bool DBusConnection::attachMainLoopContext(std::weak_ptr<MainLoopContext> mainLoopContext) {
    mainLoopContext_ = mainLoopContext;

    if (auto lockedContext = mainLoopContext_.lock()) {
        dispatchSource_ = new DBusDispatchSource(this);
        watchContext_ = new WatchContext(mainLoopContext_, dispatchSource_);
        lockedContext->registerDispatchSource(dispatchSource_);

        dbus_connection_set_wakeup_main_function(
                        connection_,
                        &DBusConnection::onWakeupMainContext,
                        &mainLoopContext_,
                        NULL);

        bool success = 0 != dbus_connection_set_watch_functions(
                connection_,
                &DBusConnection::onAddWatch,
                &DBusConnection::onRemoveWatch,
                &DBusConnection::onToggleWatch,
                watchContext_,
                NULL);

        if (!success) {
            return false;
        }

        success = 0 != dbus_connection_set_timeout_functions(
                connection_,
                &DBusConnection::onAddTimeout,
                &DBusConnection::onRemoveTimeout,
                &DBusConnection::onToggleTimeout,
                &mainLoopContext_,
                NULL);

        if (!success) {
            dbus_connection_set_watch_functions(connection_, NULL, NULL, NULL, NULL, NULL);
            return false;
        }

        return true;
    }
    return false;
}

void DBusConnection::onWakeupMainContext(void* data) {
    std::weak_ptr<MainLoopContext>* mainloop = static_cast<std::weak_ptr<MainLoopContext>*>(data);
    assert(mainloop);

    if(auto lockedContext = mainloop->lock()) {
        lockedContext->wakeup();
    }
}

dbus_bool_t DBusConnection::onAddWatch(::DBusWatch* libdbusWatch, void* data) {
    WatchContext* watchContext = static_cast<WatchContext*>(data);
    assert(watchContext);

    DBusWatch* dbusWatch = new DBusWatch(libdbusWatch, watchContext->mainLoopContext_);
    dbusWatch->addDependentDispatchSource(watchContext->dispatchSource_);
    dbus_watch_set_data(libdbusWatch, dbusWatch, NULL);

    if (dbusWatch->isReadyToBeWatched()) {
        dbusWatch->startWatching();
    }

    return TRUE;
}

void DBusConnection::onRemoveWatch(::DBusWatch* libdbusWatch, void* data) {
    assert(static_cast<WatchContext*>(data));

    DBusWatch* dbusWatch = static_cast<DBusWatch*>(dbus_watch_get_data(libdbusWatch));
    if(dbusWatch->isReadyToBeWatched()) {
        dbusWatch->stopWatching();
    }
    dbus_watch_set_data(libdbusWatch, NULL, NULL);
    delete dbusWatch;
}

void DBusConnection::onToggleWatch(::DBusWatch* libdbusWatch, void* data) {
    assert(static_cast<WatchContext*>(data));

    DBusWatch* dbusWatch = static_cast<DBusWatch*>(dbus_watch_get_data(libdbusWatch));

    if (dbusWatch->isReadyToBeWatched()) {
        dbusWatch->startWatching();
    } else {
        dbusWatch->stopWatching();
    }
}


dbus_bool_t DBusConnection::onAddTimeout(::DBusTimeout* libdbusTimeout, void* data) {
    std::weak_ptr<MainLoopContext>* mainloop = static_cast<std::weak_ptr<MainLoopContext>*>(data);
    assert(mainloop);

    DBusTimeout* dbusTimeout = new DBusTimeout(libdbusTimeout, *mainloop);
    dbus_timeout_set_data(libdbusTimeout, dbusTimeout, NULL);

    if (dbusTimeout->isReadyToBeMonitored()) {
        dbusTimeout->startMonitoring();
    }

    return TRUE;
}

void DBusConnection::onRemoveTimeout(::DBusTimeout* libdbusTimeout, void* data) {
    assert(static_cast<std::weak_ptr<MainLoopContext>*>(data));

    DBusTimeout* dbusTimeout = static_cast<DBusTimeout*>(dbus_timeout_get_data(libdbusTimeout));
    dbusTimeout->stopMonitoring();
    dbus_timeout_set_data(libdbusTimeout, NULL, NULL);
    delete dbusTimeout;
}

void DBusConnection::onToggleTimeout(::DBusTimeout* dbustimeout, void* data) {
    assert(static_cast<std::weak_ptr<MainLoopContext>*>(data));

    DBusTimeout* timeout = static_cast<DBusTimeout*>(dbus_timeout_get_data(dbustimeout));
    if (timeout->isReadyToBeMonitored()) {
        timeout->startMonitoring();
    } else {
        timeout->stopMonitoring();
    }
}

bool DBusConnection::connect(bool startDispatchThread) {
    DBusError dbusError;
    return connect(dbusError, startDispatchThread);
}

bool DBusConnection::connect(DBusError &dbusError, bool startDispatchThread) {
    assert(!dbusError);
    if (isConnected()) {
        return true;
    }

    const ::DBusBusType libdbusType = static_cast<DBusBusType>(busType_);

    connection_ = dbus_bus_get_private(libdbusType, &dbusError.libdbusError_);
    if (dbusError) {
        #ifdef _MSC_VER
                COMMONAPI_ERROR(std::string(__FUNCTION__) +
                    ": Name: " + dbusError.getName() +
                    " Message: " + dbusError.getMessage())
        #else
                COMMONAPI_ERROR(std::string(__PRETTY_FUNCTION__) +
                    ": Name: " + dbusError.getName() +
                    " Message: " + dbusError.getMessage())
        #endif
        return false;
    }

    assert(connection_);
    dbus_connection_set_exit_on_disconnect(connection_, false);

    initLibdbusObjectPathHandlerAfterConnect();

    initLibdbusSignalFilterAfterConnect();

    stopDispatching_ = !startDispatchThread;
    if (startDispatchThread) {
    	dispatchThread_ = new boost::thread(std::bind(&DBusConnection::dispatch, this->shared_from_this()));
    }

    enforcerThread_ = std::make_shared<boost::thread>(
    					std::bind(&DBusConnection::enforceAsynchronousTimeouts, shared_from_this()));

    dbusConnectionStatusEvent_.notifyListeners(AvailabilityStatus::AVAILABLE);

    return true;
}

void DBusConnection::disconnect() {
    std::lock_guard<std::mutex> dbusConnectionLock(connectionGuard_);
    if (isConnected()) {
        dbusConnectionStatusEvent_.notifyListeners(AvailabilityStatus::NOT_AVAILABLE);

        if (libdbusSignalMatchRulesCount_ > 0) {
            dbus_connection_remove_filter(connection_, &onLibdbusSignalFilterThunk, this);
            libdbusSignalMatchRulesCount_ = 0;
        }

        connectionNameCount_.clear();

        stopDispatching_ = true;

        dbus_connection_close(connection_);

        if(dispatchThread_) {
            //It is possible for the disconnect to be called from within a callback, i.e. from within the dispatch
            //thread. Self-join is prevented this way.
            if (dispatchThread_->joinable() && boost::this_thread::get_id() != dispatchThread_->get_id()) {
                dispatchThread_->join();
            } else {
                dispatchThread_->detach();
            }
            delete dispatchThread_;
            dispatchThread_ = NULL;
        }

        enforcerThreadCancelled_ = true;
        enforceTimeoutCondition_.notify_one();
        if (enforcerThread_->joinable()) {
            enforcerThread_->join();
        }
        enforcerThreadCancelled_ = false;

        dbus_connection_unref(connection_);
        connection_ = nullptr;
    }
}

bool DBusConnection::isConnected() const {
    return (connection_ != NULL);
}

DBusProxyConnection::ConnectionStatusEvent& DBusConnection::getConnectionStatusEvent() {
    return dbusConnectionStatusEvent_;
}

//Does this need to be a weak pointer?
const std::shared_ptr<DBusObjectManager> DBusConnection::getDBusObjectManager() {
    if (!dbusObjectManager_) {
        objectManagerGuard_.lock();
        if (!dbusObjectManager_) {
            dbusObjectManager_ = std::make_shared<DBusObjectManager>(shared_from_this());
        }
        objectManagerGuard_.unlock();
    }
    return dbusObjectManager_;
}

bool DBusConnection::requestServiceNameAndBlock(const std::string& serviceName) const {
    DBusError dbusError;
    bool isServiceNameAcquired = false;

    std::lock_guard<std::mutex> dbusConnectionLock(connectionGuard_);
    auto conIter = connectionNameCount_.find(serviceName);
    if (conIter == connectionNameCount_.end()) {
    	suspendDispatching();

        const int libdbusStatus = dbus_bus_request_name(connection_,
                        serviceName.c_str(),
                        DBUS_NAME_FLAG_DO_NOT_QUEUE,
                        &dbusError.libdbusError_);

        resumeDispatching();

        isServiceNameAcquired = (libdbusStatus == DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER);
        if (isServiceNameAcquired) {
            connectionNameCount_.insert( { serviceName, (uint16_t)1 } );
        }
        else {
            #ifdef _MSC_VER // Visual Studio
            COMMONAPI_ERROR(std::string(__FUNCTION__) +
                ": Name: " + dbusError.getName() +
                " Message: " + dbusError.getMessage())
            #else
            COMMONAPI_ERROR(std::string(__PRETTY_FUNCTION__) +
                            ": Name: " + dbusError.getName() +
                            " Message: " + dbusError.getMessage())
            #endif
        }
    } else {
        conIter->second = conIter->second + 1;
        isServiceNameAcquired = true;
    }

    return isServiceNameAcquired;
}

bool DBusConnection::releaseServiceName(const std::string& serviceName) const {
    DBusError dbusError;
    bool isServiceNameReleased = false;
    std::lock_guard<std::mutex> dbusConnectionLock(connectionGuard_);
    auto conIter = connectionNameCount_.find(serviceName);
    if (conIter != connectionNameCount_.end()) {
        if (conIter->second == 1) {
        	suspendDispatching();
            const int libdbusStatus = dbus_bus_release_name(connection_,
                            serviceName.c_str(),
                            &dbusError.libdbusError_);
            resumeDispatching();
            isServiceNameReleased = (libdbusStatus == DBUS_RELEASE_NAME_REPLY_RELEASED);
            if (isServiceNameReleased) {
                connectionNameCount_.erase(conIter);
            }
        } else {
            conIter->second = conIter->second - 1;
            isServiceNameReleased = true;
        }
    }
    return isServiceNameReleased;
}

bool DBusConnection::sendDBusMessage(const DBusMessage &_message) const {
    assert(_message);
    assert(isConnected());

    dbus_uint32_t dbusSerial;
    bool result = 0 != dbus_connection_send(connection_, _message.message_, &dbusSerial);
    return result;
}

DBusMessage DBusConnection::convertToDBusMessage(::DBusPendingCall* _libdbusPendingCall,
		CallStatus& _callStatus) {
	assert(_libdbusPendingCall);

	::DBusMessage* libdbusMessage = dbus_pending_call_steal_reply(_libdbusPendingCall);
	const bool increaseLibdbusMessageReferenceCount = false;
	DBusMessage dbusMessage(libdbusMessage, increaseLibdbusMessageReferenceCount);
	_callStatus = CallStatus::SUCCESS;

	if (!dbusMessage.isMethodReturnType()) {
		_callStatus = CallStatus::REMOTE_ERROR;
	}

	return dbusMessage;
}

void DBusConnection::onLibdbusPendingCallNotifyThunk(::DBusPendingCall* _libdbusPendingCall, void *_userData) {
    assert(_userData);
    assert(_libdbusPendingCall);

    auto dbusMessageReplyAsyncHandler = reinterpret_cast<DBusMessageReplyAsyncHandler*>(_userData);

    DBusMessage dbusMessage;
    CallStatus callStatus;
    dbusMessage = DBusConnection::convertToDBusMessage(_libdbusPendingCall, callStatus);

    dbusMessageReplyAsyncHandler->onDBusMessageReply(callStatus, dbusMessage);

    // libdbus calls the cleanup method below
    dbus_pending_call_unref(_libdbusPendingCall);
}

void DBusConnection::onLibdbusDataCleanup(void* userData) {
    auto dbusMessageReplyAsyncHandler = reinterpret_cast<DBusMessageReplyAsyncHandler*>(userData);
    delete dbusMessageReplyAsyncHandler;
}

//Would not be needed if libdbus would actually handle its timeouts for pending calls.
void DBusConnection::enforceAsynchronousTimeouts() const {
    std::unique_lock<std::mutex> itsLock(enforcerThreadMutex_);

    while (!enforcerThreadCancelled_) {
        enforceTimeoutMutex_.lock();

        int timeout = std::numeric_limits<int>::max(); // not really, but nearly "forever"
        if (timeoutMap_.size() > 0) {
            auto minTimeoutElement = std::min_element(timeoutMap_.begin(), timeoutMap_.end(),
                    [] (const TimeoutMapElement& lhs, const TimeoutMapElement& rhs) {
                        return std::get<0>(lhs.second) < std::get<0>(rhs.second);
            });

            auto minTimeout = std::get<0>(minTimeoutElement->second);

            std::chrono::high_resolution_clock::time_point now = std::chrono::high_resolution_clock::now();

            timeout = (int)std::chrono::duration_cast<std::chrono::milliseconds>(minTimeout - now).count();
        }

        enforceTimeoutMutex_.unlock();

        if (std::cv_status::timeout ==
            enforceTimeoutCondition_.wait_for(itsLock, std::chrono::milliseconds(timeout))) {

            //Do not access members if the DBusConnection was destroyed during the unlocked phase.
            enforceTimeoutMutex_.lock();
            auto it = timeoutMap_.begin();
            while (it != timeoutMap_.end()) {
                std::chrono::high_resolution_clock::time_point now = std::chrono::high_resolution_clock::now();

                if (now > std::get<0>(it->second)) {
                    DBusPendingCall* libdbusPendingCall = it->first;

                    if (!dbus_pending_call_get_completed(libdbusPendingCall)) {
                        dbus_pending_call_cancel(libdbusPendingCall);
                        DBusMessageReplyAsyncHandler* asyncHandler = std::get<1>(it->second);
                        DBusMessage& dbusMessageCall = std::get<2>(it->second);

                        if (mainLoopContext_.lock()) {
                        	mainloopTimeouts_.push_back(std::make_tuple(asyncHandler,
													dbusMessageCall.createMethodError(DBUS_ERROR_TIMEOUT),
													CallStatus::REMOTE_ERROR,
													nullptr));
                        							mainLoopContext_.lock()->wakeup();
                        } else {
                        	enforceTimeoutMutex_.unlock(); // unlock before making callbacks to application to avoid deadlocks
                        	asyncHandler->onDBusMessageReply(CallStatus::REMOTE_ERROR, dbusMessageCall.createMethodError(DBUS_ERROR_TIMEOUT));
                        	enforceTimeoutMutex_.lock();
                        	delete asyncHandler;
                        }
                    }
                    it = timeoutMap_.erase(it);

                    //This unref MIGHT cause the destruction of the last callback object that references the DBusConnection.
                    //So after this unref has been called, it has to be ensured that continuation of the loop is an option.
                    dbus_pending_call_unref(libdbusPendingCall);
                } else {
                    ++it;
                }
            }
            enforceTimeoutMutex_.unlock();
        }
    }
}

boost::future<CallStatus> DBusConnection::sendDBusMessageWithReplyAsync(
        const DBusMessage& dbusMessage,
        std::unique_ptr<DBusMessageReplyAsyncHandler> dbusMessageReplyAsyncHandler,
        const CommonAPI::CallInfo *_info) const {

    assert(dbusMessage);
    assert(isConnected());

    DBusPendingCall* libdbusPendingCall;
    dbus_bool_t libdbusSuccess;

    suspendDispatching();
    libdbusSuccess = dbus_connection_send_with_reply_set_notify(connection_,
    		dbusMessage.message_,
			&libdbusPendingCall,
			onLibdbusPendingCallNotifyThunk,
			dbusMessageReplyAsyncHandler.get(),
			onLibdbusDataCleanup,
			_info->timeout_);

    if (_info->sender_ != 0) {
        COMMONAPI_DEBUG("Message sent: SenderID: ", _info->sender_, " - Serial number: ", dbusMessage.getSerial());
    }

    if (!libdbusSuccess || !libdbusPendingCall) {
		#ifdef _MSC_VER // Visual Studio
            COMMONAPI_ERROR(std::string(__FUNCTION__) +
                ": (!libdbusSuccess || !libdbusPendingCall) == true")
        #else
            COMMONAPI_ERROR(std::string(__PRETTY_FUNCTION__) +
                            ": (!libdbusSuccess || !libdbusPendingCall) == true")
        #endif
		if (libdbusPendingCall) {
			dbus_pending_call_unref(libdbusPendingCall);
		}
		dbusMessageReplyAsyncHandler->onDBusMessageReply(CallStatus::CONNECTION_FAILED, dbusMessage.createMethodError(DBUS_ERROR_DISCONNECTED));
		resumeDispatching();
		return dbusMessageReplyAsyncHandler->getFuture();
    }

    DBusMessageReplyAsyncHandler* replyAsyncHandler = dbusMessageReplyAsyncHandler.release();

    if (_info->timeout_ != DBUS_TIMEOUT_INFINITE) {
        dbus_pending_call_ref(libdbusPendingCall);
        auto timeoutPoint = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(_info->timeout_);
        std::tuple<
            std::chrono::time_point<std::chrono::high_resolution_clock>,
            DBusMessageReplyAsyncHandler*,
            DBusMessage> toInsert {
                timeoutPoint,
                replyAsyncHandler,
                dbusMessage
            };

        enforceTimeoutMutex_.lock();
        timeoutMap_.insert( { libdbusPendingCall, toInsert } );
        enforceTimeoutMutex_.unlock();

        enforceTimeoutCondition_.notify_all();
    }

    boost::future<CallStatus> result = replyAsyncHandler->getFuture();

    resumeDispatching();

    return result;
}

DBusMessage DBusConnection::sendDBusMessageWithReplyAndBlock(const DBusMessage& dbusMessage,
                                                             DBusError& dbusError,
                                                             const CommonAPI::CallInfo *_info) const {
    assert(dbusMessage);
    assert(!dbusError);
    assert(isConnected());

    suspendDispatching();

    ::DBusMessage* libdbusMessageReply = dbus_connection_send_with_reply_and_block(connection_,
                                                                                   dbusMessage.message_,
                                                                                   _info->timeout_,
                                                                                   &dbusError.libdbusError_);

    if (_info->sender_ != 0) {
        COMMONAPI_DEBUG("Message sent: SenderID: ", _info->sender_, " - Serial number: ", dbusMessage.getSerial());
    }

    resumeDispatching();

    if (dbusError) {
        return DBusMessage();
    }

    const bool increaseLibdbusMessageReferenceCount = false;
    return DBusMessage(libdbusMessageReply, increaseLibdbusMessageReferenceCount);
}


bool DBusConnection::singleDispatch() {
    for (auto t : mainloopTimeouts_) {
    	std::get<0>(t)->onDBusMessageReply(std::get<2>(t), std::get<1>(t));
    	if (std::get<3>(t) != nullptr) {
    		dbus_pending_call_unref(std::get<3>(t));
    	}
    	delete std::get<0>(t);
    }
    mainloopTimeouts_.clear();

    return (dbus_connection_dispatch(connection_) == DBUS_DISPATCH_DATA_REMAINS);
}

bool DBusConnection::isDispatchReady() {
    return (dbus_connection_get_dispatch_status(connection_) == DBUS_DISPATCH_DATA_REMAINS ||
            !mainloopTimeouts_.empty());
}

bool DBusConnection::hasDispatchThread() {
    return (dispatchThread_ != NULL);
}

DBusProxyConnection::DBusSignalHandlerToken DBusConnection::subscribeForSelectiveBroadcast(
                    bool& subscriptionAccepted,
                    const std::string& objectPath,
                    const std::string& interfaceName,
                    const std::string& interfaceMemberName,
                    const std::string& interfaceMemberSignature,
                    DBusSignalHandler* dbusSignalHandler,
                    DBusProxy* callingProxy) {

    std::string methodName = "subscribeFor" + interfaceMemberName + "Selective";

    subscriptionAccepted = false;

    CommonAPI::CallStatus callStatus;
    DBusProxyHelper<CommonAPI::DBus::DBusSerializableArguments<>,
                    CommonAPI::DBus::DBusSerializableArguments<bool>>::callMethodWithReply(
                    *callingProxy, methodName.c_str(), "", &CommonAPI::DBus::defaultCallInfo, callStatus, subscriptionAccepted);

    DBusProxyConnection::DBusSignalHandlerToken subscriptionToken;
    if (callStatus == CommonAPI::CallStatus::SUCCESS && subscriptionAccepted) {
        subscriptionToken = addSignalMemberHandler(
                                objectPath,
                                interfaceName,
                                interfaceMemberName,
                                interfaceMemberSignature,
                                dbusSignalHandler,
                                true
                            );
        subscriptionAccepted = true;
    }

    return (subscriptionToken);
}

void DBusConnection::unsubscribeFromSelectiveBroadcast(const std::string& eventName,
                                                      DBusProxyConnection::DBusSignalHandlerToken subscription,
                                                      DBusProxy* callingProxy,
                                                      const DBusSignalHandler* dbusSignalHandler) {
    bool lastListenerOnConnectionRemoved = removeSignalMemberHandler(subscription, dbusSignalHandler);

    if (lastListenerOnConnectionRemoved) {
        // send unsubscribe message to stub
        std::string methodName = "unsubscribeFrom" + eventName + "Selective";
        CommonAPI::CallStatus callStatus;
        DBusProxyHelper<CommonAPI::DBus::DBusSerializableArguments<>,
                        CommonAPI::DBus::DBusSerializableArguments<>>::callMethodWithReply(
                        *callingProxy, methodName.c_str(), "", &CommonAPI::DBus::defaultCallInfo, callStatus);
    }
}

DBusProxyConnection::DBusSignalHandlerToken DBusConnection::addSignalMemberHandler(const std::string& objectPath,
                                                                                   const std::string& interfaceName,
                                                                                   const std::string& interfaceMemberName,
                                                                                   const std::string& interfaceMemberSignature,
                                                                                   DBusSignalHandler* dbusSignalHandler,
                                                                                   const bool justAddFilter) {
    DBusSignalHandlerPath dbusSignalHandlerPath(
                    objectPath,
                    interfaceName,
                    interfaceMemberName,
                    interfaceMemberSignature);
    std::lock_guard < std::mutex > dbusSignalLock(signalGuard_);

    auto signalEntry = dbusSignalHandlerTable_.find(dbusSignalHandlerPath);
    const bool isFirstSignalMemberHandler = (signalEntry == dbusSignalHandlerTable_.end());
    if (isFirstSignalMemberHandler) {
        addLibdbusSignalMatchRule(objectPath, interfaceName, interfaceMemberName, justAddFilter);
        std::set<DBusSignalHandler*> handlerList;
        handlerList.insert(dbusSignalHandler);

        dbusSignalHandlerTable_.insert( {
            dbusSignalHandlerPath,
            std::make_pair(std::make_shared<std::recursive_mutex>(), std::move(handlerList))
        } );
    } else {
        signalEntry->second.first->lock();
        signalEntry->second.second.insert(dbusSignalHandler);
        signalEntry->second.first->unlock();
    }

    return dbusSignalHandlerPath;
}

bool DBusConnection::removeSignalMemberHandler(const DBusSignalHandlerToken &dbusSignalHandlerToken,
                                               const DBusSignalHandler *dbusSignalHandler) {
    bool lastHandlerRemoved = false;
    std::lock_guard < std::mutex > dbusSignalLock(signalGuard_);

    auto signalEntry = dbusSignalHandlerTable_.find(dbusSignalHandlerToken);
    if (signalEntry != dbusSignalHandlerTable_.end()) {

        signalEntry->second.first->lock();
        auto selectedHandler = signalEntry->second.second.find(const_cast<DBusSignalHandler*>(dbusSignalHandler));
        if (selectedHandler != signalEntry->second.second.end()) {
            signalEntry->second.second.erase(selectedHandler);
            lastHandlerRemoved = (signalEntry->second.second.empty());
        }
        signalEntry->second.first->unlock();
    }

    if (lastHandlerRemoved) {
		dbusSignalHandlerTable_.erase(signalEntry);
		removeLibdbusSignalMatchRule(std::get<0>(dbusSignalHandlerToken),
				std::get<1>(dbusSignalHandlerToken),
				std::get<2>(dbusSignalHandlerToken));
	}

    return lastHandlerRemoved;
}

bool DBusConnection::addObjectManagerSignalMemberHandler(const std::string& dbusBusName,
                                                         DBusSignalHandler* dbusSignalHandler) {
    if (dbusBusName.length() < 2) {
        return false;
    }

    std::lock_guard<std::mutex> dbusSignalLock(dbusObjectManagerSignalGuard_);

    auto dbusSignalMatchRuleIterator = dbusObjectManagerSignalMatchRulesMap_.find(dbusBusName);
    const bool isDBusSignalMatchRuleFound = (dbusSignalMatchRuleIterator != dbusObjectManagerSignalMatchRulesMap_.end());

    if (!isDBusSignalMatchRuleFound) {
        if (isConnected() && !addObjectManagerSignalMatchRule(dbusBusName)) {
            return false;
        }

        auto insertResult = dbusObjectManagerSignalMatchRulesMap_.insert({ dbusBusName, 0 });
        const bool isInsertSuccessful = insertResult.second;

        if (!isInsertSuccessful) {
            if (isConnected()) {
                const bool isRemoveSignalMatchRuleSuccessful = removeObjectManagerSignalMatchRule(dbusBusName);
                assert(isRemoveSignalMatchRuleSuccessful);
            }
            return false;
        }

        dbusSignalMatchRuleIterator = insertResult.first;
    }

    size_t &dbusSignalMatchRuleReferenceCount = dbusSignalMatchRuleIterator->second;
    dbusSignalMatchRuleReferenceCount++;
    dbusObjectManagerSignalHandlerTable_.insert( { dbusBusName, dbusSignalHandler } );

    return true;
}

bool DBusConnection::removeObjectManagerSignalMemberHandler(const std::string& dbusBusName,
                                                            DBusSignalHandler* dbusSignalHandler) {
    assert(!dbusBusName.empty());

    std::lock_guard<std::mutex> dbusSignalLock(dbusObjectManagerSignalGuard_);

    auto dbusSignalMatchRuleIterator = dbusObjectManagerSignalMatchRulesMap_.find(dbusBusName);
    const bool isDBusSignalMatchRuleFound = (dbusSignalMatchRuleIterator != dbusObjectManagerSignalMatchRulesMap_.end());

    if (!isDBusSignalMatchRuleFound) {
        return true;
    }

    auto dbusObjectManagerSignalHandlerRange = dbusObjectManagerSignalHandlerTable_.equal_range(dbusBusName);
    auto dbusObjectManagerSignalHandlerIterator = std::find_if(
        dbusObjectManagerSignalHandlerRange.first,
        dbusObjectManagerSignalHandlerRange.second,
        [&](decltype(*dbusObjectManagerSignalHandlerRange.first)& it) { return it.second == dbusSignalHandler; });
    const bool isDBusSignalHandlerFound = (dbusObjectManagerSignalHandlerIterator != dbusObjectManagerSignalHandlerRange.second);
    if (!isDBusSignalHandlerFound) {
        return false;
    }

    size_t& dbusSignalMatchRuleReferenceCount = dbusSignalMatchRuleIterator->second;

    assert(dbusSignalMatchRuleReferenceCount > 0);
    dbusSignalMatchRuleReferenceCount--;

    const bool isLastDBusSignalMatchRuleReference = (dbusSignalMatchRuleReferenceCount == 0);
    if (isLastDBusSignalMatchRuleReference) {
        if (isConnected() && !removeObjectManagerSignalMatchRule(dbusBusName)) {
            return false;
        }

        dbusObjectManagerSignalMatchRulesMap_.erase(dbusSignalMatchRuleIterator);
    }

    dbusObjectManagerSignalHandlerTable_.erase(dbusObjectManagerSignalHandlerIterator);

    return true;
}

bool DBusConnection::addObjectManagerSignalMatchRule(const std::string& dbusBusName) {
    std::ostringstream dbusMatchRuleStringStream;
    dbusMatchRuleStringStream << "type='signal'"
                                << ",sender='" << dbusBusName << "'"
                                << ",interface='org.freedesktop.DBus.ObjectManager'";
    return addLibdbusSignalMatchRule(dbusMatchRuleStringStream.str());
}

bool DBusConnection::removeObjectManagerSignalMatchRule(const std::string& dbusBusName) {
    std::ostringstream dbusMatchRuleStringStream;
    dbusMatchRuleStringStream << "type='signal'"
                                << ",sender='" << dbusBusName << "'"
                                << ",interface='org.freedesktop.DBus.ObjectManager'";
    return removeLibdbusSignalMatchRule(dbusMatchRuleStringStream.str());
}

/**
 * Called only if connected
 *
 * @param dbusMatchRule
 * @return
 */
bool DBusConnection::addLibdbusSignalMatchRule(const std::string& dbusMatchRule) {
    bool libdbusSuccess = true;

    suspendDispatching();

    // add the libdbus message signal filter
    if (!libdbusSignalMatchRulesCount_) {
        libdbusSuccess = 0 != dbus_connection_add_filter(
            connection_,
            &onLibdbusSignalFilterThunk,
            this,
            NULL
        );
    }

    // finally add the match rule
    if (libdbusSuccess) {
        DBusError dbusError;
        dbus_bus_add_match(connection_, dbusMatchRule.c_str(), &dbusError.libdbusError_);
        libdbusSuccess = !dbusError;
    }

    if (libdbusSuccess) {
        libdbusSignalMatchRulesCount_++;
    }

    resumeDispatching();

    return libdbusSuccess;
}

/**
 * Called only if connected
 *
 * @param dbusMatchRule
 * @return
 */
bool DBusConnection::removeLibdbusSignalMatchRule(const std::string& dbusMatchRule) {
    if(libdbusSignalMatchRulesCount_ == 0)
        return true;

    suspendDispatching();

    dbus_bus_remove_match(connection_, dbusMatchRule.c_str(), NULL);

    libdbusSignalMatchRulesCount_--;
    if (libdbusSignalMatchRulesCount_ == 0) {
        dbus_connection_remove_filter(connection_, &onLibdbusSignalFilterThunk, this);
    }

    resumeDispatching();

    return true;
}

void DBusConnection::registerObjectPath(const std::string& objectPath) {
    assert(!objectPath.empty());
    assert(objectPath[0] == '/');

    auto handlerIterator = libdbusRegisteredObjectPaths_.find(objectPath);
    const bool foundRegisteredObjectPathHandler = handlerIterator != libdbusRegisteredObjectPaths_.end();

    if (foundRegisteredObjectPathHandler) {
        uint32_t& referenceCount = handlerIterator->second;
        referenceCount++;
        return;
    }

    libdbusRegisteredObjectPaths_.insert(LibdbusRegisteredObjectPathHandlersTable::value_type(objectPath, 1));

    if (isConnected()) {
        DBusError dbusError;
        const dbus_bool_t libdbusSuccess = dbus_connection_try_register_object_path(connection_,
                                                                                    objectPath.c_str(),
                                                                                    getDBusObjectPathVTable(),
                                                                                    this,
                                                                                    &dbusError.libdbusError_);
        assert(libdbusSuccess);
        assert(!dbusError);
    }
}

void DBusConnection::unregisterObjectPath(const std::string& objectPath) {
    assert(!objectPath.empty());
    assert(objectPath[0] == '/');

    auto handlerIterator = libdbusRegisteredObjectPaths_.find(objectPath);
    const bool foundRegisteredObjectPathHandler = handlerIterator != libdbusRegisteredObjectPaths_.end();

    assert(foundRegisteredObjectPathHandler);

    uint32_t& referenceCount = handlerIterator->second;
    if (referenceCount > 1) {
        referenceCount--;
        return;
    }

    libdbusRegisteredObjectPaths_.erase(handlerIterator);

    if (isConnected()) {
        dbus_bool_t libdbusSuccess
            = dbus_connection_unregister_object_path(connection_, objectPath.c_str());
        assert(libdbusSuccess);
    }
}

void DBusConnection::addLibdbusSignalMatchRule(const std::string& objectPath,
                                               const std::string& interfaceName,
                                               const std::string& interfaceMemberName,
                                               const bool justAddFilter) {
    DBusSignalMatchRuleTuple dbusSignalMatchRuleTuple(objectPath, interfaceName, interfaceMemberName);
    auto matchRuleIterator = dbusSignalMatchRulesMap_.find(dbusSignalMatchRuleTuple);
    const bool matchRuleFound = matchRuleIterator != dbusSignalMatchRulesMap_.end();

    if (matchRuleFound) {
        uint32_t& matchRuleReferenceCount = matchRuleIterator->second.first;
        matchRuleReferenceCount++;
        return;
    }

    const bool isFirstMatchRule = dbusSignalMatchRulesMap_.empty();

    // generate D-Bus match rule string
    std::ostringstream matchRuleStringStream;

    matchRuleStringStream << "type='signal'";
    matchRuleStringStream << ",path='" << objectPath << "'";
    matchRuleStringStream << ",interface='" << interfaceName << "'";
    matchRuleStringStream << ",member='" << interfaceMemberName << "'";

    // add the match rule string to the map with reference count set to 1
    std::string matchRuleString = matchRuleStringStream.str();
    auto success = dbusSignalMatchRulesMap_.insert(
                    DBusSignalMatchRulesMap::value_type(dbusSignalMatchRuleTuple,
                                    DBusSignalMatchRuleMapping(1, matchRuleString)));
    assert(success.second);

    if (isConnected()) {
        bool libdbusSuccess = true;
        suspendDispatching();

        // add the libdbus message signal filter
        if (isFirstMatchRule) {

            libdbusSuccess = 0 != dbus_connection_add_filter(
                                connection_,
                                &onLibdbusSignalFilterThunk,
                                this,
                                NULL);
            assert(libdbusSuccess);
        }

        if (!justAddFilter)
        {
            // finally add the match rule
            DBusError dbusError;
            dbus_bus_add_match(connection_, matchRuleString.c_str(), &dbusError.libdbusError_);
            assert(!dbusError);
        }

        if (libdbusSuccess) {
            libdbusSignalMatchRulesCount_++;
        }

        resumeDispatching();
    }
}

void DBusConnection::removeLibdbusSignalMatchRule(const std::string& objectPath,
                                                  const std::string& interfaceName,
                                                  const std::string& interfaceMemberName) {
    auto selfReference = this->shared_from_this();
    DBusSignalMatchRuleTuple dbusSignalMatchRuleTuple(objectPath, interfaceName, interfaceMemberName);
    auto matchRuleIterator = dbusSignalMatchRulesMap_.find(dbusSignalMatchRuleTuple);
    const bool matchRuleFound = matchRuleIterator != dbusSignalMatchRulesMap_.end();

    assert(matchRuleFound);

    uint32_t& matchRuleReferenceCount = matchRuleIterator->second.first;
    if (matchRuleReferenceCount > 1) {
        matchRuleReferenceCount--;
        return;
    }

    if (isConnected()) {
        const std::string& matchRuleString = matchRuleIterator->second.second;
        const bool libdbusSuccess = removeLibdbusSignalMatchRule(matchRuleString);
        assert(libdbusSuccess);
    }

    dbusSignalMatchRulesMap_.erase(matchRuleIterator);
}

void DBusConnection::initLibdbusObjectPathHandlerAfterConnect() {
    assert(isConnected());

    // nothing to do if there aren't any registered object path handlers
    if (libdbusRegisteredObjectPaths_.empty()) {
        return;
    }

    DBusError dbusError;
    dbus_bool_t libdbusSuccess;

    for (auto handlerIterator = libdbusRegisteredObjectPaths_.begin();
         handlerIterator != libdbusRegisteredObjectPaths_.end();
         handlerIterator++) {
        const std::string& objectPath = handlerIterator->first;

        dbusError.clear();

        libdbusSuccess = dbus_connection_try_register_object_path(connection_,
                                                                  objectPath.c_str(),
                                                                  getDBusObjectPathVTable(),
                                                                  this,
                                                                  &dbusError.libdbusError_);
        assert(libdbusSuccess);
        assert(!dbusError);
    }
}

void DBusConnection::initLibdbusSignalFilterAfterConnect() {
    assert(isConnected());

    // proxy/stub match rules
    for (const auto& dbusSignalMatchRuleIterator : dbusSignalMatchRulesMap_) {
        const auto& dbusSignalMatchRuleMapping = dbusSignalMatchRuleIterator.second;
        const std::string& dbusMatchRuleString = dbusSignalMatchRuleMapping.second;
        const bool libdbusSuccess = addLibdbusSignalMatchRule(dbusMatchRuleString);
        assert(libdbusSuccess);
    }

    // object manager match rules (see DBusServiceRegistry)
    for (const auto& dbusObjectManagerSignalMatchRuleIterator : dbusObjectManagerSignalMatchRulesMap_) {
        const std::string& dbusBusName = dbusObjectManagerSignalMatchRuleIterator.first;
        const bool libdbusSuccess = addObjectManagerSignalMatchRule(dbusBusName);
        assert(libdbusSuccess);
    }
}

::DBusHandlerResult DBusConnection::onLibdbusObjectPathMessage(::DBusMessage* libdbusMessage) {
    assert(libdbusMessage);

    // handle only method call messages
    if (dbus_message_get_type(libdbusMessage) != DBUS_MESSAGE_TYPE_METHOD_CALL) {
        return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    }

    bool isDBusMessageHandled = dbusObjectMessageHandler_(DBusMessage(libdbusMessage));
    return isDBusMessageHandled ? DBUS_HANDLER_RESULT_HANDLED : DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

template<typename DBusSignalHandlersTable>
void notifyDBusSignalHandlers(DBusSignalHandlersTable& dbusSignalHandlerstable,
                              typename DBusSignalHandlersTable::iterator& signalEntry,
                              const CommonAPI::DBus::DBusMessage& dbusMessage,
                              ::DBusHandlerResult& dbusHandlerResult) {
    if (signalEntry == dbusSignalHandlerstable.end() || signalEntry->second.second.empty()) {
        dbusHandlerResult = DBUS_HANDLER_RESULT_HANDLED;
        return;
    }

    signalEntry->second.first->lock();
    auto handlerEntry = signalEntry->second.second.begin();
    while (handlerEntry != signalEntry->second.second.end()) {
        DBusProxyConnection::DBusSignalHandler* dbusSignalHandler = *handlerEntry;
        dbusSignalHandler->onSignalDBusMessage(dbusMessage);
        handlerEntry++;
    }
    dbusHandlerResult = DBUS_HANDLER_RESULT_HANDLED;
    signalEntry->second.first->unlock();
}

template<typename DBusSignalHandlersTable>
void notifyDBusOMSignalHandlers(DBusSignalHandlersTable& dbusSignalHandlerstable,
                              std::pair<typename DBusSignalHandlersTable::iterator,
                                        typename DBusSignalHandlersTable::iterator>& equalRange,
                              const CommonAPI::DBus::DBusMessage &dbusMessage,
                              ::DBusHandlerResult &dbusHandlerResult) {
    if (equalRange.first != equalRange.second) {
        dbusHandlerResult = DBUS_HANDLER_RESULT_HANDLED;
    }
    while (equalRange.first != equalRange.second) {
        DBusProxyConnection::DBusSignalHandler* dbusSignalHandler = equalRange.first->second;
        dbusSignalHandler->onSignalDBusMessage(dbusMessage);
        equalRange.first++;
    }
}

::DBusHandlerResult DBusConnection::onLibdbusSignalFilter(::DBusMessage* libdbusMessage) {
    assert(libdbusMessage);

    auto selfReference = this->shared_from_this();

    // handle only signal messages
    if (dbus_message_get_type(libdbusMessage) != DBUS_MESSAGE_TYPE_SIGNAL) {
        return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    }

    const char* objectPath = dbus_message_get_path(libdbusMessage);
    const char* interfaceName = dbus_message_get_interface(libdbusMessage);
    const char* interfaceMemberName = dbus_message_get_member(libdbusMessage);
    const char* interfaceMemberSignature = dbus_message_get_signature(libdbusMessage);

    assert(objectPath);
    assert(interfaceName);
    assert(interfaceMemberName);
    assert(interfaceMemberSignature);

    DBusMessage dbusMessage(libdbusMessage);
    ::DBusHandlerResult dbusHandlerResult = DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

    signalGuard_.lock();
    auto signalEntry = dbusSignalHandlerTable_.find(DBusSignalHandlerPath(
        objectPath,
        interfaceName,
        interfaceMemberName,
        interfaceMemberSignature));
    signalGuard_.unlock();

    notifyDBusSignalHandlers(dbusSignalHandlerTable_,
                             signalEntry, dbusMessage, dbusHandlerResult);

    if (dbusMessage.hasInterfaceName("org.freedesktop.DBus.ObjectManager")) {
        const char* dbusSenderName = dbusMessage.getSender();
        assert(dbusSenderName);

        dbusObjectManagerSignalGuard_.lock();
        auto dbusObjectManagerSignalHandlerIteratorPair = dbusObjectManagerSignalHandlerTable_.equal_range(dbusSenderName);
        notifyDBusOMSignalHandlers(dbusObjectManagerSignalHandlerTable_,
                                 dbusObjectManagerSignalHandlerIteratorPair,
                                 dbusMessage,
                                 dbusHandlerResult);
        dbusObjectManagerSignalGuard_.unlock();
    }

    return dbusHandlerResult;
}

::DBusHandlerResult DBusConnection::onLibdbusSignalFilterThunk(::DBusConnection *_dbusConnection,
                                                               ::DBusMessage* libdbusMessage,
                                                               void* userData) {
    assert(_dbusConnection);
    assert(libdbusMessage);
    assert(userData);

    DBusConnection* dbusConnection = reinterpret_cast<DBusConnection*>(userData);
    assert(dbusConnection->connection_ == _dbusConnection);
    return dbusConnection->onLibdbusSignalFilter(libdbusMessage);
}

::DBusHandlerResult DBusConnection::onLibdbusObjectPathMessageThunk(::DBusConnection *_dbusConnection,
                                                                    ::DBusMessage* libdbusMessage,
                                                                    void* userData) {
    assert(_dbusConnection);
    assert(libdbusMessage);
    assert(userData);

    DBusConnection* dbusConnection = reinterpret_cast<DBusConnection*>(userData);
    assert(dbusConnection->connection_ == _dbusConnection);
    return dbusConnection->onLibdbusObjectPathMessage(libdbusMessage);
}

std::shared_ptr<DBusConnection> DBusConnection::getBus(const DBusType_t &_type) {
    return std::make_shared<DBusConnection>(_type);
}

std::shared_ptr<DBusConnection> DBusConnection::wrap(::DBusConnection *_connection) {
    return std::make_shared<DBusConnection>(_connection);
}

} // namespace DBus
} // namespace CommonAPI
