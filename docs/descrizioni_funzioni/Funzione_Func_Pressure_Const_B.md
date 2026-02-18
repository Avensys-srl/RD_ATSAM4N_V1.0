# Funzione `Func_Pressure_Const`

# 🟢 SEZIONE B — DOCUMENTAZIONE NON TECNICA (OPERATIVA / CAMPO)

## B1. Racconto operativo
- Cosa accade nel sistema: in modalita CAP il controllo varia la velocita ventilatori per mantenere la pressione aria al valore impostato.
- Quando accade: a ogni ciclo utile del regolatore, confrontando pressione attuale e setpoint.
- Perche puo non accadere: se i motori non ruotano a sufficienza, se il sensore pressione non e operativo, o se vengono raggiunti limiti min/max di velocita.
- Eventuali ritardi voluti e relazioni con altri accessori: la correzione e volutamente dilazionata su piu cicli per evitare oscillazioni; quando presente il sensore CAP il comportamento dipende dalla sua disponibilita/qualita segnale.
## B2. Comportamento normale vs percezione anomala
- E normale che la regolazione non sia istantanea: ci sono finestre temporali e step progressivi.
- E normale vedere piccoli assestamenti continui attorno al valore target.
- Se rpm sono troppo bassi la funzione sospende la regolazione.

## B3. Errori bloccanti a monte
- Logica non autorizzata: CAP non selezionato o sovrascritto da modalita prioritaria
- Logica in protezione: condizioni safety generali che bloccano la gestione motori
- Logica attiva ma attuatore non funzionante: comando speed presente ma pressione non reagisce

## B4. Checklist problem solving
1. Cosa dovrebbe accadere: mantenimento pressione vicino al setpoint CAP
2. Dati reali disponibili: setpoint CAP, pressione misurata/stimata, rpm, speed comando
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
La responsabilita dell'esito CAP va valutata separando calcolo firmware, qualita feedback e risposta reale impianto.

