# Funzione `motor_speed`

# 🔷 SEZIONE A — DOCUMENTAZIONE TECNICA (ENGINEERING)

## A1. Nome funzione
`motor_speed()`

## A2. File e posizione
`src/motor_speed.c:2314`

## A3. Scopo tecnico
Calcola e aggiorna i comandi di velocita dei motori `R` (return) e `F` (fresh) ad ogni ciclo scheduler (~1.2 s), applicando priorita operative, modalita configurate, condizioni sicurezza, input esterni e correzioni da sensori aria.

## A4. Parametri e ritorno
### Parametri in ingresso
- Nessuno

### Valore di ritorno
- Tipo: `int`
- Significato: `1` in tutti i percorsi correnti (esecuzione completata/uscita anticipata)

## A5. Logica dettagliata
- Esegue controlli bloccanti iniziali (allarmi fan control, allarmi preheater acqua) con uscita immediata e arresto motori quando richiesto.
- Gestisce modalita MBF (controllo separato motori via Modbus) con ritorno immediato dopo impostazione comandi.
- Gestisce logica Clean Event su CO2 con fase forzata a velocita 80% per 30 minuti.
- Determina stato `unit_standby` da input, stato ON/OFF EEPROM e weekly.
- Se standby: porta unita in stato standby (`switchesToStandby`) e termina.
- Se uscita da standby: riattiva alimentazione elettronica motori (`PowerMode(POWER_ON)`), poi esce al ciclo successivo.
- In startup impone velocita minima maggiorata per evitare mancato avvio motori.
- In bypass in movimento forza entrambi i rami al minimo.
- Se DPP check attivo delega e termina.
- Se non controllata da input analogico, applica regolazione base:
  - CAP (pressione costante) oppure CAF (portata costante) oppure CFS (velocita costante)
  - gestione variante weekly o impostazione manuale indice step.
- Applica sovraregolazioni prioritarie quando consentite:
  - Defrost
  - Qualita aria VOC/RH/CO2 con incremento progressivo o saturazione
  - Boost temporizzato
  - Imbalance tra rami F/R.
- Applica depotenziamento per categoria motori (se non CAP/CAF), clamp minimo velocita, regola speciale damperless.
- Scrive i comandi finali in `sData.speed_motors_R` e `sData.speed_motors_F`.

## A6. Mappa firmware con riferimenti puntuali
- **Passo**: ingresso funzione  
  **Dove**: `src/motor_speed.c:2314`  
  **Cosa osservare**: dichiarazioni variabili locali e contesto ciclo
- **Passo**: blocco fan control KO  
  **Dove**: `src/motor_speed.c:2328`  
  **Cosa osservare**: `CkAlarm(ALM_EB_CTRL_FAN)` con uscita anticipata
- **Passo**: blocco sicurezza preheater acqua  
  **Dove**: `src/motor_speed.c:2336`  
  **Cosa osservare**: allarmi `ALM_PEH_*` e forzatura velocita a `0`
- **Passo**: lettura funzioni abilitate  
  **Dove**: `src/motor_speed.c:2356`  
  **Cosa osservare**: `enab_func = read_word_eeprom(...)`
- **Passo**: ramo MBF (controllo via Modbus)  
  **Dove**: `src/motor_speed.c:2359`  
  **Cosa osservare**: `ENAB_MBF`, set `Set_MBF_return/Set_MBF_fresh`, return
- **Passo**: Clean Event e forzatura 80%  
  **Dove**: `src/motor_speed.c:2390`  
  **Cosa osservare**: `ENAB_CLEAN_EVENT`, `Clean_Event_Counter`, comandi a `80*2.55`
- **Passo**: acquisizione stato input comando  
  **Dove**: `src/motor_speed.c:2435`  
  **Cosa osservare**: `status_inputs = ctrl_comand_inputs()`
- **Passo**: definizione standby da input/weekly/eeprom  
  **Dove**: `src/motor_speed.c:2455`  
  **Cosa osservare**: `unit_standby`, `Set_Power_ON`, `status_weekly`
- **Passo**: transizione standby ON/OFF  
  **Dove**: `src/motor_speed.c:2555`  
  **Cosa osservare**: `switchesToStandby()`, `PowerMode(POWER_ON)`
- **Passo**: minima startup e stato RUN  
  **Dove**: `src/motor_speed.c:2577`  
  **Cosa osservare**: `count_switch_on`, `minSpeedMotors`, `POS_BIT_UNIT_RUN`
- **Passo**: bypass in manovra -> velocita minima  
  **Dove**: `src/motor_speed.c:2592`  
  **Cosa osservare**: `POS_BIT_BYPASS_RUN`
- **Passo**: selezione regolazione base CAP/CAF/CFS  
  **Dove**: `src/motor_speed.c:2616`  
  **Cosa osservare**: chiamate `Func_Pressure_Const*`, `Func_AirFlow_Const`, `Func_SpeedFan_Const`
- **Passo**: attivazione logiche sensori aria  
  **Dove**: `src/motor_speed.c:2716`  
  **Cosa osservare**: `probeQAirActive()` e ramo VOC/RH/CO2
- **Passo**: regolazione VOC  
  **Dove**: `src/motor_speed.c:2729`  
  **Cosa osservare**: `SetPoint_VOC`, `POS_BIT_MAX_VOC`, calcolo `spd_1`
- **Passo**: regolazione RH  
  **Dove**: `src/motor_speed.c:2790`  
  **Cosa osservare**: `SetPoint_RH`, `POS_BIT_MAX_RH`, calcolo `diff`
- **Passo**: regolazione CO2  
  **Dove**: `src/motor_speed.c:2850`  
  **Cosa osservare**: `SetPoint_CO2`, `SetPoint_Airflow_CO2`, `POS_BIT_MAX_CO2`
- **Passo**: boost temporizzato  
  **Dove**: `src/motor_speed.c:2957`  
  **Cosa osservare**: `ENAB_BOOST`, `count_Boost_on`, spegnimento automatico boost
- **Passo**: imbalance F/R  
  **Dove**: `src/motor_speed.c:2991`  
  **Cosa osservare**: `ENAB_IMBALANCE`, `Set_Imbalance[0]`, `POS_BIT_IMBALANCE_OPERATING`
- **Passo**: depotenziamento e clamp minimo  
  **Dove**: `src/motor_speed.c:3058`  
  **Cosa osservare**: `calcDepot()`, limiti minimi `minSpeedMotors`
- **Passo**: regola damperless e commit finale velocita  
  **Dove**: `src/motor_speed.c:3123`  
  **Cosa osservare**: `isDamperLessUnit()`, scrittura `sData.speed_motors_R/F`

## A7. Interfacce esterne (livello astratto)
### Segnali generati dall'RD
- Comandi velocita ramo return e fresh:
  - Pin MCU / periferica: uscite comando motori (astratte nel file)
  - Tipo: PWM/0-10V equivalente (scala interna 0..255)
  - Livello attivo: `sData.speed_motors_R`, `sData.speed_motors_F`
  - Condizione di attivazione: calcolo funzione `motor_speed()`
  - Persistenza: fino al ciclo successivo o sovrascrittura da logica prioritaria
- Comando alimentazione elettronica motori:
  - Pin MCU / periferica: relay potenza motori
  - Tipo: GPIO (via `PowerMode` e `ioport_set_pin_level`)
  - Livello attivo: ON/OFF
  - Condizione di attivazione: uscita da standby / gestione run
  - Persistenza: secondo stato macchina

### Segnali letti dall'RD
- Ingressi digitali/analogici di comando:
  - Tipo ingresso: input configurabili
  - Modalita lettura: `ctrl_comand_inputs()`, `measure_ADC_input`
  - Frequenza campionamento: ciclo funzione
  - Condizione di validita: input configurato e canale valido
- Misure sensori aria e stato accessori:
  - Tipo ingresso: valori RH/CO2/VOC + stati operativita accessori
  - Modalita lettura: `probeQAirActive()`, `DigitAccessoryOperating`, `CkAlarm`
  - Frequenza campionamento: ciclo funzione
  - Condizione di validita: accessori operativi, test mode non attivo

## A8. Catena causale (livello sistema)
| Livello | Descrizione |
|----------|------------|
| Decisione | Logica firmware prioritaria CAP/CAF/CFS + safety + IAQ |
| Interfaccia | Comandi velocita motori e stato power motori |
| Elettronica esterna | Driver/inverter motori |
| Attuatore | Motori ventilazione return/fresh |
| Effetto | Portata/pressione aria risultante in unita |

## A9. Feedback disponibile
- Se esiste conferma reale: parziale
- Tipo comando: comando diretto con feedback indiretto
- Dove viene letto il feedback: `src/motor_speed.c:2328` (allarme controllo), `src/motor_speed.c:2716` (dati IAQ), `src/motor_speed.c:3058` (adattamento comando)
- Affidabilita del feedback: media; robusta su stati/flag interni, dipende da qualitÃ  misure esterne

## A10. Punti critici firmware
- Verificare condizioni con confronti stretti su soglie (maggiore/minore uguale) per evitare oscillazioni logiche.
- Verificare coerenza timer/contatori rispetto al periodo scheduler reale.
- Verificare priorità dei rami decisionali e dei ritorni anticipati.
- Verificare possibili ambiguità tra flag di stato e comandi calcolati nello stesso ciclo.
## A11. Punti di disaccoppiamento (sistema)
- Errore logico firmware
  - Priorita operative in conflitto (CAP/CAF/BOOST/IMBALANCE)
  - Setpoint EEPROM non validi e fallback attivati
  - Early return che inibisce rami successivi (es. standby, bypass run, MBF)
- Errore interfaccia
  - Comando velocita scritto ma non applicato da stadio driver
  - Ingressi di comando non coerenti con configurazione
- Errore elettrico esterno
  - RelÃ© potenza motori non eccitato correttamente
  - Alimentazione/inverter motori assente o degradata
- Errore meccanico
  - Motori bloccati o ventilazione inefficace nonostante comando corretto

## A12. Limite di responsabilità firmware
Il firmware RD:
- Decide quando generare il comando velocita
- Imposta i livelli logici/PWM interni
- Applica sicurezza e priorita operative
- Usa feedback disponibili per adattamento

Il firmware RD NON garantisce:
- Integrita hardware esterno di potenza
- Corretto cablaggio campo
- Presenza o efficienza meccanica motori/ventilatori
- Effettiva portata fisica se catena esterna e degradata

# 🟢 SEZIONE B — DOCUMENTAZIONE NON TECNICA (OPERATIVA / CAMPO)

## B1. Racconto operativo
Questa funzione decide a che velocita devono girare i due ventilatori. Parte da una velocita base (manuale/weekly/CAP/CAF) e poi applica correzioni: sicurezza, input esterni, qualita aria, boost e sbilanciamento tra mandata/estrazione.

## B2. Comportamento normale vs percezione anomala
- E normale vedere velocita bloccata al minimo durante manovra bypass o in startup.
- E normale vedere un ramo diverso dall'altro quando imbalance e attivo.
- E normale che in standby i motori restino fermi anche se i setpoint sono validi.
- Puo sembrare anomalo il passaggio a velocita alta, ma puo essere dovuto a VOC/RH/CO2 o boost.

## B3. Errori bloccanti a monte
- Logica non autorizzata
  - Unita in standby da input/weekly/power
  - Controllo MBF che sovrascrive la logica locale
- Logica in protezione
  - Allarmi fan control o allarmi preheater acqua
  - Modalita che impongono early return
- Logica attiva ma attuatore non funzionante
  - Comando aggiornato in firmware senza risposta reale motore

## B4. Checklist problem solving
1. Cosa dovrebbe accadere: velocita attesa su ramo return/fresh
2. Dati reali disponibili: speed command R/F, stato RUN/STANDBY, status allarmi
3. Modalita attiva corretta?
4. Consensi presenti?
5. Soglie rispettate?
6. Timer completati?
7. Allarmi attivi?
8. Comando generato?
9. Segnale elettrico presente?
10. Effetto fisico osservato?

Separare:
- Problema configurazione
- Problema elettrico
- Problema meccanico
- Problema installazione

## B5. Nota gestionale (facoltativa)
La responsabilita va attribuita distinguendo sempre decisione firmware, applicazione elettrica del comando e risultato meccanico reale.

