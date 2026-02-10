#ifndef BUILD_CONFIG_H
#define BUILD_CONFIG_H

// Project-wide build switches.
// Keep these centralized to avoid duplicate/conflicting defines.

// Enable EXTRA feature set (vs BASIC).
#define TYPE_FUNC_UNIT_EXTRA

// Enable watchdog refresh in runtime modules.
#define WORK_WDT

// Enable custom mapping/tables for PCAF with PCAP sensor.
#define COMESSA_2388

// Disable bypass alarm (guarded by #ifndef in HW_func.c).
#define _DISABLE_ALARM_BYPASS_

// Optional: embed a fixed serial number in default EEPROM.
// Uncomment to enable.
// #define _EEP_SERIAL_NUMBER

#endif // BUILD_CONFIG_H
