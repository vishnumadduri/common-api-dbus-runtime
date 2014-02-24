/*
* This file was generated by the CommonAPI Generators.
* Used org.genivi.commonapi.core 2.1.5.qualifier.
* Used org.franca.core 0.8.10.201309262002.
*
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*/
#ifndef COMMONAPI_TESTS_Extended_Interface_DBUS_PROXY_H_
#define COMMONAPI_TESTS_Extended_Interface_DBUS_PROXY_H_

#include <commonapi/tests/ExtendedInterfaceProxyBase.h>
#include <commonapi/tests/TestInterfaceDBusProxy.h>

#if !defined (COMMONAPI_INTERNAL_COMPILATION)
#define COMMONAPI_INTERNAL_COMPILATION
#endif

#include <CommonAPI/DBus/DBusFactory.h>
#include <CommonAPI/DBus/DBusProxy.h>

#undef COMMONAPI_INTERNAL_COMPILATION

#include <string>

namespace commonapi {
namespace tests {

class ExtendedInterfaceDBusProxy: virtual public ExtendedInterfaceProxyBase, virtual public TestInterfaceDBusProxy {
 public:
    ExtendedInterfaceDBusProxy(
                    const std::shared_ptr<CommonAPI::DBus::DBusFactory>& factory,
                    const std::string& commonApiAddress,
                    const std::string& interfaceName,
                    const std::string& busName,
                    const std::string& objectPath,
                    const std::shared_ptr<CommonAPI::DBus::DBusProxyConnection>& dbusProxyconnection);

    virtual ~ExtendedInterfaceDBusProxy() { }



    virtual void TestIntMethodExtended(const uint32_t& inInt, CommonAPI::CallStatus& callStatus);
    virtual std::future<CommonAPI::CallStatus> TestIntMethodExtendedAsync(const uint32_t& inInt, TestIntMethodExtendedAsyncCallback callback);


    virtual void getOwnVersion(uint16_t& ownVersionMajor, uint16_t& ownVersionMinor) const;

 private:


};



} // namespace tests
} // namespace commonapi

#endif // COMMONAPI_TESTS_Extended_Interface_DBUS_PROXY_H_