# Funzione `KTS`

# SEZIONE B - DOCUMENTAZIONE NON TECNICA (OPERATIVA / CAMPO)

## B1. Racconto operativo
- Cosa accade nel sistema: il pannello KTS invia comandi all'unita e riceve stati/misure; il firmware controlla anche che il KTS sia presente e stabile.
- Quando accade: in continuo durante il funzionamento normale.
- Perche puo non accadere: cavo scollegato, alimentazione KTS instabile, disturbi seriali, oppure fault interno KTS.
- Eventuali ritardi voluti e relazioni con altri accessori: in caso guasto link, il firmware attende una finestra temporale prima di dichiarare fault e puo riavviare l'alimentazione KTS.

Cosa vede l'utente:
- pannello KTS aggiornato con stati e misure
- eventuale allarme comunicazione KTS
- possibile breve blackout/ripartenza KTS durante auto-reset

## B2. Comportamento normale vs percezione anomala
- E normale che il fault KTS non compaia immediatamente alla prima perdita pacchetto.
- E normale vedere ripristino automatico dopo reset alimentazione KTS.
- Falsa diagnosi tipica: attribuire il problema al firmware quando il guasto e nel cavo/connettore o nel KTS stesso.

## B3. Errori bloccanti a monte
- Logica non autorizzata
  - Modalita test o condizioni che alterano comportamento standard comunicazione.
- Logica in protezione
  - Allarme link KTS attivo con procedura di reset in corso.
- Logica attiva ma attuatore non funzionante
  - Firmware operativo ma KTS non risponde per guasto hardware/cablaggio.

## B4. Checklist problem solving
1. Il KTS si accende correttamente?
2. Il cavo/connettore e integro e ben inserito?
3. L'unita vede traffico KTS stabile o va in fault periodico?
4. L'alimentazione KTS e stabile durante il funzionamento?
5. Dopo reset automatico, la comunicazione torna stabile?
6. Sono presenti allarmi KTS o remoto correlati?
7. In test mode, il comportamento cambia come atteso?
8. I comandi inviati da KTS producono risposta coerente sull'unita?
9. Se il problema persiste, provare KTS/cavo noti buoni.

Separazione diagnosi:
- Problema configurazione
- Problema elettrico
- Problema installazione
- Problema dispositivo KTS

## B5. Nota gestionale (facoltativa)
Un fault KTS indica perdita o anomalia di comunicazione/alimentazione rilevata dal controllo, ma non identifica automaticamente il componente responsabile senza verifica in campo.
