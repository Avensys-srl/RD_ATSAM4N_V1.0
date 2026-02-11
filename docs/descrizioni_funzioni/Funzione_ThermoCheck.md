# Template descrizione funzione

## Nome funzione
`ThermoCheck()`

## File e posizione
`src/Clima_Func.c:935`

## Scopo
Aggiorna variabili termiche/igrometriche globali (S1..S4, T1..T5) con compensazioni fisse sui canali sensore, come base per controlli coerenza termo-fluidica.

## Parametri e ritorno
Parametro in ingresso:
- Nessuno.

Valore di ritorno:
- `int` (ritorna `1`).

Unita di misura rilevanti:
- Temperature in decimi di grado C.
- Umidita relativa in punti percentuali scalati come da acquisizione accessori.

## Logica dettagliata
- Definisce tolleranze locali `tolT=300` e `tolRH=10`; nel codice attuale sono preparatorie, perche la parte di controllo coerenza e commentata.
- Se `Tcheck==0` inizializza una prima fotografia dei canali:
- `S2_T`, `S2_RH` (supply).
- `T3` (post EHD/SSR), `T4` (exhaust da modbus), `T5` (post DX coil da modbus).
- Imposta `Tcheck=1`.
- Aggiorna ad ogni chiamata i canali principali:
- `S1_T`, `S1_RH` (fresh).
- `S4_T`, `S4_RH` (return).
- `T1`, `T2` (linea refrigerante SSR inlet/outlet).
- Esegue poi una "sola correzione" ricalcolando nuovamente `S2_T`, `S2_RH`, `T3`, `T4`, `T5` con offset fissi.
- Mantiene nel sorgente una lunga sezione commentata di logiche di coerenza (controlli monotonia termica e correzioni in base a bypass), attualmente non eseguita.

Offset applicati nel codice:
- `S1_T = EXT1.measure1 + 295`
- `S2_T = EXT2.measure1 + 385`
- `S4_T = EXT4.measure1 + 285`
- `T1 = SSR.measure1 + 60`
- `T2 = SSR.measure2 + 60`
- `T3 = SSR.measure3 + 90`
- `T4 = Temp_modbus[EXHAUST] + 70`
- `T5 = Temp_modbus[SUPPLY] + 50`

## Dipendenze
- Stato globale `sData.AccI2C[...]` per sensori esterni e SSR.
- Array modbus `Temp_modbus`.
- Flag globale `Tcheck`.
- Variabili globali di uscita: `S1_T`, `S1_RH`, `S2_T`, `S2_RH`, `S4_T`, `S4_RH`, `T1..T5`.

## Casi limite
- Nessuna validazione diretta su range sensori prima di applicare offset.
- La logica di coerenza avanzata e presente ma completamente commentata, quindi oggi la funzione si comporta come mappatura+offset.
- `tolT` e `tolRH` non influiscono nel comportamento corrente.

## Verifica
- Confrontare i valori globali `S*` e `T*` con le misure raw per verificare offset attesi.
- Verificare che alla prima chiamata venga settato `Tcheck=1`.
- Testare con frame modbus e dati I2C variabili per confermare l'aggiornamento a ogni ciclo.
- Se si riattiva la parte commentata in futuro, introdurre test dedicati per i vincoli di coerenza termica e influenza stato bypass.

## Racconto operativo (per utente/service)
Questa funzione non comanda direttamente caldo o freddo, ma prepara e aggiorna le temperature "di lavoro" che il resto del controllo usa per prendere decisioni.

In parole semplici, raccoglie i valori dalle varie sonde (aria esterna, ritorno, mandata, punti circuito) e applica piccole correzioni numeriche. Queste correzioni servono ad allineare le letture in modo coerente con la taratura prevista dal sistema.

Per chi e sul campo, il risultato e che i numeri interni usati dalla regolazione possono essere leggermente diversi dalla lettura grezza del sensore. Non e necessariamente un errore: spesso e una compensazione prevista.

Alla prima esecuzione inizializza alcuni riferimenti, poi ad ogni ciclo aggiorna i valori. Quindi se una sonda cambia rapidamente, anche i valori di questa funzione seguono quel cambiamento al ciclo successivo.

La parte avanzata che farebbe controlli di coerenza piu spinti tra una sonda e l'altra, nel codice attuale, e disattivata (commentata). Di conseguenza oggi la funzione lavora come "normalizzazione e aggiornamento continuo" dei dati termici.
