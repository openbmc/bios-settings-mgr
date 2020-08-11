#pragma once

#include "xyz/openbmc_project/BIOSConfig/Manager/server.hpp"

#include <sdbusplus/bus.hpp>
#include <sdbusplus/server/object.hpp>

using namespace sdbusplus::xyz::openbmc_project::BIOSConfig::server;

namespace bios_config
{

using DbusIface = sdbusplus::server::object::object<
    sdbusplus::xyz::openbmc_project::BIOSConfig::server::Manager>;

class Manager : public DbusIface
{
  public:
    Manager() = delete;
    Manager(const Manager&) = delete;
    Manager& operator=(const Manager&) = delete;
    Manager(Manager&&) = delete;
    Manager& operator=(Manager&&) = delete;
    virtual ~Manager() = default;

    /** @brief Constructor to put object onto bus at a dbus path.
     *         Defer signal registration (pass true for deferSignal to the
     *         base class) until after the properties are set.
     *  @param[in] bus - Bus to attach to.
     *  @param[in] path - Path to attach at.
     *  @param[in] idErr - The error entry id.
     *  @param[in] timestampErr - The commit timestamp.
     *  @param[in] severityErr - The severity of the error.
     *  @param[in] msgErr - The message of the error.
     *  @param[in] additionalDataErr - The error metadata.
     *  @param[in] objects - The list of associations.
     *  @param[in] fwVersion - The BMC code version.
     *  @param[in] parent - The error's parent.
     */
    Manager(sdbusplus::bus::bus& bus, const char* path) :
        DbusIface(bus, path){};

    virtual std::tuple<AttributeType, std::variant<int64_t, std::string>,
                       std::variant<int64_t, std::string>>
        getAttribute(std::string attributeName) override;

    virtual void setAttribute(
        std::string attributeName,
        std::variant<int64_t, std::string> attributeValue) override;
};

} // namespace bios_config
