# Funzione `isteresi` (logica trasversale)

# SEZIONE A - DOCUMENTAZIONE TECNICA (ENGINEERING)

## A1. Nome funzione
`isteresi` (logica, non funzione C dedicata)

## A2. File e posizione
Riferimenti principali:
- `src/Clima_Func.c:810` (soglie heater/cooler)
- `src/Clima_Func.c:836` (ON/OFF heater)
- `src/Clima_Func.c:897` (ON/OFF cooler)
- `src/motor_speed.c:2037` (soglie isteretiche in ramo Temp/CO2 speed)

## A3. Scopo tecnico
Stabilizzare le commutazioni evitando ON/OFF ripetuti vicino al setpoint: l'attivazione e la disattivazione usano soglie diverse (banda isteretica).

## A4. Parametri e ritorno
### Parametri in ingresso
- `SetPointTemp`, `SetPointTemp1`
- `hister_Temp_Hot[0/1]`, `hister_Temp_Cold[0/1]` (EEPROM)
- `TempRif` e misure sonde correlate

### Valore di ritorno
- Non applicabile (logica interna ad altre funzioni).

## A5. Logica dettagliata
- Calcola soglie separate ON/OFF:
  - `soglia_On_Heater`, `soglia_Off_Heater`
  - `soglia_On_Cooler`, `soglia_Off_Cooler`
- **Winter/Heater**:
  - ON quando `TempRif < soglia_On_Heater`
  - OFF quando `TempRif >= soglia_Off_Heater`
- **Summer/Cooler**:
  - ON quando `TempRif >= soglia_On_Cooler` (con vincolo su differenza fresh/return)
  - OFF quando `TempRif < soglia_Off_Cooler` (o condizione fresh/return sfavorevole)
- Include persistenza di spegnimento heater (`persist_Temp_heater_off`) per evitare commutazioni immediate.

## A6. Mappa firmware con riferimenti puntuali
- **Passo**: lettura isteresi da EEPROM
  **Dove**: `src/Clima_Func.c:810`
  **Cosa osservare**: `hister_Temp_Hot[]`, `hister_Temp_Cold[]`
- **Passo**: calcolo soglie heater/cooler
  **Dove**: `src/Clima_Func.c:813`
  **Cosa osservare**: `soglia_On_*`, `soglia_Off_*`
- **Passo**: commutazione heater con isteresi
  **Dove**: `src/Clima_Func.c:836`
  **Cosa osservare**: confronti `TempRif` vs soglie + persistenza OFF
- **Passo**: commutazione cooler con isteresi
  **Dove**: `src/Clima_Func.c:897`
  **Cosa osservare**: confronti `TempRif` vs soglie + vincoli temperatura aria esterna
- **Passo**: uso soglie in controllo proporzionale temp/CO2
  **Dove**: `src/motor_speed.c:2037`
  **Cosa osservare**: `soglia_On_Cooler`, `soglia_Off_Cooler`

## A7. Interfacce esterne (livello astratto)
### Segnali generati dall'RD
- ON/OFF heater/cooler (direttamente o via catena accessori)

### Segnali letti dall'RD
- Setpoint e isteresi da EEPROM
- Temperature di riferimento e sonde fresh/return/supply

## A8. Catena causale (livello sistema)
| Livello | Descrizione |
|----------|------------|
| Decisione | Confronto misura vs soglie ON/OFF separate |
| Interfaccia | Comando ON/OFF attuatori clima |
| Elettronica esterna | Driver riscaldamento/raffreddamento |
| Attuatore | Heater/Cooler |
| Effetto | Riduzione oscillazioni e commutazioni frequenti |

## A9. Feedback disponibile
- Esiste conferma reale?
  - Parziale: conferma principale da misura temperatura e stato accessori.
- Il comando e "fire and forget"?
  - No: i confronti vengono rivalutati ciclicamente.
- Dove viene letto il feedback?
  - In `TempRif`, sonde e stato accessori (`src/Clima_Func.c:836`, `src/Clima_Func.c:897`).
- Quanto e affidabile?

Valori criteri:
- `Origine diretta: 1`
- `Correlazione temporale: 0`
- `Correlazione univoca col comando: 1`
- `Gestione errore: 1`
- `Punteggio totale: 3`
- `Classe finale: Forte`

## A10. Punti critici firmware
- Configurazioni isteresi invertite o troppo strette possono causare instabilita.
- Deriva/sfasamento sensori altera i punti di commutazione.
- Interazioni con logiche prioritarie (bypass, safety, weekly) possono mascherare l'effetto atteso.

## A11. Punti di disaccoppiamento (sistema)
- Errore logico firmware: soglie calcolate/interpretate in modo incoerente.
- Errore interfaccia: stato accessori non aggiornato tempestivamente.
- Errore elettrico esterno: attuatore non risponde al comando ON/OFF.
- Errore meccanico: risposta termica impianto lenta o non coerente.

## A12. Limite di responsabilita firmware
Il firmware RD:
- applica le soglie isteretiche configurate
- decide commutazione ON/OFF in base alle misure

Il firmware RD NON garantisce:
- risposta termica immediata dell'impianto
- accuratezza assoluta delle sonde
- risultato fisico corretto in presenza di guasti esterni
