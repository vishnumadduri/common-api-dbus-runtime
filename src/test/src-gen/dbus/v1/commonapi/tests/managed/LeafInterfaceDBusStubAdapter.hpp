/*
* This file was generated by the CommonAPI Generators.
* Used org.genivi.commonapi.dbus 3.1.5.v201512091037.
* Used org.franca.core 0.9.1.201412191134.
*
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*/
#ifndef V1_COMMONAPI_TESTS_MANAGED_Leaf_Interface_DBUS_STUB_ADAPTER_HPP_
#define V1_COMMONAPI_TESTS_MANAGED_Leaf_Interface_DBUS_STUB_ADAPTER_HPP_

#include <v1/commonapi/tests/managed/LeafInterfaceStub.hpp>
#include "v1/commonapi/tests/managed/LeafInterfaceDBusDeployment.hpp"        

#if !defined (COMMONAPI_INTERNAL_COMPILATION)
#define COMMONAPI_INTERNAL_COMPILATION
#endif

#include <CommonAPI/DBus/DBusAddressTranslator.hpp>
#include <CommonAPI/DBus/DBusFactory.hpp>
#include <CommonAPI/DBus/DBusStubAdapterHelper.hpp>
#include <CommonAPI/DBus/DBusStubAdapter.hpp>
#include <CommonAPI/DBus/DBusDeployment.hpp>

#undef COMMONAPI_INTERNAL_COMPILATION

namespace v1 {
namespace commonapi {
namespace tests {
namespace managed {

typedef CommonAPI::DBus::DBusStubAdapterHelper<LeafInterfaceStub> LeafInterfaceDBusStubAdapterHelper;

class LeafInterfaceDBusStubAdapterInternal
    : public virtual LeafInterfaceStubAdapter,
      public LeafInterfaceDBusStubAdapterHelper
{
public:
    LeafInterfaceDBusStubAdapterInternal(
            const CommonAPI::DBus::DBusAddress &_address,
            const std::shared_ptr<CommonAPI::DBus::DBusProxyConnection> &_connection,
            const std::shared_ptr<CommonAPI::StubBase> &_stub);

    ~LeafInterfaceDBusStubAdapterInternal();

    virtual bool hasFreedesktopProperties();

    inline static const char* getInterface() {
        return LeafInterface::getInterface();
    }




    const LeafInterfaceDBusStubAdapterHelper::StubDispatcherTable& getStubDispatcherTable();
    const CommonAPI::DBus::StubAttributeTable& getStubAttributeTable();

    void deactivateManagedInstances();
    

static CommonAPI::DBus::DBusGetAttributeStubDispatcher<
        ::v1::commonapi::tests::managed::LeafInterfaceStub,
        CommonAPI::Version
        > getLeafInterfaceInterfaceVersionStubDispatcher;




static CommonAPI::DBus::DBusMethodWithReplyStubDispatcher<
    ::v1::commonapi::tests::managed::LeafInterfaceStub,
    std::tuple<int32_t, std::string>,
    std::tuple<LeafInterface::testLeafMethodError, int32_t, std::string>,
    std::tuple<CommonAPI::EmptyDeployment, CommonAPI::DBus::StringDeployment>,
    std::tuple<CommonAPI::EmptyDeployment, CommonAPI::EmptyDeployment, CommonAPI::DBus::StringDeployment>
    
    > testLeafMethodStubDispatcher;




 protected:
    virtual const char* getMethodsDBusIntrospectionXmlData() const;

 private:
    LeafInterfaceDBusStubAdapterHelper::StubDispatcherTable stubDispatcherTable_;
    CommonAPI::DBus::StubAttributeTable stubAttributeTable_;
};

class LeafInterfaceDBusStubAdapter
    : public LeafInterfaceDBusStubAdapterInternal,
      public std::enable_shared_from_this<LeafInterfaceDBusStubAdapter> {
public:
    LeafInterfaceDBusStubAdapter(
    	const CommonAPI::DBus::DBusAddress &_address,
        const std::shared_ptr<CommonAPI::DBus::DBusProxyConnection> &_connection,
        const std::shared_ptr<CommonAPI::StubBase> &_stub)
    	: CommonAPI::DBus::DBusStubAdapter(
    		_address, 
    		_connection,
            false),
          LeafInterfaceDBusStubAdapterInternal(
          	_address, 
          	_connection, 
          	_stub) { 
    }
};

} // namespace managed
} // namespace tests
} // namespace commonapi
} // namespace v1

#endif // V1_COMMONAPI_TESTS_MANAGED_Leaf_Interface_DBUS_STUB_ADAPTER_HPP_
