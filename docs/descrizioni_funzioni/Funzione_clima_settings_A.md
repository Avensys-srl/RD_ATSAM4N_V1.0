# Funzione `clima_settings` (sintesi trasversale)

# SEZIONE A - DOCUMENTAZIONE TECNICA (ENGINEERING)

## A1. Nome funzione
`clima_settings` (documento di sintesi; non e una singola funzione C)

## A2. File e posizione
Riferimenti principali:
- `src/Clima_Func.c:370` (`TemperatureRegulate`)
- `src/Clima_Func.c:93` (`managementBypass`)
- `src/motor_speed.c:2314` (`motor_speed`)
- `src/motor_speed.c:622` (`ctrl_comand_inputs`)
- `src/Weekly_Prog.c:34` (`weekly_program`)
- `src/motor_speed.c:2957` (PARTY/BOOST)

## A3. Scopo tecnico
Definire e applicare i settaggi clima che governano: stagione estate/inverno, setpoint temperatura, isteresi, modalita ventilazione (CAP/CAF/CFS), boost/party, weekly, bypass e priorita operative tra logiche concorrenti.

## A4. Parametri e ritorno
### Parametri in ingresso
- Configurazioni EEPROM (`Enab_Fuction`, setpoint, isteresi, weekly, boost, bypass)
- Misure sonde (`measure_Temp`, sensori aria, ingressi ADC)
- Stato accessori I2C (heater, cooler, preheater, DXD, sensori)
- Stato macchina (`status_unit`, allarmi, mode bits)

### Valore di ritorno
- Non applicabile (sintesi multi-funzione).

## A5. Logica dettagliata
- **Abilitazioni funzione**: `Enab_Fuction` determina quali macro-funzioni sono attive (clima, weekly, boost, CAP/CAF, imbalance).
- **Stagione SUMMER/WINTER**: selezionata da configurazione o ingressi (`ctrl_comand_inputs`) e condizionata dalla presenza accessori compatibili.
- **Regolazione termica**: `TemperatureRegulate` decide ON/OFF/modulazione di preheater/heater/cooler/DXD con isteresi e condizioni di sicurezza.
- **Bypass**: `managementBypass` decide apertura/chiusura in base a modalita, temperature, allarmi, stato accessori e ingressi esterni.
- **Ventilazione**: `motor_speed` applica CAP/CAF/CFS e poi priorita (defrost, IAQ VOC/RH/CO2, boost, imbalance).
- **Weekly**: `weekly_program` abilita RUN per fasce orarie e pubblica profilo (speed/imbal/temp ref) in `status_weekly`.
- **Party/Boost**: forza velocita massima per durata `Set_TimeBoost` e si auto-disattiva a timeout.

## A6. Mappa firmware con riferimenti puntuali
- **Passo**: lettura funzioni abilitate
  **Dove**: `src/motor_speed.c:2356`
  **Cosa osservare**: `enab_func = read_word_eeprom(ADDR_EEP(Enab_Fuction))`
- **Passo**: commutazione stagione via input
  **Dove**: `src/motor_speed.c:780`
  **Cosa osservare**: set/reset `ENAB_WINTER` / `ENAB_SUMMER`
- **Passo**: regolazione clima principale
  **Dove**: `src/Clima_Func.c:370`
  **Cosa osservare**: chiamata e rami `TemperatureRegulate()`
- **Passo**: logica bypass
  **Dove**: `src/Clima_Func.c:93`
  **Cosa osservare**: selezione modalita `Config_Bypass`, comando OPEN/CLOSE
- **Passo**: profilo weekly runtime
  **Dove**: `src/Weekly_Prog.c:70`
  **Cosa osservare**: `sData.status_weekly = MSK_BIT_WEEKLY_ENAB` + `MSK_BIT_WEEKLY_RUN`
- **Passo**: boost/party temporizzato
  **Dove**: `src/motor_speed.c:2957`
  **Cosa osservare**: `ENAB_BOOST`, `Set_TimeBoost`, `count_Boost_on`, auto-clear bit
- **Passo**: priorita e clamp finali velocita
  **Dove**: `src/motor_speed.c:2616`
  **Cosa osservare**: CAP/CAF/CFS + override + `sData.speed_motors_R/F`

## A7. Interfacce esterne (livello astratto)
### Segnali generati dall'RD
- Comandi digitali accessori clima (heater/cooler/preheater/DXD)
- Comandi velocita ventilatori (PWM/0-10V equivalente)
- Comando bypass OPEN/CLOSE

### Segnali letti dall'RD
- Sonde temperatura/umidita/qualita aria
- Ingressi esterni configurabili (run/stop, stagione, airflow, bypass)
- Stato accessori/allarmi su bus I2C
- Configurazioni EEPROM (setpoint, tempo boost, weekly, funzioni abilitate)

## A8. Catena causale (livello sistema)
| Livello | Descrizione |
|----------|------------|
| Decisione | Fusione settaggi clima + stati runtime + priorita |
| Interfaccia | Comandi uscite clima, bypass e ventilazione |
| Elettronica esterna | Driver potenza, moduli accessori, sensori |
| Attuatore | Ventilatori, serranda bypass, attuatori caldo/freddo |
| Effetto | Comfort termico e qualita aria secondo configurazione |

## A9. Feedback disponibile
- Esiste conferma reale?
  - Parziale: conferme eterogenee per sottosistema (stati/allarmi/rpm/misure), non un unico feedback centralizzato.
- Il comando e "fire and forget"?
  - In parte: alcune azioni sono ad anello chiuso (regolazioni), altre sono comandi a task separati con conferma differita.
- Dove viene letto il feedback?
  - In `status_unit`, `status_weekly`, stati accessori I2C, allarmi, rpm e misure sonde.
- Quanto e affidabile?

Valori criteri:
- `Origine diretta: 1`
- `Correlazione temporale: 0`
- `Correlazione univoca col comando: 0`
- `Gestione errore: 1`
- `Punteggio totale: 2`
- `Classe finale: Medio`

## A10. Punti critici firmware
- Priorita tra logiche concorrenti (clima, bypass, weekly, boost, IAQ, safety).
- Coerenza tra configurazione EEPROM e disponibilita accessori reali.
- Effetti collaterali di early return nei loop di regolazione.
- Drift tempo scheduler su funzioni temporizzate (weekly/boost).

## A11. Punti di disaccoppiamento (sistema)
- Errore logico firmware: configurazioni incoerenti, precedenze non attese.
- Errore interfaccia: dati sensori/accessori non aggiornati o non sincronizzati.
- Errore elettrico esterno: driver/alim non coerenti con comando.
- Errore meccanico: attuatore non segue comando pur con logica corretta.

## A12. Limite di responsabilita firmware
Il firmware RD:
- applica i settaggi clima configurati
- decide i comandi di ventilazione, bypass e accessori clima
- gestisce temporizzazioni e priorita
- usa feedback disponibili per adattamento

Il firmware RD NON garantisce:
- corretto comportamento di dispositivi esterni guasti/non alimentati
- accuratezza assoluta sensori in campo
- risultato fisico finale in presenza di vincoli meccanici/installativi
