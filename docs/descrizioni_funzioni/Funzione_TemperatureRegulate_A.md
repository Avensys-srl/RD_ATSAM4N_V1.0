# Funzione `TemperatureRegulate`

# ðŸ”· SEZIONE A â€” DOCUMENTAZIONE TECNICA (ENGINEERING)

## A1. Nome funzione
`TemperatureRegulate()`

## A2. File e posizione
`src/Clima_Func.c:370`

## A3. Scopo tecnico
Esegue la regolazione termica principale a ciclo periodico: valuta stato unita, bypass, preheater, heater, cooler, modulazione DXD e uscita SSR/EHD secondo configurazioni, allarmi e misure.

## A4. Parametri e ritorno
### Parametri in ingresso
- Nessuno

### Valore di ritorno
- Tipo: `int`
- Significato: ritorna `1` in tutti i percorsi correnti

## A5. Logica dettagliata
- Legge configurazioni base e gestisce uscite rapide in caso di unita non in RUN.
- Applica sempre la logica bypass chiamando `managementBypass`.
- Verifica disponibilita attuatori clima; se assenti esce senza regolazione.
- In fault ventilazione/allarmi critici impone stato fail-safe (spegnimento uscite clima).
- Gestisce preheater con logica elettrica temporizzata o acqua antigelo.
- Seleziona temperatura di riferimento (`TempRif`) in base a configurazione e disponibilita misure.
- Gestisce ramo DXD con controllo funzione estate/inverno, allarme compressore, rampa 0-10V e tempi regolatore.
- Calcola soglie ON/OFF heater e cooler con isteresi e persistenze.
- In inverno privilegia heater/SSR, in estate privilegia cooler con vincoli aggiuntivi legati a condizioni aria.

## A6. Mappa firmware con riferimenti puntuali
- **Passo**: lettura abilitazioni principali  
  **Dove**: `src/Clima_Func.c:377`  
  **Cosa osservare**: `enab_func = read_word_eeprom(...)`
- **Passo**: uscita rapida unita non in run (caso base)  
  **Dove**: `src/Clima_Func.c:388`  
  **Cosa osservare**: condizione su `POS_BIT_UNIT_RUN`, `cfg_bypass`, `ENAB_MBF`
- **Passo**: chiusura bypass forzata in stop  
  **Dove**: `src/Clima_Func.c:391`  
  **Cosa osservare**: `Active_Procedure_Bypass_OpCl(CLOSE_BPD, 2)`
- **Passo**: azzeramento SSR/EHD in uscita di sicurezza  
  **Dove**: `src/Clima_Func.c:404`  
  **Cosa osservare**: `write_byte_eeprom(...Set_EHD_mod..., 0)`
- **Passo**: chiamata gestione bypass  
  **Dove**: `src/Clima_Func.c:428`  
  **Cosa osservare**: `managementBypass(enab_func)`
- **Passo**: fail-safe su fault motori/allarme fan  
  **Dove**: `src/Clima_Func.c:435`  
  **Cosa osservare**: spegnimento accessori clima + ritorno
- **Passo**: controllo preheater con timer  
  **Dove**: `src/Clima_Func.c:464`  
  **Cosa osservare**: `count_preHeaterOn` e soglie su `I_PROBE_EXHAUST`
- **Passo**: selezione riferimento temperatura  
  **Dove**: `src/Clima_Func.c:542`  
  **Cosa osservare**: `ref_T_setting` e scelta `TempRif`
- **Passo**: ingresso ramo DXD  
  **Dove**: `src/Clima_Func.c:567`  
  **Cosa osservare**: presenza/abilitazione `ACC_I2C_DXD`
- **Passo**: blocco immediato su allarme compressore  
  **Dove**: `src/Clima_Func.c:647`  
  **Cosa osservare**: `CkAlarm(ALM_DXD_COMPRESS)` con reset regolatore
- **Passo**: uscita quando clima classico assente  
  **Dove**: `src/Clima_Func.c:776`  
  **Cosa osservare**: spegnimento heater/cooler + SSR
- **Passo**: regolazione inverno  
  **Dove**: `src/Clima_Func.c:830`  
  **Cosa osservare**: soglie heater/SSR e `persist_Temp_heater_off`
- **Passo**: regolazione estate  
  **Dove**: `src/Clima_Func.c:885`  
  **Cosa osservare**: condizioni cooler e blocco heater/SSR

## A7. Interfacce esterne (livello astratto)
### Segnali generati dallâ€™RD
- Comandi digitali attuatori clima (heater, cooler, preheater, DXD):
  - Pin MCU / periferica: uscita digitale/controllo accessori (astratta)
  - Tipo: GPIO / comando bus accessorio
  - Livello attivo: ON/OFF logico
  - Condizione di attivazione: superamento soglie e consensi
  - Persistenza: secondo isteresi, timer e stato allarmi
- Comando SSR/EHD:
  - Pin MCU / periferica: parametro di controllo memoria configurazione
  - Tipo: valore percentuale logico (`Set_EHD_mod`)
  - Livello attivo: 0..100
  - Condizione di attivazione: logica stagione e domanda termica
  - Persistenza: fino al prossimo ciclo regolazione

### Segnali letti dallâ€™RD
- Sonde temperatura:
  - Tipo ingresso: analogico/digitale acquisito in `sData.measure_Temp[...]`
  - Modalita lettura: campionamento periodico e confronto soglie
  - Frequenza campionamento: ciclo funzione (scheduler)
  - Condizione di validita: assenza allarmi e disponibilita canale
- Stato accessori/allarmi:
  - Tipo ingresso: stati digitali/logici
  - Modalita lettura: `DigitAccessoryOperating`, `CkAlarm`, status unit/motori
  - Frequenza campionamento: ogni chiamata
  - Condizione di validita: sincronismo con stato firmware

## A8. Catena causale (livello sistema)
| Livello | Descrizione |
|----------|------------|
| Decisione | Regole di regolazione termica e sicurezza |
| Interfaccia | Comandi ON/OFF o modulazione verso attuatori/bus |
| Elettronica esterna | Driver potenza e moduli attuazione |
| Attuatore | Resistenze, valvole, compressore, ventilazione correlata |
| Effetto | Variazione reale temperatura aria impianto |

## A9. Feedback disponibile

- Esiste conferma reale?
  - Parziale: usa stati accessori/allarmi e misure temperatura, ma i comandi ON/OFF sono mediati da task I2C separato.
- Il comando e "fire and forget"?
  - Parzialmente: invoca `TAG_DigitAccessoryOn/Off` senza attesa ack nella stessa funzione.
- Dove viene letto il feedback?
  - In `TemperatureRegulate()` su `DigitAccessoryOn/Operating`, allarmi e sonde (`src/Clima_Func.c:370`, `src/Clima_Func.c:431`, `src/Clima_Func.c:435`, `src/Clima_Func.c:647`).
- Quanto e affidabile?

Valori criteri:

- `Origine diretta: 1` (feedback da stato accessori e allarmi runtime)
- `Correlazione temporale: 0` (nessun timeout esplicito di conferma comando)
- `Correlazione univoca col comando: 0` (nessun legame per-istanza tra singolo comando e ack)
- `Gestione errore: 1` (spegnimento forzato su fault/allarmi e rami protettivi)
- `Punteggio totale: 2`
- `Classe finale: Medio`
## A10. Punti critici firmware
- Verificare condizioni con confronti stretti su soglie (maggiore/minore uguale) per evitare oscillazioni logiche.
- Verificare coerenza timer/contatori rispetto al periodo scheduler reale.
- Verificare priorità dei rami decisionali e dei ritorni anticipati.
- Verificare possibili ambiguità tra flag di stato e comandi calcolati nello stesso ciclo.
## A11. Punti di disaccoppiamento (sistema)
- Errore logico firmware:
  - Configurazione stagione/abilitazioni incoerente
  - Soglie/isteresi non allineate al comportamento atteso
  - Timer/persistenze non completate durante diagnosi rapida
- Errore interfaccia:
  - Comandi attuatore non propagati correttamente sul bus/interfaccia
  - Stato accessorio non sincronizzato con comando
- Errore elettrico esterno:
  - Driver o alimentazioni attuatori assenti
  - Cablaggio uscita danneggiato
- Errore meccanico:
  - Attuatore bloccato o inefficiente
  - Scambio termico reale insufficiente nonostante comando corretto

## A12. Limite di responsabilità firmware
Il firmware RD:
- Decide quando generare un segnale
- Imposta livelli logici
- Invia comandi su bus
- Monitora feedback disponibili
- Applica logiche di sicurezza

Il firmware RD NON garantisce:
- Integrita circuiti esterni
- Corretto cablaggio
- Presenza attuatore
- Effettiva esecuzione meccanica
- Corretta alimentazione esterna

