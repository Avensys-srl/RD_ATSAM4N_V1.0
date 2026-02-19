# Funzione `weekly_program`

# SEZIONE A - DOCUMENTAZIONE TECNICA (ENGINEERING)

## A1. Nome funzione
`weekly_program()`

## A2. File e posizione
`src/Weekly_Prog.c:34`

## A3. Scopo tecnico
Gestisce il programma orario settimanale: aggiorna il clock interno, verifica se la funzione weekly e abilitata e compone `sData.status_weekly` in base alla fascia oraria attiva del giorno corrente.

## A4. Parametri e ritorno
### Parametri in ingresso
- Nessuno

### Valore di ritorno
- Tipo: `int`
- Significato: ritorna sempre `1` per mantenere attiva la funzione nello scheduler.

## A5. Logica dettagliata
- Incrementa il clock software (`secondi/minuti/ore/giorno settimana`) con rollover.
- Legge la maschera funzioni abilitabili (`msk_Enab_Fuction`) e termina subito se il weekly non e disponibile.
- Legge le funzioni abilitate runtime (`Enab_Fuction`); se weekly attivo imposta `MSK_BIT_WEEKLY_ENAB`.
- Calcola lo step temporale da 30 minuti (`realtime_step_30min`).
- Legge il numero fasce del giorno (`numRange`) da EEPROM.
- Per ogni fascia legge `timeON/timeOFF` e verifica se l'orario corrente e interno alla finestra.
- Se presente preheater ad acqua, anticipa la gestione di mezza fascia prima dell'accensione.
- Quando trova una fascia valida, imposta `MSK_BIT_WEEKLY_RUN` e carica da EEPROM le configurazioni di temperatura, imbalance e speed nel byte `status_weekly`.
- Se weekly non attivo runtime, azzera `sData.status_weekly`.

## A6. Mappa firmware con riferimenti puntuali
- **Passo**: ingresso funzione
  **Dove**: `src/Weekly_Prog.c:34`
  **Cosa osservare**: `int weekly_program()`
- **Passo**: aggiornamento clock con rollover
  **Dove**: `src/Weekly_Prog.c:41`
  **Cosa osservare**: incremento `sClockWeek.seconds`, gestione minuti/ore/weekday
- **Passo**: controllo disponibilita funzione weekly
  **Dove**: `src/Weekly_Prog.c:60`
  **Cosa osservare**: `msk_Enab_Fuction`, uscita anticipata a `src/Weekly_Prog.c:63`
- **Passo**: abilitazione weekly runtime e calcolo step 30 min
  **Dove**: `src/Weekly_Prog.c:68`
  **Cosa osservare**: `sData.status_weekly = MSK_BIT_WEEKLY_ENAB`, `realtime_step_30min`
- **Passo**: lettura fasce giorno da EEPROM
  **Dove**: `src/Weekly_Prog.c:73`
  **Cosa osservare**: `sDayProg[weekday].numRange`
- **Passo**: lettura finestra ON/OFF per fascia
  **Dove**: `src/Weekly_Prog.c:102`
  **Cosa osservare**: `time_start`, `time_stop`
- **Passo**: pre-attivazione preheater ad acqua
  **Dove**: `src/Weekly_Prog.c:129`
  **Cosa osservare**: `TAG_DigitAccessoryOn/Off(ACC_I2C_PREHEATER)`
- **Passo**: verifica fascia attiva e RUN weekly
  **Dove**: `src/Weekly_Prog.c:155`
  **Cosa osservare**: `MSK_BIT_WEEKLY_RUN`
- **Passo**: caricamento ConfigTemp/ConfigImbal/ConfigSpeed
  **Dove**: `src/Weekly_Prog.c:167`
  **Cosa osservare**: bit packing su `status_weekly`
- **Passo**: reset stato weekly quando non abilitato runtime
  **Dove**: `src/Weekly_Prog.c:203`
  **Cosa osservare**: `sData.status_weekly = 0`

## A7. Interfacce esterne (livello astratto)
### Segnali generati dall'RD
- Comando logico accessorio preheater (solo caso acqua):
  - Pin MCU/periferica: non diretto in questa funzione
  - Tipo: comando digitale via catena accessori I2C
  - Livello attivo: ON/OFF preheater
  - Condizione di attivazione: fascia weekly e pre-riscaldo
  - Persistenza: fino al successivo ciclo di valutazione

### Segnali letti dall'RD
- Dati programmazione weekly da EEPROM:
  - Tipo ingresso: configurazione non volatile
  - Frequenza campionamento: ogni esecuzione funzione
  - Condizione validita: parametri settimanali coerenti
- Stato preheater:
  - Tipo ingresso: stato accessorio in `sData.AccI2C`
  - Frequenza campionamento: ogni ciclo funzione
  - Condizione validita: accessorio connesso

## A8. Catena causale (livello sistema)
| Livello | Descrizione |
|----------|------------|
| Decisione | Determina se weekly e in RUN e quale profilo applicare |
| Interfaccia | Lettura EEPROM + stato accessori |
| Elettronica esterna | Moduli accessori e memoria configurazione |
| Attuatore | Preheater (solo in casi previsti) |
| Effetto | Profilo orario operativo applicato alla macchina |

## A9. Feedback disponibile
- Esiste conferma reale?
  - Parziale: conferma logica su stato weekly e su stato accessorio; nessun ack esplicito per singola fascia oraria.
- Il comando e "fire and forget"?
  - Parzialmente: scrive stato weekly e comandi preheater, ma la conferma completa e demandata ai cicli successivi.
- Dove viene letto il feedback?
  - In `sData.status_weekly` e nello stato accessorio preheater (`src/Weekly_Prog.c:70`, `src/Weekly_Prog.c:163`, `src/Weekly_Prog.c:129`).
- Quanto e affidabile?

Valori criteri:
- `Origine diretta: 1` (orario e configurazioni lette direttamente da strutture/EEPROM)
- `Correlazione temporale: 0` (nessun timeout esplicito)
- `Correlazione univoca col comando: 0` (nessun ID evento per fascia)
- `Gestione errore: 1` (rami di inibizione e fallback su disable)
- `Punteggio totale: 2`
- `Classe finale: Medio`

## A10. Punti critici firmware
- Coerenza del clock software: drift o mancata sincronizzazione altera fasce ON/OFF.
- Validita campi EEPROM (`numRange`, `timeON`, `timeOFF`) non validata in modo forte in questa funzione.
- Configurazioni bitwise (`ConfigTemp/Imbal/Speed`) dipendono da shift corretti per indice fascia.
- Possibili sovrapposizioni tra fasce: la funzione usa il primo range valido e fa `break`.

## A11. Punti di disaccoppiamento (sistema)
- Errore logico firmware:
  - mapping errato dei bit weekly in `status_weekly`
  - range temporali configurati in modo incoerente
- Errore interfaccia:
  - letture EEPROM non coerenti con configurazione attesa
  - stato accessorio non aggiornato
- Errore elettrico esterno:
  - preheater non alimentato/non raggiungibile
- Errore meccanico:
  - non applicabile direttamente alla sola logica oraria

## A12. Limite di responsabilita firmware
Il firmware RD:
- aggiorna il clock weekly interno
- seleziona la fascia oraria attiva
- pubblica il profilo weekly in `status_weekly`
- comanda il preheater nei casi previsti

Il firmware RD NON garantisce:
- correttezza configurazione settimanale inserita a monte
- sincronismo orario assoluto senza fonte tempo esterna
- risposta fisica degli accessori esterni
