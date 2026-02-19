# Funzione `unbalance_airflow` (logica trasversale)

# SEZIONE A - DOCUMENTAZIONE TECNICA (ENGINEERING)

## A1. Nome funzione
`unbalance_airflow` (blocco interno in `motor_speed()`)

## A2. File e posizione
`src/motor_speed.c:2994`

## A3. Scopo tecnico
Imposta uno sbilanciamento controllato tra portata di immissione (F) ed estrazione (R), modificando i due comandi velocita in base al parametro `Set_Imbalance` e alle priorita operative attive.

## A4. Parametri e ritorno
### Parametri in ingresso
- `enab_func` (bit `ENAB_IMBALANCE`)
- `sData.status_weekly` (RUN weekly + bit config imbalance)
- `Set_Imbalance[0]` (EEPROM, signed char, +/- %)
- `Set_StepMotorsCFS_CAF[idx]` (base speed di riferimento)
- Stati inibizione: `POS_BIT_DEFROST_OPERATING`, `ENAB_CAP`

### Valore di ritorno
- Non applicabile (logica interna in `motor_speed()`).

## A5. Logica dettagliata
- Esegue il ramo imbalance solo se:
  - Defrost non attivo
  - CAP non attivo
- Attiva imbalance quando almeno una delle seguenti e vera:
  - weekly RUN con configurazione imbalance nel profilo
  - bit `ENAB_IMBALANCE` abilitato
- Legge `Set_Imbalance[0]` e lo interpreta come valore signed (gestione complemento a due).
- Calcola un riferimento `ref` dal setpoint velocita base (`Set_StepMotorsCFS_CAF[idx] * 0.255`).
- Se `tempsb < 0` (maggiore estrazione):
  - `temp_speed_motorsR = ref`
  - `temp_speed_motorsF = (100 + tempsb)% * ref`
- Se `tempsb > 0` (maggiore immissione):
  - `temp_speed_motorsF = ref`
  - `temp_speed_motorsR = (100 - tempsb)% * ref`
- Se `tempsb == 0`: nessuno sbilanciamento, clear flag operativo.
- Imposta/azzera `POS_BIT_IMBALANCE_OPERATING` in base al caso.

## A6. Mappa firmware con riferimenti puntuali
- **Passo**: ingresso ramo imbalance con condizioni di inibizione
  **Dove**: `src/motor_speed.c:2992`
  **Cosa osservare**: check `DEFROST` e `ENAB_CAP`
- **Passo**: condizione attivazione da weekly o bit funzione
  **Dove**: `src/motor_speed.c:2994`
  **Cosa osservare**: `status_weekly` + `ENAB_IMBALANCE`
- **Passo**: lettura parametro sbilanciamento
  **Dove**: `src/motor_speed.c:3001`
  **Cosa osservare**: `Set_Imbalance[0]`
- **Passo**: conversione signed/complemento a due
  **Dove**: `src/motor_speed.c:3004`
  **Cosa osservare**: costruzione `tempsb`
- **Passo**: calcolo riferimento velocita base
  **Dove**: `src/motor_speed.c:3015`
  **Cosa osservare**: `ref = Set_StepMotorsCFS_CAF[idx] * 0.255`
- **Passo**: caso maggiore estrazione
  **Dove**: `src/motor_speed.c:3021`
  **Cosa osservare**: `tempsb < 0`, assegnazioni R/F
- **Passo**: caso maggiore immissione
  **Dove**: `src/motor_speed.c:3030`
  **Cosa osservare**: `tempsb > 0`, assegnazioni R/F
- **Passo**: gestione flag operativo imbalance
  **Dove**: `src/motor_speed.c:3027`
  **Cosa osservare**: set/clear `POS_BIT_IMBALANCE_OPERATING`

## A7. Interfacce esterne (livello astratto)
### Segnali generati dall'RD
- Due comandi velocita distinti:
  - `temp_speed_motorsR`
  - `temp_speed_motorsF`
- Stato runtime: bit `POS_BIT_IMBALANCE_OPERATING`

### Segnali letti dall'RD
- Configurazioni EEPROM (`Set_Imbalance`, setpoint base speed)
- Stato weekly e funzioni abilitate
- Stato logiche inibenti (Defrost/CAP)

## A8. Catena causale (livello sistema)
| Livello | Descrizione |
|----------|------------|
| Decisione | Calcolo sbilanciamento percentuale su ramo F o R |
| Interfaccia | Comandi velocita distinti per i due rami |
| Elettronica esterna | Driver motore estrazione/immissione |
| Attuatore | Ventilatori R/F |
| Effetto | Maggiore estrazione o maggiore immissione |

## A9. Feedback disponibile
- Esiste conferma reale?
  - Parziale: conferma logica dal bit operativo e dai comandi calcolati; feedback fisico dipende da rpm reali e impianto.
- Il comando e "fire and forget"?
  - No: viene rivalutato a ogni ciclo `motor_speed()`.
- Dove viene letto il feedback?
  - Nel ciclo motori tramite stati runtime e comandi R/F (`src/motor_speed.c:2994`, `src/motor_speed.c:3027`).
- Quanto e affidabile?

Valori criteri:
- `Origine diretta: 1` (comandi e stati runtime diretti)
- `Correlazione temporale: 0` (nessun timeout esplicito dedicato)
- `Correlazione univoca col comando: 1` (parametro `Set_Imbalance` dedicato)
- `Gestione errore: 1` (inibizione da Defrost/CAP e clear flag)
- `Punteggio totale: 3`
- `Classe finale: Forte`

## A10. Punti critici firmware
- Interpretazione signed di `Set_Imbalance` (complemento a due).
- Interazione con priorita superiori che possono sovrascrivere i comandi.
- Possibile saturazione/limitazione successiva da clamp/depotenziamento.

## A11. Punti di disaccoppiamento (sistema)
- Errore logico firmware: sbilanciamento applicato al ramo opposto.
- Errore interfaccia: comando velocita differenziato non tradotto correttamente dal driver.
- Errore elettrico esterno: un ramo non segue il comando.
- Errore meccanico: perdite/ostruzioni che annullano l'effetto previsto.

## A12. Limite di responsabilita firmware
Il firmware RD:
- calcola e applica lo sbilanciamento richiesto
- imposta flag operativo di imbalance
- inibisce imbalance quando richiesto da priorita superiori

Il firmware RD NON garantisce:
- sbilanciamento fluidodinamico reale in presenza di vincoli impianto
- comportamento simmetrico dei due rami in caso di guasti esterni
