# Changelog

Questo changelog riassume le modifiche del firmware. Le voci storiche sono state estratte dal changelog presente in `src/main.c`.

## Unreleased
- Centralizzate le macro di build in `src/common/build_config.h`.
- Ripristinato `TempRif` su `T_supply` come fallback quando non sono disponibili AWP/accessori (logica in `src/Clima_Func.c`).

## 2.68 - 2026-01-26
- Added PIR strategy to RD firmware (motor_speed.c).

## 2.67 - 2025-12-18
- Fixed issue for CAP function on file motor_speed.c.

## 2.66 - 2025-12-11
- Merged version 2.65 with version of Nicola 2C65.
- Added AC/DC voltage measurement for board version 3.3 (+12V and +5V).
- Fixed issue for CAP function on file motor_speed.c.

## 2.65 - 2025-11-12
- Added WINTER/SUMMER enabling using Input 2 (motor_speed.c line 777).
- Added a second temperature setpoint for WINTER (SetPointTemp_Heater), now we have 2 for both SUMMER and WINTER.

## 2.64 - 2025-10-17
- Fixed Temperature Probe ALARM (ADC.c line 129).

## 2.63 - 2025-10-13
- Added modification for PIR strategy and weekly program (motor_speed.c line 773).

## 2.62 - 2025-09-28
- Added I2C bus recovery (9 pulses) on function init.c (BLACK SCREEN).

## 2.61 - 2025-09-21
- Added delay for PIR to be more stable when power ON the BLACK BOX (72000 ms).

## 2.60 - 2025-07-29
- Fixed bugs for Heater accessory command.
- Added voltage and current value for heater on Debug info.

## 2.59 - 2025-07-22
- Retro-compatibility - improved adding and removing accessories.

## 2.58 - 2025-07-19
- Fixed retro-compatibility problem related to EHD07 HWD/CWD accessory detection.

## 2.57 - 2025-05-18
- Added Reset Eeprom from KTS.
- Increased number of bytes received from accessory to 15.

## 2.56 - 2025-05-11
- Added damperless bypass (imbalance) in case of I0 configuration (PRIME CASE).
- I0 => Serialnumber with 90 in position (10,11) like XXXXYYYYZ90YYWWPPP.

## 2.55 - 2025-04-30
- Added version firmware for EHD08, EHD07 and BPD on debug data.

## 2.54 - 2025-04-16
- Merged files from version 2.52 and 2.53.

## 2.53 - 2025-04-14
- Change CO2 strategy from 600 ppm with ramp to max 220/260 ppm.
- Minimum CO2 airflow setting (boost) 30% (before was 40%).
- Added NOTACHO macro to compile for fans without TACHO signal.

## 2.52 - 2025-03-29
- Changed UART0 and UART2 transfer method from interrupt to DMA.
- Added KTS communication checking (error 10-7).

## 2.51 - 2025-03-13
- Added Clean Event strategy for SENSOR board.

## 2.50 - 2025-02-27
- AIR sensor data management added.

## 2.49 - 2025-02-15
- AIR sensor restart after 24 hours with value not changing added.

## 2.47 - 2024-12-10
- CAPS and CAPR management strategy added.

## 2.35 - 2022-02-16
- PIR strategy control added.

## 2.34 - 2021-11-27
- DSC bug fixed.

## 2.33 - 2021-06-04
- Introduced new I2C accessory: DSC.

## 2.32 - 2021-04-10
- Fixed EBPD/EBP2 sync bug at startup (initialize info_EBPD out of range).

## 2.31 - 2020-11-05
- Added I2C accessory EBP2 for double bypass motor (BPD01).

## 2.30 - 2020-08-22
- Fixed EHD status bug where snowflake kept blinking due to SSR command interpretation.

## 2.29 - 2020-06-30
- Fixed BPD behavior when EBPD absent.

## 2.28 - 2019-10-10
- Added events to output (SUM/WIN and MAX SPEED).

## 2.27 - 2019-xx-xx
- Branch: custom software for PSI in CH for ECS.

## 2.26 - 2019-08-01
- Fixed preheater behavior for external accessory (water/electric handling).

## 2.25 - 2019-01-02
- Added new I2C accessories: FLW1, FLW2, EBPD, SSR (Modbus managed).
- Added IPEHD test button on touchscreen.
- Added CWD air temperature in KTS protocol.

## 2.24 - 2018-07-14
- Modbus can command motors independently (MBF enable + setpoints).
- Added I2C message for accessories with 4 sensors EXT1..EXT4.

## 2.23 - 2018-01-25
- Imbalance active during external 0-10V control.

## 2.22 - 2017-11-14
- Filter pressure switch handling.
- Added verification/calibration in Filter_Func.
- DPS/DPP management with KTS.
- Modified filter reset for DPP/DPS.
- Modified accessory shell to show DPP presence.

## 2.21 - 2017-04-04
- Added TIMER_KTS_KO in HW_func; restart KTS after 40 cycles.
- Min threshold set to 25% (~2.5V).

## 2.20 - 2016-02-16
- CO2/VOC/RH probes managed also in CAF.
- Fire alarm manual reset from KTS.
- Automatic freecooling even when unit is off.
- CAF/CAP Serie 8.

## 2.19 - 2015-11-19
- CAF function for units 0073/0123.
- Increased controls to clear BDP alarm in Manual Closed.

## 2.18 - 2015-11-05
- Fixed BOOST (PARTY) counter reset bug.

## 2.17 - 2015-10-21
- Dual-function climate accessories control.
- Summer/Winter selection and probe reference selection.

## 2.16 - 2015-08-03
- Added DXD climate accessory (compressor hot/cold).

## 2.15 - 2015-07-15
- Added FranceAir FIRE management requests.

## 2.14 - 2015-07-10
- Fixed weekly program write bug.

## 2.13 - 2015-06-26
- Changed motor mapping tables for CAP/CAF.
- Modified CAF function.

## 2.12 - 2015-04-16
- Modified Modbus packets for RFM management (Protocol_Ser2.h).
- If Bypass Manual Closed, do not signal alarm.

## 2.11 - 2015-03-06
- Fixed output bug: no difference between drop/attraction.
- Added digital CAP.

## 2.10 - 2015-02-24
- Improved Seriale_2 driver with 10 samples on bit (noise).
- Delay for temperature probe KO alarms (~30 sec).
- New motor fault shutdown strategy with EEPROM off state.
- Added motor status buffer before changing StatusMotorsR/F.
- Reduced bypass fault thresholds (new motors).
- Added speed control when PEHD is ON (start 40% and ramp).

## 2.09 - 2015-01-28
- Added unit test/commissioning functions.
- Improved CAF.

## 2.08 - 2014-09-23
- Removed analog RH/CO2/VOC sensors.
- Added ModBus_KO alarm.
- Modified climate function for Heater accessory.

## 2.07 - 2014-09-16
- Added unit configuration via 54-byte string in shell.
- Startup checks SN; if empty, block unit and beep.

## 2.06 - 2014-09-10
- Configurable bypass motor rotation.
- Configurable temperature probe positions.

## 2.05 - 2014-08-30
- Fixed climate function: do not keep heater on if motor fault.
- Improved CAF.

## 2.04 - 2014-08-23
- CAP/CAF modifications.
- Climate accessory changes (byte info/size).

## 2.03 - 2014-07-24
- Modbus changes.
- Improved CAP.

## 2.02 - 2014-07-09
- Fixed WeeklyProg task error.
- Added CAP function.
- Fixed CO2, VOC, RH sensor defects.

## 2.01 - 2013-12-18
- HW MainBoard 4.0 change and complete SW rewrite for unit control.
