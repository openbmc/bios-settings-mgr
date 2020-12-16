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

#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/sha.h>

#include <sdbusplus/asio/object_server.hpp>
#include <sdbusplus/server.hpp>
#include <xyz/openbmc_project/BIOSConfig/Password/server.hpp>

#include <string>

namespace bios_config_pwd
{

static constexpr auto servicePwd = "xyz.openbmc_project.BIOSConfigPassword";
static constexpr auto objectPathPwd =
    "/xyz/openbmc_project/bios_config/password";

using Base = sdbusplus::xyz::openbmc_project::BIOSConfig::server::Password;

/** @class Password
 *
 *  @brief Implements the BIOS Password
 */
class Password : public Base
{
  public:
    Password() = delete;
    ~Password() = default;
    Password(const Password&) = delete;
    Password& operator=(const Password&) = delete;
    Password(Password&&) = delete;
    Password& operator=(Password&&) = delete;

    /** @brief Constructs Password object.
     *
     *  @param[in] objectServer  - object server
     *  @param[in] systemBus - bus connection
     */
    Password(sdbusplus::asio::object_server& objectServer,
             std::shared_ptr<sdbusplus::asio::connection>& systemBus);

    /** @brief Set the BIOS attribute with a new value, the new value is added
     *         to the PendingAttribute.
     *
     *  @param[in] userName - User name - user / admin.
     *  @param[in] currentPassword - Current user/ admin Password.
     *  @param[in] newPassword - New user/ admin Password.
     */
    void changePassword(std::string userName, std::string currentPassword,
                        std::string newPassword) override;

  private:
    sdbusplus::asio::object_server& objServer;
    std::shared_ptr<sdbusplus::asio::connection>& systemBus;
};

} // namespace bios_config_pwd
