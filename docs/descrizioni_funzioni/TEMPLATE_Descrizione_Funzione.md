# Template descrizione funzione

## Nome funzione
Inserire il nome esatto della funzione.

## File e posizione
Indicare file sorgente e riferimento rapido (funzione/linea se disponibile).

## Scopo
Descrivere in modo chiaro il comportamento atteso.

## Parametri e ritorno
Elencare parametri in ingresso, vincoli, unita di misura e valore di ritorno.

## Logica dettagliata
Spiegare il flusso interno in modo completo, includendo:
- sequenza operativa
- condizioni/rami decisionali
- gestione errori o fallback
- effetti collaterali su stato globale, pin e periferiche

## Dipendenze
Elencare chiamate ad altre funzioni, moduli e macro coinvolte.

## Casi limite
Descrivere i casi particolari e il comportamento previsto.

## Verifica
Indicare come validare la funzione (test, log, segnali HW attesi).

## Spazio tecnico (mappa firmware e righe)
Per ogni passo descritto in `Logica dettagliata`, aggiungere sempre dove si trova nel codice firmware:
- `Passo`: descrizione breve del comportamento
- `Dove`: file e riga (esempio: `src/Clima_Func.c:428`)
- `Cosa osservare`: variabile, condizione o comando che conferma quel passo

Regola di qualita:
- nessun passo senza riferimento riga
- usare riferimenti puntuali e verificabili (evitare "circa" o range lunghi)
- aggiornare le righe se il firmware viene modificato

## Racconto operativo (utente finale)
Scrivere un racconto descrittivo, meno tecnico ma molto dettagliato, pensato per utente finale o cliente generico.

Obiettivo della sezione:
- spiegare cosa fa la funzione nella pratica quotidiana
- chiarire perche un comportamento puo avvenire (es. bypass che si apre/non si apre)
- aiutare la comprensione d'uso senza richiedere conoscenza del codice

Standard da mantenere sempre:
- linguaggio semplice, frasi chiare, niente gergo interno non spiegato
- descrizione concreta dei comportamenti osservabili
- indicare le cause principali che portano a uno stato ON/OFF o apertura/chiusura
- evidenziare eventuali attese temporali o ritardi voluti (isteresi, persistenze, timer)
- distinguere comportamento normale da possibile anomalia percepita

## Operativo service (diagnosi sul campo)
Scrivere una sezione dedicata al service sul campo, con linguaggio pratico e orientato al controllo dati.
Non citare file firmware, righe di codice, nomi di macro o nomi funzione.

### Errori bloccanti a monte
Descrivere in modo completo tutte le condizioni che possono impedire alla funzione di produrre effetto reale, anche se la funzione viene chiamata.

Includere sempre:
- modalita test/manutenzione che bloccano la regolazione normale
- allarmi che mettono il sistema in protezione
- prerequisiti mancanti (funzione disabilitata, accessorio assente, consenso non valido)
- guasti comunicazione/sensori/attuatori che impediscono l'effetto reale
- differenza tra "logica non autorizzata a intervenire" e "logica in protezione"

### Checklist problem solving (dati da controllare)
Per ogni funzione inserire una checklist concreta, adattata al caso specifico.
La checklist non deve essere fissa: va costruita in base al tipo di logica (es. bypass, motori, riscaldamento, raffreddamento, sicurezza, comunicazione, ecc.).

Approccio minimo da mantenere:
- obiettivo richiesto dalla funzione (cosa dovrebbe fare)
- dati reali disponibili al momento (misure, stati, ingressi, feedback)
- condizioni che autorizzano o bloccano l'azione (soglie, consensi, modalita, prerequisiti)
- filtri temporali della decisione (isteresi, ritardi, persistenze, timer)
- protezioni e allarmi che possono inibire il comportamento
- verifica finale dell'effetto reale (comando logico vs comportamento fisico osservato)
