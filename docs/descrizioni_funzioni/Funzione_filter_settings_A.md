# Funzione `filter_settings` (gestione filtri)

# SEZIONE A - DOCUMENTAZIONE TECNICA (ENGINEERING)

## A1. Nome funzione
`filter_settings` (blocco composto da `operatingHoursCounter()`, `check_clogged_filter()`, `verifica_clogged_filter()`, `calibrazione_clogged_filter()`)

## A2. File e posizione
Riferimenti principali:
- `src/Filter_Func.c:39` (`operatingHoursCounter`)
- `src/Filter_Func.c:124` (`check_clogged_filter`)
- `src/Filter_Func.c:221` (`verifica_clogged_filter`)
- `src/Filter_Func.c:370` (`calibrazione_clogged_filter`)

## A3. Scopo tecnico
Gestisce i settaggi e lo stato manutenzione filtri in due modalita:
- **DPS (timer)**: allarme filtro sporco su base ore/giorni.
- **DPP (pressostato)**: calibrazione e verifica periodica automatica del livello sporcamento.

## A4. Parametri e ritorno
### Parametri in ingresso
- Config EEPROM: `Enab_Fuction`, `gg_manut_Filter`, `time_lastCloggedFilters`, `DPP_Calibrationvalue`.
- Stato runtime: `countHoursFunz`, `status_unit`, `ALM_FILTER`.
- Lettura analogica pressostato: `analogRead(pAnSensQual_Ari1)`.

### Valore di ritorno
- `operatingHoursCounter()`: `int`, ritorna `1`.
- Le altre routine sono `void`.

## A5. Logica dettagliata
- `operatingHoursCounter()`:
  - aggiorna ore funzionamento da stato macchina attivo;
  - salva periodicamente in EEPROM;
  - invoca controllo/verifica/calibrazione filtri.
- `check_clogged_filter()`:
  - se `ENAB_CTRL_FILTER` disabilitato: rimuove allarme filtro e termina;
  - in **DPS** (`ENAB_DPP` OFF): confronta ore correnti con soglia `gg_manut_Filter`;
  - in **DPP** (`ENAB_DPP` ON): ogni 200 ore prepara richiesta verifica o fallback a DPS se non calibrato.
- `verifica_clogged_filter()` (richiesta verifica attiva):
  - prende controllo motori con `POS_BIT_DPP_CHECK`;
  - esegue step temporizzati e misura pressostato;
  - calcola `% sporcamento` vs soglia DPP;
  - inserisce `ALM_FILTER` se supera soglia, altrimenti aggiorna stato filtro.
- `calibrazione_clogged_filter()` (richiesta calibrazione attiva):
  - forza sequenza velocita motori per trovare punto scatto DPP;
  - salva calibrazione (0, 255 o valore intermedio);
  - rilascia controllo motori e chiude richiesta.

## A6. Mappa firmware con riferimenti puntuali
- **Passo**: ingresso e scheduler 1s
  **Dove**: `src/Filter_Func.c:39`
  **Cosa osservare**: `operatingHoursCounter()`
- **Passo**: conteggio ore e persistenza EEPROM
  **Dove**: `src/Filter_Func.c:64`
  **Cosa osservare**: `countHoursFunz`, `update_data_eeprom(...)`
- **Passo**: chiamata routine filtro
  **Dove**: `src/Filter_Func.c:110`
  **Cosa osservare**: `check/verifica/calibrazione`
- **Passo**: gate funzione controllo filtro
  **Dove**: `src/Filter_Func.c:141`
  **Cosa osservare**: `ENAB_CTRL_FILTER`
- **Passo**: ramo DPS temporale
  **Dove**: `src/Filter_Func.c:153`
  **Cosa osservare**: soglia `gg_manut_Filter * 24`, `ALM_FILTER`
- **Passo**: ramo DPP periodicita 200h
  **Dove**: `src/Filter_Func.c:166`
  **Cosa osservare**: modulo 200h, richiesta verifica
- **Passo**: ingresso verifica DPP
  **Dove**: `src/Filter_Func.c:239`
  **Cosa osservare**: richiesta `(DPP_FilterLevel & 0x03)==3`
- **Passo**: controllo motori durante verifica
  **Dove**: `src/Filter_Func.c:245`
  **Cosa osservare**: set `POS_BIT_DPP_CHECK`, modifica speed
- **Passo**: calcolo sporcamento e allarme
  **Dove**: `src/Filter_Func.c:276`
  **Cosa osservare**: `sporcamento`, `InsertAlarm(ALM_FILTER)`
- **Passo**: ingresso calibrazione DPP
  **Dove**: `src/Filter_Func.c:385`
  **Cosa osservare**: richiesta calibrazione `(bit0=1, bit1=0)`
- **Passo**: ricerca punto scatto e salvataggio calibrazione
  **Dove**: `src/Filter_Func.c:421`
  **Cosa osservare**: `DPP_Calibrationvalue`

## A7. Interfacce esterne (livello astratto)
### Segnali generati dall'RD
- Allarme filtro sporco (`ALM_FILTER`).
- Richieste verifica/calibrazione scritte in EEPROM (metadati DPP).
- Comando temporaneo velocita motori durante verifiche DPP.

### Segnali letti dall'RD
- Pressostato filtro (`pAnSensQual_Ari1` via ADC).
- Ore funzionamento e parametri manutenzione da EEPROM.
- Stato funzioni abilitate (`ENAB_CTRL_FILTER`, `ENAB_DPP`).

## A8. Catena causale (livello sistema)
| Livello | Descrizione |
|----------|------------|
| Decisione | Scelta DPS vs DPP e stato filtro sporco |
| Interfaccia | EEPROM + ADC pressostato + allarmi |
| Elettronica esterna | Sensore DPP / catena misura |
| Attuatore | Non diretto (eccetto override motori per test DPP) |
| Effetto | Segnalazione manutenzione filtri e stato diagnostico |

## A9. Feedback disponibile
- Esiste conferma reale?
  - Si, con feedback DPP analogico in modalita pressostato; in DPS solo tempo/contatore.
- Il comando e "fire and forget"?
  - No: la logica e ciclica con verifica periodica e aggiornamento stato.
- Dove viene letto il feedback?
  - Da `analogRead(pAnSensQual_Ari1)`, `countHoursFunz`, `DPP_Calibrationvalue` (`src/Filter_Func.c:266`, `src/Filter_Func.c:64`, `src/Filter_Func.c:423`).
- Quanto e affidabile?

Valori criteri:
- `Origine diretta: 1` (DPP diretto in modalita pressostato)
- `Correlazione temporale: 1` (finestre 200h, cicli temporizzati)
- `Correlazione univoca col comando: 1` (stati richiesta DPP dedicati)
- `Gestione errore: 1` (fallback DPP->DPS, clear/set allarmi, reset richieste)
- `Punteggio totale: 4`
- `Classe finale: Forte`

## A10. Punti critici firmware
- Uso misto DPS/DPP con semantiche diverse su `gg_manut_Filter` (giorni vs word con payload DPP).
- Dipendenza da calibrazione DPP valida; calibrazione nulla porta fallback.
- Override motori durante DPP check puo interferire con altre logiche se non gestito correttamente.
- Aggiornamenti EEPROM frequenti richiedono coerenza contatori update.

## A11. Punti di disaccoppiamento (sistema)
- Errore logico firmware: richiesta verifica/calibrazione non chiusa correttamente.
- Errore interfaccia: ADC pressostato non affidabile o rumore segnale.
- Errore elettrico esterno: DPP scollegato/alimentazione sensore assente.
- Errore meccanico: filtro realmente sporco ma curva impianto altera soglia scatto.

## A12. Limite di responsabilita firmware
Il firmware RD:
- conteggia ore e gestisce pianificazione manutenzione filtri
- esegue verifica/calibrazione DPP automatica
- imposta/rimuove allarme filtro sporco

Il firmware RD NON garantisce:
- accuratezza assoluta del pressostato in campo
- stato fisico filtro dedotto perfettamente in tutte le condizioni impiantistiche
- correttezza installativa della linea sensore
