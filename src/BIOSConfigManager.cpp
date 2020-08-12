/*
// Copyright (c) 2020 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
*/
#include "BIOSConfigManager.hpp"

#include "xyz/openbmc_project/BIOSConfig/Common/error.hpp"

#include <boost/asio.hpp>
#include <sdbusplus/asio/connection.hpp>
#include <sdbusplus/asio/object_server.hpp>

#include <iostream>

std::tuple<BIOSConfigMgr::AttributeType, std::variant<int64_t, std::string>,
           std::variant<int64_t, std::string>>
    BIOSConfigMgr::getAttribute(std::string name)
{
    std::tuple<BIOSConfigMgr::AttributeType, std::variant<int64_t, std::string>,
               std::variant<int64_t, std::string>>
        ret;

    biosBaseTable_t tempBIOSTable = baseBIOSTable();
    auto it = tempBIOSTable.find(name);
    if (it != tempBIOSTable.end())
    {
        std::get<1>(ret) = std::get<5>(it->second);
        std::get<0>(ret) = std::get<0>(it->second);
        pendingAttributes_t tempPendingAttributes = pendingAttributes();
        auto itPending = tempPendingAttributes.find(name);
        if (itPending != tempPendingAttributes.end())
        {
            std::get<2>(ret) = std::get<1>(itPending->second);
        }
    }
    else
    {
        throw sdbusplus::xyz::openbmc_project::BIOSConfig::Common::Error::
            AttributeNotFound();
    }

    return ret;
}

void BIOSConfigMgr::setAttribute(std::string name,
                                 std::variant<int64_t, std::string> value)
{
    pendingAttributes_t tempPendingAttributes = pendingAttributes();
    auto itPending = tempPendingAttributes.find(name);
    if (itPending != tempPendingAttributes.end())
    {
        std::get<1>(itPending->second) = value;
    }
    else
    {
        std::tuple<AttributeType, std::variant<int64_t, std::string>>
            attributeValue;
        if (std::get_if<int64_t>(&value))
        {
            std::get<0>(attributeValue) = AttributeType::Integer;
        }
        else
        {
            std::get<0>(attributeValue) = AttributeType::String;
        }
        std::get<1>(attributeValue) = value;
        tempPendingAttributes.emplace(name, attributeValue);
    }
    pendingAttributes(tempPendingAttributes);
}

void monitorBaseBIOSTableChange(
    std::shared_ptr<sdbusplus::asio::connection>& systemBus,
    sdbusplus::message::message& msg)
{
    using pendingAttributes_t =
        std::map<std::string,
                 std::tuple<std::string, std::variant<int64_t, std::string>>>;
    pendingAttributes_t emptyAttributes = {};
    if (std::string(msg.get_member()) == "BaseBIOSTable")
    {
        systemBus->async_method_call(
            [](boost::system::error_code ec) {
                if (ec)
                {
                    std::cerr << "monitorBaseBIOSTableChange error (ec = " << ec
                              << ")\n";
                    return;
                }
            },
            BIOSConfigSrvName, BIOSConfigMgrBasePath,
            "org.freedesktop.DBus.Properties", "Set", BIOSConfigMgrIntf,
            "PendingAttributes",
            std::variant<pendingAttributes_t>(emptyAttributes));
    }
}

BIOSConfigMgr::BIOSConfigMgr(
    sdbusplus::asio::object_server& objectServer,
    std::shared_ptr<sdbusplus::asio::connection>& systemBus) :
    sdbusplus::xyz::openbmc_project::BIOSConfig::server::Manager(
        *systemBus, BIOSConfigMgrBasePath),
    objServer(objectServer), systemBus(systemBus)
{}

int main()
{
    boost::asio::io_service io;
    auto systemBus = std::make_shared<sdbusplus::asio::connection>(io);
    std::vector<std::unique_ptr<sdbusplus::bus::match::match>> matches;

    systemBus->request_name(BIOSConfigSrvName);
    sdbusplus::asio::object_server objectServer(systemBus);
    BIOSConfigMgr biosConfigMgr(objectServer, systemBus);

    std::function<void(sdbusplus::message::message&)> managerHandler =
        [&systemBus](sdbusplus::message::message& msg) {
            monitorBaseBIOSTableChange(systemBus, msg);
        };
    auto match = std::make_unique<sdbusplus::bus::match::match>(
        static_cast<sdbusplus::bus::bus&>(*systemBus),
        "type='signal',member='PropertiesChanged',path_namespace='" +
            std::string(BIOSConfigMgrBasePath) + "',arg0namespace='" +
            BIOSConfigMgrIntf + "'",
        std::move(managerHandler));
    matches.emplace_back(std::move(match));

    io.run();
    return 0;
}