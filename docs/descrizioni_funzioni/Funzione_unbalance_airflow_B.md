# Funzione `unbalance_airflow` (logica trasversale)

# SEZIONE B - DOCUMENTAZIONE NON TECNICA (OPERATIVA / CAMPO)

## B1. Racconto operativo
- Cosa accade nel sistema: la macchina forza una differenza tra ventilatore di immissione e ventilatore di estrazione.
- Quando accade: quando imbalance e abilitato (da weekly o da configurazione funzione) e non e bloccato da priorita superiori.
- Perche puo non accadere: se Defrost o CAP sono attivi, se parametro imbalance e a zero, o se la funzione non e abilitata.
- Eventuali ritardi voluti e relazioni con altri accessori: il comando e ricalcolato ciclo per ciclo e puo essere sovrascritto da altre logiche (safety, boost, regolazioni principali).

## B2. Comportamento normale vs percezione anomala
E normale vedere velocita diverse tra i due rami quando imbalance e attivo. E normale che l'effetto non sia costante se entrano logiche prioritarie o limiti di velocita.

## B3. Errori bloccanti a monte
- Logica non autorizzata: imbalance disabilitato in weekly e in configurazione funzione
- Logica in protezione: Defrost/CAP o altre priorita che inibiscono lo sbilanciamento
- Logica attiva ma attuatore non funzionante: comandi diversi R/F presenti, ma ramo motore non risponde

## B4. Checklist problem solving
1. Imbalance e abilitato (weekly o funzione)?
2. Valore `Set_Imbalance` impostato correttamente?
3. Il segno del valore e coerente (piu estrazione vs piu immissione)?
4. CAP o Defrost sono attivi?
5. Flag `POS_BIT_IMBALANCE_OPERATING` e attivo?
6. Comandi R/F risultano diversi in runtime?
7. Sono presenti clamp o limiti min/max che riducono l'effetto?
8. Segnali elettrici ai due rami presenti?
9. Rpm dei due rami coerenti con il comando?
10. Effetto aria in campo coerente con la richiesta?

Separazione diagnosi:
- Problema configurazione
- Problema elettrico
- Problema meccanico
- Problema installazione

## B5. Nota gestionale (facoltativa)
La richiesta di sbilanciamento e una decisione firmware; il risultato reale dipende anche da impianto, perdite e stato dei due rami ventilazione.
