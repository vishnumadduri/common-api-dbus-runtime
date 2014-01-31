/*
* This file was generated by the CommonAPI Generators.
* Used org.genivi.commonapi.core 2.1.5.qualifier.
* Used org.franca.core 0.8.10.201309262002.
*
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*/
#include "SecondRootDBusStubAdapter.h"
#include <commonapi/tests/managed/SecondRoot.h>

namespace commonapi {
namespace tests {
namespace managed {

std::shared_ptr<CommonAPI::DBus::DBusStubAdapter> createSecondRootDBusStubAdapter(
                   const std::shared_ptr<CommonAPI::DBus::DBusFactory>& factory,
                   const std::string& commonApiAddress,
                   const std::string& interfaceName,
                   const std::string& busName,
                   const std::string& objectPath,
                   const std::shared_ptr<CommonAPI::DBus::DBusProxyConnection>& dbusProxyConnection,
                   const std::shared_ptr<CommonAPI::StubBase>& stubBase) {
    return std::make_shared<SecondRootDBusStubAdapter>(factory, commonApiAddress, interfaceName, busName, objectPath, dbusProxyConnection, stubBase);
}

__attribute__((constructor)) void registerSecondRootDBusStubAdapter(void) {
    CommonAPI::DBus::DBusFactory::registerAdapterFactoryMethod(SecondRoot::getInterfaceId(),
                                                               &createSecondRootDBusStubAdapter);
}



SecondRootDBusStubAdapterInternal::~SecondRootDBusStubAdapterInternal() {
    deactivateManagedInstances();
    SecondRootDBusStubAdapterHelper::deinit();
}

void SecondRootDBusStubAdapterInternal::deactivateManagedInstances() {
    for(std::set<std::string>::iterator iter = registeredLeafInterfaceInstances.begin();
            iter != registeredLeafInterfaceInstances.end(); ++iter) {
        deregisterManagedStubLeafInterface(*iter);
    }
}

const char* SecondRootDBusStubAdapterInternal::getMethodsDBusIntrospectionXmlData() const {
    static const std::string introspectionData =
        "<method name=\"getInterfaceVersion\">\n"
            "<arg name=\"value\" type=\"uu\" direction=\"out\" />"
        "</method>\n"

        ""
    ;
    return introspectionData.c_str();
}

static CommonAPI::DBus::DBusGetAttributeStubDispatcher<
        SecondRootStub,
        CommonAPI::Version
        > getSecondRootInterfaceVersionStubDispatcher(&SecondRootStub::getInterfaceVersion, "uu");





const SecondRootDBusStubAdapterHelper::StubDispatcherTable& SecondRootDBusStubAdapterInternal::getStubDispatcherTable() {
    return stubDispatcherTable_;
}


bool SecondRootDBusStubAdapterInternal::registerManagedStubLeafInterface(std::shared_ptr<LeafInterfaceStub> stub, const std::string& instance) {
    if (registeredLeafInterfaceInstances.find(instance) == registeredLeafInterfaceInstances.end()) {
        std::string commonApiAddress = "local:commonapi.tests.managed.LeafInterface:" + instance;

        std::string interfaceName;
        std::string connectionName;
        std::string objectPath;

        CommonAPI::DBus::DBusAddressTranslator::getInstance().searchForDBusAddress(
                commonApiAddress,
                interfaceName,
                connectionName,
                objectPath);

        if (objectPath.compare(0, dbusObjectPath_.length(), dbusObjectPath_) == 0) {
            auto dbusStubAdapter = factory_->createDBusStubAdapter(stub, "commonapi.tests.managed.LeafInterface",
                    instance, "commonapi.tests.managed.LeafInterface", "local");

            bool success = CommonAPI::DBus::DBusServicePublisher::getInstance()->registerManagedService(dbusStubAdapter);
            if (success) {
                bool isServiceExportSuccessful = dbusConnection_->getDBusObjectManager()->exportManagedDBusStubAdapter(dbusObjectPath_, dbusStubAdapter);
                if (isServiceExportSuccessful) {
                    registeredLeafInterfaceInstances.insert(instance);
                    return true;
                } else {
                    const bool isManagedDeregistrationSuccessful =
                        CommonAPI::DBus::DBusServicePublisher::getInstance()->unregisterManagedService(
                                        commonApiAddress);
                }
            }
        }
    }
    return false;
}

bool SecondRootDBusStubAdapterInternal::deregisterManagedStubLeafInterface(const std::string& instance) {
    std::string commonApiAddress = "local:commonapi.tests.managed.LeafInterface:" + instance;
    if (registeredLeafInterfaceInstances.find(instance) != registeredLeafInterfaceInstances.end()) {
        std::shared_ptr<CommonAPI::DBus::DBusStubAdapter> dbusStubAdapter =
                    CommonAPI::DBus::DBusServicePublisher::getInstance()->getRegisteredService(commonApiAddress);
        if (dbusStubAdapter != nullptr) {
            dbusConnection_->getDBusObjectManager()->unexportManagedDBusStubAdapter(dbusObjectPath_, dbusStubAdapter);
            CommonAPI::DBus::DBusServicePublisher::getInstance()->unregisterManagedService(commonApiAddress);
            registeredLeafInterfaceInstances.erase(instance);
            return true;
        }
    }
    return false;
}

std::set<std::string>& SecondRootDBusStubAdapterInternal::getLeafInterfaceInstances() {
    return registeredLeafInterfaceInstances;
}

SecondRootDBusStubAdapterInternal::SecondRootDBusStubAdapterInternal(
        const std::shared_ptr<CommonAPI::DBus::DBusFactory>& factory,
        const std::string& commonApiAddress,
        const std::string& dbusInterfaceName,
        const std::string& dbusBusName,
        const std::string& dbusObjectPath,
        const std::shared_ptr<CommonAPI::DBus::DBusProxyConnection>& dbusConnection,
        const std::shared_ptr<CommonAPI::StubBase>& stub):
        CommonAPI::DBus::DBusStubAdapter(
                factory,
                commonApiAddress,
                dbusInterfaceName,
                dbusBusName,
                dbusObjectPath,
                dbusConnection,
                true),
        SecondRootDBusStubAdapterHelper(
            factory,
            commonApiAddress,
            dbusInterfaceName,
            dbusBusName,
            dbusObjectPath,
            dbusConnection,
            std::dynamic_pointer_cast<SecondRootStub>(stub),
            true),
        stubDispatcherTable_({
            }) {

    stubDispatcherTable_.insert({ { "getInterfaceVersion", "" }, &commonapi::tests::managed::getSecondRootInterfaceVersionStubDispatcher });
}

} // namespace managed
} // namespace tests
} // namespace commonapi
