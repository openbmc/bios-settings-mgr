#include "manager_serialize.hpp"

#include <cereal/archives/binary.hpp>
#include <cereal/cereal.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/tuple.hpp>
#include <cereal/types/variant.hpp>
#include <cereal/types/vector.hpp>
#include <phosphor-logging/lg2.hpp>

#include <fstream>

namespace bios_config
{

/** @brief Function required by Cereal to perform serialization.
 *
 *  @tparam Archive - Cereal archive type (binary in this case).
 *  @param[in] archive - reference to cereal archive.
 *  @param[in] entry- const reference to bios manager object
 *  @param[in] version - Class version that enables handling a serialized data
 *                       across code levels
 */
template <class Archive>
void save(Archive& archive, const Manager& entry,
          const std::uint32_t /*version*/)
{
    archive(entry.sdbusplus::xyz::openbmc_project::BIOSConfig::server::Manager::
                baseBIOSTable(),
            entry.sdbusplus::xyz::openbmc_project::BIOSConfig::server::Manager::
                pendingAttributes());
}

/** @brief Function required by Cereal to perform deserialization.
 *
 *  @tparam Archive - Cereal archive type (binary in our case).
 *  @param[in] archive - reference to cereal archive.
 *  @param[out] entry - reference to bios manager object
 *  @param[in] version - Class version that enables handling a serialized data
 *                       across code levels
 */
template <class Archive>
void load(Archive& archive, Manager& entry, const std::uint32_t /*version*/)
{
    Manager::BaseTable baseTable;
    Manager::PendingAttributes pendingAttrs;

    archive(baseTable, pendingAttrs);
    entry.sdbusplus::xyz::openbmc_project::BIOSConfig::server::Manager::
        baseBIOSTable(baseTable, true);
    entry.sdbusplus::xyz::openbmc_project::BIOSConfig::server::Manager::
        pendingAttributes(pendingAttrs, true);
}

void serialize(const Manager& obj, const fs::path& path)
{
    try
    {
        std::ofstream os(path, std::ios::out | std::ios::binary);

        if (!os.is_open())
        {
            lg2::error("Failed to open file for serialization: {FILE}", "FILE",
                       path);
            return;
        }

        cereal::BinaryOutputArchive oarchive(os);
        oarchive(obj);
    }
    catch (const std::exception& e)
    {
        lg2::error("Failed to Serialize : {ERROR} ", "ERROR", e);
    }
}

bool deserialize(const fs::path& path, Manager& entry)
{
    try
    {
        if (fs::exists(path))
        {
            std::ifstream is(path.c_str(), std::ios::in | std::ios::binary);
            if (!is.is_open())
            {
                lg2::error("Failed to open file for deserialization: {FILE}",
                           "FILE", path);
                return false;
            }
            cereal::BinaryInputArchive iarchive(is);
            iarchive(entry);
            return true;
        }
        return false;
    }
    catch (const std::exception& e)
    {
        lg2::error("Failed to serialize: {ERROR}", "ERROR", e);
        fs::remove(path);
        return false;
    }
}

} // namespace bios_config
