# Funzione `Func_AirFlow_Const`

# 🟢 SEZIONE B — DOCUMENTAZIONE NON TECNICA (OPERATIVA / CAMPO)

## B1. Racconto operativo
- Cosa accade nel sistema: in modalita CAF la macchina corregge automaticamente la velocita ventilatori per mantenere la portata aria vicina al setpoint.
- Quando accade: durante il ciclo regolatore, dopo il calcolo della portata stimata/misurata e dell'errore rispetto al setpoint.
- Perche puo non accadere: se i motori non sono in regime minimo utile, se la funzione non e selezionata, o se limiti/saturazioni impediscono ulteriore correzione.
- Eventuali ritardi voluti e relazioni con altri accessori: sono previsti cicli di attesa tra una correzione e la successiva per stabilizzare la risposta; la funzione convive con altre logiche velocita (es. override da input o safety) che possono prevalere.
## B2. Comportamento normale vs percezione anomala
- La risposta non e immediata: la funzione interviene a passi e con tempi di filtro.
- Piccole oscillazioni intorno al valore target sono normali.
- Se i ventilatori girano troppo piano, la regolazione puo fermarsi temporaneamente.

## B3. Errori bloccanti a monte
- Logica non autorizzata: CAF non selezionato o sovrascritto da altra modalita prioritaria
- Logica in protezione: condizioni sicurezza che bloccano la gestione motori
- Logica attiva ma attuatore non funzionante: comando presente senza variazione reale di portata

## B4. Checklist problem solving
1. Cosa dovrebbe accadere: mantenimento portata vicino al target CAF
2. Dati reali disponibili: setpoint CAF, speed R/F, rpm, misura/stima portata
3. Modalita attiva corretta?
4. Consensi presenti?
5. Soglie rispettate?
6. Timer completati?
7. Allarmi attivi?
8. Comando generato?
9. Segnale elettrico presente?
10. Effetto fisico osservato?

Separare:
- Problema configurazione
- Problema elettrico
- Problema meccanico
- Problema installazione

## B5. Nota gestionale (facoltativa)
La valutazione responsabilita richiede distinguere errore di settaggio, errore comando e limiti fisici dell'impianto reale.

