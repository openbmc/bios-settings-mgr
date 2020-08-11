#include "manager.hpp"

#include <sdbusplus/bus.hpp>
#include <sdbusplus/server/manager.hpp>

int main()
{
    auto bus = sdbusplus::bus::new_default();
    auto rootPath = "/xyz/openbmc_project/bios_config/manager";
    sdbusplus::server::manager::manager objManager(bus, rootPath);
    bios_config::Manager mgr(bus, rootPath);
    bus.request_name("xyz.openbmc_project.BIOSConfigManager");

    while (true)
    {
        bus.process_discard();
        bus.wait();
    }
    return 0;
}
