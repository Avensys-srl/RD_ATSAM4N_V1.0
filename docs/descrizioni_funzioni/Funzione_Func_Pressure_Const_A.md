# Funzione `Func_Pressure_Const`

# 🔷 SEZIONE A — DOCUMENTAZIONE TECNICA (ENGINEERING)

## A1. Nome funzione
`Func_Pressure_Const(unsigned int setPoint)`

## A2. File e posizione
`src/motor_speed.c:1162`

## A3. Scopo tecnico
Regola la velocita dei motori per mantenere pressione/condizione CAP al setpoint richiesto, usando misura diretta di pressione (se disponibile) oppure modello stimato da rpm e curve.

## A4. Parametri e ritorno
### Parametri in ingresso
- `setPoint`:
  - Tipo: `unsigned int`
  - Unita di misura: pressione target (scala firmware CAP)
  - Range ammesso: dipendente da configurazione impianto e validazione a monte
  - Valori invalidi gestiti: comportamento limitato da clamp velocita e limiti funzione

### Valore di ritorno
- Tipo: `void`
- Significato: aggiorna `temp_speed_motorsR/F` e misure aggregate senza ritorno esplicito

## A5. Logica dettagliata
- Filtra rpm ramo return e abbandona la regolazione se il ventilatore non sta ruotando.
- Se presente sensore pressione dedicato (`ACC_I2C_PCAP`):
  - legge `pw`, applica limiti, aggiorna `sData.measure_pressure`.
  - calcola portata stimata e ogni finestra temporale aggiorna la velocita con step incrementali/decrementali in base allo scarto dal setpoint.
- Se sensore pressione dedicato non presente:
  - stima punto di lavoro (`qw`, `pw`) da curve e rpm.
  - media le misure verso diagnostica.
  - applica regolazione fine/grossolana con ricerca binaria della nuova velocita quando necessario.
- Mantiene sempre simmetria R/F in CAP base (`temp_speed_motorsF = temp_speed_motorsR`).

## A6. Mappa firmware con riferimenti puntuali
- **Passo**: ingresso funzione CAP  
  **Dove**: `src/motor_speed.c:1162`  
  **Cosa osservare**: parametro `setPoint`
- **Passo**: filtro rpm minimo (uscita anticipata)  
  **Dove**: `src/motor_speed.c:1181`  
  **Cosa osservare**: condizione `rpm_r < 120`
- **Passo**: ramo con sensore pressione dedicato  
  **Dove**: `src/motor_speed.c:1185`  
  **Cosa osservare**: `DigitAccessoryOperating(ACC_I2C_PCAP)`
- **Passo**: clamp misura pressione e pubblicazione misura  
  **Dove**: `src/motor_speed.c:1195`  
  **Cosa osservare**: limiti `pw`, `sData.measure_pressure`
- **Passo**: conversione setpoint in portata target e limiti regolatore  
  **Dove**: `src/motor_speed.c:1204`  
  **Cosa osservare**: `GetMaxAirFlow`, `qSetPoint`, `Kmin_reg`
- **Passo**: regolazione a step nel ramo sensore diretto  
  **Dove**: `src/motor_speed.c:1254`  
  **Cosa osservare**: soglie +/-5/12.5/25/50%, update `speedNew`
- **Passo**: aggiornamento comandi motore nel ramo sensore diretto  
  **Dove**: `src/motor_speed.c:1301`  
  **Cosa osservare**: `temp_speed_motorsR/F`
- **Passo**: ramo CAP stimato (senza sensore pressione)  
  **Dove**: `src/motor_speed.c:1303`  
  **Cosa osservare**: calcolo `CalcQW`, `CalcPW`
- **Passo**: medie diagnostiche pressione/portata  
  **Dove**: `src/motor_speed.c:1335`  
  **Cosa osservare**: `acc_measure_pressure`, `sample_measure_pressure`, `sData.measure_airflow`
- **Passo**: regolazione fine  
  **Dove**: `src/motor_speed.c:1369`  
  **Cosa osservare**: banda `reg_fine`, incremento/riduzione speed
- **Passo**: ricerca binaria speed nel ramo grossolano  
  **Dove**: `src/motor_speed.c:1414`  
  **Cosa osservare**: ciclo `while`, `speed0/speed1/speedNew`
- **Passo**: fallback se speed non trovata  
  **Dove**: `src/motor_speed.c:1455`  
  **Cosa osservare**: aumento prudente speed su delta alto
- **Passo**: punti chiamata CAP nella logica motori  
  **Dove**: `src/motor_speed.c:2629`  
  **Cosa osservare**: chiamata `Func_Pressure_Const(setPoint)`

## A7. Interfacce esterne (livello astratto)
### Segnali generati dall'RD
- Comando velocita motori R/F:
  - Pin MCU / periferica: comando motori (astratto, scala 0..255)
  - Tipo: comando logico PWM/0-10V equivalente
  - Livello attivo: `temp_speed_motorsR/F`
  - Condizione di attivazione: errore pressione vs setpoint
  - Persistenza: fino al ciclo successivo

### Segnali letti dall'RD
- Sensore pressione CAP (se presente):
  - Tipo ingresso: pressione digitale
  - Modalita lettura: `sData.AccI2C[ACC_I2C_PCAP].measure1`
  - Frequenza campionamento: ciclo funzione con filtro temporale
  - Condizione di validita: `DigitAccessoryOperating(ACC_I2C_PCAP)`
- RPM motori:
  - Tipo ingresso: feedback velocita
  - Modalita lettura: `sData.rpm_motors_R[0]` + buffer medio
  - Frequenza campionamento: ogni chiamata
  - Condizione di validita: rpm sopra soglia minima

## A8. Catena causale (livello sistema)
| Livello | Descrizione |
|----------|------------|
| Decisione | Regolazione CAP per ridurre errore pressione |
| Interfaccia | Comando velocita motori (R/F) |
| Elettronica esterna | Driver/inverter ventilatori |
| Attuatore | Motori ventilazione |
| Effetto | Variazione pressione rete aria |

## A9. Feedback disponibile

- Esiste conferma reale?
  - Si, con feedback di pressione quando presente `ACC_I2C_PCAP` (`src/motor_speed.c:1185`, `src/motor_speed.c:1192`); in fallback usa stima da modello (`CalcQW/CalcPW`).
- Il comando e "fire and forget"?
  - No: e un anello di regolazione che ri-legge misura/stima e corregge la velocita a ciclo.
- Dove viene letto il feedback?
  - Pressione/portata in `sData.measure_pressure` e `sData.measure_airflow`, e misura PCAP (`src/motor_speed.c:1201`, `src/motor_speed.c:1230`, `src/motor_speed.c:1339`, `src/motor_speed.c:1348`).
- Quanto e affidabile?

Valori criteri:

- `Origine diretta: 1` (diretta con PCAP; in fallback e stimata)
- `Correlazione temporale: 0` (nessun timeout esplicito di validita misura)
- `Correlazione univoca col comando: 1` (correzione chiusa su errore `pw` vs `setPoint` nello stesso ciclo)
- `Gestione errore: 1` (limiti/saturazioni, uscita per rpm bassa, fallback minimo)
- `Punteggio totale: 3`
- `Classe finale: Forte`
## A10. Punti critici firmware
- Verificare condizioni con confronti stretti su soglie (maggiore/minore uguale) per evitare oscillazioni logiche.
- Verificare coerenza timer/contatori rispetto al periodo scheduler reale.
- Verificare priorità dei rami decisionali e dei ritorni anticipati.
- Verificare possibili ambiguità tra flag di stato e comandi calcolati nello stesso ciclo.
## A11. Punti di disaccoppiamento (sistema)
- Errore logico firmware
  - Ritardi regolatore (`cnt_delay`) non considerati in diagnosi rapida
  - Setpoint non coerente con curva impianto
- Errore interfaccia
  - Dati pressione/rpm non aggiornati correttamente
  - Comando speed non applicato a valle
- Errore elettrico esterno
  - Sensore pressione guasto o instabile
  - Driver motore non lineare o non alimentato
- Errore meccanico
  - Ventilatori/condotte alterano la risposta pressione-portata

## A12. Limite di responsabilità firmware
Il firmware RD:
- Calcola la correzione speed in funzione del setpoint CAP
- Applica limiti e strategie di regolazione
- Usa feedback disponibili per convergenza

Il firmware RD NON garantisce:
- Integrita fluidodinamica dell'impianto esterno
- Correttezza meccanica dei ventilatori
- Precisione assoluta del sensore pressione esterno

