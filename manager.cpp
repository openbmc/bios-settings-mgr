#include "manager.hpp"

#include "xyz/openbmc_project/BIOSConfig/Common/error.hpp"
#include "xyz/openbmc_project/Common/error.hpp"

using namespace sdbusplus::xyz::openbmc_project::Common::Error;
using namespace sdbusplus::xyz::openbmc_project::BIOSConfig::Common::Error;

namespace bios_config
{

std::tuple<Manager::AttributeType, std::variant<int64_t, std::string>,
           std::variant<int64_t, std::string>>
    Manager::getAttribute(std::string attributeName)
{
    auto biosTable = baseBIOSTable();
    auto itr = biosTable.find(attributeName);
    if (itr != biosTable.end())
    {
        const auto& current = std::get<5>(itr->second);
        const auto& pending =
            std::get<1>(pendingAttributes().find(attributeName)->second);
        return std::make_tuple(std::get<0>(itr->second), current, pending);
    }

    throw AttributeNotFound();
    return {};
}

void
    Manager::setAttribute(std::string /*attributeName*/,
                          std::variant<int64_t, std::string> /*attributeValue*/)
{
    throw InternalFailure();
}

} // namespace bios_config
