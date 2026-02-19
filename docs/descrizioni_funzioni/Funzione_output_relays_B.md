# Funzione `Gestione_Relays` (output)

# SEZIONE B - DOCUMENTAZIONE NON TECNICA (OPERATIVA / CAMPO)

## B1. Racconto operativo
- Cosa accade nel sistema: le due uscite relay (`Out1`, `Out2`) si attivano o disattivano in base alla funzione configurata per ciascuna uscita.
- Quando accade: a ogni ciclo di gestione output (scheduler), valutando lo stato macchina e i parametri configurati.
- Perche puo non accadere: uscita disabilitata, logica evento non soddisfatta, modalita test output attiva, o inversione logica impostata.
- Eventuali ritardi voluti e relazioni con altri accessori: in modalita estate/inverno sono previsti ritardi/persistenze; lo stato output dipende anche da clima, bypass, allarmi e velocita ventilatori.

## B2. Comportamento normale vs percezione anomala
E normale che un output sembri "invertito" se e configurato in modalita con bit inversione (128+). E normale che in modalita estate/inverno la commutazione non sia immediata per via della persistenza. E normale che output run/fault seguano lo stato generale macchina e non solo una singola funzione.

## B3. Errori bloccanti a monte
- Logica non autorizzata: `Set_Output` impostato a disable
- Logica in protezione: test output attivo o condizioni macchina non soddisfatte
- Logica attiva ma attuatore non funzionante: output logico ON ma relay/cablaggio non commuta

## B4. Checklist problem solving
1. Quale funzione e configurata su `Out1` / `Out2`?
2. E presente inversione logica (valori >= 128)?
3. Lo stato evento scelto e realmente attivo (bypass/fault/run/valvola/summer-winter/max speed)?
4. Test output e disattivato?
5. `StatusOutput` riflette lo stato atteso?
6. In modalita estate/inverno, i tempi di persistenza sono trascorsi?
7. Setpoint e misure correlate (AWP/Fresh) sono coerenti?
8. Segnale elettrico su pin output presente?
9. Relay fisico commuta?
10. Carico a valle risponde correttamente?

Separazione diagnosi:
- Problema configurazione
- Problema elettrico
- Problema meccanico
- Problema installazione

## B5. Nota gestionale (facoltativa)
La commutazione firmware dell'output non equivale automaticamente alla commutazione elettrica reale del contatto in campo.
