# Funzione `KTS`

# SEZIONE A - DOCUMENTAZIONE TECNICA (ENGINEERING)

## A1. Nome funzione
`intercepts_messages_serial_1()`

## A2. File e posizione
- Parsing messaggi KTS su seriale 1: `src/Admin_Messages.c:151`
- Verifica presenza/riavvio alimentazione KTS: `src/HW_func.c:652` (`Test_Connect_KTS()`)
- Scheduler chiamate periodiche: `src/main.c:1023`, `src/main.c:1027`

## A3. Scopo tecnico
Gestisce il protocollo KTS su seriale 1: riceve comandi, aggiorna stati/parametri firmware, invia risposte e mantiene il monitoraggio del collegamento KTS con logica di reset alimentazione dedicata.

## A4. Parametri e ritorno
### Parametri in ingresso
- `intercepts_messages_serial_1()`: nessuno (usa buffer seriale globale)
- `Test_Connect_KTS()`: nessuno (usa misure/counter globali)

### Valore di ritorno
- `intercepts_messages_serial_1()`: `int`
  - `> RUN_DOWNLOAD`: messaggio KTS valido gestito
- `Test_Connect_KTS()`: `int`
  - `1` (funzione schedulabile attiva)

## A5. Logica dettagliata
- `intercepts_messages_serial_1()` legge il frame seriale 1 e valida il messaggio.
- Alla prima ricezione valida imposta connessione KTS attiva, azzera contatore no-link e rimuove `ALM_KTS_FAULT`.
- Esegue il comando richiesto (test unit, test output, buzzer, gestione dati, letture/scritture, risposte protocollo).
- `Test_Connect_KTS()` gira periodicamente e usa assorbimento KTS + contatore assenza link per stimare presenza/guasto.
- Se sovracorrente o timeout di mancata comunicazione oltre soglia: disalimenta KTS, inserisce `ALM_KTS_FAULT`, poi tenta riaccensione dopo delay.
- In test mode forzato mantiene KTS alimentato e disattiva fault link.

## A6. Mappa firmware con riferimenti puntuali
- **Passo**: ingresso parser KTS seriale 1
  **Dove**: `src/Admin_Messages.c:151`
  **Cosa osservare**: `intercepts_messages_serial_1()`, `Read_Message_Ser1()`
- **Passo**: recovery link KTS su messaggio valido
  **Dove**: `src/Admin_Messages.c:168`
  **Cosa osservare**: `connect_KTS`, `RemoveAlarm(ALM_KTS_FAULT)`, `cnt_sec_no_link_KTS`
- **Passo**: gestione comandi KTS
  **Dove**: `src/Admin_Messages.c:179`
  **Cosa osservare**: `switch (buff_ser1[IHM1_TYPE_COMAND])`
- **Passo**: watchdog assorbimento e timeout link KTS
  **Dove**: `src/HW_func.c:652`
  **Cosa osservare**: `Test_Connect_KTS()`, `measure_I_KTS`, `cnt_sec_no_link_KTS`
- **Passo**: spegnimento/riaccensione alimentazione KTS
  **Dove**: `src/HW_func.c:674`, `src/HW_func.c:681`
  **Cosa osservare**: `ioport_set_pin_level(pSwitchOFF_KTS, ...)`
- **Passo**: schedulazione funzioni KTS
  **Dove**: `src/main.c:1023`, `src/main.c:1027`
  **Cosa osservare**: `insert_function_scheduler(polling_serial_2, 6)`, `insert_function_scheduler(Test_Connect_KTS, 15)`

## A7. Interfacce esterne (livello astratto)
### Segnali generati dall'RD
- UART0 TX/RX (canale KTS)
  - Pin MCU: UART0 (`PA9/PA10`)
  - Tipo: UART seriale
  - Livello attivo: frame protocollo RD <-> KTS
  - Condizione attivazione: parser/comandi seriale 1
  - Persistenza: continua
- Alimentazione KTS abilitata/disabilitata
  - Pin MCU: `pSwitchOFF_KTS` (`PIOA,13`)
  - Tipo: GPIO output
  - Livello attivo: HIGH = KTS ON, LOW = KTS OFF
  - Condizione attivazione: logica `Test_Connect_KTS()`

### Segnali letti dall'RD
- Corrente assorbita KTS
  - Tipo ingresso: ADC (misura `measure_I_KTS`)
  - Frequenza campionamento: ciclo ADC + controllo ogni 1.5 s
  - Condizione validita: canale analogico attivo
- UART0 RX
  - Tipo ingresso: frame protocollo KTS
  - Condizione validita: frame valido

## A8. Catena causale (livello sistema)
| Livello | Descrizione |
|----------|------------|
| Decisione | Gestione protocollo KTS e watchdog link/alimentazione |
| Interfaccia | UART0 + GPIO alimentazione KTS + ADC corrente |
| Elettronica esterna | Dispositivo KTS e linea RJ45/seriale |
| Attuatore | Alimentazione KTS (ON/OFF) |
| Effetto | Comunicazione utente con unita e recupero automatico guasti link |

## A9. Feedback disponibile
- Esiste conferma reale?
  - Si, parziale: frame KTS validi ricevuti e misura reale corrente KTS.
- Il comando e "fire and forget"?
  - No: c'e monitoraggio continuo del link con fault/reset automatico.
- Dove viene letto il feedback?
  - In `intercepts_messages_serial_1()` (frame validi) e `Test_Connect_KTS()` (corrente + timeout).
- Quanto e affidabile?

Valori criteri:
- `Origine diretta: 1`
- `Correlazione temporale: 1`
- `Correlazione univoca col comando: 0`
- `Gestione errore: 1`
- `Punteggio totale: 3`
- `Classe finale: Forte`

## A10. Punti critici firmware
- La diagnosi link KTS combina due indicatori (traffico seriale e assorbimento), da mantenere coerenti.
- Soglie correnti/timeout (`CUNSUMER_MIN_KTS`, `TIMER_KTS_KO`) influenzano sensibilita falsi positivi.
- Test mode puo mascherare temporaneamente fault reali di connessione.

## A11. Punti di disaccoppiamento (sistema)
- Errore logico firmware: parsing comando KTS o gestione flag link/fault non coerente.
- Errore interfaccia: rumore seriale, frame invalidi, perdita sincronismo.
- Errore elettrico esterno: alimentazione KTS instabile, assorbimento anomalo, cablaggio RJ45 difettoso.
- Errore meccanico: non applicabile direttamente (funzione di comunicazione/diagnosi).

## A12. Limite di responsabilita firmware
Il firmware RD:
- riceve/interpreta i comandi KTS
- invia risposte e stati KTS
- monitora link e assorbimento per fault/reset KTS

Il firmware RD NON garantisce:
- integrita fisica del dispositivo KTS
- qualita del cablaggio/connessione esterna
- disponibilita continua del KTS se hardware esterno guasto
