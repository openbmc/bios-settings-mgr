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

#include "manager.hpp"
#include "password.hpp"

#include <boost/asio.hpp>
#include <phosphor-logging/elog-errors.hpp>
#include <phosphor-logging/lg2.hpp>
#include <sdbusplus/asio/connection.hpp>
#include <sdbusplus/asio/object_server.hpp>

int main()
{
    boost::asio::io_service io;
    auto systemBus = std::make_shared<sdbusplus::asio::connection>(io);

    systemBus->request_name(bios_config::service);
    sdbusplus::asio::object_server objectServer(systemBus);

    /**
     * Manager class is responsible handling methods and signals under
     * the following object path and interface.
     *
     * Object path : /xyz/openbmc_project/bios_config/manager
     * Interface : xyz.openbmc_project.BIOSConfig.Manager
     */
    bios_config::Manager manager(objectServer, systemBus);

    /**
     * Password class is responsible handling methods and signals under
     * the following object path and interface.
     *
     * Object path : /xyz/openbmc_project/bios_config/password
     * Interface : xyz.openbmc_project.BIOSConfig.Password
     */
    bios_config_pwd::Password password(objectServer, systemBus);

    io.run();
    return 0;
}
