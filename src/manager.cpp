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

#include "manager_serialize.hpp"
#include "xyz/openbmc_project/BIOSConfig/Common/error.hpp"
#include "xyz/openbmc_project/Common/error.hpp"

#include <boost/asio.hpp>
#include <phosphor-logging/elog-errors.hpp>
#include <phosphor-logging/lg2.hpp>
#include <sdbusplus/asio/connection.hpp>
#include <sdbusplus/asio/object_server.hpp>

namespace bios_config
{

using namespace sdbusplus::xyz::openbmc_project::Common::Error;
using namespace sdbusplus::xyz::openbmc_project::BIOSConfig::Common::Error;

void Manager::setAttribute(AttributeName attribute, AttributeValue value)
{
    auto pendingAttrs = Base::pendingAttributes();
    auto iter = pendingAttrs.find(attribute);

    if (iter != pendingAttrs.end())
    {
        std::get<1>(iter->second) = value;
    }
    else
    {
        Manager::PendingAttribute attributeValue;

        if (std::get_if<int64_t>(&value))
        {
            std::get<0>(attributeValue) = AttributeType::Integer;
        }
        else
        {
            std::get<0>(attributeValue) = AttributeType::String;
        }

        std::get<1>(attributeValue) = value;
        pendingAttrs.emplace(attribute, attributeValue);
    }

    pendingAttributes(pendingAttrs);
}

Manager::AttributeDetails Manager::getAttribute(AttributeName attribute)
{
    Manager::AttributeDetails value;

    auto table = Base::baseBIOSTable();
    auto iter = table.find(attribute);

    if (iter != table.end())
    {
        std::get<0>(value) =
            std::get<static_cast<uint8_t>(Index::attributeType)>(iter->second);
        std::get<1>(value) =
            std::get<static_cast<uint8_t>(Index::currentValue)>(iter->second);

        auto pending = Base::pendingAttributes();
        auto pendingIter = pending.find(attribute);
        if (pendingIter != pending.end())
        {
            std::get<2>(value) = std::get<1>(pendingIter->second);
        }
        else if (std::get_if<std::string>(&std::get<1>(value)))
        {
            std::get<2>(value) = std::string();
        }
    }
    else
    {
        throw AttributeNotFound();
    }

    return value;
}

Manager::BaseTable Manager::baseBIOSTable(BaseTable value)
{
    pendingAttributes({});
    auto baseTable = Base::baseBIOSTable(value, false);
    serialize(*this, biosFile);
    Base::resetBIOSSettings(Base::ResetFlag::NoAction);
    return baseTable;
}

Manager::PendingAttributes Manager::pendingAttributes(PendingAttributes value)
{
    // Clear the pending attributes
    if (value.empty())
    {
        auto pendingAttrs = Base::pendingAttributes({}, false);
        serialize(*this, biosFile);
        return pendingAttrs;
    }

    // Validate all the BIOS attributes before setting PendingAttributes
    BaseTable biosTable = Base::baseBIOSTable();
    for (const auto& pair : value)
    {
        auto iter = biosTable.find(pair.first);
        // BIOS attribute not found in the BaseBIOSTable
        if (iter == biosTable.end())
        {
            lg2::error("BIOS attribute not found in the BaseBIOSTable");
            throw AttributeNotFound();
        }

        auto attributeType =
            std::get<static_cast<uint8_t>(Index::attributeType)>(iter->second);
        if (attributeType != std::get<0>(pair.second))
        {
            lg2::error("attributeType is not same with bios base table");
            throw InvalidArgument();
        }

        // Validate enumeration BIOS attributes
        if (attributeType == AttributeType::Enumeration)
        {
            // For enumeration the expected variant types is Enumeration
            if (std::get<1>(pair.second).index() == 0)
            {
                lg2::error("Enumeration property value is not enum");
                throw InvalidArgument();
            }

            const auto& attrValue =
                std::get<std::string>(std::get<1>(pair.second));
            const auto& options =
                std::get<static_cast<uint8_t>(Index::options)>(iter->second);

            bool found = false;
            for (const auto& enumOptions : options)
            {
                if ((BoundType::OneOf == std::get<0>(enumOptions)) &&
                    (attrValue ==
                     std::get<std::string>(std::get<1>(enumOptions))))
                {
                    found = true;
                    break;
                }
            }

            if (!found)
            {
                lg2::error("No valid attribute");
                throw InvalidArgument();
            }
        }

        if (attributeType == AttributeType::String)
        {
            // For enumeration the expected variant types is std::string
            if (std::get<1>(pair.second).index() == 0)
            {
                lg2::error("String property value is not string");
                throw InvalidArgument();
            }

            const auto& attrValue =
                std::get<std::string>(std::get<1>(pair.second));
            const auto& options =
                std::get<static_cast<uint8_t>(Index::options)>(iter->second);

            auto optionsIterator = options.begin();

            for (; optionsIterator != options.end(); ++optionsIterator)
            {
                if (std::get<1>(std::get<1>(*optionsIterator)) == attrValue)
                {
                    break;
                }
            }

            if (optionsIterator == options.end())
            {
                lg2::error("{ATTRVALUE} is not a valid value for {NAME}",
                           "ATTRVALUE", attrValue, "NAME", pair.first);
                throw InvalidArgument();
            }
        }

        if (attributeType == AttributeType::Integer)
        {
            // For enumeration the expected variant types is Integer
            if (std::get<1>(pair.second).index() == 1)
            {
                lg2::error("Enumeration property value is not int");
                throw InvalidArgument();
            }

            const auto& attrValue = std::get<int64_t>(std::get<1>(pair.second));
            const auto& options =
                std::get<static_cast<uint8_t>(Index::options)>(iter->second);
            int64_t lowerBound = 0;
            int64_t upperBound = 0;
            int64_t scalarIncrement = 0;

            for (const auto& integerOptions : options)
            {
                if (BoundType::LowerBound == std::get<0>(integerOptions))
                {
                    lowerBound = std::get<int64_t>(std::get<1>(integerOptions));
                }
                else if (BoundType::UpperBound == std::get<0>(integerOptions))
                {
                    upperBound = std::get<int64_t>(std::get<1>(integerOptions));
                }
                else if (BoundType::ScalarIncrement ==
                         std::get<0>(integerOptions))
                {
                    scalarIncrement =
                        std::get<int64_t>(std::get<1>(integerOptions));
                }
            }

            if ((attrValue < lowerBound) || (attrValue > upperBound))
            {
                lg2::error("Integer, bound is invalid");
                throw InvalidArgument();
            }

            if (((std::abs(attrValue - lowerBound)) % scalarIncrement) != 0)
            {
                lg2::error(
                    "((std::abs({ATTR_VALUE} - {LOWER_BOUND})) % {SCALAR_INCREMENT}) != 0",
                    "ATTR_VALUE", attrValue, "LOWER_BOUND", lowerBound,
                    "SCALAR_INCREMENT", scalarIncrement);
                throw InvalidArgument();
            }
        }
    }

    PendingAttributes pendingAttribute = Base::pendingAttributes();

    for (const auto& pair : value)
    {
        auto iter = pendingAttribute.find(pair.first);
        if (iter != pendingAttribute.end())
        {
            iter = pendingAttribute.erase(iter);
        }

        pendingAttribute.emplace(std::make_pair(pair.first, pair.second));
    }

    auto pendingAttrs = Base::pendingAttributes(pendingAttribute, false);
    serialize(*this, biosFile);

    return pendingAttrs;
}

Manager::Manager(sdbusplus::asio::object_server& objectServer,
                 std::shared_ptr<sdbusplus::asio::connection>& systemBus) :
    sdbusplus::xyz::openbmc_project::BIOSConfig::server::Manager(*systemBus,
                                                                 objectPath),
    objServer(objectServer), systemBus(systemBus)
{
    fs::path biosDir(BIOS_PERSIST_PATH);
    fs::create_directories(biosDir);
    biosFile = biosDir / biosPersistFile;
    deserialize(biosFile, *this);
}

} // namespace bios_config

int main()
{
    boost::asio::io_service io;
    auto systemBus = std::make_shared<sdbusplus::asio::connection>(io);

    systemBus->request_name(bios_config::service);
    sdbusplus::asio::object_server objectServer(systemBus);
    bios_config::Manager manager(objectServer, systemBus);

    io.run();
    return 0;
}
