#include "bootorder.hpp"

#include <cereal/archives/binary.hpp>
#include <cereal/cereal.hpp> // Added for CEREAL_CLASS_VERSION

#include <algorithm>
#include <fstream>
#include <regex>

// Register class version with Cereal
CEREAL_CLASS_VERSION(bios_config::BootOrder, 0)

namespace bios_config
{

BootOrder::BootOrder(sdbusplus::asio::object_server& objectServer,
                     std::shared_ptr<sdbusplus::asio::connection>& systemBus,
                     std::string persistPath) :
    BootOrderBase(*systemBus, bootOrderObjectPath), objServer(objectServer),
    systemBus(systemBus)
{
    fs::path bootOrderDir(persistPath);
    fs::create_directories(bootOrderDir);
    bootOrderFile = bootOrderDir / bootOrderPersistFile;
    deserialize();
}

bool BootOrder::isValidBootOrderValue(const std::string& value)
{
    // Check if it's a valid predefined value
    if (std::find(validBootOrderValues.begin(), validBootOrderValues.end(),
                  value) != validBootOrderValues.end())
    {
        return true;
    }

    // Check if it matches Boot000* pattern (Boot0001, Boot0002, etc.)
    if (std::regex_match(value, bootEntryPattern))
    {
        return true;
    }

    return false;
}

bool BootOrder::isValidBootOrderSequence(const BootOrderType& bootOrder)
{
    for (const auto& value : bootOrder)
    {
        if (!isValidBootOrderValue(value))
        {
            lg2::error("Invalid boot order value: {VALUE}", "VALUE", value);
            return false;
        }
    }
    return true;
}

BootOrder::BootOrderType BootOrder::bootOrder(BootOrderType value)
{
    // Validate the boot order sequence
    if (!isValidBootOrderSequence(value))
    {
        lg2::error("Invalid boot order sequence provided");
        throw sdbusplus::xyz::openbmc_project::Common::Error::InvalidArgument();
    }

    auto newValue = BootOrderBase::bootOrder(value, false);
    serialize();
    return newValue;
}

BootOrder::BootOrderType BootOrder::pendingBootOrder(BootOrderType value)
{
    // Validate the pending boot order sequence
    if (!isValidBootOrderSequence(value))
    {
        lg2::error("Invalid pending boot order sequence provided");
        throw sdbusplus::xyz::openbmc_project::Common::Error::InvalidArgument();
    }

    // PendingBootOrder modifies the sequence of BootOrder
    // Apply the pending changes to the current boot order
    auto newValue = BootOrderBase::pendingBootOrder(value, false);
    serialize();
    return newValue;
}

void BootOrder::serialize()
{
    try
    {
        std::filesystem::create_directories(bootOrderFile.parent_path());
        std::ofstream os(bootOrderFile.c_str(),
                         std::ios::out | std::ios::binary);
        cereal::BinaryOutputArchive oarchive(os);
        oarchive(*this);
    }
    catch (const std::exception& e)
    {
        lg2::error("Failed to serialize BootOrder: {ERROR}", "ERROR", e);
    }
}

bool BootOrder::deserialize()
{
    try
    {
        if (std::filesystem::exists(bootOrderFile))
        {
            std::ifstream is(bootOrderFile.c_str(),
                             std::ios::in | std::ios::binary);
            cereal::BinaryInputArchive iarchive(is);
            iarchive(*this);
            return true;
        }
        return false;
    }
    catch (const std::exception& e)
    {
        lg2::error("Failed to deserialize BootOrder: {ERROR}", "ERROR", e);
        return false;
    }
}
} // namespace bios_config
