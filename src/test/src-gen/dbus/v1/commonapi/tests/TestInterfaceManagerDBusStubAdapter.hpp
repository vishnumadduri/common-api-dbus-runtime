/*
* This file was generated by the CommonAPI Generators.
* Used org.genivi.commonapi.dbus 3.1.5.v201512091037.
* Used org.franca.core 0.9.1.201412191134.
*
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*/
#ifndef V1_COMMONAPI_TESTS_Test_Interface_Manager_DBUS_STUB_ADAPTER_HPP_
#define V1_COMMONAPI_TESTS_Test_Interface_Manager_DBUS_STUB_ADAPTER_HPP_

#include <v1/commonapi/tests/TestInterfaceManagerStub.hpp>
#include "v1/commonapi/tests/TestInterfaceManagerDBusDeployment.hpp"        

#if !defined (COMMONAPI_INTERNAL_COMPILATION)
#define COMMONAPI_INTERNAL_COMPILATION
#endif

#include <CommonAPI/DBus/DBusAddressTranslator.hpp>
#include <CommonAPI/DBus/DBusFactory.hpp>
#include <CommonAPI/DBus/DBusObjectManager.hpp>
#include <CommonAPI/DBus/DBusStubAdapterHelper.hpp>
#include <CommonAPI/DBus/DBusStubAdapter.hpp>
#include <CommonAPI/DBus/DBusDeployment.hpp>

#undef COMMONAPI_INTERNAL_COMPILATION

namespace v1 {
namespace commonapi {
namespace tests {

typedef CommonAPI::DBus::DBusStubAdapterHelper<TestInterfaceManagerStub> TestInterfaceManagerDBusStubAdapterHelper;

class TestInterfaceManagerDBusStubAdapterInternal
    : public virtual TestInterfaceManagerStubAdapter,
      public TestInterfaceManagerDBusStubAdapterHelper
{
public:
    TestInterfaceManagerDBusStubAdapterInternal(
            const CommonAPI::DBus::DBusAddress &_address,
            const std::shared_ptr<CommonAPI::DBus::DBusProxyConnection> &_connection,
            const std::shared_ptr<CommonAPI::StubBase> &_stub);

    ~TestInterfaceManagerDBusStubAdapterInternal();

    virtual bool hasFreedesktopProperties();

    inline static const char* getInterface() {
        return TestInterfaceManager::getInterface();
    }



    bool registerManagedStubTestInterface(std::shared_ptr<::v1::commonapi::tests::TestInterfaceStub>, const std::string&);
    bool deregisterManagedStubTestInterface(const std::string&);
    std::set<std::string>& getTestInterfaceInstances();

    const TestInterfaceManagerDBusStubAdapterHelper::StubDispatcherTable& getStubDispatcherTable();
    const CommonAPI::DBus::StubAttributeTable& getStubAttributeTable();

    void deactivateManagedInstances();
    

static CommonAPI::DBus::DBusGetAttributeStubDispatcher<
        ::v1::commonapi::tests::TestInterfaceManagerStub,
        CommonAPI::Version
        > getTestInterfaceManagerInterfaceVersionStubDispatcher;







 protected:
    virtual const char* getMethodsDBusIntrospectionXmlData() const;

 private:
    std::set<std::string> registeredTestInterfaceInstances;
    TestInterfaceManagerDBusStubAdapterHelper::StubDispatcherTable stubDispatcherTable_;
    CommonAPI::DBus::StubAttributeTable stubAttributeTable_;
};

class TestInterfaceManagerDBusStubAdapter
    : public TestInterfaceManagerDBusStubAdapterInternal,
      public std::enable_shared_from_this<TestInterfaceManagerDBusStubAdapter> {
public:
    TestInterfaceManagerDBusStubAdapter(
    	const CommonAPI::DBus::DBusAddress &_address,
        const std::shared_ptr<CommonAPI::DBus::DBusProxyConnection> &_connection,
        const std::shared_ptr<CommonAPI::StubBase> &_stub)
    	: CommonAPI::DBus::DBusStubAdapter(
    		_address, 
    		_connection,
            true),
          TestInterfaceManagerDBusStubAdapterInternal(
          	_address, 
          	_connection, 
          	_stub) { 
    }
};

} // namespace tests
} // namespace commonapi
} // namespace v1

#endif // V1_COMMONAPI_TESTS_Test_Interface_Manager_DBUS_STUB_ADAPTER_HPP_
