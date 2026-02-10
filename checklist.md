# Checklist Test Firmware (Procedurale)

Questa checklist va eseguita **sempre** dopo qualsiasi variazione di firmware. Ogni step deve essere tracciato con esito, data e operatore. Tutte le verifiche sono **procedurali** e includono retroazioni dei componenti e comunicazioni con accessori (Modbus, CTRL_FAN, KTS).

## 0) Preparazione
- [ ] Verifica versione firmware caricata (stringa `VERSION_FW`) e data build.
- [ ] Verifica configurazione hardware (numero motori, sensori NTC, accessori presenti in EEPROM).
- [ ] Carica configurazione EEPROM di test standard (profilo di collaudo).
- [ ] Collega alimentazione e strumenti: multimetro, oscilloscopio, interfaccia seriale.
- [ ] Collega gli accessori richiesti: KTS, CTRL_FAN, Modbus, accessori I2C previsti.

## 1) Boot e stabilita base
- [ ] Avvio senza errori: nessun reset ciclico, nessun beep anomalo.
- [ ] Watchdog attivo e reset cause loggato correttamente.
- [ ] Scheduler attivo: verificare tick 100 ms (o contatore equivalente).
- [ ] Nessun allarme critico all'avvio (se hardware in condizioni nominali).

## 2) Comunicazione KTS (UART0 9600)
- [ ] Ricezione polling KTS e risposta corretta.
- [ ] Lettura dati debug: valori base coerenti (stati, misure, allarmi).
- [ ] Lettura EEPROM via KTS: indirizzi corretti, CRC valido.
- [ ] Scrittura EEPROM via KTS: modifica confermata e persistente.
- [ ] Test comando KTS (es. avvio test): corretta attivazione/uscita test.
- [ ] Timeout KTS: simulare assenza KTS e verificare allarme/timeout.

## 3) Comunicazione CTRL_FAN (UART2 38400)
- [ ] Polling CTRL_FAN attivo: risposta valida con CRC.
- [ ] Feedback RPM e tensioni coerenti con setpoint.
- [ ] Stato motori (Power/Alarm) correttamente aggiornato.
- [ ] Variazione setpoint velocita: RPM segue la richiesta (entro tolleranza).
- [ ] Verifica recovery in caso di risposta mancante.

## 4) Comunicazione Modbus (UART2 38400)
- [ ] Polling Modbus attivo: pacchetto 1 e 2 corretti.
- [ ] Lettura stato unita e allarmi corretta.
- [ ] Lettura misure (temperature, pressioni, air flow, CO2/RH/VOC).
- [ ] Scrittura setpoint (MBF, setpoint temperatura/CO2) e verifica effetto.
- [ ] Gestione timeout Modbus e allarme KO.

## 5) Accessori I2C (retroazioni e comunicazione)
- [ ] Scansione accessori: presenza rilevata in `sData.AccI2C[].sts`.
- [ ] Livello link (0/10..10/10) aggiornato per ogni accessorio.
- [ ] Lettura misure coerenti per ogni accessorio connesso.
- [ ] Gestione link KO: simulare disconnessione e verificare allarme.

Accessori minimi da verificare (se presenti):
- [ ] Preheater / Heater / Cooler: comando ON/OFF e feedback stato.
- [ ] DXD: comando 0-10V e selezione summer/winter.
- [ ] CO2/RH/VOC: valori coerenti e alarmi KO se scollegati.
- [ ] PCAP/PCAF/DPP/DPS: letture pressione coerenti.
- [ ] AWP / EXT1..EXT4: letture valide.
- [ ] EBPD / EBP2 / SSR: comando e feedback coerenti.
- [ ] FLW1/FLW2: variazione portata corretta.

## 6) Sensori analogici e retroazioni
- [ ] NTC: tutte le sonde configurate con valori plausibili.
- [ ] Ingressi 0-10V: letture coerenti in standby e in run.
- [ ] Corrente KTS: lettura valida e stabile.
- [ ] Assorbimento motore bypass: valori min/max coerenti durante manovra.
- [ ] ADC esterni (ADC121C027, ADS7142): letture valide.

## 7) Motori e controllo velocita
- [ ] Avvio motori con velocita minima corretta.
- [ ] Cambio step velocita (min/med/max) con feedback RPM.
- [ ] Modalita CAP: mantenimento pressione.
- [ ] Modalita CAF: mantenimento portata.
- [ ] Modalita MBF: controllo separato F/R da Modbus.
- [ ] Gestione defrost e post-vent: transizioni coerenti.

## 8) Bypass e clima
- [ ] Bypass manuale OPEN/CLOSE: movimento completo con feedback.
- [ ] Bypass automatico: apertura/chiusura secondo setpoint.
- [ ] Verifica freecooling (auto ON/OFF).
- [ ] Preheater/Heater/Cooler: attivazione secondo setpoint.
- [ ] Defrost: attivazione su condizioni e reset corretto.

## 9) Filtri e manutenzione
- [ ] Timer ore funzionamento incrementa correttamente.
- [ ] DPS: allarme filtro dopo tempo impostato.
- [ ] DPP: calibrazione e verifica periodica.
- [ ] Reset filtri da KTS e azzeramento allarme.

## 10) Weekly program
- [ ] Range giornalieri attivati correttamente.
- [ ] Cambio velocita/setpoint/imbalance secondo programma.
- [ ] Pre-apertura preheater prima dello start (se previsto).

## 11) Allarmi e sicurezza
- [ ] Generazione allarmi su fault simulati (sensori, motori, bypass).
- [ ] Rimozione allarmi a fault risolto.
- [ ] Allarme incendio (input dedicati) e modalita ventilazione.
- [ ] Battery backup: simulare tensione bassa e verificare comportamento.

## 12) Chiusura
- [ ] Nessun allarme residuo non atteso.
- [ ] Log test compilato con esiti e note.
- [ ] Conferma che le funzioni critiche non hanno regressioni.

## Note operative
- Tutti i test devono essere eseguiti con configurazione standard e successivamente con almeno una configurazione ?accessori massimi?.
- Qualsiasi KO blocca il rilascio finche non e risolto o motivato.
