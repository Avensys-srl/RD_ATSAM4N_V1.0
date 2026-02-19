# Funzione `modbus`

# SEZIONE B - DOCUMENTAZIONE NON TECNICA (OPERATIVA / CAMPO)

## B1. Racconto operativo
- Cosa accade nel sistema: l'unita scambia dati ciclicamente con il controllo remoto Modbus (stati, misure, allarmi e parametri).
- Quando accade: in modo continuo durante il funzionamento, a intervalli scheduler.
- Perche puo non accadere: modulo remoto non rilevato, linea seriale interrotta, errore di protocollo o alimentazione assente lato remoto.
- Eventuali ritardi voluti e relazioni con altri accessori: il link viene valutato su piu cicli; la perdita comunicazione non genera allarme immediato ma dopo persistenza.

Cosa vede l'utente:
- aggiornamento periodico dati su supervisore remoto
- eventuale allarme comunicazione remota se il link cade

## B2. Comportamento normale vs percezione anomala
- E normale vedere un lieve ritardo tra cambio stato macchina e visualizzazione remota (dipende dal ciclo di polling).
- E normale che l'allarme link compaia solo dopo alcuni cicli senza risposta.
- Falsa diagnosi tipica: considerare guasto firmware quando invece manca alimentazione o cablaggio del modulo remoto.

## B3. Errori bloccanti a monte
- Logica non autorizzata
  - Modulo remoto non presente/rilevato.
- Logica in protezione
  - Link degradato con allarme remoto attivo.
- Logica attiva ma attuatore non funzionante
  - RD invia i frame, ma il dispositivo remoto non riceve/non risponde.

## B4. Checklist problem solving
1. Il modulo remoto e dichiarato presente in installazione?
2. L'alimentazione del modulo remoto e corretta?
3. Il cablaggio seriale e integro e coerente con pinout?
4. I parametri seriali sono coerenti tra i due lati?
5. L'allarme comunicazione remota e attivo?
6. Dopo il ripristino linea, il link si ristabilisce entro alcuni cicli?
7. I valori letti da remoto cambiano in modo coerente con lo stato macchina?
8. Se scrivo un parametro da remoto, il valore viene confermato?
9. Restano errori checksum/lunghezza frame?

Separazione diagnosi:
- Problema configurazione
- Problema elettrico
- Problema installazione
- Problema firmware/protocollo

## B5. Nota gestionale (facoltativa)
La sola assenza di aggiornamento lato supervisore non prova un difetto firmware RD: va verificata prima la catena completa (presenza modulo, alimentazione, linea seriale, configurazione).
