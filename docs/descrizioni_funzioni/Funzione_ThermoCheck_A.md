# Funzione `ThermoCheck`

# ðŸ”· SEZIONE A â€” DOCUMENTAZIONE TECNICA (ENGINEERING)

## A1. Nome funzione
`ThermoCheck()`

## A2. File e posizione
`src/Clima_Func.c:935`

## A3. Scopo tecnico
Aggiorna le variabili globali termiche e igrometriche usate dalle logiche di regolazione, applicando compensazioni numeriche fisse alle misure disponibili.

## A4. Parametri e ritorno
### Parametri in ingresso
- Nessuno

### Valore di ritorno
- Tipo: `int`
- Significato: ritorna `1` a fine aggiornamento

## A5. Logica dettagliata
- Definisce tolleranze locali (`tolT`, `tolRH`) attualmente non operative nella logica attiva.
- Alla prima esecuzione (`Tcheck == 0`) inizializza blocco variabili supply/coil/exhaust e imposta il flag inizializzazione.
- Ad ogni chiamata aggiorna fresh, return, canali SSR e grandezze modbus.
- Applica offset fissi alle grandezze acquisite (`S1_T`, `S2_T`, `S4_T`, `T1..T5`).
- Mantiene nel sorgente una sezione di controlli plausibilita commentata e non eseguita.

Offset applicati:
- `S1_T = EXT1.measure1 + 295`
- `S2_T = EXT2.measure1 + 385`
- `S4_T = EXT4.measure1 + 285`
- `T1 = SSR.measure1 + 60`
- `T2 = SSR.measure2 + 60`
- `T3 = SSR.measure3 + 90`
- `T4 = Temp_modbus[EXHAUST] + 70`
- `T5 = Temp_modbus[SUPPLY] + 50`

## A6. Mappa firmware con riferimenti puntuali
- **Passo**: ingresso funzione e definizione tolleranze locali  
  **Dove**: `src/Clima_Func.c:938`  
  **Cosa osservare**: `tolT`, `tolRH`
- **Passo**: verifica inizializzazione primo ciclo  
  **Dove**: `src/Clima_Func.c:941`  
  **Cosa osservare**: condizione `if(Tcheck == 0)`
- **Passo**: caricamento iniziale canali supply/coil/exhaust  
  **Dove**: `src/Clima_Func.c:943`  
  **Cosa osservare**: assegnazioni `S2_T`, `T3`, `T4`, `T5`
- **Passo**: attivazione flag inizializzazione  
  **Dove**: `src/Clima_Func.c:950`  
  **Cosa osservare**: `Tcheck = 1`
- **Passo**: aggiornamento continuo fresh/return  
  **Dove**: `src/Clima_Func.c:954`  
  **Cosa osservare**: `S1_T`, `S1_RH`, `S4_T`, `S4_RH`
- **Passo**: aggiornamento continuo canali SSR  
  **Dove**: `src/Clima_Func.c:959`  
  **Cosa osservare**: `T1`, `T2`
- **Passo**: aggiornamento continuo supply/exhaust/coil  
  **Dove**: `src/Clima_Func.c:966`  
  **Cosa osservare**: `S2_T`, `T3`, `T4`, `T5`
- **Passo**: uscita funzione  
  **Dove**: `src/Clima_Func.c:1107`  
  **Cosa osservare**: `return 1`

## A7. Interfacce esterne (livello astratto)
### Segnali generati dallâ€™RD
- Nessun comando attuatore diretto in questa funzione:
  - Pin MCU / periferica: non applicabile
  - Tipo: aggiornamento variabili globali firmware
  - Livello attivo: non applicabile
  - Condizione di attivazione: ogni chiamata funzione
  - Persistenza: fino al ciclo successivo

### Segnali letti dallâ€™RD
- Sensori I2C esterni:
  - Tipo ingresso: temperatura/umidita
  - Modalita lettura: campi `sData.AccI2C[...]`
  - Frequenza campionamento: ciclo funzione
  - Condizione di validita: canale sensore disponibile
- Dati temperatura da bus modbus:
  - Tipo ingresso: registri temperatura
  - Modalita lettura: array `Temp_modbus[...]`
  - Frequenza campionamento: ciclo funzione
  - Condizione di validita: aggiornamento dati bus

## A8. Catena causale (livello sistema)
| Livello | Descrizione |
|----------|------------|
| Decisione | Normalizzazione/compensazione valori termici |
| Interfaccia | Lettura dati sensore da strutture firmware e bus |
| Elettronica esterna | Sonde e moduli misura |
| Attuatore | Non applicabile (funzione di sola misura) |
| Effetto | Dati termici usati dalle logiche di regolazione successive |

## A9. Feedback disponibile

- Esiste conferma reale?
  - No, non nel perimetro funzione: aggiorna variabili interne da misure gia acquisite altrove.
- Il comando e "fire and forget"?
  - Non applicabile: nessun comando attuatore o transazione bus in questa funzione.
- Dove viene letto il feedback?
  - Solo lettura strutture dati (`sData.AccI2C[...]`, `Temp_modbus[...]`) e assegnazioni locali (`src/Clima_Func.c:935`, `src/Clima_Func.c:954`, `src/Clima_Func.c:966`, `src/Clima_Func.c:971`).
- Quanto e affidabile?

Valori criteri:

- `Origine diretta: 0` (nessuna conferma diretta di esecuzione reale nel corpo funzione)
- `Correlazione temporale: 0` (nessun timeout o freshness check)
- `Correlazione univoca col comando: 0` (nessun comando da correlare)
- `Gestione errore: 0` (controlli plausibilita presenti ma commentati)
- `Punteggio totale: 0`
- `Classe finale: Nessun feedback`
## A10. Punti critici firmware
- Verificare condizioni con confronti stretti su soglie (maggiore/minore uguale) per evitare oscillazioni logiche.
- Verificare coerenza timer/contatori rispetto al periodo scheduler reale.
- Verificare priorità dei rami decisionali e dei ritorni anticipati.
- Verificare possibili ambiguità tra flag di stato e comandi calcolati nello stesso ciclo.
## A11. Punti di disaccoppiamento (sistema)
- Errore logico firmware:
  - Offset non coerenti con taratura impianto
  - Uso di valori senza controlli plausibilita attivi
- Errore interfaccia:
  - Dati I2C/modbus non aggiornati ma ancora letti
  - Campi struttura sensore non sincronizzati
- Errore elettrico esterno:
  - Sensore alimentato male o guasto
  - Disturbo linea comunicazione misura
- Errore meccanico:
  - Non applicabile direttamente (nessun attuatore comandato)

## A12. Limite di responsabilità firmware
Il firmware RD:
- Legge misure disponibili
- Applica compensazioni numeriche definite
- Aggiorna variabili interne usate da altre logiche

Il firmware RD NON garantisce:
- Accuratezza assoluta sensore esterno
- Integrita cablaggio misura
- Stabilita comunicazione esterna
- Coerenza fisica impianto in assenza feedback affidabili

