# Funzione `allarmi_firmware` (catalogo completo)

# SEZIONE A - DOCUMENTAZIONE TECNICA (ENGINEERING)

## A1. Nome funzione
`allarmi_firmware` (catalogo trasversale degli allarmi `ALM_*`)

## A2. File e posizione
- Definizione allarmi: `src/alarm.h:34`
- Gestione base allarmi: `src/Alarm.c:24`
- Esposizione protocollo allarmi: `src/Admin_Messages.c:1525`

## A3. Scopo tecnico
Documentare in modo completo tutti gli allarmi presenti nel firmware, la loro organizzazione in byte/bit e la semantica funzionale.

## A4. Parametri e ritorno
### Parametri in ingresso
- Non applicabile (documento di catalogo)

### Valore di ritorno
- Non applicabile

## A5. Logica dettagliata
- Gli allarmi sono codificati in `sData.Events[]` come bitmap (byte/bit).
- Ogni identificatore `ALM_*` corrisponde a una posizione univoca.
- La libreria allarmi (`Alarm.c`) fornisce API: init, check, insert, remove, insert-delay.
- I moduli applicativi inseriscono/rimuovono allarmi in base alle loro condizioni runtime.

## A6. Mappa firmware con riferimenti puntuali
- **Passo**: definizione enum completo allarmi
  **Dove**: `src/alarm.h:34`
  **Cosa osservare**: `_LISTA_ALLARMI`
- **Passo**: struttura byte eventi
  **Dove**: `src/alarm.h:15`
  **Cosa osservare**: commenti `[00]..[13]`
- **Passo**: check/set/clear allarme
  **Dove**: `src/Alarm.c:39`
  **Cosa osservare**: `CkAlarm`, `InsertAlarm`, `RemoveAlarm`
- **Passo**: mapping protocollo modbus/kts
  **Dove**: `src/Admin_Messages.c:1525`
  **Cosa osservare**: `modbus_alarm_lo_from_events`, `modbus_alarm_hi_from_events`

## A7. Interfacce esterne (livello astratto)
### Segnali generati dall'RD
- Stati allarme aggregati verso protocollo (KTS/seriale/Modbus)

### Segnali letti dall'RD
- Eventi runtime provenienti da moduli sensori, accessori, motori, bypass, filtri, controllo remoto

## A8. Catena causale (livello sistema)
| Livello | Descrizione |
|----------|------------|
| Decisione | Moduli firmware determinano condizioni di fault |
| Interfaccia | Inserimento bit `ALM_*` in `sData.Events[]` |
| Elettronica esterna | Sensori/accessori/driver che originano fault |
| Attuatore | Non diretto |
| Effetto | Segnalazione allarme e azioni di protezione a valle |

## A9. Feedback disponibile
- Esiste conferma reale?
  - Parziale: il bit allarme conferma lo stato logico rilevato dal firmware.
- Il comando e "fire and forget"?
  - No: gli allarmi sono mantenuti, verificati e rimossi ciclicamente.
- Dove viene letto il feedback?
  - In `sData.Events[]` via `CkAlarm()` e moduli protocollo.
- Quanto e affidabile?

Valori criteri:
- `Origine diretta: 0`
- `Correlazione temporale: 1`
- `Correlazione univoca col comando: 1`
- `Gestione errore: 1`
- `Punteggio totale: 3`
- `Classe finale: Forte`

## A10. Punti critici firmware
- Semantica allarme dipende dal modulo chiamante.
- Allarmi `NONE` riservati possono creare ambiguita se usati impropriamente.
- Possibili disallineamenti tra evento reale e pulizia allarme.

## A11. Punti di disaccoppiamento (sistema)
- Errore logico firmware: bit sbagliato o clear prematuro.
- Errore interfaccia: fault reale non propagato all'API allarmi.
- Errore elettrico esterno: causa fisica presente ma non misurata.
- Errore meccanico: anomalia campo non riflessa nei sensori.

## A12. Limite di responsabilita firmware
Il firmware RD:
- codifica, mantiene ed espone gli allarmi `ALM_*`
- supporta persistenza e reset allarmi

Il firmware RD NON garantisce:
- identificazione perfetta della causa fisica root-cause
- assenza di fault intermittenti non campionati

## Catalogo completo allarmi firmware (`ALM_*`)

### Byte 00 - Accessorio EHD/HWD
- `ALM_EHD_NTC_KO`: sonda NTC guasta
- `ALM_EHD_TW_DANGER`: temperatura acqua in zona pericolo
- `ALM_EHD_TW_OVR`: temperatura acqua fuori range
- `ALM_EHD_ELECT`: fault elettrico accessorio
- `ALM_EHD_TA_OVR`: temperatura aria fuori range
- `ALM_EHD_SIZE`: mismatch taglia accessorio/unita
- `ALM_EHD_PRESS`: pressione/portata non conforme
- `ALM_EHD_LINK`: link comunicazione accessorio assente

### Byte 01 - Accessorio CWD
- `ALM_CWD_NTC_KO`
- `ALM_CWD_TW_DANGER`
- `ALM_CWD_TW_OVR`
- `ALM_CWD_ELECT`
- `ALM_CWD_TA_OVR`
- `ALM_CWD_SIZE`
- `ALM_NONE_14` (riservato)
- `ALM_CWD_LINK`

### Byte 02 - Accessorio PEHD/PHWD
- `ALM_PEH_NTC_KO`
- `ALM_PEH_TW_DANGER`
- `ALM_PEH_TW_OVR`
- `ALM_PEH_ELECT`
- `ALM_PEH_TA_OVR`
- `ALM_PEH_SIZE`
- `ALM_PEHD_PRESS`
- `ALM_PEH_LINK`

### Byte 03 - DSC / CO2 esterni
- `ALM_DSC_KO`
- `ALM_DSC_LINK`
- `ALM_PCO2_EX1_KO`
- `ALM_PCO2_EX1_LINK`
- `ALM_NONE_28` (riservato)
- `ALM_PCO2_EX2_KO`
- `ALM_PCO2_EX2_LINK`
- `ALM_NONE_31` (riservato)

### Byte 04 - RH esterni
- `ALM_NONE_32` (riservato)
- `ALM_NONE_33` (riservato)
- `ALM_PRH_EX1_KO`
- `ALM_PRH_EX1_LINK`
- `ALM_NONE_36` (riservato)
- `ALM_PRH_EX2_KO`
- `ALM_PRH_EX2_LINK`
- `ALM_NONE_39` (riservato)

### Byte 05 - VOC / DXD
- `ALM_NONE_40` (riservato)
- `ALM_NON_41` (riservato)
- `ALM_VOC_EX1_KO`
- `ALM_VOC_EX1_LINK`
- `ALM_NONE_44` (riservato)
- `ALM_NONE_45` (riservato)
- `ALM_DXD_COMPRESS`
- `ALM_DXD_LINK`

### Byte 06 - Sensori pressione
- `ALM_PCAP_KO`
- `ALM_PCAP_LINK`
- `ALM_NONE_50` (riservato)
- `ALM_PCAF_KO`
- `ALM_PCAF_LINK`
- `ALM_NONE_53` (riservato)
- `ALM_DPP_KO`
- `ALM_DPP_LINK`

### Byte 07 - Fire mode / AWP
- `ALM_SMOKE_VENT_OFF`
- `ALM_SMOKE_VENT_MAX`
- `ALM_SMOKE_ONLY_EXT`
- `ALM_SMOKE_ONLY_SUP`
- `ALM_NONE_60` (riservato)
- `ALM_NONE_61` (riservato)
- `ALM_AWP_KO`
- `ALM_AWP_LINK`

### Byte 08 - Motori ramo R
- `ALM_MR1_NO_PWR`
- `ALM_MR2_NO_PWR`
- `ALM_MR3_NO_PWR`
- `ALM_NONE_67` (riservato)
- `ALM_MR1_NO_PULSE`
- `ALM_MR2_NO_PULSE`
- `ALM_MR3_NO_PULSE`
- `ALM_NONE_71` (riservato)

### Byte 09 - Motori ramo F
- `ALM_MF1_NO_PWR`
- `ALM_MF2_NO_PWR`
- `ALM_MF3_NO_PWR`
- `ALM_NONE_75` (riservato)
- `ALM_MF1_NO_PULSE`
- `ALM_MF2_NO_PULSE`
- `ALM_MF3_NO_PULSE`
- `ALM_NONE_79` (riservato)

### Byte 10 - Allarmi misti sistema
- `ALM_EB_CTRL_FAN`
- `ALM_EB_REM_CTRL`
- `ALM_BATT_KO`
- `ALM_BYPASS_KO`
- `ALM_EEP_FAULT`
- `ALM_FILTER`
- `ALM_KTS_FAULT`
- `AALM_NONE_87` (riservato)

### Byte 11 - Sonde temperatura / EXT1-2
- `ALM_PTFRESH_KO`
- `ALM_PTRET_KO`
- `ALM_PTSUP_KO`
- `ALM_PTEXA_KO`
- `ALM_EXT1_LINK`
- `ALM_EXT1_KO`
- `ALM_EXT2_LINK`
- `ALM_EXT2_KO`

### Byte 12 - EXT3-4 / FLW1-2
- `ALM_EXT3_LINK`
- `ALM_EXT3_KO`
- `ALM_EXT4_LINK`
- `ALM_EXT4_KO`
- `ALM_FLW1_LINK`
- `ALM_FLW1_KO`
- `ALM_FLW2_LINK`
- `ALM_FLW2_KO`

### Byte 13 - EBPD / SSR / EBP2
- `ALM_EBPD_LINK`
- `ALM_EBPD_KO`
- `ALM_SSR_LINK`
- `ALM_SSR_KO`
- `ALM_EBP2_LINK`
- `ALM_EBP2_KO`
- `ALM_NONE_110` (riservato)
- `ALM_NONE_111` (riservato)
