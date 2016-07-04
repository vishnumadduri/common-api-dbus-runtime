/*
* This file was generated by the CommonAPI Generators.
* Used org.genivi.commonapi.core 3.1.2.v201507021046.
* Used org.franca.core 0.9.1.201412191134.
*
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*/
#ifndef COMMONAPI_TESTS_Test_Freedesktop_Derived_Interface_PROXY_BASE_HPP_
#define COMMONAPI_TESTS_Test_Freedesktop_Derived_Interface_PROXY_BASE_HPP_

#include <v1_0/commonapi/tests/TestFreedesktopDerivedInterface.hpp>
#include <v1_0/commonapi/tests/TestFreedesktopInterfaceProxyBase.hpp>



#if !defined (COMMONAPI_INTERNAL_COMPILATION)
#define COMMONAPI_INTERNAL_COMPILATION
#endif


#include <CommonAPI/Attribute.hpp>
#include <CommonAPI/Proxy.hpp>

#undef COMMONAPI_INTERNAL_COMPILATION

namespace v1_0 {
namespace commonapi {
namespace tests {

class TestFreedesktopDerivedInterfaceProxyBase
    : virtual public TestFreedesktopInterfaceProxyBase {
public:
    typedef CommonAPI::ObservableAttribute<uint32_t> TestAttributedFromDerivedInterfaceAttribute;


    virtual TestAttributedFromDerivedInterfaceAttribute& getTestAttributedFromDerivedInterfaceAttribute() = 0;


};

} // namespace tests
} // namespace commonapi
} // namespace v1_0

#endif // COMMONAPI_TESTS_Test_Freedesktop_Derived_Interface_PROXY_BASE_HPP_