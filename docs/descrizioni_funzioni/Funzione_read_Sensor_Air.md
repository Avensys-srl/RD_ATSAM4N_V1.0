# Funzione `read_Sensor_Air`

# 🔷 SEZIONE A — DOCUMENTAZIONE TECNICA (ENGINEERING)

## A1. Nome funzione
`read_Sensor_Air(byte addrSlave, byte iAcc, byte byte_event, byte bit_event, byte msk_clear_event)`

## A2. File e posizione
`src/Accessory.c:1685`

## A3. Scopo tecnico
Acquisisce dati dai sensori aria digitali (CO2, RH, VOC, AWP) via bus, aggiorna le strutture firmware di misura/stato e allinea i bit evento/diagnostica usati dalle logiche a valle.

## A4. Parametri e ritorno
### Parametri in ingresso
- `addrSlave`:
  - Tipo: `byte`
  - Unita di misura: indirizzo slave bus
  - Range ammesso: range indirizzi supportati dal protocollo accessori
  - Valori invalidi gestiti: lettura fallita (`statusBus != 1`)
  - Effetto sul comportamento: seleziona il sensore interrogato
- `iAcc`:
  - Tipo: `byte`
  - Unita di misura: indice accessorio in `sData.AccI2C[]`
  - Range ammesso: indici accessori digitali validi
  - Valori invalidi gestiti: comportamento non definito se indice non coerente
  - Effetto sul comportamento: decide dove salvare le misure e quali rami specifici eseguire
- `byte_event`:
  - Tipo: `byte`
  - Unita di misura: indice byte array eventi
  - Range ammesso: indice valido in `sData.Events[]`
  - Valori invalidi gestiti: nessuna protezione locale
  - Effetto sul comportamento: byte evento aggiornato dall'allarme sensore
- `bit_event`:
  - Tipo: `byte`
  - Unita di misura: posizione bit
  - Range ammesso: 0..7
  - Valori invalidi gestiti: nessuna protezione locale
  - Effetto sul comportamento: posizione del bit allarme nel byte evento
- `msk_clear_event`:
  - Tipo: `byte`
  - Unita di misura: maschera bit
  - Range ammesso: maschera valida per clear bit target
  - Valori invalidi gestiti: nessuna protezione locale
  - Effetto sul comportamento: pulizia preventiva bit evento prima del set

### Valore di ritorno
- Tipo: `Byte`
- Significato: `1` se lettura sensore riuscita, `0` se lettura non valida/fallita
- Eventuale uso nel chiamante: usato dal polling accessori per determinare esito acquisizione del canale

## A5. Logica dettagliata
- Richiede al sensore 11 byte con `ReadSlave(addrSlave, 0x01, ...)`.
- Se la lettura riesce:
  - marca accessorio operativo e aggiorna stato alimentazione sensore (`sts` bit power).
  - aggiorna misure `measure1..measure4` e campo `data`.
  - per CO2_1 e CO2_2 mantiene storico 24h (48 campioni a 30 min) e imposta flag clean-event in base a delta max-min.
  - compone valori raw gas a 32 bit.
  - gestisce caso AWP: usa allarme temperatura e aggiorna anche `I_PROBE_RETURN` quando valido.
  - aggiorna bit allarme nel byte eventi (`sData.Events[byte_event]`).
  - aggiorna mappa `info_ProbesAD` (ambient/duct) in base a `data bit0`.
- Se la lettura fallisce:
  - non aggiorna ramo misure/eventi in questo blocco
  - ritorna `0`.

## A6. Mappa firmware con riferimenti puntuali
- **Passo**: ingresso funzione sensore aria  
  **Dove**: `src/Accessory.c:1685`  
  **Cosa osservare**: firma funzione e parametri
- **Passo**: richiesta dati al sensore via bus  
  **Dove**: `src/Accessory.c:1691`  
  **Cosa osservare**: `ReadSlave(addrSlave, 0x01, &buff_Data[0], 11)`
- **Passo**: verifica esito lettura  
  **Dove**: `src/Accessory.c:1693`  
  **Cosa osservare**: condizione `if(statusBus == 1)`
- **Passo**: marking accessorio operativo  
  **Dove**: `src/Accessory.c:1698`  
  **Cosa osservare**: `TAG_DigitAccessoryOperating(iAcc)`
- **Passo**: aggiornamento misure principali  
  **Dove**: `src/Accessory.c:1703`  
  **Cosa osservare**: `measure1..measure4`, `data`
- **Passo**: gestione storico/flag clean event CO2_1  
  **Dove**: `src/Accessory.c:1709`  
  **Cosa osservare**: `CO2_1_array`, `Index_CO2_1`, `Clean_Event_Flag1`
- **Passo**: composizione raw gas CO2_1  
  **Dove**: `src/Accessory.c:1749`  
  **Cosa osservare**: `CO2_1_Raw_Gas_Value`
- **Passo**: gestione storico/flag clean event CO2_2  
  **Dove**: `src/Accessory.c:1777`  
  **Cosa osservare**: `CO2_2_array`, `Index_CO2_2`, `Clean_Event_Flag2`
- **Passo**: composizione raw gas CO2_2  
  **Dove**: `src/Accessory.c:1817`  
  **Cosa osservare**: `CO2_2_Raw_Gas_Value`
- **Passo**: ramo AWP con aggiornamento return temperature  
  **Dove**: `src/Accessory.c:1845`  
  **Cosa osservare**: `iAcc == ACC_I2C_AWP`, `sData.measure_Temp[I_PROBE_RETURN]`
- **Passo**: aggiornamento bit evento allarme  
  **Dove**: `src/Accessory.c:1869`  
  **Cosa osservare**: `sData.Events[byte_event]`
- **Passo**: aggiornamento mappa ambient/duct sonde  
  **Dove**: `src/Accessory.c:1874`  
  **Cosa osservare**: set/clear bit in `sData.info_ProbesAD`
- **Passo**: ritorno esito funzione  
  **Dove**: `src/Accessory.c:1925`  
  **Cosa osservare**: `return risp`
- **Passo**: punto chiamata nel polling accessori  
  **Dove**: `src/Accessory.c:1024`  
  **Cosa osservare**: `risp = read_Sensor_Air(...)`

## A7. Interfacce esterne (livello astratto)
### Segnali generati
- Nessun comando attuatore diretto:
  - Pin MCU: non applicabile in questa funzione
  - Tipo: aggiornamento dati/stati firmware
  - Livello attivo: non applicabile
  - Condizione di attivazione: lettura sensore valida
  - Persistenza: fino al ciclo successivo

### Segnali letti
- Bus sensori aria:
  - Tipo ingresso: frame dati digitali (status, allarmi, misure)
  - Frequenza campionamento: dipende dal ciclo di polling accessori
  - Condizione di validita: `ReadSlave` con `statusBus == 1`

## A8. Catena causale (livello sistema)
| Livello | Descrizione |
|----------|------------|
| Decisione | Acquisizione e validazione logica dati sensore |
| Interfaccia | Lettura frame bus dal sensore indirizzato |
| Elettronica esterna | Sensore RH/CO2/VOC/AWP e linea bus |
| Attuatore | Non applicabile (funzione di misura) |
| Effetto | Dati e allarmi aggiornati per logiche clima/ventilazione |

## A9. Feedback disponibile
- Esiste conferma reale? Si, esito comunicazione (`statusBus`) e coerenza frame ricevuto
- Il comando e fire and forget? No (lettura con esito esplicito)
- Dove viene letto il feedback? `src/Accessory.c:1691`, `src/Accessory.c:1693`
- Quanto e affidabile? Media-alta lato comunicazione; dipende dalla qualita elettrica bus e dal sensore

## A10. Punti critici firmware
- Uso diretto indici/bit (`iAcc`, `byte_event`, `bit_event`) senza guard-rail locali.
- Assenza di fallback esplicito su misure stale quando `ReadSlave` fallisce.
- Soglia fissa clean-event CO2 (`max-min <= 500`) sensibile a contesto impianto.
- Variabili contatore storico dipendenti da sincronismo temporale esterno (ciclo scheduler/counter globali).

## A11. Punti di disaccoppiamento (sistema)
- Errore logico firmware:
  - Mapping errato `iAcc` -> byte/bit evento
  - Interpretazione errata bit allarme in `buff_Data[1]`
- Errore interfaccia:
  - Frame bus incompleto o corrotto
  - Timeout lettura slave
- Errore elettrico esterno:
  - Sensore non alimentato
  - Cablaggio bus degradato
- Errore meccanico:
  - Non applicabile direttamente (misura, non attuazione)

## A12. Limite di responsabilita firmware
Il firmware RD:
- Interroga il sensore e decodifica il frame ricevuto
- Aggiorna misure, eventi e stato operativo interno
- Propaga i dati alle logiche a valle

Il firmware RD NON garantisce:
- Accuratezza assoluta del sensore esterno
- Integrita cablaggio/linea bus
- Disponibilita hardware del sensore in campo

# 🟢 SEZIONE B — DOCUMENTAZIONE NON TECNICA (OPERATIVA / CAMPO)

## B1. Racconto operativo
La funzione legge i sensori aria digitali e aggiorna i valori che il sistema usa per ventilazione e regolazione. Se la lettura e valida, i dati vengono aggiornati; se non lo e, il sistema rileva che il canale non ha dato risposta utile.

## B2. Comportamento normale vs percezione anomala
- E normale vedere aggiornamenti a intervalli di polling e non in continuo.
- E normale che un sensore non aggiornato mantenga valori precedenti per alcuni cicli.
- Se il bus e disturbato, possono comparire letture mancanti intermittenti senza guasto fisso.

## B3. Errori bloccanti a monte
- Logica non autorizzata: canale sensore non configurato/polling non attivo
- Logica in protezione: evento allarme sensore fuori campo
- Logica attiva ma attuatore non funzionante: dato sensore presente ma nessun effetto fisico per blocchi a valle

## B4. Checklist problem solving
1. Cosa dovrebbe accadere?
2. Modalita selezionata?
3. Valori reali coerenti?
4. Consensi presenti?
5. Allarmi attivi?
6. Timer rispettati?
7. Comando previsto coerente?
8. Segnale elettrico presente?
9. Movimento fisico osservato?

Separare:
- Problema configurazione
- Problema elettrico
- Problema meccanico
- Problema installazione

## B5. Nota gestionale (facoltativa)
L'attribuzione della responsabilita richiede distinguere errore di misura, errore di comunicazione e uso del dato nelle logiche a valle.
