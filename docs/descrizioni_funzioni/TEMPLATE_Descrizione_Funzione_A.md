# Template descrizione funzione

# 🔷 SEZIONE A — DOCUMENTAZIONE TECNICA (ENGINEERING)

> Questa sezione è destinata a firmware, progettazione elettronica e validazione tecnica.
> Deve essere oggettiva, verificabile e con riferimenti puntuali al codice.

---

## A1. Nome funzione

Indicare il nome esatto della funzione come dichiarata nel firmware.

Esempio:
`managementBypass(unsigned short enab_func)`

Scopo: identificazione univoca nel repository.

---

## A2. File e posizione

Indicare:
- Percorso file
- Numero di riga di inizio funzione

Esempio:
`src/Clima_Func.c:93`

Scopo: garantire tracciabilità e audit del codice.

---

## A3. Scopo tecnico

Descrivere cosa fa la funzione a livello logico firmware.

⚠ Non descrivere effetti meccanici come se fossero sotto controllo diretto del RD.

Rispondere alla domanda:
> Quale decisione logica prende il firmware?

---

## A4. Parametri e ritorno

### Parametri in ingresso
Per ciascun parametro indicare:
- Tipo
- Unità di misura
- Range ammesso
- Valori invalidi gestiti
- Effetto sul comportamento

### Valore di ritorno
Indicare:
- Tipo
- Significato
- Eventuale uso nel chiamante

Scopo: chiarire il contratto logico della funzione.

---

## A5. Logica dettagliata

Descrivere in modo strutturato:

- Sequenza operativa
- Rami decisionali
- Priorità rispetto ad altre logiche
- Filtri temporali (isteresi, timer, persistenze)
- Fallback
- Modifiche stato globale
- Generazione segnali logici

Rispondere alla domanda:
> Quali condizioni producono quale decisione?

Non citare schede esterne.

---

## A6. Mappa firmware con riferimenti puntuali

Per ogni passo importante indicare:

- **Passo**: cosa accade
- **Dove**: file + riga precisa
- **Cosa osservare**: variabile o condizione che dimostra il passo

Scopo: rendere verificabile ogni affermazione.

Regole:
- Nessun passo senza riga
- Vietato usare “circa”
- Aggiornare sempre dopo modifica firmware

---

## A7. Interfacce esterne (livello astratto)

Descrivere SOLO ciò che l’RD controlla o legge.

### Segnali generati
Indicare:
- Pin MCU
- Tipo (GPIO / PWM / UART / I2C / SPI / ADC)
- Livello attivo
- Condizione di attivazione
- Persistenza

### Segnali letti
Indicare:
- Tipo ingresso
- Frequenza campionamento
- Condizione di validità

⚠ Non indicare nomi di schede riceventi.

Scopo:
Separare decisione firmware da hardware esterno.

---

## A8. Catena causale (livello sistema)

Compilare la tabella:

| Livello | Descrizione |
|----------|------------|
| Decisione | Logica firmware RD |
| Interfaccia | Segnale elettrico / bus |
| Elettronica esterna | Circuito ricevente |
| Attuatore | Dispositivo fisico |
| Effetto | Risultato osservabile |

Scopo:
Visualizzare la sequenza completa dal codice all’effetto fisico.

---

## A9. Feedback disponibile

Indicare:

- Esiste conferma reale?
- Il comando è “fire and forget”?
- Dove viene letto il feedback?
- Quanto è affidabile?

Valutare l'affidabilita con i seguenti criteri (valore `0` o `1` per ciascuno):

| Criterio | Valore | Regola di assegnazione |
|----------|--------|------------------------|
| Origine diretta | 0/1 | `1` se il feedback proviene da sorgente che conferma esecuzione reale (bit stato HW, ACK dispositivo, misura sensore); `0` se solo conferma software interna (es. comando inviato). |
| Correlazione temporale | 0/1 | `1` se esiste timeout esplicito e feedback entro tempo massimo definito; `0` se non esiste vincolo temporale verificabile. |
| Correlazione univoca col comando | 0/1 | `1` se il feedback e associato in modo certo a quell'istanza comando (ID, sequenza, contesto univoco); `0` se ambiguo o generico. |
| Gestione errore | 0/1 | `1` se sono gestiti esiti negativi espliciti (NACK, timeout, fault) con azione definita; `0` se non gestiti. |

Classificazione finale:

- Calcolare `PunteggioTotale = OrigineDiretta + CorrelazioneTemporale + CorrelazioneUnivoca + GestioneErrore`
- Assegnare la classe:
  - `0` = Nessun feedback (fire-and-forget puro)
  - `1` = Feedback debole
  - `2` = Feedback medio
  - `3-4` = Feedback forte (alta affidabilita)

Formato compilazione consigliato:

- `Origine diretta: <0|1>`
- `Correlazione temporale: <0|1>`
- `Correlazione univoca col comando: <0|1>`
- `Gestione errore: <0|1>`
- `Punteggio totale: <0..4>`
- `Classe finale: <Nessun feedback|Debole|Medio|Forte>`

Scopo:
Capire se il firmware può verificare l’esecuzione reale.

---

## A10. Punti critici firmware

Descrivere punti interni al codice dove:

- Un errore logico può alterare la decisione
- Una condizione è ambigua
- Un confronto può generare comportamento inatteso
- Un timer o contatore può non essere coerente

Scopo:
Individuare vulnerabilità logiche interne.

---

## A11. Punti di disaccoppiamento (sistema)

Descrivere punti dove la catena può interrompersi anche se il firmware è corretto:

- Segnale non propagato
- Bus non inizializzato
- Alimentazione esterna assente
- Attuatore bloccato
- Feedback assente
- Modalità che inibisce

Separare sempre:

- Errore logico firmware
- Errore interfaccia
- Errore elettrico esterno
- Errore meccanico

Scopo:
Proteggere il perimetro di responsabilità.

---

## A12. Limite di responsabilità firmware

Il firmware RD:

- Decide quando generare un segnale
- Imposta livelli logici
- Invia comandi su bus
- Monitora feedback disponibili

Il firmware RD NON garantisce:

- Integrità circuiti esterni
- Presenza attuatore
- Effettiva esecuzione meccanica
- Corretta alimentazione esterna

Scopo:
Formalizzare il confine tecnico.

---

