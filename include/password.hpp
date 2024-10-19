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

#include <nlohmann/json.hpp>
#include <sdbusplus/asio/object_server.hpp>
#include <sdbusplus/server.hpp>
#include <xyz/openbmc_project/BIOSConfig/Password/server.hpp>

#include <filesystem>
#include <string>

namespace bios_config_pwd
{
static constexpr auto objectPathPwd =
    "/xyz/openbmc_project/bios_config/password";
constexpr auto biosSeedFile = "seedData";
constexpr uint8_t maxHashSize = 64;
constexpr uint8_t maxSeedSize = 32;
constexpr uint8_t maxPasswordLen = 32;
constexpr int iterValue = 1000;

using Base = sdbusplus::xyz::openbmc_project::BIOSConfig::server::Password;
namespace fs = std::filesystem;

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
    void verifyPassword(std::string userName, std::string currentPassword,
                        std::string newPassword);
    bool compareDigest(const EVP_MD* digestFunc, size_t digestLen,
                       const std::array<uint8_t, maxHashSize>& expected,
                       const std::array<uint8_t, maxSeedSize>& seed,
                       const std::string& rawData);
    bool isMatch(const std::array<uint8_t, maxHashSize>& expected,
                 const std::array<uint8_t, maxSeedSize>& seed,
                 const std::string& rawData, const std::string& algo);
    bool getParam(std::array<uint8_t, maxHashSize>& orgUsrPwdHash,
                  std::array<uint8_t, maxHashSize>& orgAdminPwdHash,
                  std::array<uint8_t, maxSeedSize>& seed,
                  std::string& hashAlgo);
    bool verifyIntegrityCheck(std::string& newPassword,
                              std::array<uint8_t, maxSeedSize>& seed,
                              unsigned int mdLen, const EVP_MD* digestFunc);
    sdbusplus::asio::object_server& objServer;
    std::shared_ptr<sdbusplus::asio::connection>& systemBus;
    std::filesystem::path seedFile;
    std::array<uint8_t, maxHashSize> mNewPwdHash;
};

} // namespace bios_config_pwd
