# RD_ATSAM4N_V1.0 (ATSAM4N16B)

Firmware per unita di ventilazione/recupero aria su MCU Microchip ATSAM4N16B, basata su ASF. Il progetto gestisce motori di ventilazione, bypass, accessori clima/sonde via I2C, protocolli seriali verso KTS/CTRL_FAN/Modbus, logging allarmi, EEPROM configurabile e funzioni di collaudo.

Versione firmware riportata nel codice: `2.68` (vedi `src/main.c`).

**Piattaforma**
- MCU: `ATSAM4N16B` (definita in `RD_ATSAM4N_V1.0.cproj`).
- Framework: ASF (Atmel Software Framework) con configurazioni in `src/config/`.
- Seriali: `UART0` 9600 bps (KTS) e `UART2` 38400 bps (CTRL_FAN + Modbus), configurate in `src/config/conf_uart_serial.h`.
- I2C: `TWI0` (accessori e sensori I2C).
- ADC: ADC interno + ADC esterni (ADC121C027 e ADS7142).

**Struttura di alto livello**
- `src/main.c`:
  - Inizializzazione clock, UART, I2C, ADC, RTC e PDC (DMA UART).
  - Loop principale: lettura analogica, scheduler, processing messaggi, watchdog e gestione battery-backup.
  - Strutture globali RAM (sData), timer, contatori e stati unita.
- `src/Scheduler.c`:
  - Mini scheduler a tick 100ms con funzioni periodiche (full-time o single-shot).
- `src/data.h`:
  - Struttura dati in RAM `SDATA` con misure, stati, allarmi, accessori, velocita motori, ecc.
- `src/eeprom_map.h`, `src/eeprom_func.c`:
  - Mappa EEPROM (240 byte), default, shadow, contatori di aggiornamento, serial number, setpoint e programmazione settimanale.

## Funzionalita (dettaglio)

**1) Controllo motori e portate (`src/motor_speed.c`)**
- Gestione velocita per ramo Fresh/Return con:
  - CFS/CAF (Constant Air Flow).
  - CAP (Constant Air Pressure).
  - MBF (Modbus Fan control) con setpoint separati per F e R.
  - Boost/Party temporizzato.
  - Imbalance (sbilanciamento rami).
  - Defrost e Post-ventilazione con logica di sicurezza.
- Tabelle di mappatura tensione/RPM per diverse taglie e configurazioni (incl. custom `COMESSA_2388`).
- Soglie minime di avvio, gestione reset/restart dei motori e fault handling.
- Calcolo portata e pressione da sensori (CAPS/CAPR/PCAP/PCAF).
- Strategie qualita aria (CO2/RH/VOC) con incremento velocita.
- Modalita "damperless bypass" basata su serial number (case PRIME) per imbalance senza bypass.

**2) Clima, bypass e defrost (`src/Clima_Func.c`, `src/HW_func.c`)**
- Bypass automatico/manuale/esterno con logiche:
  - Auto con soglie di temperatura e isteresi.
  - Manuale aperto/chiuso.
  - Controllo da ingressi digitali.
  - Auto ON/OFF con freecooling attivo anche a unita spenta.
- Comando bypass stepper con controllo assorbimenti e finecorsa.
- Defrost con sbilanciamento rami e condizioni di attivazione/uscita.
- Gestione accessori clima:
  - Preheater/Heater/Cooler (elettrico o acqua).
  - DXD (compressore) con 0-10V modulante e selezione Winter/Summer.
- Funzioni speciali:
  - Freecooling automatico.
  - Setpoint estivi/invernali separati.
  - Selezione sonda di riferimento (Return/Supply).

**3) Sensori analogici e I2C (`src/ADC.c`, `src/Accessory.c`)**
- Lettura e media di:
  - NTC temperature (2?4 sonde, con configurazione posizione).
  - Ingressi analogici 0?10V (IN1/IN2).
  - Assorbimento motore bypass.
  - Corrente KTS.
- ADC esterni:
  - `ADC121C027` e `ADS7142` per misure addizionali (tensioni/backup).
- Accessori I2C gestiti con diagnostica link, stato operativo e misure multiple:
  - Preheater, Heater (EHD/HWD), Cooler (CWD).
  - DXD (compressore).
  - Sonde CO2/RH/VOC (fino a 2 canali).
  - Sonde pressione PCAP/PCAF/DPP/DPS.
  - AWP (temperatura aria ambiente).
  - EXT1..EXT4 (multisensori).
  - FLW1/FLW2 (flussimetri).
  - EBPD/EBP2 (bypass modulante), SSR.
  - DSC e sensori AIR avanzati.
- Gestione qualita aria:
  - CO2/VOC/RH massimi, strategie di incremento portata.
  - Riavvio sensori AIR dopo 24h se valore invariato.
  - Clean Event strategy (sensori).
- Calcolo portata da flussimetri e pressione media.

**4) Filtri e manutenzione (`src/Filter_Func.c`)**
- Contatore ore funzionamento con persistenza EEPROM.
- Gestione filtro sporco in due modalita:
  - DPS (timer ore/giorni).
  - DPP (pressostato con calibrazione e verifica periodica).
- Reset filtri e calibrazione via KTS.
- Allarmi filtro con soglie e storicizzazione.

**5) Programmazione settimanale (`src/Weekly_Prog.c`)**
- Scheduler su base oraria e fasce da 30 minuti.
- Fino a 4 range al giorno con:
  - Selezione velocita (3 step).
  - Selezione setpoint temperatura (2 riferimenti).
  - Abilitazione sbilanciamento.
- Pre-apertura valvola preheater prima dello start (se presente).

**6) Allarmi ed eventi (`src/alarm.h`, `src/Alarm.c`)**
- Mappa eventi (14 byte) con diagnostica accessori/sensori/motori.
- Allarmi principali:
  - Sonde temp KO, CO2/RH/VOC link/KO, pressione, filtri.
  - Motori no power/no tacho.
  - Bypass fault, batteria, KTS fault.
  - Allarmi incendio con modalita ventilazione dedicate.
- Gestione inserimento/ritardo/rimozione allarmi.

**7) Comunicazioni seriali e protocolli (`src/Protocol_Ser1.h`, `src/Protocol_Ser2.h`, `src/Seriale_1.c`, `src/Seriale_2.c`, `src/Serial1_Class.c`, `src/Serial2_Class.c`, `src/Admin_Messages.c`)**
- Seriale 1 (UART0, 9600): comunicazione KTS <-> RD
  - Comandi test, lettura debug, lettura/scrittura EEPROM.
  - Gestione timeout KTS e watchdog.
- Seriale 2 (UART2, 38400): RD <-> CTRL_FAN + Modbus
  - Polling motori e feedback RPM/tensioni.
  - Packet1/Packet2 con stato unita, allarmi, misure e accessori.
- PDC (DMA) per RX su UART0 e UART2.
- CRC/CKSUM16 sui messaggi.

**8) EEPROM e configurazione (`src/eeprom_map.h`, `src/eeprom_func.c`, `src/shell.c`)**
- Mappa completa di configurazione con:
  - Info unita (serial, versioni, tipo unita, accessori).
  - Abilitazione funzioni (CAF/CAP/Boost/Defrost/Weekly/DPP/MBF...).
  - Parametri motori (numero, impulsi, derating, rotazione).
  - Setpoint qualita aria, temperature, bypass, input/output.
  - Programmazione settimanale.
- Shadow EEPROM in RAM e contatori di allineamento.
- Default EEPROM con profilo base e validazione.
- Menu di configurazione via shell seriale:
  - Scrittura default EEPROM.
  - Lettura configurazione.
  - Scrittura serial number.
  - Setup motori e bypass.
  - Setup NTC.
  - Testing manuale.
  - Diagnostica automatica.
  - Visualizzazione weekly program.
  - Configurazione unita via stringa lunga (54 byte).

**9) Test e collaudo (`src/Testing_HW.c`)**
- Test Bypass con verifica assorbimenti min/max.
- Test IO (rele e ingressi).
- Test Sleep/Idle.
- Test motori con step di velocita.
- Test UART1 e I2C.
- Test IPEHD dedicato.

**10) Power, watchdog e reset**
- Watchdog attivo con refresh periodico (`WORK_WDT`).
- Gestione power relay motori (PowerMode).
- Rilevazione cause reset e log in flash.
- Battery backup detection (ADS7142) e reset controllato.

## Accessori e indirizzi I2C
Elenco indirizzi e tipologie (da `src/MASTER_I2C.h`):
- `ADDR_PREHEATER` (2): preheater.
- `ADDR_HEATER` (10): heater EHD/HWD.
- `ADDR_COOLER` (20): cooler CWD.
- `ADDR_DXD` (25): compressore DXD.
- `ADDR_PROBE1_CO2` (30), `ADDR_PROBE2_CO2` (31).
- `ADDR_PROBE1_RH` (34), `ADDR_PROBE2_RH` (35).
- `ADDR_PROBE1_VOC` (38).
- `ADDR_PROBE_CAP` (40), `ADDR_PROBE_CAF` (41), `ADDR_PROBE_DSP` (42).
- `ADDR_AWP` (50): sonda temperatura aria.
- `ADDR_EXT1..EXT4` (51..54): multisensori.
- `ADDR_FLW1` (60), `ADDR_FLW2` (61).
- `ADDR_SSR` (62), `ADDR_EBPD` (63), `ADDR_EBP2` (64).
- `ADDR_DSC` (65).
- `ADDR_DPSEN` (99): sensore pressione DPP/DPS.

## Stati principali (bitfield)
- `status_unit` (in `src/data.h`):
  - Bit RUN, DEFROST, POST_VENT, IMBALANCE, BOOST.
  - Stato bypass (run/close/open).
  - Flags qualita aria max, test, DPP check, PIR.
- `status_weekly`: enable/run, riferimento temp, imbalance, speed step.
- `status_test`: enable test e singoli test (bypass, output, buzzer, IPEHD, LED).

## Organizzazione dei file principali
- `src/main.c`: bootstrap + loop principale.
- `src/Accessory.c`: gestione accessori I2C, diagnostica link, letture/scritture, clean-event.
- `src/Clima_Func.c`: logica clima/bypass/defrost/freecooling.
- `src/motor_speed.c`: regolazione motori e modalita CAP/CAF/MBF.
- `src/ADC.c`: acquisizione analogica e conversioni.
- `src/Filter_Func.c`: manutenzione filtri (DPS/DPP) e ore funzionamento.
- `src/Weekly_Prog.c`: programmazione settimanale.
- `src/HW_func.c`: buzzer, bypass stepper, power relays, watchdog.
- `src/Admin_Messages.c`: parsing/comandi seriali e integrazione protocolli.
- `src/Seriale_1.c`, `src/Seriale_2.c`: framing/cksum protocolli.
- `src/Serial1_Class.c`, `src/Serial2_Class.c`: driver UART con PDC.
- `src/eeprom_map.h`, `src/eeprom_func.c`: mappa e gestione EEPROM.
- `src/alarm.h`, `src/Alarm.c`: allarmi/eventi.
- `src/shell.c`, `src/Testing_HW.c`: shell e test hardware.
- `src/config/*`: configurazioni board/clock/uart.

## Build
Progetto Atmel Studio/ASF (`RD_ATSAM4N_V1.0.cproj`) con target `ATSAM4N16B`. Compilazione tramite toolchain ARM GCC configurata nel progetto. Il flashing e previsto via J-Link (config presente in `jlink.config`).

## Note operative
- Le funzioni abilitate dipendono da `Enab_Fuction` in EEPROM e dalle maschere di abilitazione.
- La configurazione hardware e la presenza accessori sono memorizzate in `AccessoyHW[]` in EEPROM.
- Diverse strategie e compatibilita retroattive sono documentate nel changelog storico in `src/main.c`.
