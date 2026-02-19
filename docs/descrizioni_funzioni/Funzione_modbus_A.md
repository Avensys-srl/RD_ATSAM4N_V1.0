# Funzione `modbus`

# SEZIONE A - DOCUMENTAZIONE TECNICA (ENGINEERING)

## A1. Nome funzione
`polling_ModBus(void)`

## A2. File e posizione
- Funzione principale: `src/Admin_Messages.c:1604`
- Poll scheduler seriale 2: `src/Admin_Messages.c:1965`
- Parsing messaggi in ingresso (Modbus/Remote CTRL): `src/Admin_Messages.c:801`
- Mappatura allarmi in byte protocollo: `src/Admin_Messages.c:1525`, `src/Admin_Messages.c:1568`

## A3. Scopo tecnico
Costruisce e invia periodicamente i pacchetti dati su seriale 2 verso il controllo remoto Modbus, acquisisce i messaggi di risposta/comando e aggiorna stato macchina, allarmi e parametri condivisi.

## A4. Parametri e ritorno
### Parametri in ingresso
- `polling_ModBus(void)`: nessuno
- `intercepts_messages_serial_2()`: nessuno (usa buffer globali seriale 2)

### Valore di ritorno
- `polling_ModBus(void)`: `void`
- `intercepts_messages_serial_2()`: `int`
  - `> RUN_DOWNLOAD`: messaggio valido gestito
  - `< 0`: errore protocollo (header/lunghezza/checksum)

## A5. Logica dettagliata
- `polling_serial_2()` verifica presenza hardware remota tramite ingresso digitale dedicato.
- Se presente, alterna i poll tra `polling_ModBus()` e `polling_CTRL_FAN()`.
- `polling_ModBus()` compone due pacchetti alternati (packet 1 / packet 2) con stati, misure, setpoint, allarmi aggregati e livello link.
- Ogni invio incrementa contatore perdita link; oltre soglia inserisce `ALM_EB_REM_CTRL`.
- `intercepts_messages_serial_2()` valida i frame ricevuti e gestisce i comandi (polling fan, read/write EEPROM, update stato remote control).
- In caso di risposta valida azzera contatore no-link e rimuove `ALM_EB_REM_CTRL`.
- In caso errore protocollo libera il canale seriale 2 (`free_serial_2()`).

## A6. Mappa firmware con riferimenti puntuali
- **Passo**: polling seriale 2 con selezione ramo Modbus/CTRL_FAN
  **Dove**: `src/Admin_Messages.c:1965`
  **Cosa osservare**: `ioport_get_pin_level(pInsertEB_ModBus)`, `index_poll_ser2`
- **Passo**: composizione byte allarmi LO
  **Dove**: `src/Admin_Messages.c:1525`
  **Cosa osservare**: `modbus_alarm_lo_from_events()`
- **Passo**: composizione byte allarmi HI
  **Dove**: `src/Admin_Messages.c:1568`
  **Cosa osservare**: `modbus_alarm_hi_from_events()`
- **Passo**: costruzione pacchetto dati Modbus
  **Dove**: `src/Admin_Messages.c:1604`
  **Cosa osservare**: `polling_ModBus()`, campi `IPK1U_*`, `IPK2U_*`
- **Passo**: gestione livello link e allarme remoto
  **Dove**: `src/Admin_Messages.c:1944`
  **Cosa osservare**: `cnt_no_link_Modbus`, `InsertAlarm(ALM_EB_REM_CTRL)`
- **Passo**: parsing comandi ricevuti seriale 2
  **Dove**: `src/Admin_Messages.c:801`
  **Cosa osservare**: `intercepts_messages_serial_2()`, switch `IHM2_TYPE_COMAND`
- **Passo**: recovery su risposta valida
  **Dove**: `src/Admin_Messages.c:936`
  **Cosa osservare**: reset `cnt_no_link_Modbus`, `RemoveAlarm(ALM_EB_REM_CTRL)`

## A7. Interfacce esterne (livello astratto)
### Segnali generati dall'RD
- UART2 TX/RX
  - Pin MCU: UART2 (`PA15/PA16`)
  - Tipo: UART seriale
  - Livello attivo: frame protocollo RD <-> remote control
  - Condizione attivazione: scheduler `polling_serial_2`
  - Persistenza: ciclica

### Segnali letti dall'RD
- Ingresso presenza modulo remoto
  - Pin MCU: `pInsertEB_ModBus` (`PIOA,29`)
  - Tipo ingresso: GPIO digitale
  - Frequenza campionamento: ciclo `polling_serial_2` (~0.6 s)
  - Condizione validita: livello pin alto
- UART2 RX
  - Tipo ingresso: frame protocollo
  - Condizione validita: frame con header/lunghezza/checksum corretti

## A8. Catena causale (livello sistema)
| Livello | Descrizione |
|----------|------------|
| Decisione | Poll ciclico, invio dati e gestione comandi Modbus |
| Interfaccia | UART2 + GPIO presenza modulo |
| Elettronica esterna | Scheda/modulo remote control |
| Attuatore | Non diretto (controllo logico remoto) |
| Effetto | Supervisione, configurazione e telemetria unita |

## A9. Feedback disponibile
- Esiste conferma reale?
  - Si, parziale: risposta seriale valida e aggiornamento campi ricevuti.
- Il comando e "fire and forget"?
  - No: polling continuo con controllo link e gestione timeout implicita via contatore.
- Dove viene letto il feedback?
  - In `intercepts_messages_serial_2()` e nei contatori `cnt_no_link_Modbus`.
- Quanto e affidabile?

Valori criteri:
- `Origine diretta: 1`
- `Correlazione temporale: 1`
- `Correlazione univoca col comando: 0`
- `Gestione errore: 1`
- `Punteggio totale: 3`
- `Classe finale: Forte`

## A10. Punti critici firmware
- Doppia implementazione condizionale di `intercepts_messages_serial_2()` (compilazione con/senza `NO_TACHO`).
- Allarme link remoto dipende da soglia contatore e frequenza scheduler effettiva.
- Mapping allarmi aggregati Modbus richiede coerenza continua con enum `ALM_*`.

## A11. Punti di disaccoppiamento (sistema)
- Errore logico firmware: parsing frame o mapping campi errato.
- Errore interfaccia: UART2 disturbata, framing/checksum invalidi.
- Errore elettrico esterno: modulo remoto non alimentato o pin presenza non valido.
- Errore meccanico: non applicabile direttamente (funzione di comunicazione).

## A12. Limite di responsabilita firmware
Il firmware RD:
- costruisce e invia pacchetti Modbus su UART2
- valida e interpreta le risposte
- aggiorna stato link/allarmi remoti

Il firmware RD NON garantisce:
- disponibilita continua del modulo remoto esterno
- integrita fisica cablaggio/linea seriale
- applicazione effettiva lato dispositivo remoto di ogni comando ricevuto
