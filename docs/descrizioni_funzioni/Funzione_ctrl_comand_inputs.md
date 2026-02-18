# Funzione `ctrl_comand_inputs`

# 🔷 SEZIONE A — DOCUMENTAZIONE TECNICA (ENGINEERING)

## A1. Nome funzione
`ctrl_comand_inputs()`

## A2. File e posizione
`src/motor_speed.c:622`

## A3. Scopo tecnico
Interpreta gli ingressi configurabili e restituisce l'azione operativa (`RUN/STOP/allarme/input airflow`), includendo la gestione `summer/winter` su Input 2 tramite aggiornamento dei bit `ENAB_SUMMER` e `ENAB_WINTER`.

## A4. Parametri e ritorno
### Parametri in ingresso
- Nessuno

### Valore di ritorno
- Tipo: `byte`
- Significato:
  - `0x00` nessun input attivo
  - `0x10` run da input ON/OFF
  - `0x11` stop unita
  - `0x80` run da regolazione airflow input
  - `0x81` stop da regolazione airflow/fire
  - `0x82` fire: entrambi i rami max
  - `0x83` fire: estrazione max, immissione off
  - `0x84` fire: immissione max, estrazione off

## A5. Logica dettagliata
- Legge configurazione Input1/Input2 da EEPROM.
- Valuta priorita input di stop/fire/regolazione airflow.
- Imposta eventuali comandi velocita diretti da input analogico.
- Aggiorna stato run unit (`POS_BIT_UNIT_RUN`) in base ad `action_input`.
- Esegue gestione PIR (se configurato).
- Se Input2 e configurato come `INP_SUMMER_WINTER` o `INP_WINTER_SUMMER`:
  - usa soglia ADC (`measure_ADC_input[1] > 150`) per decidere stagione richiesta.
  - abilita stagione solo se accessori coerenti sono presenti/operativi.
  - aggiorna `Enab_Fuction` in EEPROM: set bit stagione richiesta, reset bit opposto.
  - incrementa `cntUpdate_info` solo in caso di cambio reale.

## A6. Mappa firmware con riferimenti puntuali
- **Passo**: ingresso funzione e lettura setup input  
  **Dove**: `src/motor_speed.c:622`  
  **Cosa osservare**: `Set_Input[0]`, `Set_Input[1]`, `manual_reset`
- **Passo**: codifica stato base action input  
  **Dove**: `src/motor_speed.c:653`  
  **Cosa osservare**: assegnazioni `action_input = 0x10/0x80/0x00`
- **Passo**: gestione input stop/run e airflow analogico  
  **Dove**: `src/motor_speed.c:667`  
  **Cosa osservare**: ramo `INP_0V_UNIT_STOP`, `INP_12V_UNIT_STOP`, `INP_REG_AIR_FLOW`
- **Passo**: gestione allarmi incendio da input  
  **Dove**: `src/motor_speed.c:692`  
  **Cosa osservare**: `INP_12V_ALM_FIRE_*`, `action_input 0x81..0x84`, allarmi smoke
- **Passo**: lettura `Enab_Fuction` per switch stagione  
  **Dove**: `src/motor_speed.c:776`  
  **Cosa osservare**: `enab_func = read_word_eeprom(...)`
- **Passo**: logica `INP_SUMMER_WINTER` (0V summer, 12V winter)  
  **Dove**: `src/motor_speed.c:778`  
  **Cosa osservare**: soglia `measure_ADC_input[1] > 150`, set/reset `ENAB_WINTER`/`ENAB_SUMMER`
- **Passo**: abilitazione winter condizionata da accessori disponibili  
  **Dove**: `src/motor_speed.c:782`  
  **Cosa osservare**: `ACC_I2C_DXD` o `ACC_I2C_HEATER` o `ACC_I2C_SSR`
- **Passo**: abilitazione summer condizionata da accessori disponibili  
  **Dove**: `src/motor_speed.c:797`  
  **Cosa osservare**: `ACC_I2C_DXD` o `ACC_I2C_COOLER`
- **Passo**: logica `INP_WINTER_SUMMER` (0V winter, 12V summer)  
  **Dove**: `src/motor_speed.c:813`  
  **Cosa osservare**: inversione semantica e set/reset bit stagione
- **Passo**: commit cambio stagione in EEPROM  
  **Dove**: `src/motor_speed.c:788`  
  **Cosa osservare**: `write_word_eeprom(Enab_Fuction)`, incremento `cntUpdate_info`
- **Passo**: update stato RUN finale  
  **Dove**: `src/motor_speed.c:922`  
  **Cosa osservare**: set/clear `POS_BIT_UNIT_RUN`
- **Passo**: ritorno codice azione input  
  **Dove**: `src/motor_speed.c:940`  
  **Cosa osservare**: `return action_input`
- **Passo**: punto di chiamata nella gestione motori  
  **Dove**: `src/motor_speed.c:2435`  
  **Cosa osservare**: `status_inputs = ctrl_comand_inputs()`

## A7. Interfacce esterne (livello astratto)
### Segnali generati dall'RD
- Aggiornamento configurazione stagione:
  - Pin MCU / periferica: non diretto (EEPROM config)
  - Tipo: scrittura parametri firmware (`Enab_Fuction`)
  - Livello attivo: bit `ENAB_SUMMER` / `ENAB_WINTER`
  - Condizione di attivazione: input 2 configurato e stato ADC coerente
  - Persistenza: fino a nuova commutazione o modifica config
- Comando logico velocita da input (se `INP_REG_AIR_FLOW`):
  - Tipo: comando interno `temp_speed_motorsR/F`
  - Livello attivo: 52..255 (o stop sotto soglia)
  - Persistenza: ciclo corrente

### Segnali letti dall'RD
- Input analogici/digitali configurabili:
  - Tipo ingresso: ADC
  - Modalita lettura: `sData.measure_ADC_input[i]`
  - Frequenza campionamento: ciclo funzione
  - Condizione di validita: mapping input attivo in EEPROM
- Stato accessori clima:
  - Tipo ingresso: stato operativo accessori
  - Modalita lettura: `DigitAccessoryOperating(...)`
  - Frequenza campionamento: ciclo funzione
  - Condizione di validita: accessorio presente e operativo

## A8. Catena causale (livello sistema)
| Livello | Descrizione |
|----------|------------|
| Decisione | Decodifica input + selezione stagione summer/winter |
| Interfaccia | Aggiornamento bit funzione in EEPROM e status run |
| Elettronica esterna | Ingressi cablati che forniscono livello 0V/12V equivalente ADC |
| Attuatore | Non diretto in questa funzione (influenza logiche clima/motori a valle) |
| Effetto | Attivazione logica stagione e comportamento coerente caldo/freddo |

## A9. Feedback disponibile
- Se esiste conferma reale: conferma logica interna, non conferma fisica attuatore
- Se il comando e fire-and-forget: parzialmente, con conferma su bit EEPROM aggiornati
- Dove viene letto il feedback: `src/motor_speed.c:776`, `src/motor_speed.c:788`, `src/motor_speed.c:803`, `src/motor_speed.c:838`
- Affidabilita del feedback: alta lato configurazione firmware; dipende da integrita input ADC lato campo

## A10. Punti critici firmware
- Verificare condizioni con confronti stretti su soglie (maggiore/minore uguale) per evitare oscillazioni logiche.
- Verificare coerenza timer/contatori rispetto al periodo scheduler reale.
- Verificare priorità dei rami decisionali e dei ritorni anticipati.
- Verificare possibili ambiguità tra flag di stato e comandi calcolati nello stesso ciclo.
## A11. Punti di disaccoppiamento (sistema)
- Errore logico firmware
  - Input2 non configurato in modalita summer/winter, nessuna commutazione
  - Accessori richiesti non operativi, cambio stagione non applicato
- Errore interfaccia
  - Livello ADC vicino soglia (150) con commutazioni non stabili
  - Configurazione input incoerente con cablaggio reale
- Errore elettrico esterno
  - Segnale ingresso assente/rumoroso
  - Alimentazioni esterne non coerenti con 0V/12V attesi
- Errore meccanico
  - Non applicabile direttamente qui; effetti emergono nelle logiche attuatori a valle

## A12. Limite di responsabilità firmware
Il firmware RD:
- Interpreta il livello ingresso per decidere la stagione
- Aggiorna i bit funzione summer/winter
- Impedisce attivazioni senza accessori coerenti

Il firmware RD NON garantisce:
- Correttezza cablaggio input campo
- Stabilita elettrica del segnale ingresso
- Esecuzione fisica caldo/freddo in assenza componenti esterni funzionanti

# 🟢 SEZIONE B — DOCUMENTAZIONE NON TECNICA (OPERATIVA / CAMPO)

## B1. Racconto operativo
Quando l'Input 2 e configurato per stagione, il sistema decide automaticamente se lavorare in estate o inverno in base al livello del segnale. Se cambia livello, aggiorna la modalita e disattiva l'altra.

## B2. Comportamento normale vs percezione anomala
- E normale che il cambio stagione non avvenga se manca un accessorio compatibile (es. cooler per summer, heater/SSR per winter).
- E normale vedere nessun cambio se Input 2 non e configurato in modalita stagione.
- Valori vicino soglia possono dare percezione di instabilita.

## B3. Errori bloccanti a monte
- Logica non autorizzata: input non configurato come summer/winter
- Logica in protezione: accessori stagionali non operativi
- Logica attiva ma attuatore non funzionante: stagione cambiata ma resa termica assente per guasti esterni

## B4. Checklist problem solving
1. Cosa dovrebbe accadere: switch estate/inverno da Input 2
2. Dati reali disponibili: valore ADC input 2, bit stagione attivi, stato accessori
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
L'attribuzione responsabilita richiede distinguere cambio stagione logico, disponibilita hardware e risposta reale impianto.

