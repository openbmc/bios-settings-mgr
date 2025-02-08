#include "secureboot.hpp"

#include <cereal/archives/binary.hpp>

#include <fstream>

// Register class version with Cereal
CEREAL_CLASS_VERSION(bios_config::SecureBoot, 0)

namespace bios_config
{

SecureBoot::SecureBoot(sdbusplus::asio::object_server& objectServer,
                       std::shared_ptr<sdbusplus::asio::connection>& systemBus,
                       std::string persistPath) :
    sdbusplus::xyz::openbmc_project::BIOSConfig::server::SecureBoot(
        *systemBus, secureBootObjectPath),
    objServer(objectServer), systemBus(systemBus)
{
    fs::path secureBootDir(persistPath);
    fs::create_directories(secureBootDir);
    secureBootFile = secureBootDir / secureBootPersistFile;
    deserialize();
}

SecureBootBase::CurrentBootType SecureBoot::currentBoot(
    SecureBootBase::CurrentBootType value)
{
    auto ret = SecureBootBase::currentBoot(value);
    serialize();
    return ret;
}

bool SecureBoot::pendingEnable(bool value)
{
    auto ret = SecureBootBase::pendingEnable(value);
    serialize();
    return ret;
}

SecureBootBase::ModeType SecureBoot::mode(SecureBootBase::ModeType value)
{
    auto ret = SecureBootBase::mode(value);
    serialize();
    return ret;
}

void SecureBoot::serialize()
{
    try
    {
        std::filesystem::create_directories(secureBootFile.parent_path());
        std::ofstream os(secureBootFile.c_str(),
                         std::ios::out | std::ios::binary);
        cereal::BinaryOutputArchive oarchive(os);
        oarchive(*this);
    }
    catch (const std::exception& e)
    {
        lg2::error("Failed to serialize SecureBoot: {ERROR}", "ERROR", e);
    }
}

bool SecureBoot::deserialize()
{
    try
    {
        if (std::filesystem::exists(secureBootFile))
        {
            std::ifstream is(secureBootFile.c_str(),
                             std::ios::in | std::ios::binary);
            cereal::BinaryInputArchive iarchive(is);
            iarchive(*this);
            return true;
        }
        return false;
    }
    catch (const std::exception& e)
    {
        lg2::error("Failed to deserialize SecureBoot: {ERROR}", "ERROR", e);
        return false;
    }
}
} // namespace bios_config
