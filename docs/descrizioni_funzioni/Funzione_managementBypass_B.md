# Funzione `managementBypass`

# ðŸŸ¢ SEZIONE B â€” DOCUMENTAZIONE NON TECNICA (OPERATIVA / CAMPO)

## B1. Racconto operativo
- Cosa accade nel sistema: la serranda bypass viene comandata in apertura o chiusura secondo la modalita attiva (manuale, automatica, auto on/off, controllo esterno).
- Quando accade: il cambio stato avviene quando le condizioni della modalita selezionata sono soddisfatte (soglie temperatura, stato unita, ingresso esterno, consensi).
- Perche puo non accadere: il comando puo restare inibito da allarmi sonde, condizioni termiche non idonee, modalita test o indisponibilita della catena attuatore.
- Eventuali ritardi voluti e relazioni con altri accessori: sono presenti persistenze e attese per evitare commutazioni rapide; in automatico il bypass e vincolato anche allo stato degli accessori clima (es. heater/cooler/DXD) per evitare manovre non coerenti.

## B2. Comportamento normale vs percezione anomala
E normale che la serranda non cambi subito al primo cambio temperatura: la logica usa filtri temporali. In modalita freecooling auto on/off l'unita puo riattivarsi dopo una pausa; puo sembrare anomalo ma e comportamento previsto.

## B3. Errori bloccanti a monte
- Logica non autorizzata: modalita test bypass attiva, funzione non abilitata
- Logica in protezione: allarmi sonde fresh/return, condizioni termiche non idonee
- Logica attiva ma attuatore non funzionante: comando presente, nessun movimento reale per guasto catena esterna

## B4. Checklist problem solving
1. Cosa dovrebbe accadere: bypass aperto o chiuso secondo modalita attiva
2. Dati reali disponibili: temperature fresh/return, stato bypass, livello ingresso esterno
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
L'attribuzione della responsabilita non puo essere dedotta dal solo esito firmware: richiede verifica tecnica complessiva di logica, interfaccia e campo.

