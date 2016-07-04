// Copyright (C) 2013-2015 Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#if !defined (COMMONAPI_INTERNAL_COMPILATION)
#error "Only <CommonAPI/CommonAPI.hpp> can be included directly, this file may disappear or change contents."
#endif

#ifndef COMMONAPI_DBUS_DBUSPROXYHELPER_HPP_
#define COMMONAPI_DBUS_DBUSPROXYHELPER_HPP_

#include <functional>
#define BOOST_THREAD_PROVIDES_FUTURE
#include <boost/thread/future.hpp>
#include <memory>
#include <string>

#include <CommonAPI/DBus/DBusAddress.hpp>
#include <CommonAPI/DBus/DBusConfig.hpp>
#include <CommonAPI/DBus/DBusMessage.hpp>
#include <CommonAPI/DBus/DBusSerializableArguments.hpp>
#include <CommonAPI/DBus/DBusProxyAsyncCallbackHandler.hpp>
#include <CommonAPI/DBus/DBusProxyConnection.hpp>

namespace CommonAPI {
namespace DBus {

class DBusProxy;

template< class, class >
struct DBusProxyHelper;

template<
    template<class ...> class _In, class... _InArgs,
    template <class...> class _Out, class... _OutArgs>
struct DBusProxyHelper<_In<DBusInputStream, DBusOutputStream, _InArgs...>,
    					   _Out<DBusInputStream, DBusOutputStream, _OutArgs...>> {

	template <typename _DBusProxy = DBusProxy>
	static void callMethod(const _DBusProxy &_proxy,
						   const std::string &_method,
						   const std::string &_signature,
						   const _InArgs&... _in,
						   CommonAPI::CallStatus &_status) {

		if (_proxy.isAvailable()) {
			DBusMessage message = _proxy.createMethodCall(_method, _signature);
			if (sizeof...(_InArgs) > 0) {
				DBusOutputStream output(message);
				if (!DBusSerializableArguments<_InArgs...>::serialize(output, _in...)) {
					_status = CallStatus::OUT_OF_MEMORY;
					return;
				}
				output.flush();
			}

            const bool isSent = _proxy.getDBusConnection()->sendDBusMessage(message);
            _status = (isSent ? CallStatus::SUCCESS : CallStatus::OUT_OF_MEMORY);
        } else {
            _status = CallStatus::NOT_AVAILABLE;
        }
    }

    template <typename _DBusProxy = DBusProxy>
    static void callMethodWithReply(
                    const _DBusProxy &_proxy,
                    DBusMessage &_message,
					const CommonAPI::CallInfo *_info,
                    const _InArgs&... _in,
                    CommonAPI::CallStatus &_status,
                    _OutArgs&... _out) {

        if (sizeof...(_InArgs) > 0) {
            DBusOutputStream output(_message);
            if (!DBusSerializableArguments<_InArgs...>::serialize(output, _in...)) {
                _status = CallStatus::OUT_OF_MEMORY;
                return;
            }
            output.flush();
        }

        DBusError error;
        DBusMessage reply = _proxy.getDBusConnection()->sendDBusMessageWithReplyAndBlock(_message, error, _info);
        if (error || !reply.isMethodReturnType()) {
            _status = CallStatus::REMOTE_ERROR;
            return;
        }

        if (sizeof...(_OutArgs) > 0) {
            DBusInputStream input(reply);
            if (!DBusSerializableArguments<_OutArgs...>::deserialize(input, _out...)) {
                _status = CallStatus::REMOTE_ERROR;
                return;
            }
        }
        _status = CallStatus::SUCCESS;
    }

    template <typename _DBusProxy = DBusProxy>
    static void callMethodWithReply(
                const _DBusProxy &_proxy,
                const DBusAddress &_address,
                const char *_method,
                const char *_signature,
				const CommonAPI::CallInfo *_info,
                const _InArgs&... _in,
                CommonAPI::CallStatus &_status,
                _OutArgs&... _out) {
        if (_proxy.isAvailable()) {
            DBusMessage message = DBusMessage::createMethodCall(_address, _method, _signature);
            callMethodWithReply(_proxy, message, _info, _in..., _status, _out...);
        } else {
            _status = CallStatus::NOT_AVAILABLE;
        }
    }

    template <typename _DBusProxy = DBusProxy>
    static void callMethodWithReply(
                const _DBusProxy &_proxy,
                const std::string &_interface,
                const std::string &_method,
                const std::string &_signature,
				const CommonAPI::CallInfo *_info,
                const _InArgs&... _in,
                CommonAPI::CallStatus &_status,
                _OutArgs&... _out) {
    	DBusAddress itsAddress(_proxy.getDBusAddress());
    	itsAddress.setInterface(_interface);
        callMethodWithReply(
        		_proxy, itsAddress,
				_method.c_str(), _signature.c_str(),
				_info,
				_in..., _status, _out...);
    }

    template <typename _DBusProxy = DBusProxy>
    static void callMethodWithReply(
                    const _DBusProxy &_proxy,
                    const std::string &_method,
                    const std::string &_signature,
					const CommonAPI::CallInfo *_info,
                    const _InArgs&... _in,
                    CommonAPI::CallStatus &_status,
                    _OutArgs&... _out) {
        if (_proxy.isAvailable()) {
            DBusMessage message = _proxy.createMethodCall(_method, _signature);
            callMethodWithReply(_proxy, message, _info, _in..., _status, _out...);
        } else {
            _status = CallStatus::NOT_AVAILABLE;
        }
    }

    template <typename _DBusProxy = DBusProxy, typename _AsyncCallback>
    static boost::future<CallStatus> callMethodAsync(
                    const _DBusProxy &_proxy,
                    DBusMessage &_message,
					const CommonAPI::CallInfo *_info,
                    const _InArgs&... _in,
                    _AsyncCallback _callback,
                    std::tuple<_OutArgs...> _out) {
        if (sizeof...(_InArgs) > 0) {
            DBusOutputStream output(_message);
            const bool success = DBusSerializableArguments<
            						_InArgs...
            					 >::serialize(output, _in...);
            if (!success) {
                boost::promise<CallStatus> promise;
                promise.set_value(CallStatus::OUT_OF_MEMORY);
                return promise.get_future();
            }
            output.flush();
        }

        return _proxy.getDBusConnection()->sendDBusMessageWithReplyAsync(
                        _message,
                        DBusProxyAsyncCallbackHandler<
                        	_OutArgs...
                        >::create(std::move(_callback), _out),
						_info);
    }

    template <typename _DBusProxy = DBusProxy, typename _AsyncCallback>
    static boost::future<CallStatus> callMethodAsync(
                        const _DBusProxy &_proxy,
                        const DBusAddress &_address,
                        const std::string &_method,
                        const std::string &_signature,
						const CommonAPI::CallInfo *_info,
                        const _InArgs&... _in,
                        _AsyncCallback _callback,
                        std::tuple<_OutArgs...> _out) {
        if (_proxy.isAvailable()) {
            DBusMessage message = DBusMessage::createMethodCall(_address, _method, _signature);
            return callMethodAsync(_proxy, message, _info, _in..., _callback, _out);
        } else {
            CallStatus status = CallStatus::NOT_AVAILABLE;
            callCallbackOnNotAvailable(_callback, typename make_sequence<sizeof...(_OutArgs)>::type(), _out);

            boost::promise<CallStatus> promise;
            promise.set_value(status);
            return promise.get_future();
        }
    }

    template <typename _DBusProxy = DBusProxy, typename _AsyncCallback>
    static boost::future<CallStatus> callMethodAsync(
                const _DBusProxy &_proxy,
                const std::string &_interface,
                const std::string &_method,
                const std::string &_signature,
				const CommonAPI::CallInfo *_info,
                const _InArgs&... _in,
                _AsyncCallback _callback,
                std::tuple<_OutArgs...> _out) {
    	DBusAddress itsAddress(_proxy.getDBusAddress());
    	itsAddress.setInterface(_interface);
    	return callMethodAsync(
					_proxy, itsAddress,
					_method, _signature,
					_info,
					_in...,	_callback, _out);
    }

    template <typename _DBusProxy = DBusProxy, typename _AsyncCallback>
    static boost::future<CallStatus> callMethodAsync(
                    const _DBusProxy &_proxy,
                    const std::string &_method,
                    const std::string &_signature,
					const CommonAPI::CallInfo *_info,
                    const _InArgs&... _in,
                    _AsyncCallback _callback,
                    std::tuple<_OutArgs...> _out) {
        if (_proxy.isAvailable()) {
            DBusMessage message = _proxy.createMethodCall(_method, _signature);
            return callMethodAsync(_proxy, message, _info, _in..., _callback, _out);
        } else {
            callCallbackOnNotAvailable(
            	_callback, typename make_sequence<sizeof...(_OutArgs)>::type(), _out);

            CallStatus status = CallStatus::NOT_AVAILABLE;
            boost::promise<CallStatus> promise;
            promise.set_value(status);
            return promise.get_future();
    	}
    }

    template <int... _ArgIndices>
    static void callCallbackOnNotAvailable(std::function<void(CallStatus, _OutArgs&...)> _callback,
                                           index_sequence<_ArgIndices...>, std::tuple<_OutArgs...> _out) {
        const CallStatus status(CallStatus::NOT_AVAILABLE);
        _callback(status, std::get<_ArgIndices>(_out)...);
    }
};

} // namespace DBus
} // namespace CommonAPI

#endif // COMMONAPI_DBUS_DBUSPROXYHELPER_HPP_
