# Funzione `weekly_program`

# SEZIONE B - DOCUMENTAZIONE NON TECNICA (OPERATIVA / CAMPO)

## B1. Racconto operativo
- Cosa accade nel sistema: la macchina segue una programmazione settimanale a fasce orarie, decidendo quando lavorare e con quale profilo.
- Quando accade: il controllo viene fatto continuamente, con aggiornamento ogni secondo.
- Perche puo non accadere: se la funzione weekly non e abilitata, se la fascia oraria non e attiva, o se i parametri settimanali non sono configurati correttamente.
- Eventuali ritardi voluti e relazioni con altri accessori: per il preheater ad acqua puo essere previsto un pre-avvio anticipato rispetto all'accensione fascia; il risultato weekly viene poi usato dalle logiche motori/clima.

## B2. Comportamento normale vs percezione anomala
E normale che il cambio stato avvenga a step di 30 minuti (fasce) e non in modo continuo. E normale anche che, vicino all'inizio fascia, il preheater possa attivarsi prima dell'avvio completo unita. Se orario e programmazione non sono coerenti, il comportamento puo sembrare errato anche con firmware funzionante.

## B3. Errori bloccanti a monte
- Logica non autorizzata: weekly non abilitato nelle funzioni
- Logica in protezione: funzione sovrascritta da priorita superiori (safety, stati macchina)
- Logica attiva ma attuatore non funzionante: weekly RUN presente ma accessorio esterno non risponde

## B4. Checklist problem solving
1. Cosa dovrebbe accadere in questa fascia oraria?
2. Giorno/ora dell'unita sono corretti?
3. Weekly abilitato in configurazione?
4. Numero fasce e orari ON/OFF del giorno sono corretti?
5. Profilo fascia (speed/imbalance/temp) e coerente?
6. Lo stato weekly risulta RUN nella fascia attesa?
7. Ci sono allarmi o blocchi superiori attivi?
8. Se previsto preheater: comando presente?
9. Segnale elettrico/interfaccia accessorio presente?
10. Effetto fisico osservato coerente?

Separazione diagnosi:
- Problema configurazione
- Problema elettrico
- Problema meccanico
- Problema installazione

## B5. Nota gestionale (facoltativa)
La responsabilita non si deduce dal solo fatto che il weekly sia in RUN: va verificata anche la catena completa tra logica, configurazione e attuazione reale.
