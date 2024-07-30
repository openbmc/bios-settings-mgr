# bios-settings-mgr

Remote BIOS Configuration via BMC Overview Provides ability for the user to view
and modify the BIOS setup configuration parameters remotely via BMC at any Host
state. Modifications to the parameters take place upon the next system reboot or
immediate based on the host firmware. Please refer
https://github.com/openbmc/docs/blob/master/designs/remote-bios-configuration.md

Remote BIOS Configuration (RBC) service exposes D-Bus methods for BIOS settings
management operations.

RBC Manager Interface xyz.openbmc_project.BIOSConfig.Manager provides following
methods, properties.

Object Path : /xyz/openbmc_project/BIOSConfig/Manager

xyz.openbmc_project.BIOSConfig.Manager

methods: SetAttribute -To set the particular BIOS attribute with new value.
GetAttribute -To get the bios attribute current values and pending values if
again.

Properties: ResetBIOSSettings - Contain reset BIOS setting type: Interface have
to set NoAction this property, when Reset BIOS settings are informed to the
BIOS. BaseBIOSTable - Save the whole BIOS table.
map{attributeName,struct{attributeType,readonlyStatus,displayname,
description,menuPath,current,default, array{struct{optionstring,optionvalue}}}}
Example 1: {"DdrFreqLimit",
{xyz.openbmc_project.BIOSConfig.Manager.AttributeType.String, false, "Memory
Operating Speed Selection", "Force specific Memory Operating Speed or use Auto
setting.", "Advanced/Memory Configuration/Memory Operating Speed Selection",
"0x00", "0x0B", { {"OneOf", "auto"}, {"OneOf", "2133"}, {"OneOf", "2400"},
{"OneOf", "2664"}, {"OneOf", "2933"} } } } Example 2: {"BIOSSerialDebugLevel",
{xyz.openbmc_project.BIOSConfig.Manager.AttributeType.Integer, false, "BIOS
Serial Debug level", "BIOS Serial Debug level during system boot.",
"Advanced/Debug Feature Selection", 0x00, 0x01, { {"MinBound", 0}, {"MaxBound",
4}, {"ScalarIncrement",1} } } }

Signals: AttributeChanged - Signal sent out when attribute is changed

PasswordInterface:

xyz.openbmc_project.BIOSConfig.Password provides following Methods and
Properties.

xyz.openbmc_project.BIOSConfig.Password Interface

Methods: ChangePassword - Change the BIOS setup password.

Properties: PasswordInitialized - To indicate BIOS password related details are
received or not.
