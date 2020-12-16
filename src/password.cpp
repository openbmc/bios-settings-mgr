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
#include "password.hpp"

#include "xyz/openbmc_project/BIOSConfig/Common/error.hpp"
#include "xyz/openbmc_project/Common/error.hpp"

#include <boost/asio.hpp>
#include <phosphor-logging/elog-errors.hpp>
#include <sdbusplus/asio/connection.hpp>
#include <sdbusplus/asio/object_server.hpp>

#include <fstream>
#include <iostream>

namespace bios_config_pwd
{
using namespace sdbusplus::xyz::openbmc_project::Common::Error;
using namespace sdbusplus::xyz::openbmc_project::BIOSConfig::Common::Error;

bool Password::isMatch(const std::string expected, const std::string seed,
                       const std::string rawData, const std::string algo)
{
    if (algo == "SHA384")
    {
        std::vector<uint8_t> output(SHA384_DIGEST_LENGTH);
        unsigned int mdLen = 0;
        if (HMAC(EVP_sha384(), seed.c_str(), seed.length(),
                 reinterpret_cast<const unsigned char*>(rawData.c_str()),
                 rawData.length(), output.data(), &mdLen) == NULL)
        {
            std::cerr << "Generate HMAC_SHA384 Integrity Check Value failed";
            output.resize(0);
            throw InternalFailure();
        }
        std::string outputStr(output.begin(), output.end());
        if (expected == outputStr)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    return false;
}

void Password::verifyPassword(std::string userName, std::string currentPassword,
                              std::string newPassword)
{
    if (fs::exists(seedFile.c_str()))
    {
        std::ifstream ifs(seedFile.c_str());
        nlohmann::json json;
        ifs >> json;
        std::string userPassword = json["UserPwdHash"];
        std::string adminPassword = json["AdminPwdHash"];
        std::string seed = json["Seed"];
        std::string hashAlgo = json["HashAlgo"];

        if (userName == "Administrator")
        {
            if (!isMatch(adminPassword, seed, currentPassword, hashAlgo))
            {
                throw InvalidCurrentPassword();
            }
        }
        else
        {
            if (!isMatch(userPassword, seed, currentPassword, hashAlgo))
            {
                throw InvalidCurrentPassword();
            }
        }
        if (hashAlgo == "SHA384")
        {
            std::vector<uint8_t> output(SHA384_DIGEST_LENGTH);
            unsigned int mdLen = 0;
            if (HMAC(
                    EVP_sha384(), seed.c_str(), seed.length(),
                    reinterpret_cast<const unsigned char*>(newPassword.c_str()),
                    newPassword.length(), output.data(), &mdLen) == NULL)
            {
                std::cerr
                    << "Generate HMAC_SHA384 Integrity Check Value failed";
                output.resize(0);
                throw InternalFailure();
            }
            std::string outputStr(output.begin(), output.end());
            mNewPassword = outputStr;
        }

        return;
    }
    throw InternalFailure();
}
void Password::changePassword(std::string userName, std::string currentPassword,
                              std::string newPassword)
{
    std::cerr << "BIOS config changePassword";
    verifyPassword(userName, currentPassword, newPassword);
    nlohmann::json json;
    json["UserName"] = userName;
    json["CurrentPassword"] = currentPassword;
    json["NewPassword"] = mNewPassword;
    std::ofstream ofs(passwordFile.c_str(), std::ios::out);
    ofs << std::setw(4) << json << std::endl;
}
Password::Password(sdbusplus::asio::object_server& objectServer,
                   std::shared_ptr<sdbusplus::asio::connection>& systemBus) :
    sdbusplus::xyz::openbmc_project::BIOSConfig::server::Password(
        *systemBus, objectPathPwd),
    objServer(objectServer), systemBus(systemBus)
{
    std::cerr << "BIOS config password is runing";
    try
    {
        fs::path biosDir(BIOS_PERSIST_PATH);
        fs::create_directories(biosDir);
        passwordFile = biosDir / biosPasswordFile;
        seedFile = biosDir / biosSeedFile;
    }
    catch (const fs::filesystem_error& e)
    {
        phosphor::logging::log<phosphor::logging::level::ERR>(e.what());
        throw InternalFailure();
    }
}

} // namespace bios_config_pwd

int main()
{
    boost::asio::io_service io;
    auto systemBus = std::make_shared<sdbusplus::asio::connection>(io);

    systemBus->request_name(bios_config_pwd::servicePwd);
    sdbusplus::asio::object_server objectServer(systemBus);
    bios_config_pwd::Password password(objectServer, systemBus);

    io.run();
    return 0;
}
