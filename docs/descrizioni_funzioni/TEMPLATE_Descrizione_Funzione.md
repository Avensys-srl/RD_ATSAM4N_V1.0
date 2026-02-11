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

## Racconto operativo (per utente/service)
Scrivere un racconto descrittivo, meno tecnico ma molto dettagliato, pensato per utente finale, service man o cliente generico.

Obiettivo della sezione:
- spiegare cosa fa la funzione nella pratica quotidiana
- chiarire perche un comportamento puo avvenire (es. bypass che si apre/non si apre)
- aiutare la diagnosi sul campo senza richiedere conoscenza del codice

Standard da mantenere sempre:
- linguaggio semplice, frasi chiare, niente gergo interno non spiegato
- descrizione concreta dei comportamenti osservabili
- indicare le cause principali che portano a uno stato ON/OFF o apertura/chiusura
- evidenziare eventuali attese temporali o ritardi voluti (isteresi, persistenze, timer)
- distinguere comportamento normale da possibile anomalia

Struttura consigliata:
- come si comporta normalmente la funzione
- in quali condizioni cambia comportamento
- cosa controllare per primo durante un intervento sul campo
- quali casi possono sembrare un guasto ma sono invece logica prevista
