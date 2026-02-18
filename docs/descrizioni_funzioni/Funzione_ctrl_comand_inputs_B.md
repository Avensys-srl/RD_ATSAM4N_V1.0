# Funzione `ctrl_comand_inputs`

# 🟢 SEZIONE B — DOCUMENTAZIONE NON TECNICA (OPERATIVA / CAMPO)

## B1. Racconto operativo
- Cosa accade nel sistema: la funzione interpreta gli ingressi configurati (ON/OFF unita, regolazione portata, incendio, PIR, cambio stagione) e produce il comando logico di marcia/arresto o velocita.
- Quando accade: ad ogni ciclo di controllo motori, leggendo il valore ADC degli ingressi e la loro configurazione.
- Perche puo non accadere: se l'ingresso non e configurato per quella funzione, se il segnale resta fuori soglia utile, o se altre priorita (es. incendio/riarmo) prevalgono.
- Eventuali ritardi voluti e relazioni con altri accessori: la gestione PIR e vincolata da contatori/ritardi; i comandi ingresso influenzano direttamente la funzione `motor_speed` e quindi il comportamento globale della ventilazione.
## B2. Comportamento normale vs percezione anomala
- E normale che il cambio stagione non avvenga se manca un accessorio compatibile (es. cooler per summer, heater/SSR per winter).
- E normale vedere nessun cambio se Input 2 non e configurato in modalita stagione.
- Valori vicino soglia possono dare percezione di instabilita.

## B3. Errori bloccanti a monte
- Logica non autorizzata: input non configurato come summer/winter
- Logica in protezione: accessori stagionali non operativi
- Logica attiva ma attuatore non funzionante: stagione cambiata ma resa termica assente per guasti esterni

## B4. Checklist problem solving
1. Cosa dovrebbe accadere: switch estate/inverno da Input 2
2. Dati reali disponibili: valore ADC input 2, bit stagione attivi, stato accessori
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
L'attribuzione responsabilita richiede distinguere cambio stagione logico, disponibilita hardware e risposta reale impianto.

