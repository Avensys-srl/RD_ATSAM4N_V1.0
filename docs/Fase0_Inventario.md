# Fase 0 - Allineamento e Inventario

Obiettivo: creare una base condivisa di conoscenza e criteri di successo prima di qualsiasi refactor.

## 0.1 Obiettivi non funzionali (NFR)
Definire e confermare i target misurabili:
- [ ] Riduzione accoppiamento: eliminare accessi diretti a globali in moduli critici.
- [ ] Testabilita: rendere testabili protocolli e logiche senza hardware (mock layer).
- [ ] Manutenibilita: ridurre file "monolitici" e separare responsabilita per dominio.
- [ ] Stabilita: nessuna regressione sui protocolli KTS/CTRL_FAN/Modbus.
- [ ] Build/flash: nessun peggioramento dei tempi di build o uso RAM/Flash.

## 0.2 Ambito hardware/firmware
Allineare piattaforma e vincoli:
- MCU: ATSAM4N16B
- Toolchain: Atmel Studio + ASF
- UART0: 9600 (KTS)
- UART2: 38400 (CTRL_FAN + Modbus)
- I2C: TWI0 accessori
- ADC: interno + ADC121C027 + ADS7142

## 0.3 Varianti e macro di build (mappatura reale)
Macro rilevate nel codice e uso principale:
- `COMESSA_2388`
  - Definita in `src/main.c` e `src/motor_speed.c`.
  - Attiva tabelle custom per PCAF con sensore PCAP.
  - Riferimenti: `src/main.c`, `src/motor_speed.c`.
- `_DISABLE_ALARM_BYPASS_`
  - Definita in `src/main.c` e `src/HW_func.c`.
  - Disabilita l'allarme bypass (usata in guardie `#ifndef` in `HW_func.c`).
  - Riferimenti: `src/main.c`, `src/HW_func.c`.
- `_EEP_SERIAL_NUMBER`
  - Definita (commentata) in `src/main.c`, usata in `src/eeprom_func.c`.
  - Se attiva, inserisce un serial number custom nei default EEPROM.
  - Riferimenti: `src/main.c`, `src/eeprom_func.c`.
- `WORK_WDT`
  - Definita in `src/main.c` e `src/ADC.c`.
  - Attiva il refresh del watchdog in piu' moduli.
  - Riferimenti: `src/main.c`, `src/ADC.c`, `src/HW_func.c`, `src/Admin_Messages.c`, `src/Accessory.c`, `src/Testing_HW.c`.
- `TYPE_FUNC_UNIT_EXTRA`
  - Definita in `src/main.c`.
  - Imposta l'unita con funzionalita EXTRA (controllo abilitazioni).
  - Riferimenti: `src/main.c`.
- `NOTACHO`
  - Citata nel changelog (possibile macro per fan senza TACHO).
  - Non trovata nel codice attuale (potenziale legacy).
  - Riferimento: commento in `src/main.c`.

Azioni (da completare):
- [ ] Decidere quale macro deve diventare configurazione runtime in EEPROM.
- [ ] Consolidare le macro in un unico file `build_config.h`.
- [ ] Eliminare duplicazioni di definizione (es. `COMESSA_2388`, `_DISABLE_ALARM_BYPASS_` in piu' file).

## 0.4 Mappa dipendenze ad alto livello (estratta dal progetto)
Elenco "chi chiama chi" principale:
- Core
  - `src/main.c` -> `read_analogInput`, `process_scheduler`, `processes_messages`, `motor_speed`, `TemperatureRegulate`, `pollingDigitAccessory`, `weekly_program`, `operatingHoursCounter`.
  - `src/Scheduler.c` gestisce il tick e richiama funzioni registrate.
- Drivers
  - UART: `src/Serial1_Class.c`, `src/Serial2_Class.c`, `src/Seriale_1.c`, `src/Seriale_2.c`.
  - I2C: `src/MASTER_I2C.h` + implementazioni in `src/Accessory.c`.
  - ADC: `src/ADC.c`, ADC esterni in `src/main.c`.
  - GPIO/Timer/RTC: `src/HW_func.c`, `src/main.c`.
- Features
  - Motori: `src/motor_speed.c`.
  - Clima/Bypass/Defrost: `src/Clima_Func.c`, `src/HW_func.c`.
  - Filtri: `src/Filter_Func.c`.
  - Weekly: `src/Weekly_Prog.c`.
  - Allarmi: `src/alarm.h`, `src/Alarm.c`.
  - Test/Collaudo: `src/Testing_HW.c`, `src/shell.c`.
- Protocols
  - KTS: `src/Protocol_Ser1.h`, parsing in `src/Admin_Messages.c` + framing in `src/Seriale_1.c`.
  - CTRL_FAN + Modbus: `src/Protocol_Ser2.h`, parsing in `src/Admin_Messages.c` + framing in `src/Seriale_2.c`.
- Storage
  - EEPROM: `src/eeprom_map.h`, `src/eeprom_func.c`.

Note:
- `src/Admin_Messages.c` e `src/main.c` sono hub ad alto accoppiamento.
- Possibile dipendenza circolare logica tra motori, clima e accessori tramite `sData`.

Azioni (da completare):
- [ ] Disegnare un diagramma minimale con flusso principale e moduli.
- [ ] Identificare dipendenze circolari concrete per refactor mirato.

## 0.5 Inventario funzionalita critiche (con file)
Funzioni critiche e riferimenti:
- Controllo motori CAP/CAF/MBF: `src/motor_speed.c`.
- Bypass automatico/manuale + stepper: `src/Clima_Func.c`, `src/HW_func.c`.
- Defrost + post-vent: `src/motor_speed.c`, `src/Clima_Func.c`.
- Gestione accessori I2C + link status: `src/Accessory.c`, `src/MASTER_I2C.h`.
- Protocolli KTS/CTRL_FAN/Modbus:
  - KTS: `src/Protocol_Ser1.h`, `src/Seriale_1.c`, `src/Admin_Messages.c`.
  - CTRL_FAN/Modbus: `src/Protocol_Ser2.h`, `src/Seriale_2.c`, `src/Admin_Messages.c`.
- Allarmi + eventi: `src/alarm.h`, `src/Alarm.c`.
- EEPROM map + contatori update: `src/eeprom_map.h`, `src/eeprom_func.c`.
- Weekly program: `src/Weekly_Prog.c`.
- Filtri DPS/DPP: `src/Filter_Func.c`.
- Shell/Testing: `src/shell.c`, `src/Testing_HW.c`.

Azioni (da completare):
- [ ] Validare che ogni funzione critica abbia test manuale in `checklist.md`.
- [ ] Identificare eventuali funzioni critiche mancanti (es. battery backup, reset cause).

## 0.6 Criteri di regressione
Definire cosa significa "ok":
- [ ] Nessun allarme nuovo a parita di HW.
- [ ] Messaggi seriali invariati (layout byte-by-byte).
- [ ] Tempi ciclo loop entro tolleranza.
- [ ] Nessuna differenza nei setpoint applicati.

## 0.7 Artefatti da produrre in Fase 0
- [ ] Documento inventario moduli e responsabilita.
- [ ] Tabella macro di build con significato.
- [ ] Mappa dipendenze (lista o diagramma).
- [ ] Lista "funzioni critiche" con file di riferimento.

---

## Output generati
- `docs/Fase0_Inventario.md` (questo file)
