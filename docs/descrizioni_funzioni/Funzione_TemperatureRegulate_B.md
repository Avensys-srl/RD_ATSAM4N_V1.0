# Funzione `TemperatureRegulate`

# ðŸŸ¢ SEZIONE B â€” DOCUMENTAZIONE NON TECNICA (OPERATIVA / CAMPO)

## B1. Racconto operativo
- Cosa accade nel sistema: la funzione decide quali accessori clima attivare/disattivare (preheater, heater, cooler, DXD) e coordina la logica termica con il bypass.
- Quando accade: a ogni ciclo della regolazione temperatura, in base a setpoint, sonde, modalita stagione e stati accessori.
- Perche puo non accadere: comandi e regolazioni possono essere inibiti da allarmi, motori non disponibili, funzione disabilitata o condizioni termiche non compatibili.
- Eventuali ritardi voluti e relazioni con altri accessori: sono presenti isteresi e contatori temporali per evitare ON/OFF rapidi; la logica e vincolata allo stato del bypass e alla disponibilita degli accessori I2C.
## B2. Comportamento normale vs percezione anomala
E normale che caldo/freddo non partano esattamente sul valore setpoint per presenza di isteresi. In alcuni casi il cooler resta spento anche con richiesta freddo perche la logica prova prima il bypass. Se ci sono fault ventilazione o allarmi compressore, lo spegnimento e comportamento corretto.

## B3. Errori bloccanti a monte
- Logica non autorizzata: unita non in stato operativo o funzione stagionale disabilitata
- Logica in protezione: fault motori, allarmi compressore, condizioni sicurezza attive
- Logica attiva ma attuatore non funzionante: comando emesso ma nessun effetto termico reale

## B4. Checklist problem solving
1. Cosa dovrebbe accadere: caldo, freddo, mantenimento o stop protetto
2. Dati reali disponibili: setpoint, temperatura riferimento, stato uscite clima
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
La responsabilita operativa va attribuita dopo verifica completa di condizioni logiche, interfacce e risposta impianto, non dal solo stato firmware.

