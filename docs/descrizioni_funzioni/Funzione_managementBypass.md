# Template descrizione funzione

## Nome funzione
`managementBypass(unsigned short enab_func)`

## File e posizione
`src/Clima_Func.c:93`

## Scopo
Gestisce apertura/chiusura del bypass aria in base alla configurazione EEPROM, allo stato sonde, allo stato accessori e al contesto operativo (automatico, manuale, controllo esterno, auto on/off con freecooling).

## Parametri e ritorno
Parametro in ingresso:
- `enab_func`: bitmask funzioni abilitate (estate, inverno, altre modalita); usata in particolare per la logica bypass in automatico.

Valore di ritorno:
- Nessuno (`void`).

Unita di misura rilevanti:
- Temperature in decimi di grado C.
- Tensioni ingressi analogici in decimi/centesimi di volt scalati ADC (soglie usate: `250` e `610`).

## Logica dettagliata
- Se il test bypass e attivo (`MSK_TEST_BYPASS`), esce subito senza agire.
- Legge `Config_Bypass` da EEPROM e imposta il comportamento:
- `BPD_MANUAL_CLOSE`: forza comando chiusura.
- `BPD_MANUAL_OPEN`: forza comando apertura.
- `BPD_AUTO_ON_OFF`: gestisce una logica di freecooling con cicli ON/OFF temporizzati e possibile accensione/spegnimento unita via `Set_Power_ON` e `PowerMode`.
- In fase ON mantiene attivazione per 60 cicli scheduler.
- In fase OFF attende 360 cicli prima di riesaminare riaccensione.
- Se prima era in standby (`standby_before`), ripristina lo stato precedente.
- `BPD_AUTOMATIC`: se DXD e acceso chiude bypass; altrimenti valuta temperatura ritorno/fresh e soglie EEPROM:
- Apertura con condizioni combinate: ritorno alta vs setpoint, fresh sopra minima, fresh abbastanza piu fredda di return, cooler spento.
- Chiusura con condizioni di esclusione: fresh >= return, fresh sotto minima, return sotto setpoint-hysteresis, heater/cooler accesi.
- Usa persistenza (`persist_temp_bypass_on`) per evitare commutazioni impulsive.
- `BPD_EXT_CTRL`: usa input esterno configurato (`Set_Input[0]/[1]`) per tradurre tensione ingresso in comando apertura/chiusura con due modalita:
- `INP_12V_BYPASS_OPEN` (12V=open, 0V=close).
- `INP_0V_BYPASS_OPEN` (0V=open, 12V=close).
- Al termine calcola un tempo di attesa dipendente da velocita motori, poi esegue realmente il movimento bypass con `Active_Procedure_Bypass_OpCl(OPEN_BPD/CLOSE_BPD, 9)` solo se lo stato corrente lo richiede.

## Dipendenze
- EEPROM: `read_byte_eeprom`, `read_word_eeprom`, `write_byte_eeprom`.
- Stato unita e misure: `sData` (`status_unit`, `measure_Temp`, `measure_ADC_input`, `speed_motors_F`).
- Accessori/stato allarmi: `DigitAccessoryOn`, `DigitAccessoryOperating`, `CkAlarm`.
- Comandi attuatori: `Active_Procedure_Bypass_OpCl`, `PowerMode`.
- Parametri e macro: `Config_Bypass`, `Bypass_minTempExt`, `SetPointTemp[]`, `idxSetPointT`, `Set_Power_ON`, `MSK_STS_BYPASS`, `POS_BIT_BYPASS_CLOSE`.
- Variabili globali di persistenza: `persist_temp_bypass_on`, `count_active_freecooling_on`, `count_active_freecooling_off`, `standby_before`.

## Casi limite
- Se sonde fresh/return sono in allarme (`ALM_PTFRESH_KO` o `ALM_PTRET_KO`), in automatico forza comportamento prudente di chiusura bypass.
- In `BPD_EXT_CTRL` se input 1 non e configurato per bypass, prova input 2.
- La commutazione e filtrata con contatori di persistenza per ridurre oscillazioni.
- Il codice usa un controllo stato in chiusura con confronto su negazione logica del bitmask (`== !mskBPD_Close`), da verificare in validazione per coerenza con il bitmask stato bypass.

## Verifica
- Forzare ciascun `Config_Bypass` e osservare comando/stato bypass su log diagnostico e stato `sData.status_unit`.
- Simulare temperature per trigger di apertura e chiusura in automatico e verificare persistenza.
- In `BPD_EXT_CTRL`, iniettare livelli equivalenti a <2.5V e >6.1V e confermare la direzione comando.
- In `BPD_AUTO_ON_OFF`, verificare cicli ON (60) e OFF (360) e ripristino standby.

## Spazio tecnico (mappa firmware e righe)
- Test bypass attivo: `src/Clima_Func.c:103`
- Lettura configurazione bypass: `src/Clima_Func.c:107`
- Modalita manuale chiuso: `src/Clima_Func.c:126`
- Modalita manuale aperto: `src/Clima_Func.c:131`
- Modalita auto on/off freecooling: `src/Clima_Func.c:136`
- Modalita automatica da temperature: `src/Clima_Func.c:207`
- Modalita comando esterno ingressi: `src/Clima_Func.c:271`
- Decisione comando apertura/chiusura (flag cmd): `src/Clima_Func.c:355`
- Esecuzione movimento bypass (open/close): `src/Clima_Func.c:356` e `src/Clima_Func.c:358`

## Racconto operativo (utente finale)
Questa logica decide se far passare l'aria dentro lo scambiatore oppure nel canale bypass. In pratica, il sistema prova a scegliere la strada che aiuta di piu a raggiungere la temperatura desiderata con meno consumo.

Se il bypass e in manuale, il comportamento e semplice: resta sempre aperto o sempre chiuso, in base all'impostazione scelta. Se invece e in automatico, il controllo guarda soprattutto due temperature: aria esterna e aria di ritorno ambiente.

Il bypass tende ad aprirsi quando fuori c'e una condizione utile a raffrescare naturalmente l'ambiente, quindi quando l'aria esterna e favorevole rispetto all'interno. Al contrario tende a chiudersi quando fuori non conviene, quando fa troppo freddo, oppure quando stanno gia lavorando riscaldamento o raffreddamento attivo.

Un punto importante sul campo e che il sistema non cambia stato in modo "nervoso": usa tempi di persistenza e attese prima di confermare apertura o chiusura. Quindi e normale non vedere la serranda muoversi subito al primo cambio di temperatura.

In modalita con comando esterno, il bypass segue direttamente il segnale elettrico di ingresso. In quel caso, se il bypass non si apre o non si chiude, il primo controllo pratico da fare e verificare livello del segnale in ingresso e configurazione della modalita di ingresso.

In modalita auto on/off (freecooling), il sistema puo perfino accendere o spegnere l'unita per sfruttare meglio l'aria esterna. Se sul campo sembra "riaccendersi da solo", spesso e proprio questa funzione che sta lavorando come previsto.

## Operativo service (diagnosi sul campo)
### Errori bloccanti a monte
- Modalita test/manutenzione attiva: il bypass non viene gestito automaticamente.
- Allarmi sonde aria esterna o aria di ritorno: il controllo passa in comportamento prudente e tende a non aprire.
- Configurazione non coerente tra modalita bypass e comando disponibile (manuale/automatico/ingresso esterno).
- Segnale ingresso esterno non valido o fuori soglia quando il bypass e comandato da ingresso.
- Guasto attuatore bypass o catena di potenza/comando: comando presente ma nessun movimento reale.
- Stato gia raggiunto: se il bypass e gia nella posizione richiesta, non c'e nuova manovra.

### Checklist problem solving (dati da controllare)
- Modalita bypass impostata: manuale chiuso, manuale aperto, automatico, comando esterno, auto on/off.
- Temperatura richiesta e temperatura reale: confrontare aria di ritorno e aria esterna.
- Soglia minima per usare il bypass: verificare che l'aria esterna sia sopra la soglia minima prevista.
- Delta termico utile: verificare se l'aria esterna e realmente favorevole rispetto all'aria interna.
- Isteresi e persistenza: confermare che siano trascorsi i tempi minimi prima di aspettarsi il cambio stato.
- Stato riscaldamento/raffreddamento attivo: se sono gia attivi possono inibire l'apertura bypass.
- Stato allarmi sonde: con allarmi attivi il bypass puo restare chiuso per sicurezza.
- Comando inviato vs effetto fisico: verificare se la serranda si muove davvero (feedback, posizione, assorbimento, rumore motore).
