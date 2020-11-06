#pragma once

#include "manager.hpp"

#include <filesystem>

namespace bios_config
{

/** @brief Serialize and persist the bios manager object
 *
 *  @param[in] obj - bios manager object
 *  @param[in] path - path to the file where the bios manager object
 *                    is to be serialized
 */
void serialize(const Manager& obj, const fs::path& path);

/** @brief Deserialize the persisted data and populate the bios manager object
 *
 *  @param[in] path - path to the persisted file
 *  @param[in/out] entry - reference to the bios manager object which is the
 *                         target of deserialization.
 *
 *  @return bool - true if the deserialization was successful, false otherwise.
 */
bool deserialize(const fs::path& path, Manager& entry);

} // namespace bios_config