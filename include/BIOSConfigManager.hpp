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
#pragma once
#include <sdbusplus/asio/object_server.hpp>
#include <sdbusplus/server.hpp>
#include <string>
#include <xyz/openbmc_project/BIOSConfig/Manager/server.hpp>

static constexpr const char *BIOSConfigSrvName =
    "xyz.openbmc_project.biosconfig-manager";
static constexpr const char *BIOSConfigMgrBasePath =
    "/xyz/openbmc_project/BIOSConfig/Manager";

class BIOSConfigMgr
    : sdbusplus::xyz::openbmc_project::BIOSConfig::server::Manager {
public:
  BIOSConfigMgr(sdbusplus::asio::object_server &objectServer,
                std::shared_ptr<sdbusplus::asio::connection> &systemBus);
  ~BIOSConfigMgr() = default;
  using biosBaseTable_t =
      std::map<std::string,
               std::tuple<AttributeType, bool, std::string, std::string,
                          std::string, std::variant<int64_t, std::string>,
                          std::variant<int64_t, std::string>,
                          std::vector<std::tuple<
                              BoundType, std::variant<int64_t, std::string>>>>>;
  using pendingAttributes_t =
      std::map<std::string, std::tuple<BIOSConfigMgr::AttributeType,
                                       std::variant<int64_t, std::string>>>;

  void setAttribute(std::string attributeName,
                    std::variant<int64_t, std::string> attributeValue) override;
  std::tuple<AttributeType, std::variant<int64_t, std::string>,
             std::variant<int64_t, std::string>>
  getAttribute(std::string attributeName) override;

private:
  sdbusplus::asio::object_server &objServer;
  std::shared_ptr<sdbusplus::asio::connection> &systemBus;
};
