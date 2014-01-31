/*
* This file was generated by the CommonAPI Generators.
* Used org.genivi.commonapi.core 2.1.5.qualifier.
* Used org.franca.core 0.8.10.201309262002.
*
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*/
#ifndef COMMONAPI_TESTS_MANAGED_Branch_Interface_STUB_DEFAULT_H_
#define COMMONAPI_TESTS_MANAGED_Branch_Interface_STUB_DEFAULT_H_


#include <commonapi/tests/managed/BranchInterfaceStub.h>
#include <sstream>

namespace commonapi {
namespace tests {
namespace managed {

/**
 * Provides a default implementation for BranchInterfaceStubRemoteEvent and
 * BranchInterfaceStub. Method callbacks have an empty implementation,
 * remote set calls on attributes will always change the value of the attribute
 * to the one received.
 *
 * Override this stub if you only want to provide a subset of the functionality
 * that would be defined for this service, and/or if you do not need any non-default
 * behaviour.
 */
class BranchInterfaceStubDefault : public virtual BranchInterfaceStub {
public:
    BranchInterfaceStubDefault();

    BranchInterfaceStubRemoteEvent* initStubAdapter(const std::shared_ptr<BranchInterfaceStubAdapter>& stubAdapter);

    const CommonAPI::Version& getInterfaceVersion(std::shared_ptr<CommonAPI::ClientId> clientId);


    virtual void testBranchMethod(const std::shared_ptr<CommonAPI::ClientId> clientId, int32_t inInt, std::string inString, BranchInterface::testBranchMethodError& methodError, int32_t& outInt, std::string& outString);
    virtual void testBranchMethod(int32_t inInt, std::string inString, BranchInterface::testBranchMethodError& methodError, int32_t& outInt, std::string& outString);




protected:
private:
    class RemoteEventHandler: public BranchInterfaceStubRemoteEvent {
     public:
        RemoteEventHandler(BranchInterfaceStubDefault* defaultStub);


     private:
        BranchInterfaceStubDefault* defaultStub_;
    };

    RemoteEventHandler remoteEventHandler_;


    CommonAPI::Version interfaceVersion_;
};

} // namespace managed
} // namespace tests
} // namespace commonapi

#endif // COMMONAPI_TESTS_MANAGED_Branch_Interface_STUB_DEFAULT_H_
