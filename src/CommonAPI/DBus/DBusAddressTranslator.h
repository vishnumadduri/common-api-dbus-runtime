/* Copyright (C) 2013 BMW Group
 * Author: Manfred Bathelt (manfred.bathelt@bmw.de)
 * Author: Juergen Gehring (juergen.gehring@bmw.de)
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */


#ifndef DBUSNAMESERVICE_H_
#define DBUSNAMESERVICE_H_

#include "DBusFunctionalHash.h"

#include <algorithm>
#include <unordered_map>


namespace CommonAPI {
namespace DBus {


//connectionName, objectPath, interfaceName
typedef std::tuple<std::string, std::string, std::string> DBusServiceAddress;


class DBusAddressTranslator {
public:
    ~DBusAddressTranslator();

    static DBusAddressTranslator& getInstance();

    void searchForDBusAddress(const std::string& commonApiAddress,
                              std::string& interfaceName,
                              std::string& connectionName,
                              std::string& objectPath);

    void searchForCommonAddress(const std::string& interfaceName,
                                const std::string& connectionName,
                                const std::string& objectPath,
                                std::string& commonApiAddress);

private:
    DBusAddressTranslator();
    DBusAddressTranslator(const DBusAddressTranslator&) = delete;
    DBusAddressTranslator& operator=(const DBusAddressTranslator&) = delete;

    void init();

    void findFallbackDBusAddress(const std::string& instanceId,
                                    std::string& interfaceName,
                                    std::string& connectionName,
                                    std::string& objectPath) const;

    void findFallbackCommonAddress(std::string& instanceId,
                                      const std::string& interfaceName,
                                      const std::string& connectionName,
                                      const std::string& objectPath) const;

    void fillUndefinedValues(DBusServiceAddress& dbusServiceAddress, const std::string& commonApiAddress) const;

    std::unordered_map<std::string, DBusServiceAddress> knownDBusAddresses;
    std::unordered_map<DBusServiceAddress, std::string> knownCommonAddresses;
};


}// namespace DBus
}// namespace CommonAPI

#endif /* DBUSNAMESERVICE_H_ */
