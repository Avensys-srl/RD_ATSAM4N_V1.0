# Fase 1 - Stabilizzazione e basi tecniche

Obiettivo: creare fondamenta comuni e ridurre duplicazioni senza cambiare il comportamento.

## 1.1 Centralizzazione macro di build (COMPLETATO)
Azioni:
- [x] Creato `src/common/build_config.h` con le macro di build principali.
- [x] Rimosse definizioni duplicate da `src/main.c`, `src/HW_func.c`, `src/motor_speed.c`, `src/ADC.c`.
- [x] Inclusa la configurazione in moduli che usano le macro: 
  - `src/main.c`
  - `src/ADC.c`
  - `src/HW_func.c`
  - `src/motor_speed.c`
  - `src/Accessory.c`
  - `src/Admin_Messages.c`
  - `src/Testing_HW.c`

## 1.2 Header comuni e convenzioni (DA FARE)
Azioni previste:
- [ ] Introdurre `src/common/types.h` per tipi comuni e macro di pack.
- [ ] Introdurre `src/common/status_bits.h` con maschere e bit di stato centralizzati.
- [ ] Allineare nomi macro e commenti per uniformita.

## 1.3 Separazione interfacce (DA FARE)
Azioni previste:
- [ ] Aggiungere header dedicati dove mancano (es. `Accessory.h`, `Clima_Func.h`, `Filter_Func.h`).
- [ ] Ridurre prototipi duplicati in `main.c` spostandoli nei rispettivi header.

## 1.4 Verifica regressione minima (DA FARE)
Azioni previste:
- [ ] Verificare build senza warning critici.
- [ ] Eseguire subset della checklist: seriali, accessori I2C, motori, bypass.

---

## Modifiche introdotte in questa fase
- `src/common/build_config.h` (nuovo)
- Aggiornati includes in:
  - `src/ADC.c`
  - `src/HW_func.c`
  - `src/motor_speed.c`
  - `src/Accessory.c`
  - `src/Admin_Messages.c`
  - `src/Testing_HW.c`
  - `src/main.c`

## Note
- Non e' stato modificato alcun comportamento funzionale.
- Le macro di build restano identiche come effetto, ora solo centralizzate.
