# Funzione `ThermoCheck`

# ðŸŸ¢ SEZIONE B â€” DOCUMENTAZIONE NON TECNICA (OPERATIVA / CAMPO)

## B1. Racconto operativo
- Cosa accade nel sistema: la funzione aggiorna le variabili interne di temperatura/umidita applicando compensazioni ai dati sensore gia acquisiti.
- Quando accade: a ogni ciclo della funzione, con inizializzazione dedicata al primo passaggio.
- Perche puo non accadere: non genera azioni fisiche dirette; eventuali anomalie dipendono soprattutto da dati a monte non aggiornati o non validi.
- Eventuali ritardi voluti e relazioni con altri accessori: non introduce ritardi di attuazione; i valori prodotti vengono usati dalle logiche clima successive e quindi influenzano indirettamente heater/cooler/bypass.
## B2. Comportamento normale vs percezione anomala
Una variazione rapida sonda viene riflessa al ciclo successivo, non in tempo continuo. Valori apparentemente "sfalsati" possono essere normali se corrispondono agli offset previsti. Un dato fermo nel tempo puo indicare blocco comunicazione, non necessariamente errore della funzione.

## B3. Errori bloccanti a monte
- Logica non autorizzata: non applicabile (funzione di aggiornamento dati)
- Logica in protezione: dati sensore non validi o non aggiornati
- Logica attiva ma attuatore non funzionante: non applicabile direttamente, ma dati errati possono portare a decisioni errate a valle

## B4. Checklist problem solving
1. Cosa dovrebbe accadere: aggiornamento coerente variabili termiche interne
2. Dati reali disponibili: valori raw sensori e valori compensati interni
3. Modalita attiva corretta?
4. Consensi presenti?
5. Soglie rispettate?
6. Timer completati?
7. Allarmi attivi?
8. Comando generato?
9. Segnale elettrico presente?
10. Effetto fisico osservato?

Separazione diagnosi:
- Problema configurazione
- Problema elettrico
- Problema meccanico
- Problema installazione

## B5. Nota gestionale (facoltativa)
L'analisi responsabilita richiede separare errore misura, errore comunicazione e uso a valle del dato nella regolazione.

