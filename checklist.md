# Checklist Test Firmware (Procedurale + Criteri OK/KO)

Questa checklist va eseguita **sempre** dopo qualsiasi variazione firmware.  
Ogni step va registrato con: `Data`, `Operatore`, `Esito`, `Note`.

Formato obbligatorio di valutazione:
- `OK atteso`: condizione che deve verificarsi per superare il test.
- `KO/Fallito`: condizione che determina regressione o non conformita.

## Scheda stampa collaudo

Data test: ____________________
Ora inizio: ____________________
Ora fine: ____________________

Operatore: ____________________
Revisore: ____________________

Versione firmware (`VERSION_FW`): ____________________
Commit/Tag Git: ____________________
Build date/time: ____________________

Scheda/Unita under test (SN): ____________________
Configurazione hardware (motori/NTC/accessori): ____________________
Profilo EEPROM caricato: ____________________

Strumentazione usata (seriale/oscilloscopio/multimetro): ____________________
Ambiente prova (banco/campo): ____________________

Esito complessivo:
- [ ] PASS
- [ ] FAIL

Blocco rilascio:
- [ ] NO
- [ ] SI

Note generali test:
______________________________________________________________________________
______________________________________________________________________________
______________________________________________________________________________

Firma operatore: ____________________    Firma revisore: ____________________

## 0) Preparazione
- [ ] Versione firmware e build
`OK atteso`: `VERSION_FW` e data build corrispondono al rilascio da validare.
`KO/Fallito`: versione non coerente o build non tracciabile.
- [ ] Configurazione hardware (motori, NTC, accessori EEPROM)
`OK atteso`: configurazione reale impianto allineata ai parametri caricati.
`KO/Fallito`: mismatch tra hardware e configurazione attiva.
- [ ] Caricamento profilo EEPROM di collaudo
`OK atteso`: profilo standard caricato e confermato via lettura.
`KO/Fallito`: valori diversi dal profilo di test.
- [ ] Strumentazione collegata (multimetro, oscilloscopio, seriale)
`OK atteso`: strumenti operativi e letture stabili.
`KO/Fallito`: strumentazione non disponibile o non affidabile.
- [ ] Accessori collegati (KTS, CTRL_FAN, Modbus, I2C previsti)
`OK atteso`: tutti i nodi richiesti raggiungibili.
`KO/Fallito`: uno o piu nodi non raggiungibili.

## 1) Boot e stabilita base
- [ ] Boot unit
`OK atteso`: nessun reset ciclico, nessun beep anomalo, avvio regolare.
`KO/Fallito`: reboot loop, freeze o segnalazioni anomale.
- [ ] Watchdog e reset cause
`OK atteso`: watchdog attivo e cause reset coerenti.
`KO/Fallito`: cause reset inattese o watchdog non operativo.
- [ ] Scheduler (tick base)
`OK atteso`: tick temporale regolare (100 ms o equivalente).
`KO/Fallito`: jitter evidente o task non eseguite.
- [ ] Allarmi critici a freddo
`OK atteso`: nessun allarme critico con impianto nominale.
`KO/Fallito`: allarmi critici senza fault reale.

## 2) Comunicazione KTS (UART0 9600)
- [ ] Polling KTS
`OK atteso`: richiesta/risposta valide e continue.
`KO/Fallito`: timeout, frame errati, perdita link.
- [ ] Lettura debug
`OK atteso`: stati/misure/allarmi coerenti con condizione reale.
`KO/Fallito`: valori incoerenti o non aggiornati.
- [ ] Lettura EEPROM via KTS
`OK atteso`: indirizzi e dati corretti, controllo integrita valido.
`KO/Fallito`: indirizzi errati o dati corrotti.
- [ ] Scrittura EEPROM via KTS
`OK atteso`: valore scritto e persistente dopo rilettura/reset.
`KO/Fallito`: scrittura non persistente o su indirizzo errato.
- [ ] Comandi test da KTS
`OK atteso`: ingresso/uscita test corretti e reversibili.
`KO/Fallito`: stato test bloccato o non coerente.
- [ ] Timeout KTS simulato
`OK atteso`: attivazione gestione timeout/allarme prevista.
`KO/Fallito`: nessuna gestione timeout o recovery non funzionante.

## 3) Comunicazione CTRL_FAN (UART2 38400)
- [ ] Polling CTRL_FAN
`OK atteso`: risposta valida, continua, senza errori protocollo.
`KO/Fallito`: frame invalidi o perdita polling.
- [ ] Feedback RPM/tensioni
`OK atteso`: feedback coerente con setpoint inviato.
`KO/Fallito`: feedback fuori trend o non aggiornato.
- [ ] Stato motori (Power/Alarm)
`OK atteso`: bit stato coerenti con comportamento reale.
`KO/Fallito`: bit allarme/power incoerenti.
- [ ] Variazione setpoint velocita
`OK atteso`: RPM segue richiesta entro tolleranza definita.
`KO/Fallito`: nessuna variazione o errore oltre tolleranza.
- [ ] Recovery da frame mancanti
`OK atteso`: link ristabilito senza riavvio forzato.
`KO/Fallito`: blocco comunicazione o necessita reset.

## 4) Comunicazione Modbus (UART2 38400)
- [ ] Polling Modbus (packet 1 e 2)
`OK atteso`: pacchetti completi e coerenti.
`KO/Fallito`: pacchetti incompleti/errati o timeout frequenti.
- [ ] Lettura stato unita/allarmi
`OK atteso`: mappa registri coerente con stato reale.
`KO/Fallito`: registri disallineati o non aggiornati.
- [ ] Lettura misure (T, P, airflow, CO2/RH/VOC)
`OK atteso`: misure plausibili e consistenti con sensori.
`KO/Fallito`: misure fisse, sature o non plausibili.
- [ ] Scrittura setpoint Modbus
`OK atteso`: comando accettato e effetto verificabile.
`KO/Fallito`: comando ignorato o effetto errato.
- [ ] Timeout Modbus simulato
`OK atteso`: gestione KO conforme (stato/allarme/recovery).
`KO/Fallito`: assenza gestione errore o degradazione non controllata.

## 5) Accessori I2C (retroazioni e comunicazione)
- [ ] Scansione accessori
`OK atteso`: presenza corretta in `sData.AccI2C[].sts`.
`KO/Fallito`: accessori presenti non rilevati o falsi positivi.
- [ ] Livello link per accessorio
`OK atteso`: livello link aggiornato e coerente (0/10..10/10).
`KO/Fallito`: livello bloccato o incoerente.
- [ ] Lettura misure accessori
`OK atteso`: valori coerenti con stato fisico dell'impianto.
`KO/Fallito`: valori nulli/fissi o errati.
- [ ] Simulazione link KO
`OK atteso`: allarme link attivo e recovery a riconnessione.
`KO/Fallito`: nessun allarme o mancato recovery.

Accessori minimi (se presenti):
- [ ] Preheater/Heater/Cooler ON/OFF
`OK atteso`: comando e feedback stato allineati.
`KO/Fallito`: comando senza effetto o stato non coerente.
- [ ] DXD 0-10V + Summer/Winter
`OK atteso`: modulazione e funzione applicate correttamente.
`KO/Fallito`: modulazione non applicata o funzione errata.
- [ ] CO2/RH/VOC
`OK atteso`: letture plausibili, KO se sensore scollegato.
`KO/Fallito`: nessuna segnalazione KO o valori anomali persistenti.
- [ ] PCAP/PCAF/DPP/DPS
`OK atteso`: pressione coerente con condizioni impianto.
`KO/Fallito`: letture incoerenti o assenti.
- [ ] AWP/EXT1..EXT4
`OK atteso`: letture valide e aggiornate.
`KO/Fallito`: canale non aggiornato o non plausibile.
- [ ] EBPD/EBP2/SSR
`OK atteso`: comando e feedback coerenti.
`KO/Fallito`: comando non eseguito o stato errato.
- [ ] FLW1/FLW2
`OK atteso`: variazione portata coerente con manovra.
`KO/Fallito`: portata non variabile o non plausibile.

## 6) Sensori analogici e retroazioni
- [ ] NTC (TS/TF/TR/TE)
`OK atteso`: tutte le sonde forniscono valori plausibili.
`KO/Fallito`: outlier, drift anomalo o sonda non aggiornata.
- [ ] Ingressi 0-10V
`OK atteso`: lettura coerente in standby e run.
`KO/Fallito`: saturazione o scala errata.
- [ ] Corrente KTS
`OK atteso`: valore stabile e nel range previsto.
`KO/Fallito`: valore instabile o fuori range senza causa.
- [ ] Assorbimento bypass
`OK atteso`: min/max coerenti durante manovra.
`KO/Fallito`: valori incoerenti o assenti.
- [ ] ADC esterni (ADC121C027, ADS7142)
`OK atteso`: conversioni valide e periodiche.
`KO/Fallito`: canali bloccati o non coerenti.

## 7) Motori e controllo velocita
- [ ] Avvio a velocita minima
`OK atteso`: avvio regolare senza fault.
`KO/Fallito`: mancato avvio o fault immediato.
- [ ] Step velocita min/med/max
`OK atteso`: RPM cresce/decresce coerentemente.
`KO/Fallito`: assenza risposta o risposta invertita.
- [ ] CAP (pressione costante)
`OK atteso`: pressione stabilizzata nel target.
`KO/Fallito`: pressione non controllata.
- [ ] CAF (portata costante)
`OK atteso`: portata stabilizzata nel target.
`KO/Fallito`: portata instabile o fuori target.
- [ ] MBF (Fresh/Return separati)
`OK atteso`: controllo indipendente F/R attivo.
`KO/Fallito`: accoppiamento non previsto tra rami.
- [ ] Defrost/post-vent
`OK atteso`: transizioni stato corrette e reversibili.
`KO/Fallito`: stato bloccato o sequenza non corretta.

## 8) Bypass e clima
- [ ] Bypass manuale OPEN/CLOSE
`OK atteso`: corsa completa con feedback stato coerente.
`KO/Fallito`: mancata corsa o stato errato.
- [ ] Bypass automatico
`OK atteso`: apertura/chiusura coerenti con setpoint.
`KO/Fallito`: logica opposta o assenza intervento.
- [ ] Freecooling auto ON/OFF
`OK atteso`: attivazione/disattivazione nelle condizioni previste.
`KO/Fallito`: attivazione intempestiva o assente.
- [ ] Preheater/Heater/Cooler su setpoint
`OK atteso`: attuazione coerente con soglie/isteresi.
`KO/Fallito`: short-cycle o mancata attuazione.
- [ ] Defrost
`OK atteso`: attivazione corretta e reset al termine.
`KO/Fallito`: defrost non entra/esce correttamente.

## 9) Filtri e manutenzione
- [ ] Ore funzionamento
`OK atteso`: contatore incrementa e persiste.
`KO/Fallito`: contatore fermo o non persistente.
- [ ] DPS (timer filtro)
`OK atteso`: allarme filtro al superamento soglia.
`KO/Fallito`: allarme assente o anticipato.
- [ ] DPP (calibrazione/verifica)
`OK atteso`: ciclo di calibrazione/verifica completato.
`KO/Fallito`: ciclo non completabile o stato incoerente.
- [ ] Reset filtri da KTS
`OK atteso`: allarme azzerato e stato aggiornato.
`KO/Fallito`: allarme persistente o reset parziale.

## 10) Weekly program
- [ ] Range giornalieri
`OK atteso`: attivazioni rispettano gli slot configurati.
`KO/Fallito`: slot ignorati o attivati fuori orario.
- [ ] Speed/setpoint/imbalance per fascia
`OK atteso`: set applicati correttamente in ogni range.
`KO/Fallito`: set non applicati o applicati male.
- [ ] Pre-apertura preheater (se prevista)
`OK atteso`: anticipo rispettato prima dello start.
`KO/Fallito`: nessun anticipo o anticipo errato.

## 11) Allarmi e sicurezza
- [ ] Fault simulati (sensori/motori/bypass)
`OK atteso`: allarme corretto e tracciato.
`KO/Fallito`: allarme errato o assente.
- [ ] Rimozione allarme a fault risolto
`OK atteso`: clear conforme alla logica prevista.
`KO/Fallito`: allarme non cancellabile o cancellato prematuramente.
- [ ] Allarme incendio (input dedicati)
`OK atteso`: modalita ventilazione applicata correttamente.
`KO/Fallito`: modalita non coerente con configurazione incendio.
- [ ] Battery backup (tensione bassa simulata)
`OK atteso`: comportamento di sicurezza conforme.
`KO/Fallito`: nessuna reazione o reazione non prevista.

## 12) Chiusura
- [ ] Allarmi residui
`OK atteso`: nessun allarme non giustificato.
`KO/Fallito`: allarmi residui non spiegati.
- [ ] Report collaudo
`OK atteso`: log compilato con esiti e note per ogni step.
`KO/Fallito`: report incompleto/non tracciabile.
- [ ] Valutazione regressione
`OK atteso`: nessuna regressione sulle funzioni critiche.
`KO/Fallito`: regressione confermata o dubbia.

## Note operative
- Eseguire sempre con configurazione standard e con una configurazione `accessori massimi`.
- Qualsiasi `KO/Fallito` blocca il rilascio finche non e risolto o formalmente accettato con motivazione.
