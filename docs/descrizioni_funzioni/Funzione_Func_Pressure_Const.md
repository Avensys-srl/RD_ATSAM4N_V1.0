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
- Se esiste conferma reale: si, parziale
- Se il comando e fire-and-forget: no, usa feedback misura/rpm
- Dove viene letto il feedback: `src/motor_speed.c:1192`, `src/motor_speed.c:1177`, `src/motor_speed.c:1335`
- Affidabilita del feedback: medio-alta se sensore/rpm validi, ridotta in assenza sensore pressione diretto

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

# 🟢 SEZIONE B — DOCUMENTAZIONE NON TECNICA (OPERATIVA / CAMPO)

## B1. Racconto operativo
In CAP il sistema prova a mantenere costante la pressione dell'aria variando automaticamente la velocita dei ventilatori. Se la pressione e bassa aumenta la velocita, se e alta la riduce.

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

