# Template descrizione funzione

## Nome funzione
`TemperatureRegulate()`

## File e posizione
`src/Clima_Func.c:370`

## Scopo
Esegue la regolazione clima periodica (scheduler 5 s): gestione bypass, preheater, heater, cooler, DXD e uscita SSR/EHD in base a modalita operative, allarmi, disponibilita accessori e temperature misurate.

## Parametri e ritorno
Parametro in ingresso:
- Nessuno.

Valore di ritorno:
- `int` (nel codice ritorna `1` in tutti i percorsi).

Unita di misura rilevanti:
- Temperature in decimi di grado C.
- Periodo scheduler atteso: 5 secondi.
- Regolazione DXD 0-10V in percentuale (`val_0_10V` 0..100, step `STEP_REG_DXD=5`).

## Logica dettagliata
- Legge configurazioni base (`Enab_Fuction`, `Config_Bypass`) e gestisce subito i casi di unita non in run:
- Se unita spenta e non in auto-on/off bypass ne MBF, chiude bypass, spegne accessori clima, azzera comando SSR (`Set_EHD_mod`) e termina.
- Se unita spenta ma in contesto MBF non applicabile, spegne comunque accessori e SSR.
- Esegue sempre `managementBypass(enab_func)` per la logica bypass.
- Se non esiste nessun attuatore utile (heater/cooler/preheater/DXD/SSR), esce.
- Se rileva fault motori o allarme controllo ventilatori (non MBF), spegne subito preheater/heater/cooler/DXD, azzera SSR e termina.
- Gestione preheater (`ACC_I2C_PREHEATER`) se non in test `MSK_TEST_IPEHD`:
- Se elettrico: controllo temporizzato ON/OFF in funzione di temperatura exhaust con timer `count_preHeaterOn`.
- Se ad acqua: anti-gelo semplice, ON sotto 5.0 C (`<=50`) e OFF sopra 7.0 C (`>70`).
- Se funzione non operativa, forza spegnimento e reset timer.
- Seleziona temperatura di riferimento (`TempRif`) in base a `Ref_T_setting`:
- Se riferimento su supply, preferisce misura AWP oppure misura accessorio clima disponibile; fallback su sonda supply.
- Se riferimento su return, usa direttamente sonda return.
- Blocco dedicato DXD (se presente in EEPROM, anche se non operativo):
- Imposta funzione compressore estate/inverno (`S`/`W`) e sincronizza verso accessorio tramite flag `processor_I2C`.
- Se estate e inverno entrambe disabilitate: spegne DXD, azzera regolatore e termina.
- Calcola tempo reazione regolatore da `Ref_T_setting` (min 30 s, max 180 s).
- Applica eventuale delta supply (`DeltaTemp_Supply`) al setpoint quando riferimento e su supply.
- In allarme compressore (`ALM_DXD_COMPRESS`) spegne DXD, porta 0-10V a zero, imposta update I2C e termina.
- Ogni `time_regulator` aggiorna la potenza compressore:
- In riscaldamento (`W`): aumenta o riduce `val_0_10V` in base a errore rispetto setpoint e limiti su supply.
- In raffreddamento (`S`): logica simmetrica con soglie invertite.
- Gestisce avvio con pre-carica 60% e tempi morti aggiuntivi (`count=-24`).
- Se DXD non presente, azzera struttura regolatore DXD.
- Se clima classico non disponibile (heater/cooler/SSR assenti), spegne uscite e termina.
- Calcola soglie ON/OFF di heater/cooler usando setpoint e isteresi EEPROM (`hister_Temp_*`).
- In inverno:
- Spegne cooler.
- Se heater o SSR presente: ON sotto soglia ON, OFF sopra soglia OFF con persistenza di spegnimento (`persist_Temp_heater_off`).
- Se heater non disponibile, forza OFF e azzera persistenza.
- In estate:
- Spegne heater e SSR.
- Gestisce cooler con condizione aggiuntiva legata al gradiente fresh/return per privilegiare bypass prima del raffreddamento attivo.
- Se ne estate ne inverno (e non MBF), spegne tutti gli attuatori clima.

## Dipendenze
- EEPROM/config: `read_byte_eeprom`, `read_word_eeprom`, `write_byte_eeprom`.
- Bypass: `managementBypass`, `Active_Procedure_Bypass_OpCl`.
- Accessori digitali: `DigitAccessoryOperating`, `DigitAccessoryOn`, `TAG_DigitAccessoryOn`, `TAG_DigitAccessoryOff`, `AccessoryPresent_EepList`.
- Allarmi/stato: `CkAlarm`, `sData` (temperature, status unit, status motori, accessori, regolatore DXD).
- Uscita SSR/EHD: parametro EEPROM `Set_EHD_mod`.
- Variabili globali di persistenza: `count_preHeaterOn`, `persist_Temp_heater_off`.

## Casi limite
- Tutti i percorsi di uscita ritornano `1`; non c'e un codice differenziato errore/successo.
- In assenza accessori o con fault motori la funzione diventa fail-safe (spegne attuatori).
- La selezione `TempRif` dipende da disponibilita accessori e puo cambiare dinamicamente se un accessorio non e operativo.
- In DXD la rampa 0-10V e saturata in [0,100], con ritardi di riarmo per evitare stress compressore.
- Il bypass viene sempre gestito prima della parte clima, quindi influenza direttamente la richiesta cooler.

## Verifica
- Test funzionale per modalita: inverno, estate, disabilitato, MBF.
- Test fault injection: allarme compressore DXD, guasto motori, allarmi sonde bypass.
- Verifica soglie con dataset temperatura vicino ai punti ON/OFF per confermare isteresi e persistenze.
- Verifica comandi I2C su accessori e scrittura `Set_EHD_mod` in ogni transizione.
- Verifica temporalita preheater e regolatore DXD con scheduler reale a 5 s.

## Racconto operativo (per utente/service)
Questa e la regia generale della climatizzazione: ogni pochi secondi guarda lo stato macchina, le temperature e gli allarmi, poi decide cosa tenere acceso e cosa spegnere tra bypass, preheater, riscaldamento, raffreddamento e compressore DXD.

Dal punto di vista pratico, se l'unita e ferma o c'e un guasto importante ai ventilatori, la funzione mette tutto in sicurezza e spegne gli attuatori clima. Questo spiega molti casi in cui sul campo "non parte il caldo/freddo": il blocco puo dipendere da una condizione di protezione a monte.

Quando il sistema lavora in inverno, privilegia il riscaldamento; in estate privilegia il raffreddamento. Le soglie non sono un unico valore secco: ci sono margini di attacco e distacco (isteresi) per evitare continui ON/OFF. Per questo un utente puo vedere che il dispositivo non cambia stato esattamente al setpoint, ma un po' prima o un po' dopo.

Il preheater viene gestito in modo protettivo, soprattutto per evitare rischio gelo. In pratica, con aria molto fredda resta acceso o viene ciclicamente mantenuto per proteggere la batteria.

Se e presente DXD, la macchina puo regolare la potenza in modo progressivo invece che solo acceso/spento. Sul campo questo si percepisce come funzionamento piu "morbido": il compressore aumenta o riduce gradualmente in base a quanto la temperatura reale e lontana da quella desiderata.

Un aspetto chiave per assistenza e che il bypass viene valutato prima del raffreddamento attivo: quando l'aria esterna aiuta, la macchina prova prima a sfruttarla. Quindi in alcuni casi il cooler resta spento non per guasto, ma per scelta energetica corretta del controllo.
