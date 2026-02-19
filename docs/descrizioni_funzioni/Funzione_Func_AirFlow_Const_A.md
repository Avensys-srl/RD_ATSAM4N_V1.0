# Funzione `Func_AirFlow_Const`

# 🔷 SEZIONE A — DOCUMENTAZIONE TECNICA (ENGINEERING)

## A1. Nome funzione
`Func_AirFlow_Const(unsigned int setPoint)`

## A2. File e posizione
`src/motor_speed.c:1770`

## A3. Scopo tecnico
Regola la velocita dei motori per mantenere la portata aria costante (CAF), calcolando lo scostamento tra portata target e portata stimata/misurata e adattando la velocita con step progressivi.

## A4. Parametri e ritorno
### Parametri in ingresso
- `setPoint`:
  - Tipo: `unsigned int`
  - Unita di misura: percentuale portata in scala firmware (0.1%)
  - Range ammesso: definito dai setpoint CAF in EEPROM
  - Valori invalidi gestiti: limitazione automatica tramite `Kmin_reg`/`Kmax` e clamp speed

### Valore di ritorno
- Tipo: `void`
- Significato: aggiorna `temp_speed_motorsR/F` e misure aggregate

## A5. Logica dettagliata
- Esegue media mobile sugli rpm per rendere stabile la stima di portata.
- Se rpm insufficienti esce senza correggere.
- Applica filtro temporale (`cnt_delay`) e interviene solo a finestre definite.
- Converte `setPoint` in portata target `qSetPoint` usando `GetMaxAirFlow()`.
- Calcola punto di lavoro (`qw`, `pw`) tramite curve (o tramite misura pressione diretta in build specifica `COMESSA_2388`).
- Aggiorna medie diagnostiche di pressione/portata.
- Calcola errore di portata `delta_Q`.
- Se errore supera banda `errMax`, modifica speed con step proporzionali (+/-1,2,5%) e clamp tra `Kmin_reg` e `100%`.
- Propaga la stessa velocita su entrambi i rami (`R` e `F`).

## A6. Mappa firmware con riferimenti puntuali
- **Passo**: ingresso funzione CAF  
  **Dove**: `src/motor_speed.c:1770`  
  **Cosa osservare**: parametro `setPoint`
- **Passo**: media rpm su buffer storico  
  **Dove**: `src/motor_speed.c:1785`  
  **Cosa osservare**: `rpm_r_preview[]`, media `rpm_r`
- **Passo**: uscita su rpm basso  
  **Dove**: `src/motor_speed.c:1796`  
  **Cosa osservare**: condizione `rpm_r <= 150`
- **Passo**: filtro temporale regolatore  
  **Dove**: `src/motor_speed.c:1799`  
  **Cosa osservare**: `cnt_delay`
- **Passo**: conversione setpoint in portata target  
  **Dove**: `src/motor_speed.c:1802`  
  **Cosa osservare**: `MaxAirFlow`, `qSetPoint`
- **Passo**: calcolo limite minimo regolazione  
  **Dove**: `src/motor_speed.c:1810`  
  **Cosa osservare**: loop su `qmax_rpm[]`, `Kmin_reg`
- **Passo**: calcolo punto di lavoro da curve (ramo standard)  
  **Dove**: `src/motor_speed.c:1855`  
  **Cosa osservare**: `CalcQW`, `CalcPW`, clamp `pw`
- **Passo**: set default speed corrente  
  **Dove**: `src/motor_speed.c:1877`  
  **Cosa osservare**: `temp_speed_motorsR/F = sData.speed_motors_R`
- **Passo**: medie diagnostiche pressione/portata  
  **Dove**: `src/motor_speed.c:1884`  
  **Cosa osservare**: `sData.measure_pressure`, `sData.measure_airflow`
- **Passo**: errore portata e finestra di aggiornamento  
  **Dove**: `src/motor_speed.c:1904`  
  **Cosa osservare**: `delta_Q`, reset `cnt_delay`
- **Passo**: regolazione step speed in base a errore  
  **Dove**: `src/motor_speed.c:1933`  
  **Cosa osservare**: `speedNew`, condizioni `errMax*2/*3`
- **Passo**: commit nuovo comando PWM interno  
  **Dove**: `src/motor_speed.c:1976`  
  **Cosa osservare**: `calc_new_PWM`, `temp_speed_motorsR/F`
- **Passo**: punti chiamata CAF nella logica motori  
  **Dove**: `src/motor_speed.c:2639`  
  **Cosa osservare**: chiamata `Func_AirFlow_Const(setPoint)`

## A7. Interfacce esterne (livello astratto)
### Segnali generati dall'RD
- Comando velocita motori:
  - Pin MCU / periferica: interfaccia comando motori (astratta)
  - Tipo: PWM/0-10V equivalente (scala 0..255)
  - Livello attivo: `temp_speed_motorsR/F`
  - Condizione di attivazione: errore portata fuori banda
  - Persistenza: fino a ciclo successivo

### Segnali letti dall'RD
- Feedback rpm motore:
  - Tipo ingresso: tachimetrico/feedback velocita
  - Modalita lettura: `sData.rpm_motors_R[0]` + media
  - Frequenza campionamento: ogni chiamata
  - Condizione di validita: rpm sopra soglia
- Grandezze per stima q/p:
  - Tipo ingresso: modelli firmware o misura pressione in build dedicata
  - Modalita lettura: `CalcQW`, `CalcPW`, opzionale `ACC_I2C_PCAP`
  - Frequenza campionamento: ciclo regolatore
  - Condizione di validita: dati motore coerenti

## A8. Catena causale (livello sistema)
| Livello | Descrizione |
|----------|------------|
| Decisione | Regolazione CAF su errore di portata |
| Interfaccia | Comando speed motori R/F |
| Elettronica esterna | Driver motori |
| Attuatore | Ventilatori |
| Effetto | Portata aria tendenzialmente costante al variare impianto |

## A9. Feedback disponibile

- Esiste conferma reale?
  - Parziale: in configurazione standard la portata e stimata da modello (`CalcQW/CalcPW`); feedback diretto pressione solo in ramo specifico (`COMESSA_2388`).
- Il comando e "fire and forget"?
  - No: la funzione ricalcola errore di portata e riadatta PWM a ciclo.
- Dove viene letto il feedback?
  - Da `rpm_r` e variabili calcolate/mediate (`src/motor_speed.c:1770`, `src/motor_speed.c:1857`, `src/motor_speed.c:1899`, `src/motor_speed.c:1907`).
- Quanto e affidabile?

Valori criteri:

- `Origine diretta: 0` (prevalentemente stima software, non misura diretta garantita)
- `Correlazione temporale: 0` (solo cadenza di ciclo, nessun timeout esplicito)
- `Correlazione univoca col comando: 1` (errore `delta_Q` legato al setpoint attivo)
- `Gestione errore: 1` (controlli rpm minima, limiti PWM, saturazioni)
- `Punteggio totale: 2`
- `Classe finale: Medio`
## A10. Punti critici firmware
- Verificare condizioni con confronti stretti su soglie (maggiore/minore uguale) per evitare oscillazioni logiche.
- Verificare coerenza timer/contatori rispetto al periodo scheduler reale.
- Verificare priorità dei rami decisionali e dei ritorni anticipati.
- Verificare possibili ambiguità tra flag di stato e comandi calcolati nello stesso ciclo.
## A11. Punti di disaccoppiamento (sistema)
- Errore logico firmware
  - Taratura curve non aderente all'impianto reale
  - Finestra temporale troppo lenta rispetto dinamica richiesta
- Errore interfaccia
  - Speed calcolata ma non applicata correttamente ai motori
  - Feedback rpm non aggiornato
- Errore elettrico esterno
  - Disturbi su catena driver o alimentazione motori
  - Sensore pressione (se usato) con dato non affidabile
- Errore meccanico
  - Perdita di efficienza ventilatori/filtri/circuito aria non compensabile dal solo comando speed

## A12. Limite di responsabilità firmware
Il firmware RD:
- Stima il punto di lavoro e corregge la speed per inseguire la portata target
- Applica limiti min/max e logica anti-instabilita
- Aggiorna feedback diagnostici disponibili

Il firmware RD NON garantisce:
- Portata reale identica al target in presenza di vincoli fisici esterni
- Integrita meccanica/elettrica della catena ventilazione
- Validita assoluta del modello di conversione in tutte le condizioni campo

