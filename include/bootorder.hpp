#pragma once

#include <cereal/access.hpp>
#include <cereal/cereal.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>
#include <phosphor-logging/lg2.hpp>
#include <sdbusplus/asio/object_server.hpp>
#include <sdbusplus/server.hpp>
#include <xyz/openbmc_project/Control/Boot/BootOrder/server.hpp>

#include <algorithm>
#include <filesystem>
#include <regex>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace bios_config
{
static constexpr auto bootOrderObjectPath =
    "/xyz/openbmc_project/bios_config/boot_order";
static constexpr auto bootOrderPersistFile = "bootOrderData";

// Valid boot order values according to specification
static const std::vector<std::string> validBootOrderValues = {
    "DISK", "RESERVED", "NETWORK", "USB",           "HTTP",
    "IPv4", "IPv6",     "DEFAULT", "ExternalMedia", "RemovableMedia"};

// Regex pattern for Boot000* entries (Boot0001, Boot0002, etc.)
static const std::regex bootEntryPattern(R"(^Boot\d{4}$)");

using BootOrderBase =
    sdbusplus::xyz::openbmc_project::Control::Boot::server::BootOrder;

class BootOrder : public BootOrderBase
{
  public:
    BootOrder() = delete;
    ~BootOrder() = default;
    BootOrder(const BootOrder&) = delete;
    BootOrder& operator=(const BootOrder&) = delete;
    BootOrder(BootOrder&&) = delete;
    BootOrder& operator=(BootOrder&&) = delete;

    using BootOrderType = std::vector<std::string>;

    /** @brief Constructs BootOrder object.
     *
     *  @param[in] objectServer  - object server
     *  @param[in] systemBus - bus connection
     *  @param[in] persistPath - path to the bootorder data file
     */
    BootOrder(sdbusplus::asio::object_server& objectServer,
              std::shared_ptr<sdbusplus::asio::connection>& systemBus,
              std::string persistPath);

    /** @brief Indicates the UEFI Boot Order state during the current boot
     * cycle
     *
     *  @param[in] value - Boot Type during the current cycle
     *
     *  @return On success, return the CurrentBootType
     */
    BootOrderType bootOrder(BootOrderType value) override;

    /** @brief Indicates the pending UEFI Boot Order for next boot
     *
     *  @param[in] value - new value for the attribute
     *
     *  @return On success, return the new attribute
     */
    BootOrderType pendingBootOrder(BootOrderType value) override;

    /** @brief Validates if a boot order value is valid according to
     * specification
     *
     *  @param[in] value - boot order value to validate
     *
     *  @return true if valid, false otherwise
     */
    bool isValidBootOrderValue(const std::string& value);

    /** @brief Validates the entire boot order sequence
     *
     *  @param[in] bootOrder - boot order sequence to validate
     *
     *  @return true if valid, false otherwise
     */
    bool isValidBootOrderSequence(const BootOrderType& bootOrder);

  private:
    sdbusplus::asio::object_server& objServer;
    std::shared_ptr<sdbusplus::asio::connection>& systemBus;
    std::filesystem::path bootOrderFile;

    friend class cereal::access;

    /** @brief Save the SecureBoot object to the persistent storage
     *
     *  @param[in] archive - archive
     *  @param[in] version - version
     */
    template <class Archive>
    void save(Archive& archive, const std::uint32_t version) const
    {
        (void)(version);
        archive(sdbusplus::xyz::openbmc_project::Control::Boot::server::
                    BootOrder::bootOrder(),
                sdbusplus::xyz::openbmc_project::Control::Boot::server::
                    BootOrder::pendingBootOrder());
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
        BootOrder::BootOrderType bootOrderValue;
        BootOrder::BootOrderType pendingBootOrderValue;
        archive(bootOrderValue, pendingBootOrderValue);

        sdbusplus::xyz::openbmc_project::Control::Boot::server::BootOrder::
            bootOrder(bootOrderValue, true);
        sdbusplus::xyz::openbmc_project::Control::Boot::server::BootOrder::
            pendingBootOrder(pendingBootOrderValue, true);
    }

    /** @brief Serialize the BootOrder object to the persistent storage
     */
    void serialize();

    /** @brief Deserialize the BootOrder object from the persistent storage
     *
     *  @return On success, return true
     *  @return On failure, return false
     */
    bool deserialize();
};
} // namespace bios_config
