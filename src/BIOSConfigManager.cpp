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
#include <boost/asio.hpp>
#include <boost/crc.hpp>
#include <boost/process/child.hpp>
#include <boost/process/io.hpp>
#include <boost/process/system.hpp>
#include <iostream>
#include <phosphor-logging/log.hpp>
#include <sdbusplus/asio/connection.hpp>
#include <sdbusplus/asio/object_server.hpp>

/*baseBIOSTable
map{attributeName,struct{attributeType,readonlyStatus,displayname,
              description,menuPath,current,default,
              array{struct{optionstring,optionvalue}}}}
*/
std::tuple<BIOSConfigMgr::AttributeType, std::variant<int64_t, std::string>,
           std::variant<int64_t, std::string>>
BIOSConfigMgr::getAttribute(std::string name) {
  std::tuple<BIOSConfigMgr::AttributeType, std::variant<int64_t, std::string>,
             std::variant<int64_t, std::string>>
      ret;

  try {
    biosBaseTable_t::iterator it;
    pendingAttributes_t::iterator itPending;

    biosBaseTable_t tempBIOSTable = baseBIOSTable();
    it = tempBIOSTable.find(name);
    if (it != tempBIOSTable.end()) {
      std::get<1>(ret) = std::get<5>(it->second);
      std::get<0>(ret) = std::get<0>(it->second);
    }
    pendingAttributes_t tempPendingAttributes = pendingAttributes();
    itPending = tempPendingAttributes.find(name);
    if (itPending != tempPendingAttributes.end()) {
      std::get<2>(ret) = std::get<1>(itPending->second);
      std::get<0>(ret) = std::get<0>(itPending->second);
    }
  } catch (const std::exception &e) {
    phosphor::logging::log<phosphor::logging::level::ERR>(
        "Failed in getAttribute.",
        phosphor::logging::entry("ERR=%s", e.what()));
  }
  return ret;
}

/*
 The Pending attribute name and new value.
              ex- { {"QuietBoot",Type.Integer, 0x1},
                    { "DdrFreqLimit",Type.String,"2933"}
                  }
*/
void BIOSConfigMgr::setAttribute(std::string name,
                                 std::variant<int64_t, std::string> value) {
  try {
    pendingAttributes_t::iterator itPending;
    pendingAttributes_t tempPendingAttributes = pendingAttributes();
    itPending = tempPendingAttributes.find(name);
    if (itPending != tempPendingAttributes.end()) {
      std::get<1>(itPending->second) = value;
    } else {
      std::tuple<AttributeType, std::variant<int64_t, std::string>> item;
      if (std::get_if<int64_t>(&value)) {
        std::get<0>(item) = AttributeType::Integer;
      } else {
        std::get<0>(item) = AttributeType::String;
      }
      std::get<1>(item) = value;
      tempPendingAttributes.emplace(name, item);
    }
    pendingAttributes(tempPendingAttributes);
  } catch (const std::exception &e) {
    phosphor::logging::log<phosphor::logging::level::ERR>(
        "Failed in setAttribute.",
        phosphor::logging::entry("ERR=%s", e.what()));
  }
}

BIOSConfigMgr::BIOSConfigMgr(
    sdbusplus::asio::object_server &objectServer,
    std::shared_ptr<sdbusplus::asio::connection> &systemBus)
    : sdbusplus::xyz::openbmc_project::BIOSConfig::server::Manager(
          *systemBus, BIOSConfigMgrBasePath),
      systemBus(systemBus), objServer(objectServer) {
  try {
    std::tuple<BoundType, std::variant<int64_t, std::string>> subItem(
        BoundType::OneOf, "optionvalue");
    std::tuple<
        AttributeType, bool, std::string, std::string, std::string,
        std::variant<int64_t, std::string>, std::variant<int64_t, std::string>,
        std::vector<std::tuple<BoundType, std::variant<int64_t, std::string>>>>
        item(AttributeType::String, true, "displayname", "description",
             "menuPath", "currentValue", "defaultValue", {subItem});
    std::tuple<AttributeType, std::variant<int64_t, std::string>> itemPending(
        AttributeType::String, "pendingValue");

    biosBaseTable_t defaultBIOSTable;
    pendingAttributes_t defaultPendingAttributes;

    defaultBIOSTable.emplace("testAttributeName", item);
    defaultPendingAttributes.emplace("testAttributeName", itemPending);
    baseBIOSTable(defaultBIOSTable);
    pendingAttributes(defaultPendingAttributes);
  } catch (const std::exception &e) {
    phosphor::logging::log<phosphor::logging::level::ERR>(
        "Failed in BIOSConfigMgr.",
        phosphor::logging::entry("ERR=%s", e.what()));
  }
}

int main() {
  boost::asio::io_service io;
  auto systemBus = std::make_shared<sdbusplus::asio::connection>(io);

  systemBus->request_name(BIOSConfigSrvName);
  sdbusplus::asio::object_server objectServer(systemBus);
  BIOSConfigMgr biosConfigMgr(objectServer, systemBus);

  io.run();
  return 0;
}
