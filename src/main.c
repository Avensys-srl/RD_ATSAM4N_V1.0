/**
 * \file main.cpp
 *
 * \brief Empty user application template
 *
 */

/**
 * \mainpage User Application template doxygen documentation
 *
 * \par Empty user application template
 *
 * Bare minimum empty user application template
 *
 * \par Content
 *
 * -# Include the ASF header files (through asf.h)
 * -# "Insert system clock initialization code here" comment
 * -# Minimal main function that starts with a call to board_init()
 * -# "Insert application code here" comment
 *
 */

/*
 * Include header files for all drivers that have been imported from
 * Atmel Software Framework (ASF).
 */
/*
 * Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
 */

 /************************************************************************************************


 +--------+----------+-----------+------------------------------------------------------------------------+
 Author | Relaise  |    Date   |                  Descripions
 +--------+----------+-----------+------------------------------------------------------------------------+
 Maury      2.01      18/12/13   - Cambiato HW MainBoard 4.0,e Cambiato totalmente il SW che
 gestisce l'unit‡
 +--------+----------+-----------+------------------------------------------------------------------------+
 Maury      2.02      09/07/14   - Corretto errore nella task Weeklyprog.
 - Inserito la funzione CAP
 - Sistemato alcuni difetti con i sensori CO2, VOC, RH
 +--------+----------+-----------+------------------------------------------------------------------------+
 Maury      2.03      24/07/14   - Inserito le modifiche per il Modbus
 - Migliorata la funz. CAP
 +--------+----------+-----------+------------------------------------------------------------------------+
 Nicola     2.04      23/08/14   - Inserito le modifiche per il CAP - CAF
 Maury                28/08/14   - Fatto modifiche per accessori Clima (Byte info, Byte size)
 +--------+----------+-----------+------------------------------------------------------------------------+
 Maury      2.05      30/08/14   - Corretto errore nella funz. Clima. Se c'Ë un motore guasto
 non spegenvo l'heater.
 - Migliorata la funz. CAF
 +--------+----------+-----------+------------------------------------------------------------------------+
 Maury      2.06      10/09/14   - Inserita la possibilit‡ di configurare il senso di rotazione
 del motorino per chiudere il Bypass
 - Inserita la possibilit‡ di configurare la posizione delle sonde
 di Temperatura
 +--------+----------+-----------+------------------------------------------------------------------------+
 Maury      2.07      16/09/14   - Inserito Possibilit‡ di configurazione Unit‡ tramite
 una stringa di 54 Byte nella shell
 - In fase di partenza controlla il SN, se vuoto blocca l'unit‡
 e segnala lo stato di blocco con un bip ad intermittenza.
 +--------+----------+-----------+------------------------------------------------------------------------+
 Maury      2.08      23/09/14    - Tolto i sensori Analogici RH, CO2, VOC.
 - Inserito l'allarme ModBus_KO
 - Modificato la funzione Clima per l'accessorio Heater
 +--------+----------+-----------+------------------------------------------------------------------------+
 Maury      2.09     28/01/15     - Inserito le funzioni per il collaudo dell'UNITa'
 - Migliorata la funzione CAF
 +--------+----------+-----------+------------------------------------------------------------------------+
 Maury      2.10     24/02/15     - Migliorato il driver della Seriale_2 (per i disturbi) con 10 sample sul bit
 - Per gli allarmi le sonde Temp. KO introdotto Delay (~30 sec.)
 - Cambiato sistema per lo spegnimento dell'unit‡ dopo aver rilevato un guasto
 Motore, l'unit‡ continua a girare per oltre 1 minuto, dopo di che si spegne per 80 sec.
 Alla terza ripartenza va a scrivere in eeprom lo stato di off. e ci rimane.
 Questo fa si che se ricevo un comando da remoto di ripartire posso farlo,
 senza dover disalimentare l'unit‡.
 - Inserito buffer sulla lettura degli stati dei motore, dopo la
 conferma di 3 dati uguali cambio le variabili di
 sData.StatusMotorsR & sData.StatusMotorsF.
 - Ridotte le soglie minime di bypass fault (nuovi motorini).
 27/02/15     - Introdotto un controllo della velocit‡ quando il PEHD Ë ON.
 Si parte dalla soglia minima del 40% a salire fino a che l'allarme di
 Prox. o Press_ko. non sparisce.
 +--------+----------+-----------+------------------------------------------------------------------------+
 Maury      2.11     06/03/15     - Risolto piccolo Baco sugli output: non differiva tra caduta e attrazione.
 - Inserito il CAP digitale.
 +--------+----------+-----------+------------------------------------------------------------------------+
 Maury      2.12     16/04/15     - Modificato i 2 pacchetti dati della scheda Modbus, per la gestione RFM.
 vedi file <Protocol_Ser2.h>
 - Aggiunta piccola modifica se impostato Bypass Mauale chiuso
 non ci deve essere l'allarme
 +--------+----------+-----------+------------------------------------------------------------------------+
 Maury      2.13      26/06/15    - Cambiato tabelle mappatura motori per CAP/CAF
 - Modificato funzione CAF.
 +--------+----------+-----------+------------------------------------------------------------------------+
 Maury      2.14      10/07/15    - CORRETTO BACO scrittura zona  <Weeklyprogram>
 +--------+----------+-----------+------------------------------------------------------------------------+
 Maury      2.15      15/07/15    - Inserito rischieste FranceAir Gestione FIRE.
 +--------+----------+-----------+------------------------------------------------------------------------+
 Maury      2.16      03/08/15    - Inserito Nuovo ACCESSORIO DXD (acc. CLIMA Caldo/freddo con compressore)
 +--------+----------+-----------+------------------------------------------------------------------------+
 Maury      2.17      21/10/15    - Inserito il controllo per accessori Clima a doppia funzione.
 a) Selezione tra Summer (ENAB_COOLER) e winter (ENAB_HEATING)
 b) Selezione sonda di riferimento tra  T_Return o T_Supply
 +--------+----------+-----------+------------------------------------------------------------------------+
 Maury      2.18      05/11/15    - Corretto Baco nella funzione BOOST (PARTY), non veniva azzerato il
 contatore dopo la disattivazione.
 +--------+----------+-----------+------------------------------------------------------------------------+
 Maury      2.19      19/11/15    - Inserito la possibilit‡ di avere la funzione CAF per le unit‡ 0073
 0123.
 - Aumentato i controlli per cancellare l'allarme BDP quando Ë in
 Manuale Chiuso
 +--------+----------+-----------+------------------------------------------------------------------------+
 Nicola     2.20      16/02/16    - Possibilit‡ di gestire le sonde di CO2 VOC e RH anche in modalit‡ CAF
 - Fire Alarm: manual reset da KST
 - Freecooling Automatico, anche in caso di unit‡ spenta
 - CAF/CAP Serie 8
 +--------+----------+-----------+------------------------------------------------------------------------+
 Nicola     2.21      04/04/17    - Introdotto tempo TIMER_KTS_KO in Hw_func dopo di che viene riavviato il KTS
 - Impostato TIMER_KTS_KO = 40 cicli (60 sec.)
 - Modificata soglia minima = 25% (circa 2.5V)
 +--------+----------+-----------+------------------------------------------------------------------------+
 Nicola     2.22      14/11/17    - Gestione del pressostato per il filtro sporco
 - Inserite funzioni di verfica e calibrazione in Filter_Func
 - Possibilit‡ di gestire DPS e DPP con il KTS
 - Modificato il Reset filtri nel caso DPP o DPS
 - Modificato lo stringone per l'accettazione del DPP
 - Modificata la shell accessori in modo che possa essere visualizzata
 anche la presenza del DPP
 +--------+----------+-----------+------------------------------------------------------------------------+
 Nicola     2.23      25/01/18    - Sbilanciamento attivo anche durante il controllo 0-10V esterno.
 +--------+----------+-----------+------------------------------------------------------------------------+
 Nicola     2.24      14/07/18    - Il modbus puÚ comandare i motori singolarmente. Aggiungiamo un ENABLE
 per questa nuova modalit‡ (MBF coil n.22) e 2 setpoint da 0 a 100 (il modbus
 puÚ fermare i motori) uno per ogni motore. Quanto sono in questa funzione motor_speed
 passa completamente il comando.
 - Introdotto il nuovo msg in I2C per gli accessori con 4 sensori a bordo
 EXT1, EXT2, EXT3, EXT4.
 +--------+----------+-----------+------------------------------------------------------------------------+
 Nicola     2.25      02/01/19    - introdotti 4 nuovi accessori in I2C: FLW1, FLW2, EBPD, SSR. Gestiti via Modbus
 03/02/19    - Tasto TEST IPEHD su touchscreen: funziona relativa di test
 02/05/19    - Aggiunto Temeratura Aria del CWD nel protocollo per il KTS
 +--------+----------+-----------+------------------------------------------------------------------------+
 Nicola     2.26      01/08/19    - Corretta la funzione preheater nel caso di accessorio esterno. Controllo
 il caso acqua o elettrico e di conseguenza agisco con tecniche diverse.
 +--------+----------+-----------+------------------------------------------------------------------------+
 Nicola     2.27      -------     - Biforcazione: software personalizzato per PSI in CH per ECS
 +--------+----------+-----------+------------------------------------------------------------------------+
 Nicola     2.28      10/10/19    - Aggiungeti eventi all'output (SUM/WIN e MAX SPEED)
 +--------+----------+-----------+------------------------------------------------------------------------+
 Nicola     2.29      30/06/20    - Corretto baco sul funzionamento del BPD in assenza del EBPD.
 +--------+----------+-----------+------------------------------------------------------------------------+
 Nicola     2.30      22/08/20    - Corretto baco sullo stato del EHD: una volta accesa, il fiocco
 continua a lampeggiare e lo stato rimane ON nonostante sia spenta realmente.
 Stato non si aggiorna, causa errata intrepretazione comando SSR.
 +--------+----------+-----------+------------------------------------------------------------------------+
 Nicola     2.31      05/11/20    - Introdotto nuovo accessorio in I2C: EBP2. Serve per utilizzare un
 un doppio motore bypass tramite la scheda BPD01 in moto complementare
 da 0 a 100. Per la regolazione di movimento orario / antiorario si
 deve fare riferimento a EBPD. Quest'ultimo lavora in slave rispetto al
 precedente.
 +--------+----------+-----------+------------------------------------------------------------------------+
 Nicola     2.32      10/04/21    - Corretto il baco sul EBPD/EBP2 : nel caso di controllo diretto (non MBF)
 non si sincronizzavano i bypass al ciclo di avvio. Causa: la variabile
 info_EBPD viene inizializzata con un valore valido di controllo, non
 stimolando cosÏ l'aggiornamento del comando sin dal principio. Soluzione:
 inizializzata ad un valore fuori range in modo che si sincronizzi fin da subito.
 +--------+----------+-----------+------------------------------------------------------------------------+
 Zakaria     2.33      04/06/21    - Introdotto nuovo accessorio in I2C: DSC
 +--------+----------+-----------+------------------------------------------------------------------------+
 Zakaria     2.34      27/11/21    - DSC  bug fixed
 +--------+----------+-----------+------------------------------------------------------------------------+
 Zakaria     2.35      16/02/22    - PIR strategy control added
 +--------+----------+-----------+------------------------------------------------------------------------+
 Zakaria / Nicola     2.47      10/12/24    - CAPS and CAPR management strategy added
 +--------+----------+-----------+------------------------------------------------------------------------+
 Zakaria / Nicola     2.49      15/02/25    - AIR sensor restart after 24 hours with value not changing added
 +--------+----------+-----------+------------------------------------------------------------------------+
 Zakaria / Nicola     2.50      27/02/25    - AIR sensor data management added
 +--------+----------+-----------+------------------------------------------------------------------------+
 Zakaria / Nicola     2.51      13/03/25    - Added Clean Event strategy for SENSOR board
 +--------+----------+-----------+------------------------------------------------------------------------+
 Zakaria / Nicola     2.52      29/03/25    - changed UART0 and UART2 Transfert methode from interrupt to DMA, Added KTS communication checking ( error 10-7 )
 +--------+----------+-----------+------------------------------------------------------------------------+
 Nicola				  2.53      14/04/25    - Change CO2 strategy from 600 ppm with ramp to max 220/260 ppm.
 - Minimum CO2 airflow setting (boost) 30% (before was 40%)
 - Added NOTACHO macro to compile for fans without TACHO signal
 +--------+----------+-----------+------------------------------------------------------------------------+
  Zakaria / Nicola    2.54      16/04/25    - Merged files from version 2.52 and 2.53
 +--------+----------+-----------+------------------------------------------------------------------------+
 Zakaria / Nicola     2.55      30/04/25    - Added version firmware for EHD08, EHD07 and BPD on debug data 
 +--------+----------+-----------+------------------------------------------------------------------------+
 Nicola     2.56      11/05/25    - Added damperless bypass (imbalance) in case of I0 configuration (PRIME CASE)
									I0 => Serialnumber with 90 in position (10,11)
									like XXXXYYYYZ90YYWWPPP. For example 999900300902519001
 +--------+----------+-----------+------------------------------------------------------------------------+
 Zakaria / Nicola     2.57      18/05/25    - Added Reset Eeprom from KTS.
											- increased number of bytes received frm accessory to 15
 +--------+----------+-----------+------------------------------------------------------------------------+
 Zakaria / Nicola     2.58      19/07/25    - Fixed Retro-compatibility problem related to EHD07 HWD/CWD accessory detection
 +--------+----------+-----------+------------------------------------------------------------------------+
 Zakaria / Nicola     2.59      22/07/25    - Retro-compatibility - improved adding and removing acccessories
 +--------+----------+-----------+------------------------------------------------------------------------+
 Zakaria / Nicola     2.60      29/07/25    - Fixed bugs for Heater accessory command
											- added Voltage and current value for heater on Debug info
 +--------+----------+-----------+------------------------------------------------------------------------+
 Zakaria / Nicola     2.61      21/09/25    - Added delay for PIR to be more stable when power ON the BLACK BOX ( 72000 ms)
 +--------+----------+-----------+------------------------------------------------------------------------+
 Zakaria / Nicola     2.62      28/09/25    - Added I2C bus recovery (9 pulses) on function init.c ( BLACK SCREEN )
 +--------+----------+-----------+------------------------------------------------------------------------+
 Zakaria / Nicola     2.63      13/10/25    - Added modification for PIR strategy and weekly program on file motor_speed.c line 773
 +--------+----------+-----------+------------------------------------------------------------------------+
 Zakaria / Nicola     2.64      17/10/25    - Fixed Temperature Probe ALARM , File ADC.c line 129
 +--------+----------+-----------+------------------------------------------------------------------------+
 Zakaria / Nicola     2.65      12/11/25    - Added WINTER/SUMMER enabling using Input 2 , File motor_speed.c line 777
											- added a second temperature setpoint for WINTER ( SetPointTemp_Heater ), now we have 2 for both SUMMER and WINTER
 +--------+----------+-----------+------------------------------------------------------------------------+
 Zakaria / Nicola     2.66      11/12/25    - Merged version 2.65 with version of Nicola 2C65
										   - added AC/DC voltage measurement for board version 3.3 ( +12v and +5v )
										   - Fixed issue for CAP function on file motor_speed.c
+--------+----------+-----------+------------------------------------------------------------------------+
Zakaria / Nicola     2.67      18/12/25    - Fixed issue for CAP function on file motor_speed.c
+--------+----------+-----------+------------------------------------------------------------------------+
Zakaria / Nicola     2.68      26/01/26    - Added PIR strategy to RD firmware( motor_speed.c)
 */

#include "asf.h"

#include "MASTER_I2C.h"
#include "data.h"
#include "eeprom_map.h"
#include "Protocol_Ser1.h"
#include "Protocol_Ser2.h"
#include "alarm.h"
#include "Scheduler.h"
#include <string.h>

// indirizzo del chip eeprom.
#define   DEVICE_24LCO2B		   0x50   // 256 Byte fmax 100KHz
#define   DEVICE_ADC121C027_IC7    0x51
#define   DEVICE_ADC121C027_IC8    0x52
#define   DEVICE_ADS7142_IC9	   0x1F

//---------------------------- MACRO -------------------------------
// Build-time switches are centralized in build_config.h
#include "common/build_config.h"

// Versione el FW.
const char VERSION_FW[5]= {' ','2', '.','6','8'};

//----------------------- Funzioni a custom sul FW -------------------------
//#define _MAPPING_FUNC               // Se abilito la define Attivo la funzione di mappatura dei motori, stampando a video rmp etc..
//#define _CLEAR_ALARMS_POWER_MOTORS  // Se abilito la define Maschero gli allarmi 8-01, 8-02, 8-03, 9-01, 9-02, 9-03.
//#define _CLEAR_ALARMS_PREHEATER     // Se abilito la define Maschero gli allarmi del preheater


#ifdef  _EEP_SERIAL_NUMBER
const char SN[18]= {'9', '9', '9', '9',  // Ordine di vendita
	'0', '0', '3', '5',  // Portata unit‡ & serie
	'0', '1', '4',  // Configurazione
	'1', '8', '3', '0',  // anno / Week
'0', '0', '1'};
#endif

// ------------- Funzioni Abilitate in base alla config. ---------------
//  bit[7]:CAF] | bit[6]:CAP  | bit[5]:CSF | bit[4]:ImbalanON | bit[3]:StepLess | bit[2]:WeeklyENAB | bit[1]:BoostON | bit[0]:DeFrostENAB.
#define ENAB_FUNC_EXTRA    0xFF  //      1            1             1              1                  1                   1                   1               1
#define ENAB_FUNC_BASIC    0x29  //      0            0             1              0                  1                   0                   0               1

//--------------------------------
#define NEW_LINE      'n'
#define NO_NEW_LINE    0

// stato del Rele' e delle tensione dell'elettronica
#define POWER_SAVE     0
#define POWER_ON      'O'


//#define VIEW_DEBUG_CODE_NO_KTS
#ifdef VIEW_DEBUG_CODE_NO_KTS
Byte dbg_view_dbg = 1;
#endif

//---------------------------------------------------------------
//             Proto Global
//---------------------------------------------------------------
//---- eeprom_func ----
extern void ctrl_eeprom();
extern int update_data_eeprom(unsigned short start_addr, unsigned short end_addr, Byte *buff);

//----  Hw_func ----
extern void Active_Procedure_Bypass_OpCl(Byte direct_bpd, int SecondsWait);
extern void ActiveBuzzer(Byte numBeepBeep, unsigned int freq_100ms);
extern void Ctrl_FunzionamentoBPD();
extern int  operatingHoursCounter();
extern int  Test_Connect_KTS();
extern int  Gestione_Relays();

//----- testing_HW -------
extern void test_bypass(void);
extern void test_IO(void);
extern void testSleepMode(void);
extern void testMotors(void);
extern void testUart1(void);
extern void Check_SerialNumber(void);
extern int IPEHD_test();

//---- Seriali ----
extern int intercepts_messages_serial_1();
extern int intercepts_messages_serial_2();
extern int GestioneCTRL_FAN(void);
extern void polling_CTRL_FAN(void);
extern int polling_serial_2(void);

//----- motors ----
extern unsigned short GetMaxAirFlow();
extern void PowerMode(Byte sts);
extern int motor_speed();

//----- shell ----
extern void shell_config_menu(Byte dec_sec);

extern int weekly_program();
extern int pollingDigitAccessory(void);
extern int CkUpdate_EepromToShadow (void);
extern int TemperatureRegulate();

extern void init_Seriale_2(long int baudRate);
extern void free_serial_2();
extern int Read_Message_Ser2(void);
extern void Send_Message_Ser2(Byte *pBuff, int len);

void Setup ( void );
extern void read_analogInput(void);
extern void processes_messages();
void ADC121C027_IC7_Initialize ( void );
uint32_t ADC121C027_IC7_Read ( void );
void eep_write_byte(unsigned int addr, Byte data_to_send);
void Check_Reset_Cause ( void );
Byte eep_read_byte(unsigned int addr);
void WriteEepromDefault_KTS( void );
void ADS7142_IC9_Initialize ( void );
void ADS7142_IC9_Start_Conversion ( void );
void ADS7142_IC9_Read ( void );
void ADS7142_IC9_Abort_Conversion ( void );
void enter_sleep_mode(void);


//----------------------------------------------
//               Seriali
//----------------------------------------------

//---- STATI RICEZIONE SERIALI --------
#define  RUN_DOWNLOAD              1
#define  BUFFER_RX_EMPY            0
#define  ERROR_HEAD_MESSAGE       -1
#define  ERROR_LEN_MESSAGE        -2
#define  ERROR_CKSUM_16           -3

//------------ Seriale 1 : RD2 (Slave) -----------------
#define BAUDRATE                 9600
#define DIM_MAX_BUFF_SER1         128  // Attenzione il driver della uart1 HA DIMENSIONE massima di 128 Byte


//------------------------x_dbg
short debug_var16[4];
//-----------------------

short cnt_no_Pressure_for_Electrical_Heater = 0;

Byte buff_ser1[DIM_MAX_BUFF_SER1];     // buffer di appoggio
int  cnt_sec_no_link_KTS = 0;
Byte connect_KTS         = 0;


//------------ Seriale 2 : RD2 (Master) ----------------
#define BAUDRATE_SER2           38400
#define DIM_MAX_BUFF_SER2         128

Byte buff_ser2[DIM_MAX_BUFF_SER2];   // buffer di appoggio
int  available_ser2 = 0;             // numero di Byte che il buffer Ë riempito

//-----------  Strutture Dati RAM / EEPROM -----------
SDATA  sData;
Byte   countdown_Update_EepromToShadow = 0xFF;
short  Temp_modbus[4];

//---------- indici sonde di temperatura -------------
Byte i_TFresh  = 0;
Byte i_TReturn = 1;
Byte i_TSupply = 2;
Byte i_TExaust = 3;


//--------- variabili controllo coerenza termodinamica -----

Bool Tcheck = 0;

short S1_T  = 0;
short S1_RH = 0;
short S2_T  = 0;
short S2_RH = 0;
short S4_T  = 0;
short S4_RH = 0;

short T1 =0;
short T2 =0;
short T3 =0;
short T4 =0;
short T5 =0;



//-----------  stato rele' unita ---------------------
Byte PowerMotors   = 0;

// analogic
const int  pAnProbe_Temp1    = 0;
const int  pAnProbe_Temp2    = 1;
const int  pAnProbe_Temp3    = 2;
const int  pAnProbe_Temp4    = 3;
const int  pAnSensQual_Ari1  = 4;
const int  pAnSensQual_Ari2  = 5;
const int  pAnAssorb_ByPass  = 6;
const int  pAnInp1           = 7;
const int  pAnInp2           = 8;
const int  pAnDepot          = 9;
const int  pAnAssorb_KTS     = 10;

//------- Led 5 Rosso, E' montato sulla scheda RD2 --------
#define LED5_BLINK  /*(PORTFINV = 0x0001)*/
volatile int count_blink_led5 = 0;


//------------------ Stati BYPASS ---------------------
#define ANTIORARIO       0xFF
#define ORARIO           0
#define CLOSE_BPD        1   // bypass chiuso, il flusso dell'aria passa dallo scambiatore
#define OPEN_BPD         0   // bypass aperto 100%, il flusso dell'aria passa solo dal canale Bypass.


//-----------------------------------------------------
// Definizione timer di sistema.
//-----------------------------------------------------
S_CLOCK_WEEK    sClockWeek;

unsigned long countHoursFunz  = 0;

//-----------------------------------------------------
// Variabili di appoggio per calcolo portata se presenti
// FLW1 e FLW 2.
//-----------------------------------------------------

#define K_FLOW1          1
#define K_FLOW2          2
#define MAX_KF_SAMPLE    1
//motore 052    // motore 046 / 057 in coclea
const float KF1_1  =      0.0000145424; // 0.0000200253;
const float KF1_2  =     -0.0119456481; //-0.0114827694;
const float KF1_3  =      5.1888249047; // 3.6594526211;
const float KF1_4  =     10.3616153284; // 0.3049370363;

int   contaflow_1  = 0;
int   contaflow_2  = 0;
float sumpress_1   = 0;
float sumpress_2   = 0;
short flow_motor_1 = 0;  //FLW 1
short flow_motor_2 = 0;  //FLW 2

//-----------------------------------------------------
// Variabili di appoggio per EBPD e SSR
//-----------------------------------------------------


Byte info_SSR = 0;
Byte info_EBPD= 0xFF; // definisco un valore iniziale fuori dal range in modo tale da obbligare la sincronizzazione sin dal primo ciclo.
Byte change_SSR = 1;
Byte change_EBPD = 1;
Byte change_EBP2 = 1;


//-----------------------------------------------------
// Variabili di appoggio per TEST IPEHD
//-----------------------------------------------------

bool flag_ipehd = 0;
int counter_ipehd = 0;

volatile uint32_t Scheduler_Counter = 0;

uint32_t adc_val = 0;               // holds ADC value read from ADC channel

uint32_t volatile UART2_Timeout = 0;
uint32_t volatile UART0_Timeout = 0;

volatile uint32_t pir_counter  = 0;
volatile uint32_t twi_counter  = 0;

volatile uint32_t CO2_1_counter  = 0;
volatile uint32_t CO2_2_counter  = 0;

volatile uint32_t CO2_1_Average_Counter  = 0;
volatile uint32_t CO2_2_Average_Counter  = 0;

volatile uint32_t Clean_Event_Counter = 0;

volatile uint32_t Clean_Event_Counter_2_hours = 0;

Pdc* UART0_PDC;
pdc_packet_t pdc_uart0_packet;
uint8_t USART0_buffRX[128];

Pdc* UART2_PDC;
pdc_packet_t pdc_uart2_packet;
uint8_t USART2_buffRX[128];

volatile uint32_t KTS_10min_Counter = 0;

#define LOG_ADDRESS 0x00450000
#define PAGE_SIZE   IFLASH_PAGE_SIZE

uint32_t ul_rc;

uint8_t Reset_Log_Buffer[IFLASH_PAGE_SIZE];
uint32_t Reset_Cause = 0;
uint8_t Num_Data_Saved = 0;

extern bool RD_Clock_Set;

bool Pir_Startup = false;

extern volatile uint32_t PIR_Delay_Counter;

S_EEPROM* Eeprom_Pointer;
extern Byte Shadow_eep[256];

struct {
	uint16_t CH0;
	uint16_t CH1;
	float CH0_Value;
	float CH1_Value;
	bool State;
	bool Start_Conversion;
	} ADS7142_IC9;

bool Battery_Backup = false;

//-----------------------------------------------------
// calcola il size della macchina
//-----------------------------------------------------
unsigned short calc_size_unit()
{
	Byte temp;
	unsigned short size_unit;
	
	// in base ai dati del SN ricavo il size della unita'
	// Example SN: 1565 0023 115 1351 002.
	temp =  read_byte_eeprom(ADDR_EEP(SerialString[4])) - '0';
	size_unit = (temp * 100);
	temp =  read_byte_eeprom(ADDR_EEP(SerialString[5])) - '0';
	size_unit += (temp * 10);
	temp =  read_byte_eeprom(ADDR_EEP(SerialString[6])) - '0';
	size_unit += temp;
	
	return size_unit;
}

//-----------------------------------------------------
// Inizializza la struttura dei dati in ram
//-----------------------------------------------------
void init_sdata()
{
	int i;
	Byte val;
	
	sData.KTS_Connect        = 0;
	sData.level_link_CTRLFAN = 0;
	
	sData.size_unit = calc_size_unit();
	
	for(i=0; i < TOT_ACCESSORY_I2C; i++) {
		sData.AccI2C[i].comand      = 0;         // Comando di Accensione o Spegnimento
		sData.AccI2C[i].sts         = 0;         // Stato in cui si trova l'accessorio: ....,bit[4]Stato Comando Acc.ON/OFF, bit[3]:1=Elect/0=Water, bit[2]:Stato Acc. 1=ON/0=OFF, bit[1]:1=OPEARATIVO, bit[0]:1 = Collegato
		sData.AccI2C[i].measure1    = 0;         // misura principale
		sData.AccI2C[i].measure2    = 0;         // misura secondaria
		sData.AccI2C[i].measure3    = 0;         // misura secondaria
		sData.AccI2C[i].measure4    = 0;         // misura secondaria
		sData.AccI2C[i].measure5    = 0;         // misura secondaria
		sData.AccI2C[i].data        = 0;         // byte Data: Per gli accessori HWD, EHD, PEHD, Ë la taglia, per le sonde viene passato l'info. se sono da canale 'd', oppure d'ambiente 'a'
		sData.AccI2C[i].level_link  = 0;         // livello  della comunicazione (valore espresso in 10/10)
		sData.AccI2C[i].cnt_link_ok = 0;         // Contatore Link buoni
		sData.AccI2C[i].cnt_link_ko = 0;         // Contatore Link falliti
		sData.AccI2C[i].cnt_persist_link_ko = 0; // contatore di persistenza di collegamenti Falliti
		sData.AccI2C[i].status_comand = 0;
	}
	
	sData.RegDXD.processor_I2C   = 0;
	sData.RegDXD.count           = 0;
	
	// (HIGH)bit[15]:--- | bit[14]:--- | bit[13]:---    | bit[12]:PREHEATING| bit[11]:ENAB_SUMMER | bit[10]:ENAB_WINTER | bit[9]:CtrlFilter | bit[8]:PASSWORD
	if(read_word_eeprom(ADDR_EEP(Enab_Fuction)) & 0x0800) {
		sData.RegDXD.func_compressor   = 'S';
		sData.AccI2C[ACC_I2C_DXD].info = 'S';
		}else {
		sData.RegDXD.func_compressor   = 'W';
		sData.AccI2C[ACC_I2C_DXD].info = 'W';
	}
	
	sData.info_ProbesAD  = 0xFF;                // 7.P2CO2, 6.P1CO2, 5.BPD, 4.P2RH, 3.P1RH, 2.SSR, 1.P1VOC, 0.-----   (bit: 0 = Ambiente, 1 = Duct)
	
	// setting par.
	sData.speed_motors_R = 0;                   // comando ai motori Returns, 0 -> 255 , il dato si pu√≤ discostare dal valore in eeprom se vi √® in corso un Defrost, oppure una Co2 Alarm,...
	sData.speed_motors_F = 0;                   // comando ai motori Returns, 0 -> 255 , il dato si pu√≤ discostare dal valore in eeprom se vi √® in corso un Defrost, oppure una Co2 Alarm,...
	
	sData.increase_speed_RH_CO2 = 10;           // Coefficiente moltiplicativo per 10,  10= 1.0 ed Ë l'incremento della velocit‡ rispetto al set-poin a causa: RH alta, o CO2 alta, o VOCs alta.
	
	// misure
	sData.feedbk_v_speed_motors_R = 0;          // tensione letta sul comando dei motori ramo RETURN (0.00V a 11.00V) il valore √® moltipl. per 100, quindi 1100 = 11.00V
	sData.feedbk_v_speed_motors_F = 0;          // tensione letta sul comando dei motori ramo FRESH  (0.00V a 11.00V) il valore √® moltipl. per 100, quindi 1100 = 11.00V
	sData.measure_CO2_max         = 0;          // valore massimo di Co2 tra i vari sensori: espresso in PPM ( da 0 a 2000 PPM)
	sData.measure_RH_max          = 0;          // valore massimo di Umidit√  tra i vari sensori: da 0% a 99%
	sData.measure_VOC_max         = 0;          // valore massimo di VOC tra i vari sensori: da 0  a 10000 ug/M3
	sData.measure_pressure        = 0;          // valore espresso in Pascal da 0 a 1000Pa
	sData.measure_RH_Analogic     = 0;          // Misura della sonda RH  interna all'Unita'
	sData.measure_VOC_Analogic    = 0;          // Misura della sonda VOC interna all'Unita'
	sData.measure_CO2_Analogic    = 0;          // Misura della sonda CO2 interna all'Unita'
	
	for(i=0; i < 4; i++)                        // 0=T_Fresh, 1=T_Return, 2=T_Supply, 3=T_Exhaust  (i valori temp. sono espressi per 10, es: 102 = 10.2 ¬∞C)
	sData.measure_Temp[i] = 0;
	
	sData.measure_ADC_input[0] = 0;             // i valori sono grezzi (0 a 1024)
	sData.measure_ADC_input[1] = 0;
	
	sData.measure_I_KTS    = 0;                 // valori espresi in mA: FS=1024 = 250mA
	
	sData.absorption_bypass_min = 0;            // Assorbimento  minimo mA del motore Bypass
	sData.absorption_bypass_max = 0;            // Assorbimento massimo mA del motore Bypass
	
	for(i=0; i < 3; i++){
		sData.rpm_motors_R[i] = 0;
		sData.rpm_motors_F[i] = 0;
	}
	
	// stati
	sData.status_unit   = 0;
	sData.status_weekly = 0;                    // 1 byte:  bit[7,6]:1,0=SPEED3/ 0,1=SPEED2/ 0,0=SPEED1 | bit[5,4]: 0,1=IMBAL_1_ON/ 0,0=IMBAL_OFF | bit[3,2]:0,1=RIF_TEMP2/ 0,0=RIF_TEMP1 | bit[1]: WEEKLY_RUN, bit[0]:WEEKLY_ENABLE
	sData.status_Leds_ctrlFan = 0;              // 1 byte:   [1,0] LED BYPASS    (00 = OFF, 01=Toggle, 10= ON, 11= none)
	//           [3,2] LED ALARM     (00 = OFF, 01=Toggle, 10= ON, 11= none)
	//           [5,4] LED MOTORS R  (00 = OFF, 01=Toggle, 10= ON, 11= none)
	//           [7,6] LED MOTORS F  (00 = OFF, 01=Toggle, 10= ON, 11= none)
	sData.StatusMotorsR = 0;                    // 1 byte:   bit.. 7.none | 6.AlarmM3 | 5.AlarmM2 | 4.AlarmM1 | 3.none | 2.PowerON_M3 | 1.PowerON_M2 | 0.PowerON_M1
	sData.StatusMotorsF = 0;                    // 1 byte:   bit.. 7.none | 6.AlarmM3 | 5.AlarmM2 | 4.AlarmM1 | 3.none | 2.PowerON_M3 | 1.PowerON_M2 | 0.PowerON_M1

	sData.status_test   = 0;
	sData.timeout_test  = 0;
	
	sData.Status_RemoteCTRL = 0;
	// List AccessoyHW[1]:    DPS, PCAF,   PCAP, -----,  ----,  ----,   RFM,  MBUS  (LSB)
	val   = read_byte_eeprom(ADDR_EEP(AccessoyHW[1]));
	if(val & 0x01)
	sData.Status_RemoteCTRL =  0x80; // Modbus
	
	if(val & 0x02)
	sData.Status_RemoteCTRL |= 0x40; // RFM
	
	// Allarmi/ Eventi
	InitAlarm();
	
	eep_write_byte( ADDR_EEP(KTS_Watchdog), 0);
	// start 10 min counter
	KTS_10min_Counter = 0;
	
}

int main (void)
{

	static usart_serial_options_t uart0_options;
	uart0_options.baudrate =   CONF_UART0_BAUDRATE;
	uart0_options.charlength = CONF_UART0_CHAR_LENGTH;
	uart0_options.paritytype = CONF_UART0_PARITY;
	uart0_options.stopbits =   CONF_UART0_STOP_BITS;
	
	static usart_serial_options_t uart2_options;
	uart2_options.baudrate =   CONF_UART2_BAUDRATE;
	uart2_options.charlength = CONF_UART2_CHAR_LENGTH;
	uart2_options.paritytype = CONF_UART2_PARITY;
	uart2_options.stopbits =   CONF_UART2_STOP_BITS;
	
	static twi_master_options_t twi0_options;
	twi0_options.speed = 400000;
	
	struct adc_config adc_conf;     // struct for configuring ADC
	
	Reset_Cause = rstc_get_reset_cause( RSTC );
	
    pmc_switch_sclk_to_32kxtal(PMC_OSC_XTAL);   // enable external 32.768kHz crystal
    while (!pmc_osc_is_ready_32kxtal()){};        // wait until oscillator is ready
	
	/* Insert system clock initialization code here (sysclk_init()). */
	sysclk_init();
	
	Reset_Cause = Reset_Cause >> 8;
	
	board_init();
	
	rtc_set_hour_mode(RTC, 0);
	
	/* Configure RTC interrupts */
	rtc_enable_interrupt(RTC, RTC_IER_SECEN );
	
	/* Insert application code here, after the board has been initialized. */
	
	// initialize the UART0
	usart_serial_init(CONF_UART0, &uart0_options);
	
	/* Get pointer to UART PDC register base */
	UART0_PDC = uart_get_pdc_base(CONF_UART0);
	
	/* Initialize PDC data packet for transfer */
	pdc_uart0_packet.ul_addr = USART0_buffRX;
	pdc_uart0_packet.ul_size = 120;
	
	/* Configure PDC for data receive */
	pdc_rx_init(UART0_PDC, &pdc_uart0_packet, NULL);
	
	/* Enable PDC transfers */
	pdc_enable_transfer(UART0_PDC, PERIPH_PTCR_RXTEN);

	Serial_begin();
	//uart_enable_interrupt(CONF_UART0, UART_IER_RXRDY );
	//NVIC_EnableIRQ(UART0_IRQn);
	
	// initialize the UART2
	usart_serial_init(CONF_UART2, &uart2_options);
	
	/* Get pointer to UART PDC register base */
	UART2_PDC = uart_get_pdc_base(CONF_UART2);
	
	/* Initialize PDC data packet for transfer */
	pdc_uart2_packet.ul_addr = USART2_buffRX;
	pdc_uart2_packet.ul_size = 100;
	
	/* Configure PDC for data receive */
	pdc_rx_init(UART2_PDC, &pdc_uart2_packet, NULL);
	
	/* Enable PDC transfers */
	pdc_enable_transfer(UART2_PDC, PERIPH_PTCR_RXTEN);
	
	Serial2_begin();
	//uart_enable_interrupt(CONF_UART2, UART_IER_RXRDY );
	//NVIC_EnableIRQ(UART2_IRQn);
	
	// initialize the TWI0
	twi_master_setup(MAIN_I2C, &twi0_options);
	
	delay_ms ( 100 );
	
	// configure and enable the ADC
	adc_enable();                           // enable ADC
	adc_get_config_defaults(&adc_conf);     // read ADC default values
	adc_conf.resolution = ADC_10_BITS;
	adc_init(ADC, &adc_conf);               // initialize the ADC with default values
	adc_set_trigger(ADC, ADC_TRIG_SW);      // set ADC to trigger with software for polling
	adc_channel_enable(ADC, ADC_CHANNEL_0); // enable ADC channel 0 (AD0, pin PA17 on ATSAM4N16B)
	adc_channel_enable(ADC, ADC_CHANNEL_1); // enable ADC channel 1 (AD1, pin PA18 on ATSAM4N16B)
	adc_channel_enable(ADC, ADC_CHANNEL_2); // enable ADC channel 2 (AD2, pin PA19 on ATSAM4N16B)
	adc_channel_enable(ADC, ADC_CHANNEL_3); // enable ADC channel 3 (AD3, pin PA20 on ATSAM4N16B)
	adc_channel_enable(ADC, ADC_CHANNEL_5); // enable ADC channel 5 (AD5, pin PB1 on ATSAM4N16B)
	adc_channel_enable(ADC, ADC_CHANNEL_6); // enable ADC channel 6 (AD5, pin PB2 on ATSAM4N16B)
	adc_channel_enable(ADC, ADC_CHANNEL_7); // enable ADC channel 7 (AD7, pin PB3 on ATSAM4N16B)
	adc_channel_enable(ADC, ADC_CHANNEL_8); // enable ADC channel 8 (AD8, pin PA21 on ATSAM4N16B)
	
	SysTick_Config(sysclk_get_cpu_hz() / 1000);
	
	ADC121C027_IC7_Initialize ( );
	
	ADS7142_IC9_Initialize ( );
	
	delay_ms ( 100 );
	
	//SysTick_Config(sysclk_get_cpu_hz() / 1000);
	ul_rc = flash_init(FLASH_ACCESS_MODE_128, 6);
	
	Setup ( );
	
	Check_Reset_Cause ( );
	
	wdt_restart(WATCH_DOG);
	
	Pir_Startup = true;
	count_blink_led5 = 0;
	
	while (1)
	{
		
		// Leggi i segnali analogici
		read_analogInput();
		
		// precessa le funzione nello scheduler
		process_scheduler();
		
		// verifica se vi sono messaggi in giacenza
		processes_messages();
		if ( !Battery_Backup )
		{
			if ( eep_read_byte(ADDR_EEP(KTS_Watchdog )) == 1 ) // write default eeprom from KTS command
			{
				eep_write_byte( ADDR_EEP(KTS_Watchdog), 0);
				WriteEepromDefault_KTS( );
				NVIC_SystemReset ();
			}
		}
		
		//if ( KTS_10min_Counter >= 60000 )
			//NVIC_SystemReset ();
			
		if ( ADS7142_IC9.State )
		{
			if ( (sData.status_unit & MSK_UNIT_OPERATING ) == 0) // unit in standby mode 6.2 v
			{
				if ( ADS7142_IC9.CH1_Value < 2.5)
				{
					Battery_Backup = true;
					//delay_ms ( 6000 );
					//enter_sleep_mode();
					//NVIC_SystemReset ();
				}
				else
				{
					if (Battery_Backup)
						NVIC_SystemReset ();
				}
			}
			else
			{
				if ( ADS7142_IC9.CH1_Value < 2.5)
				{
					Battery_Backup = true;
					//delay_ms ( 6000 );
					//enter_sleep_mode();
					//NVIC_SystemReset ();
				}
				else
				{
					if (Battery_Backup)
						NVIC_SystemReset ();
				}
			}
		}
			
		if (( Reset_Cause != 0) && ( Reset_Cause < 6))
			{
				if (RD_Clock_Set)
				{
					Check_Reset_Cause ( );
					Reset_Cause = 10;
				}
			}	
		else
			if ( Reset_Cause == 0 )
				{
					if (RD_Clock_Set)
					{
						Check_Reset_Cause ( );
						Reset_Cause = 10;
					}
				}
	}
}

volatile uint32_t tick = 0;

void SysTick_Handler ( void )
{
	tick++;
	UART0_Timeout++;
	pir_counter++;
	UART2_Timeout++;
	twi_counter++;
	CO2_1_counter++;
	CO2_2_counter++;
	CO2_1_Average_Counter++;
	CO2_2_Average_Counter++;
	Clean_Event_Counter++;
	Clean_Event_Counter_2_hours++;
	KTS_10min_Counter++;
	count_blink_led5++;
	PIR_Delay_Counter++;
	if ( Scheduler_Counter >= 100 )
		{
			Scheduler_Counter = 0;
			Scheduler_Timer_Callback ();
		}
	else	
		Scheduler_Counter++;
}

unsigned long millis( void )
{
	return tick;
}

Byte constrain(Byte x, Byte a, Byte b) {
	if(x < a) {
		return a;
	}
	else if(b < x) {
		return b;
	}
	else
	return x;
}

Byte eep_read_byte(unsigned int addr)
{
	Byte Data_Received;
	
	twi_package_t packet_read;
	packet_read.addr[0]      = addr;      // TWI slave memory address data
	packet_read.addr_length  = sizeof (uint8_t);    // TWI slave memory address data size
	packet_read.chip         = DEVICE_24LCO2B;      // TWI slave bus address
	packet_read.buffer       = &Data_Received; // transfer data source buffer
	packet_read.length       = sizeof(Data_Received);  // transfer data size (bytes)
	
	twi_master_read(MAIN_I2C, &packet_read);
	return Data_Received;
}

void eep_write_byte(unsigned int addr, Byte data_to_send)
{
	
	twi_package_t packet_write;
	packet_write.addr[0]      = addr;      // TWI slave memory address data
	packet_write.addr_length  = sizeof (uint8_t);    // TWI slave memory address data size
	packet_write.chip         = DEVICE_24LCO2B;      // TWI slave bus address
	packet_write.buffer       = &data_to_send; // transfer data source buffer
	packet_write.length       = sizeof(data_to_send);  // transfer data size (bytes)
	
	while (twi_master_write(MAIN_I2C, &packet_write) != TWI_SUCCESS);
}

void Setup ( void )
{
	 Byte val;
	 
	 #ifdef WORK_WDT
	 // check for a previous WDT Event
	 /*if (ReadEventWDT())
	 {
		 DisableWDT();       // A WDT event did occur
		 ClearEventWDT();    // clear the WDT event flag so a subsequent event can set the event bit
	 }*/
	 #endif
	 
	 //val = ADDR_EEP(Set_Power_ON);
	 //Erase_EEprom (); // erase full eeprom
	 //for (int  j=0; j<18 ; j++ )
		//eep_write_byte(ADDR_EEP(SerialString[j]) , SN[j]);
	 //eep_write_byte(ADDR_EEP(SetPointTemp_Heater) , 0xDC);
	 //eep_write_byte(ADDR_EEP(SetPointTemp_Heater) + 1, 0x00);
	 
	 PowerMode(POWER_ON);                   // si parte con il power , D26: PowerMotors 0=ON/ 1= OFF
	  
	 //---------- Verifichiamo la eeprom ----------------
	 ctrl_eeprom();
	 
	 //---------- Init struct Data ----------------------
	 init_sdata();
	 
	 Eeprom_Pointer = &Shadow_eep;
	 
	 ioport_set_pin_level(pBuz, IOPORT_PIN_LEVEL_HIGH);
	 
	 //wdt_disable(WATCH_DOG);					 
	 //----------- Se viene premuto un tasto entraimo nella shell ------------
	 shell_config_menu(20);                // Diamo tempo massimo per entrare nella shell 2,0 sec.
	 
	 wdt_restart(WATCH_DOG);
	 
	 ioport_set_pin_level(pStatusLedI2C, IOPORT_PIN_LEVEL_LOW);  // led SPENTO
	 
	 //---------- Init miniKernel -----------------------
	 init_scheduler();
	 
	 ioport_set_pin_level(pBuz, IOPORT_PIN_LEVEL_LOW);
	 ioport_set_pin_level(pLED5, IOPORT_PIN_LEVEL_LOW);
	 
	 wdt_restart(WATCH_DOG);
	 
	 //------------------ verifica la validit‡ del serial Number, se Ë falso  blocca l'unit‡ -------
	 Check_SerialNumber();
	 
	 ioport_set_pin_level(pSwitchOFF_KTS, IOPORT_PIN_LEVEL_HIGH);  // KTS HIGH
	 
	 // diamo il comando di chiusura bypass, per allineare l'HW al fine corsa di chiusura.
	 // Attenzione al senso di rotazione per la chiusura.
	 if(read_byte_eeprom(ADDR_EEP(Config_Bypass)) != BPD_MANUAL_CLOSE)
		Active_Procedure_Bypass_OpCl(CLOSE_BPD, 0);
	 
	 
	 // Verifica gli indici delle sonde NTC
	 val = read_byte_eeprom(ADDR_EEP(Posiz_NTC));
	 
	 if(val != 0xFF) {
		 i_TFresh  = (val & 0x03);
		 i_TReturn = (val >> 2) & 0x03;
		 i_TSupply = (val >> 4) & 0x03;
		 i_TExaust = (val >> 6) & 0x03;
	 }
	  
	 // piccolo tempo di attesa.
	 delay_ms (100);
	 wdt_restart(WATCH_DOG);
	 
	 //--------------- Carica lo Scheduler (mini Kernel) --------------
	 insert_function_scheduler(motor_speed,                12);   // sample  1.2 sec.
	 insert_function_scheduler(IPEHD_test,                 30);   // sample  3.0 sec.
	 insert_function_scheduler(operatingHoursCounter,      10);   // sample  1.0 sec.
	 insert_function_scheduler(weekly_program,             10);   // sample  1.0 sec.
	 insert_function_scheduler(polling_serial_2,            6);   // sample  0.6 sec.
	 insert_function_scheduler(TemperatureRegulate,        50);   // sample  5.0 sec.
	 //  insert_function_scheduler(ThermoCheck,                30);   // sample  5.0 sec. // NON ANCORA PRONTA
	 insert_function_scheduler(pollingDigitAccessory,       2);   // sample  0.2 sec.  //era 0.4 sec.
	 insert_function_scheduler(Test_Connect_KTS,           15);   // sample  1.5 sec.
	 insert_function_scheduler(Gestione_Relays,            25);   // sample  2.5 sec.
	 
	 insert_function_scheduler(CkUpdate_EepromToShadow,    13);   // sample  1.3 sec.

	 #ifdef WORK_WDT
	 // WDT timeout period is set in the Device Configuration (WDTPS) defaukt 0.7 sec.
	 //EnableWDT(); // enable the WDT
	 #endif
}

unsigned analogRead ( Byte Channel_Num )
{
	
	switch ( Channel_Num )
	{
		case /*pAnProbe_Temp1*/0:
			// start the ADC conversion
			adc_start_software_conversion(ADC);
			// wait until the conversion complete bit for AD0 is set
			while (!(adc_get_interrupt_status(ADC) & 1));
			// read the value from the ADC for channel 0
			adc_val = adc_channel_get_value(ADC, ADC_CHANNEL_0);
		break;
		
		case /*pAnProbe_Temp2*/1:
			// start the ADC conversion
			adc_start_software_conversion(ADC);
			// wait until the conversion complete bit for AD1 is set
			while (!(adc_get_interrupt_status(ADC) & 2));
			// read the value from the ADC for channel 1
			adc_val = adc_channel_get_value(ADC, ADC_CHANNEL_1);
		break;
		
		case /*pAnProbe_Temp3*/2:
			// start the ADC conversion
			adc_start_software_conversion(ADC);
			// wait until the conversion complete bit for AD2 is set
			while (!(adc_get_interrupt_status(ADC) & 4));
			// read the value from the ADC for channel 2
			adc_val = adc_channel_get_value(ADC, ADC_CHANNEL_2);
		break;

		case /*pAnProbe_Temp4*/3:
			// start the ADC conversion
			adc_start_software_conversion(ADC);
			// wait until the conversion complete bit for AD3 is set
			while (!(adc_get_interrupt_status(ADC) & 8));
			// read the value from the ADC for channel 3
			adc_val = adc_channel_get_value(ADC, ADC_CHANNEL_3);
		break;
		
		case /*pAnSensQual_Ari1*/4:
			adc_val = ADC121C027_IC7_Read ( );
		break;
		
		case /*pAnAssorb_ByPass*/6:
			// start the ADC conversion
			adc_start_software_conversion(ADC);
			// wait until the conversion complete bit for AD6 is set
			while (!(adc_get_interrupt_status(ADC) & 0x40));
			// read the value from the ADC for channel 6
			adc_val = adc_channel_get_value(ADC, ADC_CHANNEL_6);
		break;
		
		case /*pAnAssorb_KTS*/10:
			// start the ADC conversion
			adc_start_software_conversion(ADC);
			// wait until the conversion complete bit for AD5 is set
			while (!(adc_get_interrupt_status(ADC) & 0x20));
			// read the value from the ADC for channel 5
			adc_val = adc_channel_get_value(ADC, ADC_CHANNEL_5);
		break;
		
		case /*pAnInp1*/7:
			// start the ADC conversion
			adc_start_software_conversion(ADC);
			// wait until the conversion complete bit for AD7 is set
			while (!(adc_get_interrupt_status(ADC) & 0x80));
			// read the value from the ADC for channel 7
			adc_val = adc_channel_get_value(ADC, ADC_CHANNEL_7);
		break;
		
		case /*pAnInp2*/8:
			// start the ADC conversion
			adc_start_software_conversion(ADC);
			// wait until the conversion complete bit for AD8 is set
			while (!(adc_get_interrupt_status(ADC) & 0x100));
			// read the value from the ADC for channel 8
			adc_val = adc_channel_get_value(ADC, ADC_CHANNEL_8);
		break;
		
		default:
			adc_val = 0;
		break;
	}
	
	return adc_val;
}

void ADC121C027_IC7_Initialize ( void )
{
	unsigned int addr; 
	Byte data_to_send;
	
	data_to_send = 0x60;
	twi_package_t packet_write;
	packet_write.addr[0]      = 0x2;      // TWI slave memory address data
	packet_write.addr_length  = sizeof (uint8_t);    // TWI slave memory address data size
	packet_write.chip         = DEVICE_ADC121C027_IC7;      // TWI slave bus address
	packet_write.buffer       = &data_to_send; // transfer data source buffer
	packet_write.length       = sizeof(data_to_send);  // transfer data size (bytes)
	
	while (twi_master_write(MAIN_I2C, &packet_write) != TWI_SUCCESS);
}

uint32_t ADC121C027_IC7_Read ( void )
{
	uint32_t	Read_Value;
	Byte data_received[2];
	
	twi_package_t packet_read;
	
	packet_read.addr[0]      = 0;      // TWI slave memory address data
	packet_read.addr_length  = sizeof (uint8_t);    // TWI slave memory address data size
	packet_read.chip         = DEVICE_ADC121C027_IC7;      // TWI slave bus address
	packet_read.buffer       = &data_received; // transfer data source buffer
	packet_read.length       = sizeof(data_received);  // transfer data size (bytes)
	
	twi_master_read(MAIN_I2C, &packet_read);
	Read_Value = 0;
	Read_Value = (uint16_t)(data_received[0] << 8) | data_received[1];
	
	Read_Value = ( Read_Value * 1023) / 4095;
	
	return Read_Value;
}

void ADS7142_IC9_Initialize ( void )
{
	unsigned int addr;
	Byte data_to_send[3];
	twi_package_t packet_write;
	Byte data_received;
	twi_package_t packet_read;
	
	ADS7142_IC9.State = false;
	ADS7142_IC9.Start_Conversion = false;
	ADS7142_IC9.CH0_Value = 12;
	ADS7142_IC9.CH1_Value = 5;
	if ( twi_probe(MAIN_I2C, DEVICE_ADS7142_IC9) == TWI_SUCCESS )
		ADS7142_IC9.State = true;
	else
		return;
		
	data_to_send[0] = 0x08;
	data_to_send[1] = 0x14;
	data_to_send[2] = 0x01;
	
	//packet_write.addr[0]      = 0x8;      // ADS7142_IC9 write sequence
	//packet_write.addr[1]      = 0x20;      // ADS7142_IC9 AUTO_SEQ_CHEN select both channels for auto sequencing
	packet_write.addr_length  = 0;    // TWI slave memory address data size
	packet_write.chip         = DEVICE_ADS7142_IC9;      // TWI slave bus address
	packet_write.buffer       = &data_to_send; // transfer data source buffer
	packet_write.length       = sizeof(data_to_send);  // transfer data size (bytes)
	
	while (twi_master_write(MAIN_I2C, &packet_write) != TWI_SUCCESS);	
		
	data_to_send[0] = 0x08;
	data_to_send[1] = 0x20;
	data_to_send[2] = 0x03;
	
	//packet_write.addr[0]      = 0x8;      // ADS7142_IC9 write sequence
	//packet_write.addr[1]      = 0x20;      // ADS7142_IC9 AUTO_SEQ_CHEN select both channels for auto sequencing
	packet_write.addr_length  = 0;    // TWI slave memory address data size
	packet_write.chip         = DEVICE_ADS7142_IC9;      // TWI slave bus address
	packet_write.buffer       = &data_to_send; // transfer data source buffer
	packet_write.length       = sizeof(data_to_send);  // transfer data size (bytes)
	
	while (twi_master_write(MAIN_I2C, &packet_write) != TWI_SUCCESS);
	
	data_to_send[0] = 0x08;
	data_to_send[1] = 0x1C;
	data_to_send[2] = 0x04;
	
	//packet_write.addr[0]      = 0x8;      // ADS7142_IC9 write sequence
	//packet_write.addr[1]      = 0x1C;      // ADS7142_IC9 AUTO_SEQ_CHEN select both channels for auto sequencing
	packet_write.addr_length  = 0;    // TWI slave memory address data size
	packet_write.chip         = DEVICE_ADS7142_IC9;      // TWI slave bus address
	packet_write.buffer       = &data_to_send; // transfer data source buffer
	packet_write.length       = sizeof(data_to_send);  // transfer data size (bytes)
	
	while (twi_master_write(MAIN_I2C, &packet_write) != TWI_SUCCESS);
	
	ADS7142_IC9_Start_Conversion ( );
}

void ADS7142_IC9_Start_Conversion ( void )
{
	unsigned int addr;
	Byte data_to_send[3];
	
	if ( ADS7142_IC9.Start_Conversion )
		return;
		
	ADS7142_IC9.Start_Conversion = false;
	
	data_to_send[0] = 0x08;
	data_to_send[1] = 0x1E;
	data_to_send[2] = 0x01;
	
	twi_package_t packet_write;
	//packet_write.addr[0]      = 0x8;      // ADS7142_IC8 write sequence
	//packet_write.addr[1]      = 0x1E;      // ADS7142_IC8 AUTO_SEQ_CHEN select both channels for auto sequencing
	packet_write.addr_length  = 0;    // TWI slave memory address data size
	packet_write.chip         = DEVICE_ADS7142_IC9;      // TWI slave bus address
	packet_write.buffer       = &data_to_send; // transfer data source buffer
	packet_write.length       = sizeof(data_to_send);  // transfer data size (bytes)
	
	while (twi_master_write(MAIN_I2C, &packet_write) != TWI_SUCCESS);
	ADS7142_IC9.Start_Conversion = true; 
}

void ADS7142_IC9_Read ( void )
{
	uint32_t	Read_Value;
	Byte data_received[20];
	Byte data_to_send[2];
	
	
	twi_package_t packet_read;
	twi_package_t packet_write;
	
	if ( ADS7142_IC9.Start_Conversion )
	{
		
		//packet_read.addr[0]      = 0x30;      // TWI slave memory address data
		//packet_read.addr[1]      = 0x8;
		packet_read.addr_length  = 0;    // TWI slave memory address data size
		packet_read.chip         = DEVICE_ADS7142_IC9;      // TWI slave bus address
		packet_read.buffer       = &data_received; // transfer data source buffer
		packet_read.length       = 20;  // transfer data size (bytes)
		
		while ( twi_master_read(MAIN_I2C, &packet_read) != TWI_SUCCESS);
		
		ADS7142_IC9.CH0 = 0;
		ADS7142_IC9.CH0 |=  data_received[1] >> 4;
		
		ADS7142_IC9.CH0 |= (uint16_t)data_received[0] << 4;
		ADS7142_IC9.CH0 &= 0xFFF;
		
		ADS7142_IC9.CH1 = 0;
		ADS7142_IC9.CH1 |=  data_received[3] >> 4;
		
		ADS7142_IC9.CH1 |= (uint16_t)data_received[2] << 4;
		ADS7142_IC9.CH1 &= 0xFFF;
		
		ADS7142_IC9.CH0_Value = ((( 3.3 / 4095) * ADS7142_IC9.CH0)  * 138) / 38; // 12v
		ADS7142_IC9.CH1_Value = ((( 3.3 / 4095) * ADS7142_IC9.CH1)  * 151.5) / 100; // 5v
		
		//ADS7142_IC9_Abort_Conversion ();
		
	}
	
}

void ADS7142_IC9_Abort_Conversion ( void )
{
	unsigned int addr;
	Byte data_to_send[3];
	
	data_to_send[0] = 0x08;
	data_to_send[1] = 0x1F;
	data_to_send[2] = 0x01;
	
	twi_package_t packet_write;
	
	//packet_write.addr[0]      = 0x8;      // ADS7142_IC8 write sequence
	//packet_write.addr[1]      = 0x1F;      // ADS7142_IC8 AUTO_SEQ_CHEN select both channels for auto sequencing
	packet_write.addr_length  = 0;    // TWI slave memory address data size
	packet_write.chip         = DEVICE_ADS7142_IC9;      // TWI slave bus address
	packet_write.buffer       = &data_to_send; // transfer data source buffer
	packet_write.length       = sizeof(data_to_send);  // transfer data size (bytes)
	
	while (twi_master_write(MAIN_I2C, &packet_write) != TWI_SUCCESS);
	ADS7142_IC9.Start_Conversion = false;
}

void enter_sleep_mode(void) {

	// Enter Sleep Mode
	//scb_disable_interrupts(); // Disable interrupts before sleep
	//pmc_sleep(PMC_SLEEP_MODE_SLEEP); // Put CPU to Sleep
	wdt_disable( WATCH_DOG );
	SysTick->CTRL &= ~SysTick_CTRL_TICKINT_Msk;
	SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
	__disable_irq();
	pmc_sleep(SAM_PM_SMODE_SLEEP_WFI); // for Wait Mode
	__enable_irq();
	// Or pmc_sleep(PMC_SLEEP_MODE_BACKUP); for Backup Mode
	//scb_enable_interrupts(); // Re-enable interrupts after wake-up
}

void WDT_Handler ( void )
{
	//Serial_print1("Watchdog Event Trigered");

}

void Check_Reset_Cause ( void )
{
	uint8_t* Pointer;
	uint32_t year;
	uint32_t month;
	uint32_t day;
	uint32_t weekday;
	uint32_t hour;
	uint32_t minute;
	uint32_t second;
	uint8_t Index;
	
	memset ( Reset_Log_Buffer, 0, sizeof(Reset_Log_Buffer));
	Num_Data_Saved = *(uint8_t*)( LOG_ADDRESS );
	rtc_get_date(RTC, &year, &month, &day, &weekday);
	rtc_get_time(RTC, &hour, &minute, &second);
	
	year = 0;
	year = sClockWeek.Year;
	year = year + ( sClockWeek.Century * 100);
	month = sClockWeek.Mounth;
	day = sClockWeek.Day;
	
	if  ( (day == 0 ) && ( month == 0))
		return;
	
	if ( Num_Data_Saved > 20 ) // LOG EMPTY , FIll with 0
	{
		Num_Data_Saved = 1;
		Reset_Log_Buffer[0] = Num_Data_Saved;
		Reset_Log_Buffer[(Num_Data_Saved - 1) + 1] = Reset_Cause;
		Reset_Log_Buffer[(Num_Data_Saved - 1) + 2] = year >> 8;
		Reset_Log_Buffer[(Num_Data_Saved - 1) + 3] = year & 0xFF;
		Reset_Log_Buffer[(Num_Data_Saved - 1) + 4] = month;
		Reset_Log_Buffer[(Num_Data_Saved - 1) + 5] = day;
		Reset_Log_Buffer[(Num_Data_Saved - 1) + 6] = read_byte_eeprom(ADDR_EEP(hour_runnig[3]));
		Reset_Log_Buffer[(Num_Data_Saved - 1) + 7] = read_byte_eeprom(ADDR_EEP(hour_runnig[2]));
		Reset_Log_Buffer[(Num_Data_Saved - 1) + 8] = read_byte_eeprom(ADDR_EEP(hour_runnig[1]));
		Reset_Log_Buffer[(Num_Data_Saved - 1) + 9] = read_byte_eeprom(ADDR_EEP(hour_runnig[0]));
	    wdt_restart(WATCH_DOG);
		ul_rc = flash_erase_sector(LOG_ADDRESS);
		ul_rc = flash_unlock(LOG_ADDRESS ,LOG_ADDRESS + ((10 * IFLASH_PAGE_SIZE) - 1), 0, 0);
		wdt_restart(WATCH_DOG);
		ul_rc = flash_write(LOG_ADDRESS , Reset_Log_Buffer, IFLASH_PAGE_SIZE, 0);
	}
	else
		if ( Num_Data_Saved < 20 ) // STILL free place on Log, enter new data
		{
			Pointer = (uint8_t*)( LOG_ADDRESS );
			memcpy (Reset_Log_Buffer, Pointer, 512);
			Num_Data_Saved++;
			Reset_Log_Buffer[0] = Num_Data_Saved;
			Reset_Log_Buffer[((Num_Data_Saved - 1)*9) + 1] = Reset_Cause;
			Reset_Log_Buffer[((Num_Data_Saved - 1)*9) + 2] = year >> 8;
			Reset_Log_Buffer[((Num_Data_Saved - 1)*9) + 3] = year & 0xFF;
			Reset_Log_Buffer[((Num_Data_Saved - 1)*9) + 4] = month;
			Reset_Log_Buffer[((Num_Data_Saved - 1)*9) + 5] = day;
			Reset_Log_Buffer[((Num_Data_Saved - 1)*9) + 6] = read_byte_eeprom(ADDR_EEP(hour_runnig[3]));
			Reset_Log_Buffer[((Num_Data_Saved - 1)*9) + 7] = read_byte_eeprom(ADDR_EEP(hour_runnig[2]));
			Reset_Log_Buffer[((Num_Data_Saved - 1)*9) + 8] = read_byte_eeprom(ADDR_EEP(hour_runnig[1]));
			Reset_Log_Buffer[((Num_Data_Saved - 1)*9) + 9] = read_byte_eeprom(ADDR_EEP(hour_runnig[0]));
			wdt_restart(WATCH_DOG);
			ul_rc = flash_erase_sector(LOG_ADDRESS);
			ul_rc = flash_unlock(LOG_ADDRESS ,LOG_ADDRESS + (( 10 * IFLASH_PAGE_SIZE ) - 1), 0, 0);
			wdt_restart(WATCH_DOG);
			ul_rc = flash_write(LOG_ADDRESS , Reset_Log_Buffer, IFLASH_PAGE_SIZE, 0);
		}
		else
			if ( Num_Data_Saved == 20 ) // LOG full, Restart LOG counter from 0
			{
				Num_Data_Saved = 1;
				Reset_Log_Buffer[0] = Num_Data_Saved;
				Reset_Log_Buffer[(Num_Data_Saved - 1) + 1] = Reset_Cause;
				Reset_Log_Buffer[(Num_Data_Saved - 1) + 2] = year >> 8;
				Reset_Log_Buffer[(Num_Data_Saved - 1) + 3] = year & 0xFF;
				Reset_Log_Buffer[(Num_Data_Saved - 1) + 4] = month;
				Reset_Log_Buffer[(Num_Data_Saved - 1) + 5] = day;
				Reset_Log_Buffer[(Num_Data_Saved - 1) + 6] = read_byte_eeprom(ADDR_EEP(hour_runnig[3]));
				Reset_Log_Buffer[(Num_Data_Saved - 1) + 7] = read_byte_eeprom(ADDR_EEP(hour_runnig[2]));
				Reset_Log_Buffer[(Num_Data_Saved - 1) + 8] = read_byte_eeprom(ADDR_EEP(hour_runnig[1]));
				Reset_Log_Buffer[(Num_Data_Saved - 1) + 9] = read_byte_eeprom(ADDR_EEP(hour_runnig[0]));
				wdt_restart(WATCH_DOG);
				ul_rc = flash_erase_sector(LOG_ADDRESS);
				ul_rc = flash_unlock(LOG_ADDRESS ,LOG_ADDRESS + (( 10 * IFLASH_PAGE_SIZE ) - 1), 0, 0);
				wdt_restart(WATCH_DOG);
				ul_rc = flash_write(LOG_ADDRESS , Reset_Log_Buffer, IFLASH_PAGE_SIZE, 0);
			}
}

void RTC_Handler(void)
{
	uint32_t ul_status = rtc_get_status(RTC);
	
	/* Second increment interrupt */
	if ((ul_status & RTC_SR_SEC) == RTC_SR_SEC) {
		/* Disable RTC interrupt */
		rtc_disable_interrupt(RTC, RTC_IDR_SECDIS);

		rtc_clear_status(RTC, RTC_SCCR_SECCLR);

		rtc_enable_interrupt(RTC, RTC_IER_SECEN);
	}
}

void WriteEepromDefault_KTS( void )
{
	unsigned int a, addr_end;
	
	addr_end = ADDR_EEP(check_eeprom[1]);
	for(a=0; a <= addr_end; a++) {
		//eep_write_byte(a,  vect_default_eeprom[a]);
		Shadow_eep[a] = vect_default_eeprom[a];	
	}
	
	Shadow_eep[ADDR_EEP(numMotors)] = eep_read_byte(ADDR_EEP(numMotors));
	Shadow_eep[ADDR_EEP(numPulseMotors)] = eep_read_byte(ADDR_EEP(numPulseMotors));
	Shadow_eep[ADDR_EEP(typeMotors)] = eep_read_byte(ADDR_EEP(typeMotors));
	Shadow_eep[ADDR_EEP(depotMotors)] = eep_read_byte(ADDR_EEP(depotMotors));
	Shadow_eep[ADDR_EEP(numNTC)] = eep_read_byte(ADDR_EEP(numNTC));
	Shadow_eep[ADDR_EEP(Posiz_NTC)] = eep_read_byte(ADDR_EEP(Posiz_NTC));
	Shadow_eep[ADDR_EEP(RotazioneBypass)] = eep_read_byte(ADDR_EEP(RotazioneBypass));
	
	Shadow_eep[ADDR_EEP(hour_runnig[0])] = eep_read_byte(ADDR_EEP(hour_runnig[0]));
	Shadow_eep[ADDR_EEP(hour_runnig[1])] = eep_read_byte(ADDR_EEP(hour_runnig[1]));
	Shadow_eep[ADDR_EEP(hour_runnig[2])] = eep_read_byte(ADDR_EEP(hour_runnig[2]));
	Shadow_eep[ADDR_EEP(hour_runnig[3])] = eep_read_byte(ADDR_EEP(hour_runnig[3]));
	
	Shadow_eep[ADDR_EEP(time_lastCloggedFilters[0])] = eep_read_byte(ADDR_EEP(time_lastCloggedFilters[0]));
	Shadow_eep[ADDR_EEP(time_lastCloggedFilters[1])] = eep_read_byte(ADDR_EEP(time_lastCloggedFilters[1]));
	Shadow_eep[ADDR_EEP(time_lastCloggedFilters[2])] = eep_read_byte(ADDR_EEP(time_lastCloggedFilters[2]));
	Shadow_eep[ADDR_EEP(time_lastCloggedFilters[3])] = eep_read_byte(ADDR_EEP(time_lastCloggedFilters[3]));
	
	for(a=0; a <18; a++) {
		Shadow_eep[ADDR_EEP(SerialString[a])] = eep_read_byte(ADDR_EEP(SerialString[a]));
	}
	
	addr_end = ADDR_EEP(check_eeprom[1]);
	for(a=0; a <= addr_end; a++) {
		eep_write_byte(a,  Shadow_eep[a]);
		//Shadow_eep[a] = vect_default_eeprom[a];
	}
}