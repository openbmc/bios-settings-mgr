#pragma once

#include <cereal/access.hpp>
#include <cereal/cereal.hpp>
#include <phosphor-logging/lg2.hpp>
#include <sdbusplus/asio/object_server.hpp>
#include <sdbusplus/server.hpp>
#include <xyz/openbmc_project/BIOSConfig/SecureBoot/server.hpp>

#include <filesystem>
#include <string>

namespace fs = std::filesystem;

namespace bios_config
{
static constexpr auto secureBootObjectPath =
    "/xyz/openbmc_project/bios_config/secure_boot";
static constexpr auto secureBootPersistFile = "securebootData";

using SecureBootBase =
    sdbusplus::xyz::openbmc_project::BIOSConfig::server::SecureBoot;

class SecureBoot : public SecureBootBase
{
  public:
    SecureBoot() = delete;
    ~SecureBoot() = default;
    SecureBoot(const SecureBoot&) = delete;
    SecureBoot& operator=(const SecureBoot&) = delete;
    SecureBoot(SecureBoot&&) = delete;
    SecureBoot& operator=(SecureBoot&&) = delete;

    /** @brief Constructs SecureBoot object.
     *
     *  @param[in] objectServer  - object server
     *  @param[in] systemBus - bus connection
     *  @param[in] persistPath - path to the secureboot data file
     */
    SecureBoot(sdbusplus::asio::object_server& objectServer,
               std::shared_ptr<sdbusplus::asio::connection>& systemBus,
               std::string persistPath);

    /** @brief Indicates the UEFI Secure Boot state during the current boot
     * cycle
     *
     *  @param[in] value - Boot Type during the current cycle
     *
     *  @return On success, return the CurrentBootType
     */
    CurrentBootType currentBoot(CurrentBootType value) override;

    /** @brief Indicates whether the UEFI Secure Boot takes effect on next boot
     *
     *  @param[in] value - new value for the attribute
     *
     *  @return On succes, return the new attribute
     */
    bool pendingEnable(bool value) override;

    /** @brief Indicates the current UEFI Secure Boot Mode
     *
     *  @param[in] value - new value for the attribute
     *
     *  @return On success, return the new attribute
     */
    ModeType mode(ModeType value) override;

  private:
    sdbusplus::asio::object_server& objServer;
    std::shared_ptr<sdbusplus::asio::connection>& systemBus;
    std::filesystem::path secureBootFile;

    friend class cereal::access;

    /** @brief Save the SecureBoot object to the persistent storage
     *
     *  @param[in] archive - archive
     *  @param[in] version - version
     */
    template <class Archive>
    void save(Archive& archive, const std::uint32_t version) const
    {
        // version is not used currently
        lg2::error("Save is called with version {VER}", "VER", version);
        archive(sdbusplus::xyz::openbmc_project::BIOSConfig::server::
                    SecureBoot::currentBoot(),
                sdbusplus::xyz::openbmc_project::BIOSConfig::server::
                    SecureBoot::pendingEnable(),
                sdbusplus::xyz::openbmc_project::BIOSConfig::server::
                    SecureBoot::mode());
    }

    /** @brief Load the SecureBoot object from the persistent storage
     *
     *  @param[in] archive - archive
     *  @param[in] version - version
     */
    template <class Archive>
    void load(Archive& archive, const std::uint32_t version)
    {
        (void)(version);
        SecureBoot::CurrentBootType currentBootValue =
            SecureBoot::CurrentBootType::Unknown;
        bool enableValue = false;
        SecureBoot::ModeType modeValue = SecureBoot::ModeType::Unknown;

        archive(currentBootValue, enableValue, modeValue);
        sdbusplus::xyz::openbmc_project::BIOSConfig::server::SecureBoot::
            currentBoot(currentBootValue, true);
        sdbusplus::xyz::openbmc_project::BIOSConfig::server::SecureBoot::
            pendingEnable(enableValue, true);
        sdbusplus::xyz::openbmc_project::BIOSConfig::server::SecureBoot::mode(
            modeValue, true);
    }

    /** @brief Serialize the SecureBoot object to the persistent storage
     */
    void serialize();

    /** @brief Deserialize the SecureBoot object from the persistent storage
     *
     *  @return On success, return true
     *  @return On failure, return false
     */
    bool deserialize();
};
} // namespace bios_config
