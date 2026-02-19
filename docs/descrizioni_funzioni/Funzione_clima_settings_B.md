# Funzione `clima_settings` (sintesi trasversale)

# SEZIONE B - DOCUMENTAZIONE NON TECNICA (OPERATIVA / CAMPO)

## B1. Racconto operativo
- Cosa accade nel sistema: i settaggi clima stabiliscono come la macchina gestisce caldo/freddo, ventilazione, bypass, programma settimanale e boost/party.
- Quando accade: continuamente durante il funzionamento, con cicli periodici delle funzioni principali.
- Perche puo non accadere: una richiesta puo essere bloccata da priorita superiori (allarmi, safety, standby, vincoli modalita, condizioni sensori).
- Eventuali ritardi voluti e relazioni con altri accessori: sono presenti isteresi e timer (weekly/boost/persistenze) e il comportamento dipende dalla disponibilita reale degli accessori collegati.

## B2. Comportamento normale vs percezione anomala
E normale che il sistema non cambi stato immediatamente al primo superamento soglia (isteresi/filtri). E normale che weekly o boost modifichino temporaneamente il comportamento standard. E normale che, con bypass in manovra o safety attive, la risposta ventilazione sembri limitata.

## B3. Errori bloccanti a monte
- Logica non autorizzata: funzione disabilitata in configurazione
- Logica in protezione: allarmi sensori/motori/accessori o modalità test
- Logica attiva ma attuatore non funzionante: comando presente ma nessun effetto fisico

## B4. Checklist problem solving
1. Qual e il comportamento atteso (clima/weekly/boost/bypass)?
2. Funzioni abilitate in configurazione coerenti?
3. Stagione attiva (summer/winter) corretta?
4. Setpoint e isteresi coerenti?
5. Weekly RUN attivo nella fascia corretta?
6. Boost/party attivo o scaduto?
7. Allarmi attivi o blocchi sicurezza presenti?
8. Comandi firmware risultano emessi?
9. Segnali elettrici/interfacce presenti?
10. Effetto fisico osservato coerente?

Separazione diagnosi:
- Problema configurazione
- Problema elettrico
- Problema meccanico
- Problema installazione

## B5. Nota gestionale (facoltativa)
La diagnosi dei clima settings richiede sempre di separare la decisione firmware dalla risposta elettrica e dall'effetto fisico in impianto.
