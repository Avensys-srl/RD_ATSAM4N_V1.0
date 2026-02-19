# Funzione `read_Sensor_Air`

# ðŸŸ¢ SEZIONE B â€” DOCUMENTAZIONE NON TECNICA (OPERATIVA / CAMPO)

## B1. Racconto operativo
- Cosa accade nel sistema: la funzione interroga i sensori aria su bus I2C e aggiorna misure/stati usati dalle logiche di ventilazione e clima.
- Quando accade: durante il polling accessori, a ogni ciclo di comunicazione previsto.
- Perche puo non accadere: se il nodo non risponde, se la transazione fallisce (ACK/CKSUM), o se il dispositivo non e presente/configurato.
- Eventuali ritardi voluti e relazioni con altri accessori: il comportamento segue la cadenza di polling del bus; in caso di mancate risposte ripetute si degradano stato operativo e affidabilita dei dati usati dalle funzioni a valle.
## B2. Comportamento normale vs percezione anomala
- E normale vedere aggiornamenti a intervalli di polling e non in continuo.
- E normale che un sensore non aggiornato mantenga valori precedenti per alcuni cicli.
- Se il bus e disturbato, possono comparire letture mancanti intermittenti senza guasto fisso.

## B3. Errori bloccanti a monte
- Logica non autorizzata: canale sensore non configurato/polling non attivo
- Logica in protezione: evento allarme sensore fuori campo
- Logica attiva ma attuatore non funzionante: dato sensore presente ma nessun effetto fisico per blocchi a valle

## B4. Checklist problem solving
1. Cosa dovrebbe accadere?
2. Modalita selezionata?
3. Valori reali coerenti?
4. Consensi presenti?
5. Allarmi attivi?
6. Timer rispettati?
7. Comando previsto coerente?
8. Segnale elettrico presente?
9. Movimento fisico osservato?

Separare:
- Problema configurazione
- Problema elettrico
- Problema meccanico
- Problema installazione

## B5. Nota gestionale (facoltativa)
L'attribuzione della responsabilita richiede distinguere errore di misura, errore di comunicazione e uso del dato nelle logiche a valle.

