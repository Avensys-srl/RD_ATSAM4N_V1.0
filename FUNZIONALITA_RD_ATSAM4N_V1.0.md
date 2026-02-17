# RD_ATSAM4N_V1.0 - Funzionalita Complete

Questo documento racconta in modo organico le funzionalita del firmware `RD_ATSAM4N_V1.0` per MCU `ATSAM4N16B`.

Nota versione:
- Nome progetto: `RD_ATSAM4N_V1.0`
- Changelog storico: ultima release indicata `2.68` (26/01/2026)
- Codice `src/main.c`: costante firmware impostata a `2.69` (stato piu recente nel sorgente)

## 1) Gestione unita di ventilazione (core macchina)

Il firmware governa una unita di ventilazione/recupero aria completa:
- Avvio, arresto e stati macchina (`RUN`, `DEFROST`, `POST_VENT`, `BOOST`, `IMBALANCE`).
- Ciclo principale con scheduler periodico a tick 100 ms.
- Controlli di sicurezza e fallback operativi in caso di guasti.
- Supervisione del power mode e gestione relay principali.

## 2) Controllo motori ventilatori (Fresh/Return)

La regolazione motori include diverse strategie:
- `CAF` (Constant Air Flow): mantenimento portata costante.
- `CAP` (Constant Air Pressure): mantenimento pressione costante.
- `MBF` (Modbus Fan): comando indipendente dei due motori via Modbus.
- `BOOST/PARTY`: sovra-velocita temporizzata.
- `IMBALANCE`: sbilanciamento controllato tra i due rami.
- Gestione soglie minime, fault motore, restart e modalita compatibili con ventole senza tacho (`NOTACHO`).

Sono presenti tabelle di mappatura e logiche per diverse taglie/unita, incluse varianti custom.

## 3) Bypass aria, freecooling e modalita stagionali

La logica bypass supporta:
- Manuale aperto/chiuso.
- Automatico su condizioni termiche (fresh/return/setpoint/isteresi).
- Controllo esterno da ingresso digitale/analogico configurato.
- Modalita auto on/off con freecooling (anche in condizioni particolari a unita spenta).

Sono gestite:
- Apertura/chiusura con persistenze anti-oscillazione.
- Comando bypass stepper con diagnostica.
- Caso "damperless bypass" (sbilanciamento senza bypass fisico) per configurazioni dedicate.

## 4) Climatizzazione e accessori termici

La regolazione clima integra:
- `Preheater` (elettrico/acqua) con logiche antigelo.
- `Heater` e `Cooler`.
- `DXD` (compressore caldo/freddo) con modulazione progressiva 0-10V.
- Gestione estate/inverno e doppio setpoint stagionale.
- Selezione sonda riferimento (`Return` o `Supply`) con fallback automatici.

Le logiche agiscono con isteresi, persistenze e priorita energetiche (es. preferenza bypass prima del raffreddamento attivo quando conveniente).

## 5) Sensori e acquisizioni (analogico + I2C)

Il firmware acquisisce e normalizza:
- Sonde temperatura (NTC e sonde accessorio).
- Ingressi analogici 0-10V.
- Misure di corrente/tensione utili anche a diagnostica.
- ADC interni ed esterni (`ADC121C027`, `ADS7142`).

Gestisce una rete accessori I2C estesa, tra cui:
- Sonde `CO2`, `RH`, `VOC` (anche multiple).
- Sonde pressione (`CAP`, `CAF`, `DPP/DPS`).
- Accessori `EXT1..EXT4`, `FLW1/FLW2`, `AWP`, `DSC`, `EBPD/EBP2`, `SSR`.

Sono presenti strategie per robustezza sensori (es. restart sensori AIR dopo lunghi periodi con valore bloccato, clean event strategy).

## 6) Qualita aria e strategie automatiche

In base alle misure aria interna/esterna:
- Incremento automatico velocita su soglie CO2/VOC/RH.
- Gestione massimi e limiti minimi di portata in funzione della qualita aria.
- Integrazione della strategia `PIR` nelle logiche velocita/weekly nelle versioni recenti.

## 7) Filtri e manutenzione

Sono implementate funzioni service:
- Contatore ore macchina con persistenza EEPROM.
- Gestione filtro sporco tramite:
- DPS (approccio a tempo/ore-giorni).
- DPP (pressostato differenziale con verifica/calibrazione).
- Reset filtri da HMI/protocollo e storicizzazione eventi/allarmi.

## 8) Programmazione settimanale (Weekly Program)

La pianificazione include:
- Fasce giornaliere a slot da 30 minuti.
- Fino a 4 intervalli al giorno.
- Scelta step velocita, setpoint temperatura e abilitazione imbalance per fascia.
- Interazione con funzioni clima (es. preapertura valvola preheater in scenari previsti).

## 9) Allarmi, eventi e protezioni

Il sistema mantiene una mappa eventi/allarmi con gestione temporale (inserimento/ritardo/rimozione), incluse categorie:
- Guasti sonde temperatura/qualita aria/pressione.
- Guasti comunicazione accessori.
- Fault motori (power/tacho).
- Fault bypass e filtri.
- Allarmi incendio con comportamenti dedicati.
- Fault comunicazione KTS/Modbus.

## 10) Comunicazioni e protocolli seriali

Due canali principali:
- `UART0` (KTS): comandi, diagnostica, EEPROM read/write, test.
- `UART2` (CTRL_FAN + Modbus): scambio stato macchina, comandi motori, feedback.

Caratteristiche:
- Driver seriali dedicati con framing e checksum/CRC.
- Uso `PDC` (DMA) per RX seriale nelle versioni evolute.
- Pacchetti protocollo separati per gestione stato, misure e accessori.

## 11) EEPROM, configurazione e persistenza

La EEPROM contiene mappa completa configurativa:
- Identita unita (seriale/versioni/tipo).
- Abilitazioni funzione (`CAF`, `CAP`, `BOOST`, `DEFROST`, `WEEKLY`, `MBF`, ecc.).
- Parametri motori e sensori.
- Setpoint clima, bypass, input/output.
- Programmazione settimanale.

Funzioni principali:
- Default, shadow in RAM, allineamento e validazione.
- Reset EEPROM da KTS (introdotto nelle versioni recenti).
- Configurazione unita anche tramite stringa estesa da shell.

## 12) Testing e collaudo integrato

Sono disponibili routine di test hardware/commissioning:
- Test bypass (movimento e assorbimento).
- Test ingressi/uscite e relay.
- Test motori a step.
- Test seriali e bus I2C.
- Test accessori specifici (es. IPEHD).

## 13) Robustezza di sistema (watchdog, reset, alimentazione)

Il firmware include:
- Watchdog con refresh periodico.
- Rilevazione cause di reset.
- Gestione battery backup e condizioni di alimentazione board.
- Strategie di recovery comunicazione (incluse routine anti-blocco bus I2C nelle versioni recenti).

## 14) In sintesi operativa

`RD_ATSAM4N_V1.0` e un firmware completo per unita VMC/HRV che unisce:
- Controllo ventilazione avanzato.
- Climatizzazione con accessori modulari.
- Gestione bypass/freecooling/defrost.
- Sensoristica ampia su I2C e analogico.
- Diagnostica, allarmi, test e manutenzione.
- Configurabilita profonda via EEPROM/protocolli.

L'architettura e progettata per essere retrocompatibile con molte varianti hardware/accessori e per mantenere comportamento sicuro in presenza di fault.
