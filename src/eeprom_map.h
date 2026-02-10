/*
 * eeprom_map.h
 *
 */ 


#ifndef EEPROM_MAP_H_
#define EEPROM_MAP_H_

#include "asf.h"

// indirizzo del chip eeprom.
//#define   DEVICE_24LCO2B    0x50   // 256 Byte fmax 100KHz

//LIB_24XX02_EEPROM  eep(DEVICE_24LCO2B);

//---- Byte che identifica la versione dell'EEPROM ----
#define CHECK_EEP        'E'
#define VERSION_EEP      '0'

//#define _PACK   __attribute__ ((aligned(1),packed))
#ifndef _PACK
	#define _PACK    __attribute__ ((aligned(1),packed))
#endif

//------------------------------------------------------------------------------------
// List AccessoyHW[0]:    DXD,   BPD,   AWP,   CWD,   EHD,   HWD,  PHWD,  PEHD  (LSB)     dalla 2.15 inserito accessorio DXD
// List AccessoyHW[1]:    DPS, PCAF,   PCAP,   INP,   OUT, DPPV2,   RFM,  MBUS  (LSB)     dove PCAP = Probe Costant Air Flow
// List AccessoyHW[2]:  P2CO2, P1CO2,  EBPD,  P2RH,  P1RH,   SSR, P1VOC,  EBP2, (LSB)
// List AccessoyHW[3]:  -----, -----,  FLW2,  FLW1,  EXT4,  EXT3,  EXT2,  EXT1, (LSB)
//------------------------------------------------------------------------------------
enum _NUMBER_ACCESSORY_EEP {
	//     0              1              2              3             4               5              6              7          // bit
	ACC_EEP_PEHD,  ACC_EEP_PHWD,  ACC_EEP_HWD,   ACC_EEP_EHD,   ACC_EEP_CWD,   ACC_EEP_AWP,   ACC_EEP_BPD,   ACC_EEP_DXD,    // Byte 1
	ACC_EEP_MBUS,  ACC_EEP_RFM,   ACC_EEP_DPPV2, ACC_EEP_OUT,   ACC_EEP_INP,   ACC_EEP_PCAP,  ACC_EEP_PCAF,  ACC_EEP_DPP,    // Byte 2
	ACC_EEP_EBP2, ACC_EEP_P1VOC, ACC_EEP_SSR,   ACC_EEP_P1RH,  ACC_EEP_P2RH,  ACC_EEP_EBPD,  ACC_EEP_P1CO2, ACC_EEP_P2CO2,  // Byte 3
	ACC_EEP_EXT1,  ACC_EEP_EXT2,  ACC_EEP_EXT3,  ACC_EEP_EXT4,  ACC_EEP_FLW1,  ACC_EEP_FLW2,  ACC_EEP_DSC, ACC_EEP_NP_31,  // Byte 4
};

//----------------------------------------------------------------
// Tipi di Configurazioni Bypass Damper
//----------------------------------------------------------------
enum _SEL_BYPASS {
	BPD_AUTOMATIC,
	BPD_EXT_CTRL,
	BPD_MANUAL_CLOSE,
	BPD_MANUAL_OPEN,
	BPD_AUTO_ON_OFF
};

//----------------------------------------------------------------
// Tipi di Configurazioni input
//----------------------------------------------------------------
enum _CFG_INPUT {
	INP_DISABLE,                  // 0= Disable
	INP_0V_UNIT_STOP,             // 1= 12V->Unit RUN , 0V->Unit STOP
	INP_12V_UNIT_STOP,            // 2= 0V->Unit RUN , 12V->Unit STOP
	INP_REG_AIR_FLOW,             // 3= 0-10V Air flow regulation
	INP_12V_BYPASS_OPEN,          // 4= 12V->Bypass Open ,  0V->Bypass Closed
	INP_0V_BYPASS_OPEN,           // 5=  0V->Bypass Open , 12V->Bypass Closed
	INP_12V_CLIMA_OFF,            // 6= 12V->spegne tutti gli accessori clima
	INP_SUMMER_WINTER,            // 7= 0V -> SUMMER, 12V -> WINTER
	INP_WINTER_SUMMER,            // 8= 0V -> WINTER, 12V -> SUMMER
	INP_PIR,					  // 9= PIR Strategy
	INP_CFG_10_NONE,              //10= * * *
	INP_12V_ALM_FIRE_UNIT_STOP,   //11= 12V-> Unit STOP + messaggio: 'ALARM Incendio Ventilazione ferma'
	INP_12V_ALM_FIRE_VENT_MAX,    //12= 12V-> Ventilazione MAX + messaggio:'ALARM Incendio Ventilazione al massimo'
	INP_12V_ALM_FIRE_ONLY_EXTRACT,//13= 12V-> Vent. Estrazione al Massimo, Immisione spenta + messaggio: 'ALARM Incendio Solo estrazione aria'
	INP_12V_ALM_FIRE_ONLY_SUPPLY, //14= 12V-> Vent. Immisione al Massimo, Estrazione spenta + messaggio: 'ALARM Incendio Solo estrazione aria'
};

// -------------- bit: Enab_Fuction ----------------
//  (LOW)  bit[7]:CAF  | bit[6]:CAP  | bit[5]:CSF    | bit[4]:ImbalanON | bit[3]:STPL            | bit[2]:WeeklyON        | bit[1]:BoostON   | bit[0]:DeFrostON.
//  (HIGH) bit[15]:--- | bit[14]:MBF | bit[13]:DPP   | bit[12]:PreHeat  | bit[11]:COOLING(Summer)| bit[10]:HEATING(winter)| bit[9]:CtrlFilter| bit[8]:Password
enum _BIT_NUMBER_ENAB_FUCTIONS {
	// bit0, ... , bit7
	ENAB_DEFROST, ENAB_BOOST, ENAB_WEEKLY, ENAB_STEPLESS,  ENAB_IMBALANCE, ENAB_CSF, ENAB_CAP,  ENAB_CAF,
	//bit8, ... , bit15
	ENAB_PASSWORD, ENAB_CTRL_FILTER, ENAB_WINTER,  ENAB_SUMMER,  ENAB_PREHEATING, ENAB_DPP, ENAB_MBF, ENAB_CLEAN_EVENT,
	// ENAB_PASSWORD, ENAB_CTRL_FILTER, ENAB_HEATING, ENAB_COOLING, ENAB_PREHEATING, ENAB_DPP, ENAB_MBF, ENAB_CLEAN_EVENT,

};


// -------------- Struct Weekly Day ----------------
typedef struct SDAYPROG_TAG
{
	Byte numRange;        //  1 Byte: numbers of ranges (from 0= disable to 4)
	Byte timeON[4];       //  4 Byte: timeON, Values from 0 to 48 (step di 30')
	Byte timeOFF[4];      //  4 Byte: timeOFF, Values from 0 to 48 (step di 30')
	// per ogni range si posso configurare finoi a : 4 step di velocita', 4 rif. di temp., 4 stati di imbalance
	char ConfigSpeed;     //  1 Byte: bit[7,6]:Step Speed range 4 | bit[5,4]:Step Speed range 3 | bit[3,2]:Step Speed range 2 | bit[1,0]:Step Speed range 1.
	char ConfigImbal;     //  1 Byte: bit[7,6]:Set Imbal. range 4 | bit[5,4]:Set Imbal. range 3 | bit[3,2]:Set Imbal. range 2 | bit[1,0]:Set Imbal. range 1.
	char ConfigTemp;      //  1 Byte: bit[7,6]:Rif. Temp. range 4 | bit[5,4]:Rif. Temp. range 3 | bit[3,2]:Rif. Temp. range 2 | bit[1,0]:Rif. Temp.range 1.
}_PACK SDAYPROG;          // Totale 12 Byte for day


//------------------------------------------------------------------------
// Definizione della struttare dei dati in RAM e relativi indirizzi
//------------------------------------------------------------------------
typedef struct EEPROM_DATA_TAG {
	//----------  Information (59 Byte) ------------------
	/*000*/ Byte  AddrUnit;               //  1 Byte:  da 1 a 32, lo 0 Ã¨ utilizzato per i comandi di broadcast.
	/*001*/ Byte  Type_func;              //  1 Byte:  configurazione funzionalita' UnitÃ :  0= BASIC, 1= EXTRA
	/*002*/ Byte  HW_Vers[4];             //  4 Byte:  Ascii: versione hardware MB     Es: " 4.0"
	/*006*/ Byte  SW_Vers[5];             //  5 Byte:  Ascii: versione software MB     Es: "2.01" oppure "2.10"
	/*011*/ Byte  SerialString[18];       // 18 Byte:  Ascii:  "OrVe SIZE CFG DATA PRG",
	//              es:  "1565 0023 115 1351 002"
	//          SIZE:  0023, 0043, 0053,... 0423
	//          CFG:   [8]: '0'= OSC, '1'= SSC, '2'= EOS, '3'= FOS, .. | [9]: '0'=none, '1'='A', '2'='B',  ... | [10]: '0'= none, '1', '2', ... , '9'
	//          DATA:  [11,12]: '13' = Anno | [13,14]: '51' = Settimana
	//          PROG:  Progress. Number
	Byte  SW_Vers_ModBus[2];      //  2 Byte: Number: Versione Modbus.   Es:  1.01
	Byte  SignTEST[2];            //  2 Byte: Ascii codice operato che ha eseguito il test
	Byte  CodeErrTest;            //  1 Byte:   se = 0  esito positivo, altrimenti viene inserito un codic di errore.
	
	/*034*/ Byte  hour_runnig[4];         //  4 Byte:  ogni 8 ore memorizza in eeprom l'avanzamento del timer.
	/*038*/ Byte  time_lastCloggedFilters[4]; //  4 Byte:  tempo in ore dell'ultima pulizia filtri.
	
	/*042*/ Byte  AccessoyHW[4];          //  4 Byte:  parte 1 list accessory HW   (MSB)    DXD,   BPD,   AWP,   CWD,   EHD,   HWD,  PHWD,  PEHD  (LSB)
	//           parte 2 list accessory HW   (MSB)    DPS,  PCAF,  PCAP,   INP,   OUT, DDPV2,   RFM,  MBUS  (LSB)
	//           parte 3 list accessory HW   (MSB)  P2CO2, P1CO2,  EBPD,  P2RH,  P1RH,   SSR, P1VOC,  EBP2  (LSB)
	//           parte 4 list accessory HW   (MSB)  -----, -----, -----, -----,  EXT4,  EXT3,  EXT2,  EXT1  (LSB)
	// Leggenda Acronimi:  DPS=Controllo Filtri con Press., P1CO2=Probe 1 CO2,  PCAP= Probe Constant Air Pressure
	
	/*046*/ unsigned short  Enab_Fuction; //  2 Byte:  (LOW) bit[7]:CAF  | bit[6]:CAP  | bit[5]:CSF     | bit[4]:ImbalanON  | bit[3]:STPL       | bit[2]:WeeklyON   | bit[1]:BoostON    | bit[0]:DeFrostON.
	//           (HIGH)bit[15]:--- | bit[14]:MBF | bit[13]:DPP    | bit[12]:PREHEATING| bit[11]:EN_SUMMER | bit[10]:EN_WINTER | bit[9]:CtrlFilter | bit[8]:PASSWORD
	unsigned short  msk_Enab_Fuction; // i bit=0, disabilitano la possibilità di attivare la funzione
	
	/*050*/ Byte  Dsc_Sdcard_Update_Delay;	// 1 Byte : 3 to 60
	
	Byte  Pir_Update_Delay;		// 1 Byte : 3 to 30
	
	unsigned short  calibra_cap1;     //  2 Byte:
	
	Byte  Clean_Event_Delay;		// 1 Byte : 30, 60 , 90
	
	Byte KTS_Watchdog;				// 1 Byte 0 or 1 ( default 0 )
	
	//Byte  size1_free[2];          //  2 Byte:  liberi per ampliamenti futuri
	short SetPointTemp_Heater;      //  2 Byte:  Set-Point Temperature 1 Heater value 16.0 Â°C a 32.0 Â°C.
	
	Byte  cntUpdate_info;         //  1 Byte:  contatore degli aggiornamenti dell'eeprom
	
	//--- configurazioni HW di fabbrica non modificabili (17 Byte) ---
	Byte  numMotors;             //  1 Byte:  valori: 2, 4, 6.
	Byte  numPulseMotors;        //  1 Byte:  numero di pulse per giro. 1, 2, 3, 4,..., 8
	Byte  typeMotors;            //  1 Byte:  'F' = Forward , 'B' = Backward
	Byte  chWireless;            //  1 Byte:  numero canale wireless da 1 a 16 (era  progMotors = 'S'  non usato)
	Byte  depotMotors;           //  1 Byte:  depotenziamento dei motori da 100% a 40%
	Byte  numNTC;                //  1 Byte:  numero sonde da 2 a 4.
	Byte  Posiz_NTC;             //  1 Byte:  bit 7,6 = Exuast, bit 5,4 = Supply, bit 3,2 = Return, bit 1,0 = Fresh
	Byte  RotazioneBypass;       //  1 Byte:  0xFF = bypass chiuso in senso AntiOrario(default), 0= bypass chiuso in senso Orario
	
	//Byte  size2_free[9];         // 9 Byte:  liberi per ampliamenti futuri
	unsigned short Set_StepMotors_CAP1[4];		//  8 byte: 4 Soglie di velocita'Â  dei motori in modalita'Â : CAP  (Range da 30Pa a 250/350/450Pa), ..[3]= per funz. in Stepless
	Byte  size2_free[1];						//  9 byte:  liberi per ampliamenti futuri
	
	//---------  Setting/Config. Param.(51 Byte) ---------------
	/*075*/ Byte  Set_Power_ON;           //  1 Byte:  1= SET UNIT IDLE (power_on), 0 = SET UNIT STANDBY (power_off)
	Byte  Config_Bypass;          //  1 Byte:  0= Automatic,  1= Ext Input 1,  2= Ext Input 2,  3= Manual CLOSE,  4= Manual OPEN
	Byte  Set_Input[2];           //  2 Byte:  0= Disable
	//           1= 10V->Unit RUN , 0V->Unit STOP
	//           2=  0V->Unit RUN ,10V->Unit STOP
	//           3= 0-10V Air flow regulation
	//           4= 10V->Bypass Open ,  0V->Bypass Closed
	//           5=  0V->Bypass Open , 10V->Bypass Closed
	//           6= PIR
	Byte  Set_Output[2];          //   0= Disable                   (il rele rimane non alimentato)
	//   1= Bypass Status Open        (il rele passa in attrazione se: Bypass Open)
	//   2= Common Fault Status       (il rele passa in attrazione se: Unità in allarme)
	//   3= Unit is Run (ex: SDD)     (il rele passa in attrazione se: Unita è in funz.)
	// 128= Disable                   (il rele rimane in attrazione)
	// 129= Bypass Status Open        (il rele passa in Diseccitaz. se: Bypass Open)
	// 130= Common Fault Status       (il rele passa in Diseccitaz. se: Unità in allarme)
	// 131= Unit is Run (ex: SDD)     (il rele passa in Diseccitaz. se: Unita è in funz.)
	Byte  sel_idxStepMotors;      //  1 Byte:   Selettore dell'indice degli array Set_StepMotors*
	//            bit 7: puntatore array 0= CFS_CAF, 1= CAP[4]:
	//            bit 3,2,1,0: index array
	unsigned short  Set_StepMotorsCFS_CAF[4]; //  8 Byte: 4 Soglie di velocita'  dei motori in modalita' : CFS/CAF (Range: da 20.0% a 100.0%),   ..[3]= per funz. in Stepless
	unsigned short  Set_StepMotors_CAP[4];    //  8 Byte: 4 Soglie di velocita'  dei motori in modalita' : CAP  (Range da 30Pa a 250/350/450Pa), ..[3]= per funz. in Stepless
	signed char  Set_Imbalance[2];       //  2 Byte: 2 Set di Imbalance,  values max: +/-70 %.
	Byte  Set_TimeBoost;          //  1 Byte: valore espresso in minuti(min:15, max:240) se abilitato va al massimo della velocita'  per il tempo indicato.
	unsigned short  SetPoint_CO2; //  2 Byte: (da 70 a 150)*10 = da 700 a 1500 PPM
	Byte  SetPoint_RH;            //  1 Byte: da 20% a 99%
	unsigned short  SetPoint_VOC;    //  2 Byte: SetPoint_VOC, da 1 a 100 PPM
	unsigned short  gg_manut_Filter; //  2 Byte: gg funz., trascorso il quale occorre fare manutenzione ai Filter
	Byte  service_password[5];       //  5 Byte: password, 5 caratteri in Ascii numerici da'0' a '9'.
	Byte  user_password[5];          //  5 Byte: password, 5 caratteri in Ascii numerici da'0' a '9'.
	unsigned short  calibra_cap;     //  2 Byte:
	Byte manual_reset;               //  1 Byte: 1 = riarmo manuale inserito (incendio) / 0 = riarmo manuale ripristinato
	Byte DPP_Calibrationvalue;       //  1 Byte: valore % di calibrazione, se vale 0 nessuna calibrazione
	Byte  Set_MBF_fresh;             //  1 Byte: set modbus fan fresh  (Range: da 0.0% a 100.0%) // il modbus controlla in modo indipendente i motori
	Byte  Set_MBF_return;            //  1 Byte: set modbus fan return (Range: da 0.0% a 100.0%) // il modbus controlla in modo indipendente i motori
	//Byte  size3_free[1];             //  1 Byte: liberi per ampliamenti futuri
	Byte  SetPoint_Airflow_CO2;		//  1 Byte: da 40% a 100%
	
	
	Byte  cntUpdate_SettingPar;   //  1 Byte:
	
	//---------- Calibrazione Sonde ( 6 Byte) -----------------
	Byte  Calibr[6];              //  6 Byte: 4 sonde temp + 2 Qual_Air
	
	//--------- Soglie Temperature (20 Byte)----------------
	short Bypass_minTempExt;                    //  2 Byte:  valore espresso in 15.0 Â°C
	short SetPointTemp[2];                      //  4 Byte:  Set-Point Temperature 1&2 value 16.0 Â°C a 32.0 Â°C.
	char  idxSetPointT;                         //  1 Byte:  ..., bit[0]: 1=Rif.Temp2, 0=Rif.Temp1
	signed char  hister_AWP_Temp_Hot[2];        //  2 Byte:  isteresi temperature per attivare HWD/EHD (0= OFF, 1= ON), con AWP presente.
	signed char  hister_AWP_Temp_Cold[2];       //  2 Byte:  isteresi temperature 1&2 per attivare CWD (0= OFF, 1= ON), con AWP presente.
	signed char  hister_Temp_Hot[2];            //  2 Byte:  isteresi temperature per attivare HWD/EHD (0= OFF, 1= ON), loop aperto
	signed char  hister_Temp_Cold[2];           //  2 Byte:  isteresi temperature 1&2 per attivare CWD (0= OFF, 1= ON), loop aperto
	Byte  Ref_T_setting;                        //  1 Byte:  ...bit[6]: 1=START test IPEHD 0=STOP test IPEHD, bit[5,4,3,2]: speed_regulator , bit[1]: 1= TempAmb=TR / TempAmb=TS, bit[0]: *
	Byte  DeltaTemp_Supply;                     //  1 Byte:  isteresi da applicare alla temperatura in uscita al compressore se il controllo è sulla sonda supply
	Byte  Set_EHD_mod;                          //  1 Byte: set EHD modulation level (Range: da 0.0% a 100.0%) // il modbus controlla in modo indipendente un EHD modulante
	Byte  Set_BPD_mod;                          //  1 Byte: set BPD modulation level (Range: da 0.0% a 100.0%) // il modbus controlla in modo indipendente un BPD modulante
	
	//Byte  size4_free[0];          //  0 Byte:  liberi per ampliamenti futuri
	
	Byte  cntUpdate_SetTemp;      //  1 Byte:
	
	//--------- weekly program (85 Byte) -------------------
	SDAYPROG sDayProg[7];         // 84 Byte:  12 Byte * 7 days
	
	Byte  cntUpdate_dayProg;      //  1 Byte:
	
	//------------------------  (2 Byte) -------------------
	Byte  version_eeprom;         //  1 Byte: '1'
	Byte  check_eeprom[2];        //  1 Byte:  contatore aggiornamenti della eeprom
}_PACK S_EEPROM;   // totale  Byte: 240


//------------------------------------------------------------------------
// definizione puntatore alla struttura eeprom.
//------------------------------------------------------------------------
#define ADDR_EEP(x)	(unsigned int)(&((S_EEPROM *)0x0000)->x)

//------------------------- proto global ---------------------------------
extern Byte read_byte_eeprom(unsigned short addr_eep);
extern unsigned short read_word_eeprom(unsigned short addr_eep);
extern unsigned long int read_Dword_eeprom(unsigned short addr_eep);

extern void write_byte_eeprom(unsigned short addr_eep, Byte val);
extern void write_word_eeprom(unsigned short addr_eep, unsigned short val_16);
extern void write_Dword_eeprom(unsigned short addr_eep, unsigned long val_32);

extern void Erase_EEprom ( void );

extern void check_control_speed_func();
extern void check_control_speed_func1();

//--------------------- array eeprom virtuale ----------------------------
extern  Byte Shadow_eep[256];

//Versione el FW.
extern const char VERSION_FW[5];

extern const int  pAnAssorb_ByPass;

extern const Byte vect_default_eeprom[];

//motore 052    // motore 046 / 057 in coclea
extern const float KF1_1; // 0.0000200253;
extern const float KF1_2; //-0.0114827694;
extern const float KF1_3; // 3.6594526211;
extern const float KF1_4; // 0.3049370363;

extern const int  pAnInp1;
extern const int  pAnAssorb_KTS;
extern const int  pAnInp2;

#endif /* EEPROM_MAP_H_ */