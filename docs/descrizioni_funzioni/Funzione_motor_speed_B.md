# Funzione `motor_speed`

# 🟢 SEZIONE B — DOCUMENTAZIONE NON TECNICA (OPERATIVA / CAMPO)

## B1. Racconto operativo
- Cosa accade nel sistema: la funzione determina la velocita finale dei ventilatori combinando setpoint base e priorita operative (input, weekly, CAP/CAF, qualita aria, safety, bypass).
- Quando accade: a ogni chiamata scheduler della regolazione motori.
- Perche puo non accadere: l'azione richiesta puo essere bloccata da stato standby, allarmi, bypass in movimento o vincoli di protezione.
- Eventuali ritardi voluti e relazioni con altri accessori: sono previsti tempi di startup, post-ventilazione e filtri di transizione; il risultato dipende dallo stato di accessori e funzioni concorrenti (clima, sensori aria, ingressi esterni).
## B2. Comportamento normale vs percezione anomala
- E normale vedere velocita bloccata al minimo durante manovra bypass o in startup.
- E normale vedere un ramo diverso dall'altro quando imbalance e attivo.
- E normale che in standby i motori restino fermi anche se i setpoint sono validi.
- Puo sembrare anomalo il passaggio a velocita alta, ma puo essere dovuto a VOC/RH/CO2 o boost.

## B3. Errori bloccanti a monte
- Logica non autorizzata
  - Unita in standby da input/weekly/power
  - Controllo MBF che sovrascrive la logica locale
- Logica in protezione
  - Allarmi fan control o allarmi preheater acqua
  - Modalita che impongono early return
- Logica attiva ma attuatore non funzionante
  - Comando aggiornato in firmware senza risposta reale motore

## B4. Checklist problem solving
1. Cosa dovrebbe accadere: velocita attesa su ramo return/fresh
2. Dati reali disponibili: speed command R/F, stato RUN/STANDBY, status allarmi
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
La responsabilita va attribuita distinguendo sempre decisione firmware, applicazione elettrica del comando e risultato meccanico reale.

