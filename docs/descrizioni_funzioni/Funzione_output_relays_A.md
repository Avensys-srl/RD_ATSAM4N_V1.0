# Funzione `Gestione_Relays` (output)

# SEZIONE A - DOCUMENTAZIONE TECNICA (ENGINEERING)

## A1. Nome funzione
`Gestione_Relays()`

## A2. File e posizione
`src/HW_func.c:709`

## A3. Scopo tecnico
Pilota i due relay di uscita (`Out1`, `Out2`) in base alla configurazione EEPROM `Set_Output[0..1]`, applicando logiche evento (bypass, fault, run, valvola, summer/winter, max speed) e inversione di polarita logica.

## A4. Parametri e ritorno
### Parametri in ingresso
- Nessuno (usa stato/configurazione globale)

### Valore di ritorno
- Tipo: `int`
- Significato: ritorna `1` a fine ciclo (funzione scheduler)

## A5. Logica dettagliata
- Se e attivo test output (`MSK_TEST_OUTPUT`) esce senza modificare relays.
- Per ciascun relay `i`:
  - legge `cfg_relay = Set_Output[i]`.
  - ricava inversione con bit7 (`0x80`):
    - bit7=0: relay eccitato quando condizione vera.
    - bit7=1: relay diseccitato quando condizione vera.
  - valuta `condizione_on` in base a `cfg_relay & 0x07`:
    - `0`: disable
    - `1`: bypass open
    - `2`: common fault
    - `3`: unit run
    - `4`: pilotaggio valvola con AWP/setpoint e stagione
    - `5`: estate/inverno con persistenza su T fresh vs soglia esterna
    - `6`: max speed raggiunta
- Scrive stato fisico relay su pin `pOut1/pOut2` e aggiorna bit corrispondente in `sData.StatusOutput`.

## A6. Mappa firmware con riferimenti puntuali
- **Passo**: ingresso funzione output
  **Dove**: `src/HW_func.c:709`
  **Cosa osservare**: `int Gestione_Relays()`
- **Passo**: bypass test output
  **Dove**: `src/HW_func.c:718`
  **Cosa osservare**: `MSK_TEST_OUTPUT`
- **Passo**: lettura configurazione relay
  **Dove**: `src/HW_func.c:737`
  **Cosa osservare**: `Set_Output[i]`
- **Passo**: decodifica condizione tipo 1..6
  **Dove**: `src/HW_func.c:744`
  **Cosa osservare**: `switch (cfg_relay & 0x07)`
- **Passo**: logica estate/inverno con persistenze
  **Dove**: `src/HW_func.c:787`
  **Cosa osservare**: `count_sum`, `count_win`, `output_on`
- **Passo**: logica max speed
  **Dove**: `src/HW_func.c:840`
  **Cosa osservare**: confronto `speed_motors_*` con `MaxSpeed`
- **Passo**: pilotaggio pin fisici output
  **Dove**: `src/HW_func.c:851`
  **Cosa osservare**: `ioport_set_pin_level(pOut1/pOut2, ...)`
- **Passo**: aggiornamento stato output runtime
  **Dove**: `src/HW_func.c:857`
  **Cosa osservare**: bit `sData.StatusOutput`

## A7. Interfacce esterne (livello astratto)
### Segnali generati dall'RD
- `Out1`, `Out2`:
  - Tipo: uscite relay digitali
  - Livello attivo: eccitato/diseccitato secondo configurazione e inversione
  - Condizione di attivazione: eventi runtime (bypass/fault/run/valvola/stagione/max speed)
  - Persistenza: fino al prossimo ciclo valutazione

### Segnali letti dall'RD
- Config EEPROM: `Set_Output[]`, setpoint, `Bypass_minTempExt`
- Stato macchina: `status_unit`, `status_Leds_ctrlFan`, velocita motori
- Misure: `measure_AWP_max`, `measure_Temp[I_PROBE_FRESH]`
- Funzioni abilitate: `Enab_Fuction`

## A8. Catena causale (livello sistema)
| Livello | Descrizione |
|----------|------------|
| Decisione | Selezione condizione evento per ogni relay |
| Interfaccia | Comando digitale `Out1/Out2` |
| Elettronica esterna | Bobina relay / carico collegato |
| Attuatore | Dispositivo a valle del contatto relay |
| Effetto | Segnalazione o comando esterno ON/OFF |

## A9. Feedback disponibile
- Esiste conferma reale?
  - Debole: conferma firmware su `sData.StatusOutput`; nessun feedback elettrico del contatto realmente commutato.
- Il comando e "fire and forget"?
  - Si, lato uscita fisica: setta pin relay e continua.
- Dove viene letto il feedback?
  - Stato logico interno `sData.StatusOutput` (`src/HW_func.c:857`).
- Quanto e affidabile?

Valori criteri:
- `Origine diretta: 0`
- `Correlazione temporale: 0`
- `Correlazione univoca col comando: 1`
- `Gestione errore: 0`
- `Punteggio totale: 1`
- `Classe finale: Debole`

## A10. Punti critici firmware
- Configurazione inversione bit7 non intuitiva puo invertire il comportamento atteso.
- Modalita `Estate/Inverno` usa persistenze (`count_sum/count_win`) che introducono ritardo deliberato.
- Modalita `Valvola` dipende da `ENAB_SUMMER` e da misura AWP coerente.
- Mancanza feedback elettrico reale puo mascherare guasti di uscita/relay.

## A11. Punti di disaccoppiamento (sistema)
- Errore logico firmware: evento corretto ma mappatura output errata.
- Errore interfaccia: pin pilotato ma relay esterno non cablato correttamente.
- Errore elettrico esterno: bobina relay o alimentazione uscita assente.
- Errore meccanico: contatto relay bloccato/incollato.

## A12. Limite di responsabilita firmware
Il firmware RD:
- decide lo stato logico di `Out1/Out2`
- pilota i pin relay secondo configurazione evento
- aggiorna `sData.StatusOutput`

Il firmware RD NON garantisce:
- continuita contatto elettrico reale sul campo
- stato meccanico del relay esterno
- corretto cablaggio del carico a valle
