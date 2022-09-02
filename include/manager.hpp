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

#include "config.h"

#include <sdbusplus/asio/object_server.hpp>
#include <sdbusplus/server.hpp>
#include <xyz/openbmc_project/BIOSConfig/Manager/server.hpp>

#include <filesystem>
#include <string>

namespace bios_config
{

static constexpr auto service = "xyz.openbmc_project.BIOSConfigManager";
static constexpr auto objectPath = "/xyz/openbmc_project/bios_config/manager";
constexpr auto biosPersistFile = "biosData";

using Base = sdbusplus::xyz::openbmc_project::BIOSConfig::server::Manager;
namespace fs = std::filesystem;

/** @class Manager
 *
 *  @brief Implements the BIOS Manager
 */
class Manager : public Base
{
  public:
    using BaseTable = std::map<
        std::string,
        std::tuple<AttributeType, bool, std::string, std::string, std::string,
                   std::variant<int64_t, std::string>,
                   std::variant<int64_t, std::string>,
                   std::vector<std::tuple<
                       BoundType, std::variant<int64_t, std::string>>>>>;

    using ResetFlag = std::map<std::string, ResetFlag>;

    using PendingAttributes =
        std::map<std::string,
                 std::tuple<AttributeType, std::variant<int64_t, std::string>>>;

    using PendingAttribute =
        std::tuple<AttributeType, std::variant<int64_t, std::string>>;

    using AttributeName = std::string;
    using AttributeValue = std::variant<int64_t, std::string>;
    using CurrentValue = std::variant<int64_t, std::string>;
    using PendingValue = std::variant<int64_t, std::string>;
    using AttributeDetails =
        std::tuple<AttributeType, CurrentValue, PendingValue>;

    Manager() = delete;
    ~Manager() = default;
    Manager(const Manager&) = delete;
    Manager& operator=(const Manager&) = delete;
    Manager(Manager&&) = delete;
    Manager& operator=(Manager&&) = delete;

    /** @brief Constructs Manager object.
     *
     *  @param[in] objectServer  - object server
     *  @param[in] systemBus - bus connection
     */
    Manager(sdbusplus::asio::object_server& objectServer,
            std::shared_ptr<sdbusplus::asio::connection>& systemBus);

    /** @brief Set the BIOS attribute with a new value, the new value is added
     *         to the PendingAttribute.
     *
     *  @param[in] attribute - attribute name
     *  @param[in] value - new value for the attribute
     *
     *  @return On error, throw exception
     */
    void setAttribute(AttributeName attribute, AttributeValue value) override;

    /** @brief Get the details of the BIOS attribute
     *
     *  @param[in] attribute - attribute name
     *
     *  @return On success, return the attribute details: attribute type,
     *          current value, pending value. On error, throw exception
     */
    AttributeDetails getAttribute(AttributeName attribute) override;

    /** @brief Set the BaseBIOSTable property and clears the PendingAttributes
     *         property
     *
     *  @param[in] value - new BaseBIOSTable
     *
     *  @return The new BaseBIOSTable that is applied.
     */
    BaseTable baseBIOSTable(BaseTable value) override;

    bool enableAfterReset(bool value) override;

    ResetFlag resetBIOSSettings(ResetFlag value);

    /** @brief Set the PendingAttributes property, additionally checks if the
     *         attributes are in the BaseBIOSTable, whether the attributes are
     *         read only and validate the attribute value based on the
     *         attribute type. PendingAttributes is cleared if value is empty.
     *
     *  @param[in] value - new PendingAttributes to append to the
     *                     PendingAttributes property
     *
     *  @return On success, return the new PendingAttributes property that is
     *          set.Throw exception if the validation fails.
     */
    PendingAttributes pendingAttributes(PendingAttributes value) override;

  private:
    /** @enum Index into the fields in the BaseBIOSTable
     */
    enum class Index : uint8_t
    {
        attributeType = 0,
        readOnly,
        displayName,
        description,
        menuPath,
        currentValue,
        defaultValue,
        options,
    };

    sdbusplus::asio::object_server& objServer;
    std::shared_ptr<sdbusplus::asio::connection>& systemBus;
    std::filesystem::path biosFile;
};

} // namespace bios_config
