/*
* This file was generated by the CommonAPI Generators.
* Used org.genivi.commonapi.core 2.1.5.qualifier.
* Used org.franca.core 0.8.10.201309262002.
*
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*/
#ifndef COMMONAPI_TESTS_MANAGED_Root_Interface_PROXY_H_
#define COMMONAPI_TESTS_MANAGED_Root_Interface_PROXY_H_

#include "RootInterfaceProxyBase.h"


#if !defined (COMMONAPI_INTERNAL_COMPILATION)
#define COMMONAPI_INTERNAL_COMPILATION
#endif


#undef COMMONAPI_INTERNAL_COMPILATION

namespace commonapi {
namespace tests {
namespace managed {

template <typename ... _AttributeExtensions>
class RootInterfaceProxy: virtual public RootInterface, virtual public RootInterfaceProxyBase
, public _AttributeExtensions... {
public:
    RootInterfaceProxy(std::shared_ptr<CommonAPI::Proxy> delegate);
    ~RootInterfaceProxy();

    typedef RootInterface InterfaceType;




    /**
     * Calls testRootMethod with synchronous semantics.
     * 
    * All const parameters are input parameters to this method.
    * All non-const parameters will be filled with the returned values.
     * The CallStatus will be filled when the method returns and indicate either
     * "SUCCESS" or which type of error has occurred. In case of an error, ONLY the CallStatus
     * will be set.
     */
    virtual void testRootMethod(const int32_t& inInt, const std::string& inString, CommonAPI::CallStatus& callStatus, RootInterface::testRootMethodError& methodError, int32_t& outInt, std::string& outString);
    /**
     * Calls testRootMethod with asynchronous semantics.
     * 
     * The provided callback will be called when the reply to this call arrives or
     * an error occurs during the call. The CallStatus will indicate either "SUCCESS"
     * or which type of error has occurred. In case of any error, ONLY the CallStatus
     * will have a defined value.
     * The std::future returned by this method will be fulfilled at arrival of the reply.
     * It will provide the same value for CallStatus as will be handed to the callback.
     */
    virtual std::future<CommonAPI::CallStatus> testRootMethodAsync(const int32_t& inInt, const std::string& inString, TestRootMethodAsyncCallback callback);

    virtual CommonAPI::ProxyManager& getProxyManagerLeafInterface();
    virtual CommonAPI::ProxyManager& getProxyManagerBranchInterface();

    /**
     * Returns the CommonAPI address of the remote partner this proxy communicates with.
     */
    virtual std::string getAddress() const;

    /**
     * Returns the domain of the remote partner this proxy communicates with.
     */
    virtual const std::string& getDomain() const;

    /** 
     * Returns the service ID of the remote partner this proxy communicates with.
     */
    virtual const std::string& getServiceId() const;

    /**
     * Returns the instance ID of the remote partner this proxy communicates with.
     */
    virtual const std::string& getInstanceId() const;

    /**
     * Returns true if the remote partner for this proxy is currently known to be available.
     */
    virtual bool isAvailable() const;

    /**
     * Returns true if the remote partner for this proxy is available.
     */
    virtual bool isAvailableBlocking() const;

    /**
     * Returns the wrapper class that is used to (de-)register for notifications about
     * the availability of the remote partner of this proxy.
     */
    virtual CommonAPI::ProxyStatusEvent& getProxyStatusEvent();

    /**
     * Returns the wrapper class that is used to access version information of the remote
     * partner of this proxy.
     */
    virtual CommonAPI::InterfaceVersionAttribute& getInterfaceVersionAttribute();

 private:
    std::shared_ptr<RootInterfaceProxyBase> delegate_;
};


//
// RootInterfaceProxy Implementation
//
template <typename ... _AttributeExtensions>
RootInterfaceProxy<_AttributeExtensions...>::RootInterfaceProxy(std::shared_ptr<CommonAPI::Proxy> delegate):
        delegate_(std::dynamic_pointer_cast<RootInterfaceProxyBase>(delegate)),
        _AttributeExtensions(*(std::dynamic_pointer_cast<RootInterfaceProxyBase>(delegate)))... {
}

template <typename ... _AttributeExtensions>
RootInterfaceProxy<_AttributeExtensions...>::~RootInterfaceProxy() {
}

template <typename ... _AttributeExtensions>
void RootInterfaceProxy<_AttributeExtensions...>::testRootMethod(const int32_t& inInt, const std::string& inString, CommonAPI::CallStatus& callStatus, RootInterface::testRootMethodError& methodError, int32_t& outInt, std::string& outString) {
    delegate_->testRootMethod(inInt, inString, callStatus, methodError, outInt, outString);
}

template <typename ... _AttributeExtensions>
std::future<CommonAPI::CallStatus> RootInterfaceProxy<_AttributeExtensions...>::testRootMethodAsync(const int32_t& inInt, const std::string& inString, TestRootMethodAsyncCallback callback) {
    return delegate_->testRootMethodAsync(inInt, inString, callback);
}

template <typename ... _AttributeExtensions>
std::string RootInterfaceProxy<_AttributeExtensions...>::getAddress() const {
    return delegate_->getAddress();
}

template <typename ... _AttributeExtensions>
const std::string& RootInterfaceProxy<_AttributeExtensions...>::getDomain() const {
    return delegate_->getDomain();
}

template <typename ... _AttributeExtensions>
const std::string& RootInterfaceProxy<_AttributeExtensions...>::getServiceId() const {
    return delegate_->getServiceId();
}

template <typename ... _AttributeExtensions>
const std::string& RootInterfaceProxy<_AttributeExtensions...>::getInstanceId() const {
    return delegate_->getInstanceId();
}

template <typename ... _AttributeExtensions>
bool RootInterfaceProxy<_AttributeExtensions...>::isAvailable() const {
    return delegate_->isAvailable();
}

template <typename ... _AttributeExtensions>
bool RootInterfaceProxy<_AttributeExtensions...>::isAvailableBlocking() const {
    return delegate_->isAvailableBlocking();
}

template <typename ... _AttributeExtensions>
CommonAPI::ProxyStatusEvent& RootInterfaceProxy<_AttributeExtensions...>::getProxyStatusEvent() {
    return delegate_->getProxyStatusEvent();
}

template <typename ... _AttributeExtensions>
CommonAPI::InterfaceVersionAttribute& RootInterfaceProxy<_AttributeExtensions...>::getInterfaceVersionAttribute() {
    return delegate_->getInterfaceVersionAttribute();
}

template <typename ... _AttributeExtensions>
CommonAPI::ProxyManager& RootInterfaceProxy<_AttributeExtensions...>::getProxyManagerLeafInterface() {
    return delegate_->getProxyManagerLeafInterface();
}
template <typename ... _AttributeExtensions>
CommonAPI::ProxyManager& RootInterfaceProxy<_AttributeExtensions...>::getProxyManagerBranchInterface() {
    return delegate_->getProxyManagerBranchInterface();
}

} // namespace managed
} // namespace tests
} // namespace commonapi


#endif // COMMONAPI_TESTS_MANAGED_Root_Interface_PROXY_H_
