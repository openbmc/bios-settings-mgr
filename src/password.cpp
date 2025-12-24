/*
 Copyright (c) 2020 Intel Corporation

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

      http:www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
*/
#include "password.hpp"

#include "xyz/openbmc_project/BIOSConfig/Common/error.hpp"
#include "xyz/openbmc_project/Common/error.hpp"

#include <boost/algorithm/hex.hpp>
#include <boost/asio.hpp>
#include <phosphor-logging/elog-errors.hpp>
#include <phosphor-logging/lg2.hpp>
#include <sdbusplus/asio/connection.hpp>
#include <sdbusplus/asio/object_server.hpp>

#include <fstream>
#include <iostream>

namespace bios_config_pwd
{
using namespace sdbusplus::xyz::openbmc_project::Common::Error;
using namespace sdbusplus::xyz::openbmc_project::BIOSConfig::Common::Error;

bool Password::compareDigest(const EVP_MD* digestFunc, size_t digestLen,
                             const std::array<uint8_t, maxHashSize>& expected,
                             const std::array<uint8_t, maxSeedSize>& seed,
                             const std::string& rawData)
{
    std::vector<uint8_t> output(digestLen);
    unsigned int hashLen = digestLen;

    if (!PKCS5_PBKDF2_HMAC(reinterpret_cast<const char*>(rawData.c_str()),
                           rawData.length() + 1,
                           reinterpret_cast<const unsigned char*>(seed.data()),
                           seed.size(), iterValue, digestFunc, hashLen,
                           output.data()))
    {
        lg2::error("Generate PKCS5_PBKDF2_HMAC Integrity Check Value failed");
        throw InternalFailure();
    }

    if (std::memcmp(output.data(), expected.data(),
                    output.size() * sizeof(uint8_t)) == 0)
    {
        return true;
    }

    return false;
}

bool Password::isMatch(const std::array<uint8_t, maxHashSize>& expected,
                       const std::array<uint8_t, maxSeedSize>& seed,
                       const std::string& rawData, const std::string& algo)
{
    lg2::error("isMatch");

    if (algo == "SHA256")
    {
        return compareDigest(EVP_sha256(), SHA256_DIGEST_LENGTH, expected, seed,
                             rawData);
    }

    if (algo == "SHA384")
    {
        return compareDigest(EVP_sha384(), SHA384_DIGEST_LENGTH, expected, seed,
                             rawData);
    }

    return false;
}

bool Password::getParam(std::array<uint8_t, maxHashSize>& orgUsrPwdHash,
                        std::array<uint8_t, maxHashSize>& orgAdminPwdHash,
                        std::array<uint8_t, maxSeedSize>& seed,
                        std::string& hashAlgo)
{
    try
    {
        nlohmann::json json = nullptr;
        std::ifstream ifs(seedFile.c_str());
        if (ifs.is_open())
        {
            try
            {
                json = nlohmann::json::parse(ifs, nullptr, false);
            }
            catch (const nlohmann::json::parse_error& e)
            {
                lg2::error("Failed to parse JSON file: {ERROR}", "ERROR", e);
                return false;
            }

            if (!json.is_discarded())
            {
                orgUsrPwdHash = json["UserPwdHash"];
                orgAdminPwdHash = json["AdminPwdHash"];
                seed = json["Seed"];
                hashAlgo = json["HashAlgo"];
            }
        }
    }
    catch (nlohmann::detail::exception& e)
    {
        lg2::error("Failed to parse JSON file: {ERROR}", "ERROR", e);
        return false;
    }

    return true;
}

bool Password::verifyIntegrityCheck(
    std::string& newPassword, std::array<uint8_t, maxSeedSize>& seed,
    unsigned int mdLen, const EVP_MD* digestFunc)
{
    mNewPwdHash.fill(0);

    if (!PKCS5_PBKDF2_HMAC(reinterpret_cast<const char*>(newPassword.c_str()),
                           newPassword.length() + 1,
                           reinterpret_cast<const unsigned char*>(seed.data()),
                           seed.size(), iterValue, digestFunc, mdLen,
                           mNewPwdHash.data()))
    {
        lg2::error("Verify PKCS5_PBKDF2_HMAC Integrity Check failed");
        return false;
    }

    return true;
}

void Password::verifyPassword(std::string userName, std::string currentPassword,
                              std::string newPassword)
{
    if (fs::exists(seedFile.c_str()))
    {
        std::array<uint8_t, maxHashSize> orgUsrPwdHash;
        std::array<uint8_t, maxHashSize> orgAdminPwdHash;
        std::array<uint8_t, maxSeedSize> seed;
        std::string hashAlgo = "";

        if (getParam(orgUsrPwdHash, orgAdminPwdHash, seed, hashAlgo))
        {
            if (orgUsrPwdHash.empty() || orgAdminPwdHash.empty() ||
                seed.empty() || hashAlgo.empty())
            {
                return;
            }
        }
        else
        {
            throw InternalFailure();
        }

        if (userName == "AdminPassword")
        {
            if (!isMatch(orgAdminPwdHash, seed, currentPassword, hashAlgo))
            {
                throw InvalidCurrentPassword();
            }
        }
        else
        {
            if (!isMatch(orgUsrPwdHash, seed, currentPassword, hashAlgo))
            {
                throw InvalidCurrentPassword();
            }
        }
        if (hashAlgo == "SHA256")
        {
            if (!verifyIntegrityCheck(newPassword, seed, 32, EVP_sha256()))
            {
                throw InternalFailure();
            }
        }
        if (hashAlgo == "SHA384")
        {
            if (!verifyIntegrityCheck(newPassword, seed, 48, EVP_sha384()))
            {
                throw InternalFailure();
            }
        }
        return;
    }
    throw InternalFailure();
}
void Password::changePassword(std::string userName, std::string currentPassword,
                              std::string newPassword)
{
    lg2::debug("BIOS config changePassword");
    verifyPassword(userName, currentPassword, newPassword);

    std::ifstream fs(seedFile.c_str());
    nlohmann::json json = nullptr;

    if (fs.is_open())
    {
        try
        {
            json = nlohmann::json::parse(fs, nullptr, false);
        }
        catch (const nlohmann::json::parse_error& e)
        {
            lg2::error("Failed to parse JSON file: {ERROR}", "ERROR", e);
            throw InternalFailure();
        }

        if (json.is_discarded())
        {
            throw InternalFailure();
        }
        json["AdminPwdHash"] = mNewPwdHash;
        json["IsAdminPwdChanged"] = true;

        std::ofstream ofs(seedFile.c_str(), std::ios::out);
        const auto& writeData = json.dump(4);
        ofs << writeData;
        ofs.close();
    }
    else
    {
        lg2::debug("Cannot open file stream");
        throw InternalFailure();
    }
}
Password::Password(sdbusplus::asio::object_server& objectServer,
                   std::shared_ptr<sdbusplus::asio::connection>& systemBus,
                   std::string persistPath) :
    sdbusplus::xyz::openbmc_project::BIOSConfig::server::Password(
        *systemBus, objectPathPwd)
{
    // unused today; ABI kept to match main.cpp
    (void)objectServer;
    (void)systemBus;

    lg2::debug("BIOS config password is running");
    try
    {
        fs::path biosDir(persistPath);
        fs::create_directories(biosDir);
        seedFile = biosDir / biosSeedFile;
    }
    catch (const fs::filesystem_error& e)
    {
        lg2::error("Failed to parse JSON file: {ERROR}", "ERROR", e);
        throw InternalFailure();
    }
}

} // namespace bios_config_pwd
