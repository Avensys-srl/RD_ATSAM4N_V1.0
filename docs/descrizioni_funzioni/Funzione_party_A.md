# Funzione `PARTY (BOOST)`

# SEZIONE A - DOCUMENTAZIONE TECNICA (ENGINEERING)

## A1. Nome funzione
`PARTY (BOOST)` - logica interna in `motor_speed()`

## A2. File e posizione
`src/motor_speed.c:2956`

## A3. Scopo tecnico
Forza temporaneamente i ventilatori alla massima velocita quando il bit `ENAB_BOOST` e attivo, e disattiva automaticamente la funzione allo scadere del tempo configurato.

## A4. Parametri e ritorno
### Parametri in ingresso
- Nessun parametro funzione dedicato (la logica usa variabili/stati globali):
  - `enab_func` (bitmask funzioni abilitate)
  - `Set_TimeBoost` (EEPROM, minuti)
  - `count_Boost_on` (contatore cicli)

### Valore di ritorno
- Non applicabile (blocco interno alla funzione `motor_speed()`).

## A5. Logica dettagliata
- Verifica se `ENAB_BOOST` e attivo in `enab_func`.
- Incrementa `count_Boost_on` a ogni ciclo di `motor_speed()`.
- Legge `Set_TimeBoost` da EEPROM e lo converte in cicli (`minuti * 50`, dato ciclo ~1.2 s).
- Finche `count_Boost_on < setPoint` forza `temp_speed_motorsR/F = 255` e imposta `POS_BIT_BOOST_OPERATING`.
- A timeout raggiunto:
  - disattiva `ENAB_BOOST` in EEPROM,
  - azzera `count_Boost_on`,
  - incrementa `cntUpdate_info`,
  - rimuove la modalita boost dai cicli successivi.
- Se boost non attivo, pulisce `POS_BIT_BOOST_OPERATING`.

## A6. Mappa firmware con riferimenti puntuali
- **Passo**: ingresso ramo BOOST
  **Dove**: `src/motor_speed.c:2957`
  **Cosa osservare**: `if (enab_func & (1 << ENAB_BOOST))`
- **Passo**: incremento contatore e lettura tempo boost
  **Dove**: `src/motor_speed.c:2958`
  **Cosa osservare**: `count_Boost_on++`, `Set_TimeBoost`
- **Passo**: conversione minuti in cicli
  **Dove**: `src/motor_speed.c:2960`
  **Cosa osservare**: `setPoint *= 50`
- **Passo**: forzatura velocita massima
  **Dove**: `src/motor_speed.c:2968`
  **Cosa osservare**: `temp_speed_motorsR = 255`, `temp_speed_motorsF = 255`
- **Passo**: stato operativo BOOST
  **Dove**: `src/motor_speed.c:2970`
  **Cosa osservare**: `POS_BIT_BOOST_OPERATING`
- **Passo**: scadenza boost e disabilitazione funzione
  **Dove**: `src/motor_speed.c:2973`
  **Cosa osservare**: clear bit `ENAB_BOOST`, write EEPROM
- **Passo**: reset flag boost quando non attivo
  **Dove**: `src/motor_speed.c:2988`
  **Cosa osservare**: clear `POS_BIT_BOOST_OPERATING`

## A7. Interfacce esterne (livello astratto)
### Segnali generati dall'RD
- Comando velocita ventilatori:
  - Tipo: PWM/controllo velocita motori
  - Livello attivo: massimo (`255`)
  - Condizione di attivazione: `ENAB_BOOST` attivo e timer non scaduto
  - Persistenza: fino a timeout BOOST

### Segnali letti dall'RD
- Configurazione EEPROM:
  - `Enab_Fuction` (bit `ENAB_BOOST`)
  - `Set_TimeBoost` (durata)
- Stato runtime:
  - `count_Boost_on`
  - `sData.status_unit` bit `POS_BIT_BOOST_OPERATING`

## A8. Catena causale (livello sistema)
| Livello | Descrizione |
|----------|------------|
| Decisione | Abilita/disabilita boost in base a bit funzione e tempo |
| Interfaccia | Scrittura setpoint velocita motori a massimo |
| Elettronica esterna | Driver ventilatori |
| Attuatore | Motori mandata/estrazione |
| Effetto | Ventilazione al massimo per tempo limitato |

## A9. Feedback disponibile
- Esiste conferma reale?
  - Parziale: conferma interna via stato/contatore e bit operativi; non c'e ack hardware dedicato alla modalita PARTY.
- Il comando e "fire and forget"?
  - No: il blocco ricontrolla stato e timeout a ogni ciclo.
- Dove viene letto il feedback?
  - In `count_Boost_on`, `enab_func` e `sData.status_unit` (`src/motor_speed.c:2958`, `src/motor_speed.c:2957`, `src/motor_speed.c:2970`).
- Quanto e affidabile?

Valori criteri:
- `Origine diretta: 0` (feedback principalmente software interno)
- `Correlazione temporale: 1` (timeout esplicito tramite `Set_TimeBoost`)
- `Correlazione univoca col comando: 1` (bit `ENAB_BOOST` e contatore dedicato)
- `Gestione errore: 1` (auto-disabilitazione, reset contatore e stato)
- `Punteggio totale: 3`
- `Classe finale: Forte`

## A10. Punti critici firmware
- Conversione tempo `minuti -> cicli` dipende dal periodo reale dello scheduler.
- Se `Set_TimeBoost` e fuori range, durata non coerente (range validato altrove).
- Interazioni con altre priorita velocita in `motor_speed()` possono ridurre prevedibilita percepita.

## A11. Punti di disaccoppiamento (sistema)
- Errore logico firmware:
  - bit boost non aggiornato correttamente
  - contatore non azzerato in condizioni anomale
- Errore interfaccia:
  - scrittura EEPROM `Enab_Fuction`/`cntUpdate_info` fallita
- Errore elettrico esterno:
  - driver motori non in grado di erogare velocita massima
- Errore meccanico:
  - ventole bloccate/non in regime nonostante comando max

## A12. Limite di responsabilita firmware
Il firmware RD:
- abilita boost a velocita massima per tempo configurato
- spegne automaticamente boost a timeout
- aggiorna stato e configurazione persistente

Il firmware RD NON garantisce:
- portata reale massima in campo
- integrita elettrica/meccanica della catena ventilatori
- rispetto assoluto del tempo se scheduler non stabile
