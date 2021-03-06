/*
* This file was generated by the CommonAPI Generators.
* Used org.genivi.commonapi.dbus 3.1.5.v201512091037.
* Used org.franca.core 0.9.1.201412191134.
*
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*/
#include <v1/commonapi/tests/TestFreedesktopInterface.hpp>
#include <v1/commonapi/tests/TestFreedesktopInterfaceDBusStubAdapter.hpp>

namespace v1 {
namespace commonapi {
namespace tests {

std::shared_ptr<CommonAPI::DBus::DBusStubAdapter> createTestFreedesktopInterfaceDBusStubAdapter(
                   const CommonAPI::DBus::DBusAddress &_address,
                   const std::shared_ptr<CommonAPI::DBus::DBusProxyConnection> &_connection,
                   const std::shared_ptr<CommonAPI::StubBase> &_stub) {
    return std::make_shared<TestFreedesktopInterfaceDBusStubAdapter>(_address, _connection, _stub);
}

INITIALIZER(registerTestFreedesktopInterfaceDBusStubAdapter) {
    CommonAPI::DBus::Factory::get()->registerStubAdapterCreateMethod(
    	TestFreedesktopInterface::getInterface(), &createTestFreedesktopInterfaceDBusStubAdapter);
}

TestFreedesktopInterfaceDBusStubAdapterInternal::~TestFreedesktopInterfaceDBusStubAdapterInternal() {
    deactivateManagedInstances();
    TestFreedesktopInterfaceDBusStubAdapterHelper::deinit();
}

void TestFreedesktopInterfaceDBusStubAdapterInternal::deactivateManagedInstances() {

}

const char* TestFreedesktopInterfaceDBusStubAdapterInternal::getMethodsDBusIntrospectionXmlData() const {
    static const std::string introspectionData =
        "<method name=\"getInterfaceVersion\">\n"
            "<arg name=\"value\" type=\"uu\" direction=\"out\" />"
        "</method>\n"
        "<property name=\"TestPredefinedTypeAttribute\" type=\"u\" access=\"readwrite\" />\n"
        "<property name=\"TestReadonlyAttribute\" type=\"u\" access=\"read\" />\n"
        "<property name=\"TestDerivedStructAttribute\" type=\"(sqi)\" access=\"readwrite\" />\n"
        "<property name=\"TestDerivedArrayAttribute\" type=\"at\" access=\"readwrite\" />\n"

    ;
    return introspectionData.c_str();
}

CommonAPI::DBus::DBusGetAttributeStubDispatcher<
        ::v1::commonapi::tests::TestFreedesktopInterfaceStub,
        CommonAPI::Version
        > TestFreedesktopInterfaceDBusStubAdapterInternal::getTestFreedesktopInterfaceInterfaceVersionStubDispatcher(&TestFreedesktopInterfaceStub::getInterfaceVersion, "uu");

CommonAPI::DBus::DBusGetFreedesktopAttributeStubDispatcher<
        ::v1::commonapi::tests::TestFreedesktopInterfaceStub,
        uint32_t
        > TestFreedesktopInterfaceDBusStubAdapterInternal::getTestPredefinedTypeAttributeAttributeStubDispatcher(
            &::v1::commonapi::tests::TestFreedesktopInterfaceStub::getTestPredefinedTypeAttributeAttribute
            );
CommonAPI::DBus::DBusSetFreedesktopObservableAttributeStubDispatcher<
        ::v1::commonapi::tests::TestFreedesktopInterfaceStub,
        uint32_t
        > TestFreedesktopInterfaceDBusStubAdapterInternal::setTestPredefinedTypeAttributeAttributeStubDispatcher(
                &::v1::commonapi::tests::TestFreedesktopInterfaceStub::getTestPredefinedTypeAttributeAttribute,
                &TestFreedesktopInterfaceStubRemoteEvent::onRemoteSetTestPredefinedTypeAttributeAttribute,
                &TestFreedesktopInterfaceStubRemoteEvent::onRemoteTestPredefinedTypeAttributeAttributeChanged
                ,&TestFreedesktopInterfaceStubAdapter::fireTestPredefinedTypeAttributeAttributeChanged
                );
CommonAPI::DBus::DBusGetFreedesktopAttributeStubDispatcher<
        ::v1::commonapi::tests::TestFreedesktopInterfaceStub,
        uint32_t
        > TestFreedesktopInterfaceDBusStubAdapterInternal::getTestReadonlyAttributeAttributeStubDispatcher(
            &::v1::commonapi::tests::TestFreedesktopInterfaceStub::getTestReadonlyAttributeAttribute
            );
CommonAPI::DBus::DBusGetFreedesktopAttributeStubDispatcher<
        ::v1::commonapi::tests::TestFreedesktopInterfaceStub,
        ::commonapi::tests::DerivedTypeCollection::TestStructExtended,
        ::commonapi::tests::DerivedTypeCollection_::TestStructExtendedDeployment_t
        > TestFreedesktopInterfaceDBusStubAdapterInternal::getTestDerivedStructAttributeAttributeStubDispatcher(
            &::v1::commonapi::tests::TestFreedesktopInterfaceStub::getTestDerivedStructAttributeAttribute
            );
CommonAPI::DBus::DBusSetFreedesktopObservableAttributeStubDispatcher<
        ::v1::commonapi::tests::TestFreedesktopInterfaceStub,
        ::commonapi::tests::DerivedTypeCollection::TestStructExtended,
        ::commonapi::tests::DerivedTypeCollection_::TestStructExtendedDeployment_t
        > TestFreedesktopInterfaceDBusStubAdapterInternal::setTestDerivedStructAttributeAttributeStubDispatcher(
                &::v1::commonapi::tests::TestFreedesktopInterfaceStub::getTestDerivedStructAttributeAttribute,
                &TestFreedesktopInterfaceStubRemoteEvent::onRemoteSetTestDerivedStructAttributeAttribute,
                &TestFreedesktopInterfaceStubRemoteEvent::onRemoteTestDerivedStructAttributeAttributeChanged
                ,&TestFreedesktopInterfaceStubAdapter::fireTestDerivedStructAttributeAttributeChanged
                );
CommonAPI::DBus::DBusGetFreedesktopAttributeStubDispatcher<
        ::v1::commonapi::tests::TestFreedesktopInterfaceStub,
        ::commonapi::tests::DerivedTypeCollection::TestArrayUInt64,
        ::commonapi::tests::DerivedTypeCollection_::TestArrayUInt64Deployment_t
        > TestFreedesktopInterfaceDBusStubAdapterInternal::getTestDerivedArrayAttributeAttributeStubDispatcher(
            &::v1::commonapi::tests::TestFreedesktopInterfaceStub::getTestDerivedArrayAttributeAttribute
            );
CommonAPI::DBus::DBusSetFreedesktopObservableAttributeStubDispatcher<
        ::v1::commonapi::tests::TestFreedesktopInterfaceStub,
        ::commonapi::tests::DerivedTypeCollection::TestArrayUInt64,
        ::commonapi::tests::DerivedTypeCollection_::TestArrayUInt64Deployment_t
        > TestFreedesktopInterfaceDBusStubAdapterInternal::setTestDerivedArrayAttributeAttributeStubDispatcher(
                &::v1::commonapi::tests::TestFreedesktopInterfaceStub::getTestDerivedArrayAttributeAttribute,
                &TestFreedesktopInterfaceStubRemoteEvent::onRemoteSetTestDerivedArrayAttributeAttribute,
                &TestFreedesktopInterfaceStubRemoteEvent::onRemoteTestDerivedArrayAttributeAttributeChanged
                ,&TestFreedesktopInterfaceStubAdapter::fireTestDerivedArrayAttributeAttributeChanged
                );




void TestFreedesktopInterfaceDBusStubAdapterInternal::fireTestPredefinedTypeAttributeAttributeChanged(const uint32_t& value) {
    CommonAPI::DBus::DBusStubFreedesktopPropertiesSignalHelper<
        uint32_t,
        CommonAPI::EmptyDeployment
    > ::sendPropertiesChangedSignal(
            *this,
            "TestPredefinedTypeAttribute",
            value,
            static_cast<CommonAPI::EmptyDeployment*>(nullptr)
    );
}
void TestFreedesktopInterfaceDBusStubAdapterInternal::fireTestReadonlyAttributeAttributeChanged(const uint32_t& value) {
    CommonAPI::DBus::DBusStubFreedesktopPropertiesSignalHelper<
        uint32_t,
        CommonAPI::EmptyDeployment
    > ::sendPropertiesChangedSignal(
            *this,
            "TestReadonlyAttribute",
            value,
            static_cast<CommonAPI::EmptyDeployment*>(nullptr)
    );
}
void TestFreedesktopInterfaceDBusStubAdapterInternal::fireTestDerivedStructAttributeAttributeChanged(const ::commonapi::tests::DerivedTypeCollection::TestStructExtended& value) {
    CommonAPI::DBus::DBusStubFreedesktopPropertiesSignalHelper<
        ::commonapi::tests::DerivedTypeCollection::TestStructExtended,
        ::commonapi::tests::DerivedTypeCollection_::TestStructExtendedDeployment_t
    > ::sendPropertiesChangedSignal(
            *this,
            "TestDerivedStructAttribute",
            value,
            static_cast<::commonapi::tests::DerivedTypeCollection_::TestStructExtendedDeployment_t*>(nullptr)
    );
}
void TestFreedesktopInterfaceDBusStubAdapterInternal::fireTestDerivedArrayAttributeAttributeChanged(const ::commonapi::tests::DerivedTypeCollection::TestArrayUInt64& value) {
    CommonAPI::DBus::DBusStubFreedesktopPropertiesSignalHelper<
        ::commonapi::tests::DerivedTypeCollection::TestArrayUInt64,
        ::commonapi::tests::DerivedTypeCollection_::TestArrayUInt64Deployment_t
    > ::sendPropertiesChangedSignal(
            *this,
            "TestDerivedArrayAttribute",
            value,
            static_cast<::commonapi::tests::DerivedTypeCollection_::TestArrayUInt64Deployment_t*>(nullptr)
    );
}



const TestFreedesktopInterfaceDBusStubAdapterHelper::StubDispatcherTable& TestFreedesktopInterfaceDBusStubAdapterInternal::getStubDispatcherTable() {
    return stubDispatcherTable_;
}

const CommonAPI::DBus::StubAttributeTable& TestFreedesktopInterfaceDBusStubAdapterInternal::getStubAttributeTable() {
    return stubAttributeTable_;
}

TestFreedesktopInterfaceDBusStubAdapterInternal::TestFreedesktopInterfaceDBusStubAdapterInternal(
        const CommonAPI::DBus::DBusAddress &_address,
        const std::shared_ptr<CommonAPI::DBus::DBusProxyConnection> &_connection,
        const std::shared_ptr<CommonAPI::StubBase> &_stub)
	: CommonAPI::DBus::DBusStubAdapter(_address, _connection,false),
      TestFreedesktopInterfaceDBusStubAdapterHelper(_address, _connection, std::dynamic_pointer_cast<TestFreedesktopInterfaceStub>(_stub), false),
      stubDispatcherTable_({
            }),
        stubAttributeTable_({
        {
            "TestPredefinedTypeAttribute",
            {
                &commonapi::tests::TestFreedesktopInterfaceDBusStubAdapterInternal::getTestPredefinedTypeAttributeAttributeStubDispatcher,
                &commonapi::tests::TestFreedesktopInterfaceDBusStubAdapterInternal::setTestPredefinedTypeAttributeAttributeStubDispatcher
            }
        },
        {
            "TestReadonlyAttribute",
            {
                &commonapi::tests::TestFreedesktopInterfaceDBusStubAdapterInternal::getTestReadonlyAttributeAttributeStubDispatcher,
                NULL
            }
        },
        {
            "TestDerivedStructAttribute",
            {
                &commonapi::tests::TestFreedesktopInterfaceDBusStubAdapterInternal::getTestDerivedStructAttributeAttributeStubDispatcher,
                &commonapi::tests::TestFreedesktopInterfaceDBusStubAdapterInternal::setTestDerivedStructAttributeAttributeStubDispatcher
            }
        },
        {
            "TestDerivedArrayAttribute",
            {
                &commonapi::tests::TestFreedesktopInterfaceDBusStubAdapterInternal::getTestDerivedArrayAttributeAttributeStubDispatcher,
                &commonapi::tests::TestFreedesktopInterfaceDBusStubAdapterInternal::setTestDerivedArrayAttributeAttributeStubDispatcher
            }
        }
        }
        ) {

    stubDispatcherTable_.insert({ { "getInterfaceVersion", "" }, &commonapi::tests::TestFreedesktopInterfaceDBusStubAdapterInternal::getTestFreedesktopInterfaceInterfaceVersionStubDispatcher });
}

bool TestFreedesktopInterfaceDBusStubAdapterInternal::hasFreedesktopProperties() {
    return true;
}

} // namespace tests
} // namespace commonapi
} // namespace v1
