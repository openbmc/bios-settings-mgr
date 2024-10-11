# Remote BIOS Configuration

[![License](https://img.shields.io/badge/license-Apache%202.0-blue.svg)](LICENSE)

## Overview

The **biosconfig_manager** service enables users to view and modify the BIOS
setup configuration parameters remotely through the Baseboard Management
Controller (BMC) at any host state. Changes to these parameters will take effect
upon the next system reboot or immediately, depending on the host firmware.

For more details, please refer to [design document][rbmc-design-document].

## Features

- **Remote management** of BIOS settings.
- **Immediate updates** or scheduled changes upon reboot.
- **Reset BIOS Settings** support through the dbus.
- **ChangePassword** support to change the BIOS setup password.

## RBC Manager Interface

The Manager interface exposes methods and properties to Get & Set BIOS
attributes via dbus and its documented [here][pdi-manager-bios]

### Object Path

```txt
/xyz/openbmc_project/BIOSConfig/Manager
```

### Methods

- **SetAttribute** Sets a specific BIOS attribute to a new value.
- **GetAttribute** Retrieves the current and pending values of a BIOS attribute.

### Properties

- **ResetBIOSSettings** To reset the BIOS settings based on the Reset Flag.
- **BaseBIOSTable** Captures the entire BIOS table (collective information of
  all the BIOS attributes and their properties)

## Signature of `BaseBIOSTable`

The `BaseBIOSTable` property in the RBC Manager Interface is a complex
dictionary that defines the structure of BIOS attributes. Its type signature is
as follows:

```plaintext
dict[string, struct[
    enum[self.AttributeType],
    boolean,
    string,
    string,
    string,
    variant[int64, string],
    variant[int64, string],
    array[struct[enum[self.BoundType], variant[int64, string], string]]
]]
```

This structure consists of:

- **Attribute Name (string)**: The name of the BIOS attribute.
- **Attribute Type (enum)**: The type of the BIOS attribute (e.g., String,
  Integer).
- **Read-only Status (boolean)**: Whether the attribute is read-only.
- **Display Name (string)**: The human-readable name of the attribute.
- **Description (string)**: A description of what the attribute does.
- **Menu Path (string)**: The BIOS menu path where this attribute can be found.
- **Current Value (variant[int64, string])**: The current value of the attribute.
- **Default Value (variant[int64, string])**: The default value of the attribute.
- **Options (array of structs)**: The available options or bounds for this
  attribute.

### Examples

Here is an example json structure of a `String` attribute with `attributeName`
`DrdFreqLimit` & its various properties in BaseBIOSTable signature.

```json
{
  "DdrFreqLimit": {
    "attributeType": "xyz.openbmc_project.BIOSConfig.Manager.AttributeType.String",
    "readonlyStatus": false,
    "displayname": "Memory Operating Speed Selection",
    "description": "Force specific Memory Operating Speed or use Auto setting.",
    "menuPath": "Advanced/Memory Configuration/Memory Operating Speed Selection",
    "current": "0x00",
    "default": "0x0B",
    "options": [
      { "optionstring": "auto", "optionvalue": "enum0" },
      { "optionstring": "2133", "optionvalue": "enum1" },
      { "optionstring": "2400", "optionvalue": "enum2" },
      { "optionstring": "2664", "optionvalue": "enum3" },
      { "optionstring": "2933", "optionvalue": "enum4" }
    ]
  }
}
```

Here is another example json structure of a `Integer` attribute with attribute
with name `BIOSSerialDebugLevel` & its various properties in BaseBIOSTable
signature.

```json
{
  "BIOSSerialDebugLevel": {
    "attributeType": "xyz.openbmc_project.BIOSConfig.Manager.AttributeType.Integer",
    "readonlyStatus": false,
    "displayname": "BIOS Serial Debug level",
    "description": "BIOS Serial Debug level during system boot.",
    "menuPath": "Advanced/Debug Feature Selection",
    "current": 0x00,
    "default": 0x01,
    "options": [
      { "optionstring": "MinBound", "optionvalue": 0 },
      { "optionstring": "MaxBound", "optionvalue": 4 },
      { "optionstring": "ScalarIncrement", "optionvalue": 1 }
    ]
  }
}
```

## Initialization of `BaseBIOSTable`

When the `bios-settings-mgr` daemon starts, it initializes with an empty
`BaseBIOSTable`. It is the responsibility of provider daemons, such as **PLDM**
or **IPMI**, to populate this table by fetching or defining the BIOS settings.
These provider daemons are expected to gather the necessary BIOS attributes and
values from their respective sources (ex: bmc, system firmware) and then
initialize the `BaseBIOSTable` property with those settings.

### BIOS with PLDM as Communication Protocol

For systems that use the **PLDM (Platform Level Data Model)** protocol between
BMC & Host, OEM vendors can define their own BIOS attributes in the form of
[JSON files][pldm-bios-json]. The PLDM daemon parses these files and initializes
the `BaseBIOSTable` property accordingly. This allows for flexible and custom
BIOS configuration options based on the vendor's specifications.

For more details , refer to the [BIOS Support in PLDM][pldm-bios].

### BIOS with IPMI as Communication Protocol

For systems that use the **Intelligent Platform Management Interface** protocol
between BMC & Host, BIOS attributes are gathered from BIOS as an `xml file` &
`BaseBIOSTable` would then be initialized with the attributes data from the
parsed xml file.

For more details, refer to the code [BIOS Support in IPMI][ipmi-intel-bios].

## RBC Password Interface

### Object Path

```txt
xyz.openbmc_project.BIOSConfig.Password
```

### Methods

- **ChangePassword**  
  Used to change the BIOS setup password.

### Properties

- **PasswordInitialized** Used to indicate whether the BIOS password-related
  details have been received.

[rbmc-design-document]:
  https://github.com/openbmc/docs/blob/master/designs/remote-bios-configuration.md
[pldm-bios-json]:
  https://github.com/openbmc/pldm/blob/master/oem/ibm/configurations/bios/com.ibm.Hardware.Chassis.Model.Rainier2U/bios_attrs.json
[pldm-bios]: https://github.com/openbmc/pldm?tab=readme-ov-file#bios-support
[ipmi-intel-bios]:
  https://github.com/openbmc/intel-ipmi-oem/blob/master/src/biosconfigcommands.cpp
[pdi-manager-bios]:
  https://github.com/openbmc/phosphor-dbus-interfaces/blob/master/yaml/xyz/openbmc_project/BIOSConfig/Manager.interface.yaml
