# Template descrizione funzione

# 🟢 SEZIONE B — DOCUMENTAZIONE NON TECNICA (OPERATIVA / CAMPO)

> Sezione destinata a service, installatori e supporto tecnico.
> Non citare file o righe firmware.

---

## B1. Racconto operativo

Descrivere in modo semplice:

- Cosa accade nel sistema
- Quando accade
- Perché può non accadere
- Eventuali ritardi voluti e relazioni con altri accessori (esplicitale se vincolanti nel funzionamento )

Rispondere alla domanda:
> Cosa vede l’utente?

---

## B2. Comportamento normale vs percezione anomala

Spiegare:

- Quando un comportamento è corretto ma può sembrare errato
- Tempi di attesa normali
- Condizioni che generano falsa diagnosi

Scopo:
Ridurre interventi non necessari.

---

## B3. Errori bloccanti a monte

Elencare condizioni che impediscono effetto fisico:

- Modalità test
- Allarmi attivi
- Funzione disabilitata
- Sensore non valido
- Assenza alimentazione
- Attuatore guasto

Distinguere:

- Logica non autorizzata
- Logica in protezione
- Logica attiva ma attuatore non funzionante

---

## B4. Checklist problem solving

1. Cosa dovrebbe accadere?
2. Modalità selezionata?
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

---

## B5. Nota gestionale (facoltativa)

⚠ L’attribuzione della responsabilità è una decisione gestionale.
Non può essere dedotta automaticamente dal solo comportamento firmware.

---

# Regola finale

Una funzione è considerata documentata solo se:

- È separata decisione logica da effetto fisico
- Sono indicati punti critici firmware
- Sono indicati punti di disaccoppiamento
- È presente checklist operativa
- È esplicitato limite responsabilità

