# Funzione `managementBypass`

# ðŸ”· SEZIONE A â€” DOCUMENTAZIONE TECNICA (ENGINEERING)

## A1. Nome funzione
`managementBypass(unsigned short enab_func)`

## A2. File e posizione
`src/Clima_Func.c:93`

## A3. Scopo tecnico
Decide il comando logico di apertura/chiusura bypass in base a configurazione, stato macchina, temperature, allarmi e input esterni, quindi attiva la procedura firmware di manovra bypass.

## A4. Parametri e ritorno
### Parametri in ingresso
- `enab_func`:
  - Tipo: `unsigned short` (bitmask funzioni abilitate)
  - Unita di misura: non applicabile
  - Range ammesso: 0..65535 (bit significativi secondo mappa firmware)
  - Valori invalidi gestiti: bit non previsti ignorati dalle condizioni specifiche

### Valore di ritorno
- Tipo: `void`
- Significato: nessun valore restituito; effetto su stato e comando bypass

## A5. Logica dettagliata
- Esce subito se e attivo test bypass.
- Legge la modalita bypass da EEPROM e seleziona il ramo operativo.
- Manuale: forza `cmd_bypass` a chiuso o aperto.
- Auto on/off: gestisce cicli temporizzati di freecooling con contatori ON/OFF e possibile cambio stato unita.
- Automatico: usa temperature return/fresh, setpoint, isteresi minima esterna, stato heater/cooler/DXD e allarmi sonde.
- Controllo esterno: interpreta ingresso analogico secondo modalita 12V-open o 0V-open.
- Applica filtri temporali con persistenza (`persist_temp_bypass_on`) e contatori freecooling.
- Esegue comando fisico solo se la posizione richiesta e diversa da quella attuale.

## A6. Mappa firmware con riferimenti puntuali
- **Passo**: blocco test bypass, uscita anticipata  
  **Dove**: `src/Clima_Func.c:103`  
  **Cosa osservare**: `sData.status_test & MSK_TEST_BYPASS`
- **Passo**: acquisizione configurazione bypass  
  **Dove**: `src/Clima_Func.c:107`  
  **Cosa osservare**: `cfg_bypass = read_byte_eeprom(...)`
- **Passo**: inizializzazione stato freecooling per modalita automatica/esterna/auto on-off  
  **Dove**: `src/Clima_Func.c:111`  
  **Cosa osservare**: `count_active_freecooling_off = 1`
- **Passo**: ramo manuale chiuso  
  **Dove**: `src/Clima_Func.c:126`  
  **Cosa osservare**: `cmd_bypass = 0`
- **Passo**: ramo manuale aperto  
  **Dove**: `src/Clima_Func.c:131`  
  **Cosa osservare**: `cmd_bypass = 1`
- **Passo**: ramo auto on/off con gestione standby e timer ON/OFF  
  **Dove**: `src/Clima_Func.c:136`  
  **Cosa osservare**: `count_active_freecooling_on`, `count_active_freecooling_off`, `standby_before`
- **Passo**: ramo automatico su temperature e allarmi  
  **Dove**: `src/Clima_Func.c:207`  
  **Cosa osservare**: `measure_Temp[...]`, `set_point_temp`, `persist_temp_bypass_on`
- **Passo**: ramo controllo esterno da input analogico  
  **Dove**: `src/Clima_Func.c:271`  
  **Cosa osservare**: `Set_Input[0/1]`, `measure_ADC_input[...]`, soglie `250` e `610`
- **Passo**: decisione finale manovra e attesa in base velocita motori  
  **Dove**: `src/Clima_Func.c:305`  
  **Cosa osservare**: `second_to_wait`, `cmd_bypass`
- **Passo**: comando apertura bypass  
  **Dove**: `src/Clima_Func.c:356`  
  **Cosa osservare**: `Active_Procedure_Bypass_OpCl(OPEN_BPD, 9)`
- **Passo**: comando chiusura bypass  
  **Dove**: `src/Clima_Func.c:358`  
  **Cosa osservare**: `Active_Procedure_Bypass_OpCl(CLOSE_BPD, 9)`

## A7. Interfacce esterne (livello astratto)
### Segnali generati dallâ€™RD
- Attivazione manovra bypass tramite procedura firmware:
  - Pin MCU / periferica: non diretto in questa funzione (instradato da `Active_Procedure_Bypass_OpCl`)
  - Tipo: comando digitale logico verso catena attuatore
  - Livello attivo: `OPEN_BPD` o `CLOSE_BPD`
  - Condizione di attivazione: `cmd_bypass` diverso da stato corrente
  - Persistenza: impulso di comando con tempo interno procedura

### Segnali letti dallâ€™RD
- Temperature fresh/return:
  - Tipo ingresso: sonde temperatura
  - Modalita lettura: campi `sData.measure_Temp[...]`
  - Frequenza campionamento: ciclo regolazione firmware
  - Condizione di validita: assenza allarmi sonde dedicate
- Input analogico esterno bypass:
  - Tipo ingresso: analogico
  - Modalita lettura: `sData.measure_ADC_input[...]` con soglie codificate
  - Frequenza campionamento: ciclo funzione
  - Condizione di validita: ingresso configurato come bypass

## A8. Catena causale (livello sistema)
| Livello | Descrizione |
|----------|------------|
| Decisione | Definizione `cmd_bypass` da logica firmware |
| Interfaccia | Comando logico OPEN/CLOSE verso procedura bypass |
| Elettronica esterna | Driver e catena potenza attuatore |
| Attuatore | Motore/servo serranda bypass |
| Effetto | Variazione percorso aria (bypass o scambio) |

## A9. Feedback disponibile
- Conferma reale: parziale (stato logico posizione bypass, non misura meccanica diretta in questa funzione)
- Tipo comando: principalmente fire-and-forget con verifica stato logico
- Dove viene letto: `src/Clima_Func.c:305` (confronto stato bypass), `src/Clima_Func.c:355` (decisione comando)
- Affidabilita: media; conferma logica alta, conferma meccanica dipende da catena esterna

## A10. Punti critici firmware
- Verificare condizioni con confronti stretti su soglie (maggiore/minore uguale) per evitare oscillazioni logiche.
- Verificare coerenza timer/contatori rispetto al periodo scheduler reale.
- Verificare priorità dei rami decisionali e dei ritorni anticipati.
- Verificare possibili ambiguità tra flag di stato e comandi calcolati nello stesso ciclo.
## A11. Punti di disaccoppiamento (sistema)
- Errore logico firmware:
  - Configurazione bypass non coerente con modalita attesa
  - Allarmi/condizioni che inibiscono apertura
  - Input esterno non mappato correttamente
- Errore interfaccia:
  - Procedura comando bypass non eseguita correttamente
  - Segnale ingresso analogico fuori soglia utile
- Errore elettrico esterno:
  - Driver attuatore non alimentato
  - Cablaggio comando/ritorno interrotto
- Errore meccanico:
  - Serranda bloccata
  - Attuatore bloccato o usurato

## A12. Limite di responsabilità firmware
Il firmware RD:
- Decide quando generare un segnale
- Imposta livelli logici
- Monitora feedback disponibili
- Applica logiche di sicurezza

Il firmware RD NON garantisce:
- Integrita circuiti esterni
- Corretto cablaggio
- Presenza attuatore
- Effettiva esecuzione meccanica
- Corretta alimentazione esterna

# ðŸŸ¢ SEZIONE B â€” DOCUMENTAZIONE NON TECNICA (OPERATIVA / CAMPO)

## B1. Racconto operativo
Il sistema decide quando aprire o chiudere il bypass aria in base alla modalita impostata. In manuale mantiene una posizione fissa. In automatico valuta condizioni termiche e apre solo quando e utile al bilancio energetico. Con comando esterno segue il livello del segnale ingresso. Sono previsti ritardi e persistenze per evitare continui cambi posizione.

## B2. Comportamento normale vs percezione anomala
E normale che la serranda non cambi subito al primo cambio temperatura: la logica usa filtri temporali. In modalita freecooling auto on/off l'unita puo riattivarsi dopo una pausa; puo sembrare anomalo ma e comportamento previsto.

## B3. Errori bloccanti a monte
- Logica non autorizzata: modalita test bypass attiva, funzione non abilitata
- Logica in protezione: allarmi sonde fresh/return, condizioni termiche non idonee
- Logica attiva ma attuatore non funzionante: comando presente, nessun movimento reale per guasto catena esterna

## B4. Checklist problem solving
1. Cosa dovrebbe accadere: bypass aperto o chiuso secondo modalita attiva
2. Dati reali disponibili: temperature fresh/return, stato bypass, livello ingresso esterno
3. Modalita attiva corretta?
4. Consensi presenti?
5. Soglie rispettate?
6. Timer completati?
7. Allarmi attivi?
8. Comando generato?
9. Segnale elettrico presente?
10. Effetto fisico osservato?

Separazione diagnosi:
- Problema configurazione
- Problema elettrico
- Problema meccanico
- Problema installazione

## B5. Nota gestionale (facoltativa)
L'attribuzione della responsabilita non puo essere dedotta dal solo esito firmware: richiede verifica tecnica complessiva di logica, interfaccia e campo.

