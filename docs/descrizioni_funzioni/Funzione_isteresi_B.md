# Funzione `isteresi` (logica trasversale)

# SEZIONE B - DOCUMENTAZIONE NON TECNICA (OPERATIVA / CAMPO)

## B1. Racconto operativo
- Cosa accade nel sistema: il controllo non accende/spegne subito sul setpoint, ma usa una banda di isteresi per evitare continui cambi stato.
- Quando accade: durante la regolazione caldo/freddo a ogni ciclo di controllo.
- Perche puo non accadere: se le soglie sono configurate male, se una logica prioritaria blocca il comando, o se i sensori non sono coerenti.
- Eventuali ritardi voluti e relazioni con altri accessori: il comportamento e volutamente "smorzato"; interagisce con bypass, safety e gestione accessori clima.

## B2. Comportamento normale vs percezione anomala
E normale che l'attuatore non cambi immediatamente al primo superamento del setpoint. E normale che resti acceso/spento finche non supera la soglia opposta della banda isteretica.

## B3. Errori bloccanti a monte
- Logica non autorizzata: funzione clima disabilitata o stagione non attiva
- Logica in protezione: allarmi/fault che impongono OFF
- Logica attiva ma attuatore non funzionante: comando ON/OFF presente, nessun effetto fisico

## B4. Checklist problem solving
1. Setpoint corretto?
2. Valori isteresi ON/OFF coerenti?
3. Temperatura riferimento corretta (TR/TS/AWP)?
4. Soglia ON superata realmente?
5. Soglia OFF raggiunta realmente?
6. Ci sono filtri/persistenze attive?
7. Allarmi o blocchi sicurezza presenti?
8. Comando firmware ON/OFF emesso?
9. Segnale elettrico verso attuatore presente?
10. Effetto termico osservato coerente?

Separazione diagnosi:
- Problema configurazione
- Problema elettrico
- Problema meccanico
- Problema installazione

## B5. Nota gestionale (facoltativa)
L'isteresi riduce oscillazioni logiche ma non puo compensare difetti di sensori, impianto o attuatori.
