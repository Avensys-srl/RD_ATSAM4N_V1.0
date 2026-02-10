/*
 * data.h
 *
 *  Data: dic.2013
 *
 *  Descrizione: Definizione struttura dati che girano in ram.
 *
 *  Autore: Maurizio Colmone
 */ 


#ifndef DATA_H_
#define DATA_H_

#include <compiler.h>

// allinea tutto ad 1 Byte
//#define _PACK   __attribute__ ((aligned(1),packed))
#ifndef _PACK
	#define _PACK    __attribute__ ((aligned(1),packed))
#endif

//--------------- Maschere: status_unit -------------------
#define MSK_UNIT_OPERATING           0x003F   // Maschera bit[5,...,0], b000000 = equivale a  UNITA' IN STANDBY
#define POS_BIT_UNIT_RUN                  0   // bit[0]: se =1 l'unita' sta facendo girare i motori.
#define POS_BIT_DEFROST_OPERATING         1   // bit[1]: FUNZIONAMENTO IN DEFROST
#define POS_BIT_POST_VENT_OPERATING       2   // bit[2]: FUNZIONAMENTO POST VENTILAZIONE
#define POS_BIT_IMBALANCE_OPERATING       3   // bit[3]: FUNZIONAMENTO CON  I 2 RAMI SBILANCIATI
#define POS_BIT_BOOST_OPERATING           4   // bit[4]: FUNZIONAMENTO IN BOOST ATTIVO
#define POS_BIT_NONE_OPERATING            5   // bit[5]: FUNZIONAMENTO * none *
#define MSK_STS_BYPASS               0x00C0   // Maschera bit[7,6]
#define POS_BIT_BYPASS_RUN                6   // bit[6]: STATO BYPASS IN APERTURA/CHIUSURA
#define POS_BIT_BYPASS_CLOSE              7   // bit[7]: STATO BYPASS CHIUSO
#define POS_BIT_CMD_FAN_INPUT             8   // bit[8]: Sta Comandando la velocit‡ dei motori, in funz. della lettura degli inputs
#define POS_BIT_MAX_RH                    9
#define POS_BIT_MAX_CO2                  10
#define POS_BIT_MAX_VOC                  11
#define POS_BIT_IN_TESTING               12
#define POS_BIT_DPP_CHECK                13  //  bit[13]: Sta Comandando la velocit‡  dei motori, in funz. della lettura della taratura o verifica DPP
#define POS_BIT_CLEANEVENT_CHECK         14  //  bit[14 : Clean Event Activated
#define POS_BIT_PIR_CHECK                15  //  bit[15]: PIR level 1 detect

// dal bit 14 al bit 15  * LIBERI *

//------------- Indici Accessori con uscita digitale (bus I2C) -------------
enum _INDEX_DIGITAL_ACCESSOTY {
	ACC_I2C_HEATER,
	ACC_I2C_COOLER,
	ACC_I2C_PREHEATER,
	ACC_I2C_CO2_1,
	ACC_I2C_CO2_2,
	ACC_I2C_RH_1,
	ACC_I2C_RH_2,
	ACC_I2C_VOC,
	ACC_I2C_AWP,
	ACC_I2C_PCAP,
	ACC_I2C_PCAF,
	ACC_I2C_DPP,
	ACC_I2C_DXD,                 // dalla 2.16 Accessorio Clima CALDO/FREDDO con compressore
	ACC_I2C_EXT1,				   // dalla 2.24 ZH v1
	ACC_I2C_EXT2,
	ACC_I2C_EXT3,
	ACC_I2C_EXT4,
	ACC_I2C_FLW1,				   // dalla 2.25 ZH v2
	ACC_I2C_FLW2,
	ACC_I2C_EBPD,
	ACC_I2C_SSR,
	ACC_I2C_EBP2,
	ACC_I2C_DSC,
	TOT_ACCESSORY_I2C            // totale 17 accessori.
};

//------------------ Definizione Test attivi ------------------------
#define MSK_TEST_ENABLE    0x0001   // bit 0
#define MSK_TEST_BYPASS    0x0002   // bit 1
#define MSK_TEST_OUTPUT    0x0004   // bit 2
#define MSK_TEST_BUZZER    0x0008   // bit 3
#define MSK_TEST_IPEHD     0x0010   // bit 4
#define MSK_TEST_LEDS      0x0020   // bit 5


//---------------------Maschere bit: status_weekly ------------------------------
#define MSK_BIT_WEEKLY_ENAB 0x01   // bit[0]
#define MSK_BIT_WEEKLY_RUN  0x02   // bit[1]
#define POS_BIT_RIF_TEMP       2   // bit[3,2]:1,1=---/ 1,0=---    / 0,1=RIF_TEMP2 / 0,0=RIF_TEMP1
#define POS_BIT_SET_IMBAL      4   // bit[5,4]:1,1=---/ 1,0=---    / 0,1=IMBAL_1_ON/ 0,0=IMBAL_OFF
#define POS_STEP_SPEED         6   // bit[7,6]:1,1=---/ 1,0=SPEED_3/ 0,1=SPEED_2   / 0,0=SPEED_1

//--------------------- index: Probes Ambient. -----------------------------
enum _INDEX_PROBES_TEMPERATURE {
	I_PROBE_FRESH,
	I_PROBE_RETURN,
	I_PROBE_SUPPLY,
	I_PROBE_EXHAUST
};

//----------------- Type Probes Quality Ambient.(status_SensQAir) ---------------------
#define  PROBES_RH    0
#define  PROBES_CO2   1
#define  PROBES_VOC   2

#define DEC 10
#define HEX 16
#define OCT 8
#define BIN 2
#define BYTE 0

//------------------------------------------------------------------------
// Definizione della timer
//------------------------------------------------------------------------
typedef struct S_CLOCK_TAG{
	Byte  seconds;
	Byte  minutes;
	Byte  hours;
	Byte  weekday;
	Byte  Day;
	Byte  Mounth;
	Byte  Year;
	Byte  Century;
}_PACK S_CLOCK_WEEK;

//------------------------------------------------------------------------
// Definizione dei Processi da applicare tramite I2C verso scheda
// accessorio DXD es: modificare il valore di comando 0_10V
//------------------------------------------------------------------------
#define CHANGE_FUNC_COMPRESS    0x01
#define CHANGE_VAL_0_10V        0x02


//------------------------------------------------------------------------
//------------------------- Accessorio DXD -------------------------------
//------------------------------------------------------------------------
typedef struct S_REGULATE_DXD_TAG {
	Byte processor_I2C;      // Gestisce le priorit‡ di processi in scrittura: CHANGE_FUNC_COMPRESS, SWITCH_OFF_COMPRESSOR,...
	Byte val_0_10V;          //
	Byte countdown_aligns_val;
	Byte func_compressor;    // 'W' winter / 'S' summer
	Byte defrost_on;         // Se l'accessorio sta facendo il defrost, dobbiamo anche noi andare in defrost con lo sbilanciamento dei rami.
	Byte cnt_link_err_write; // contatore del numero di volte di mancata comunicazione ai 2 comandi di scrittura
	int  count;              // Contatore per la velocit‡ del regolatore
}_PACK S_REGULATE_DXD;

//------------------------------------------------------------------------
// Definizione della struttura Dati degli Accessori
//------------------------------------------------------------------------
typedef struct S_DIG_ACCESSORY_TAG{
	/*000*/  Byte  comand;              // ..., bit[1], 1= DISABLE_SLAVE, bit[0]=(1=Switch_ON /0=Switch_OFF)
	/*001*/  Byte  sts;                 // Stato in cui si trova l'accessorio: bit[3]:1=ELECT/0=ACQUA, bit[2]:1=ACCESO/0=SPENTO, bit[1]:1=OPEARATIVO, bit[0]:1=Collegato
	/*002*/  Byte  numCmdDisable;       // numero di comandi di disabilita, dopo 3 tenetativi che l'accessorio non risponde cancellalo comunque
	/*003*/  short measure1;            // misura 1: per le temp. il valore Ë per 10
	/*005*/  short measure2;            // misura 2: per le temp. il valore Ë per 10
	/*007*/  short measure3;            // misura 3: per le temp. il valore Ë per 10
	/*009*/  short measure4;            // misura 4: per le temp. il valore Ë per 10
	/*011*/  short measure5;            // misura 4: per le temp. il valore Ë per 10
	/*013*/  Byte  data;                // data: Per gli accessori HWD, EHD, PEHD, Ë la taglia, per le sonde viene passato l'info. se sono da canale 'd', oppure d'ambiente 'a'..
	/*014*/  Byte  info;                // vengono passate delle Informazioni, es: sonde collegate
	/*015*/  Byte  level_link;          // bonta' della comunicazione (valore espresso in 10/10)
	/*016*/  Byte  cnt_link_ok;         // Contatore messaggi buoni
	/*017*/  Byte  cnt_link_ko;         // Contatore messaggi falliti
	/*018*/  Byte  cnt_persist_link_ko; // contatore di persistenza di collegamenti Falliti
	/*019*/  Byte  status_comand;       // lo stato dell'ultimo comando inviato
}_PACK S_DIG_ACCESSORY;  // tot: 18 Byte

//------------------ Definizione stati accesorio ------------------------
#define STS_ACC_CONNECT     0x01  // bit.0: l'accesorio e' stato rilevato dall'unita'† almeno 1 volta
#define STS_ACC_OPERATIVE   0x02  // bit.1: l'accessorio e' operativo (no link KO)
#define STS_ACC_ON          0x04  // bit.2: l'accessorio e' acceso

#define STS_ACC_ELECT       0x08  // bit.3: l'accessorio e' tipo Elettrico/ acqua
#define STS_CMD_ACC_ON      0x10  // bit.4: l'accessorio ha ricevuto il seguente comando dall'Unit‡ ON/OFF

#define STS_FUNC_COMPRESS   0x08  // bit.3: Tipo di funzionamento compresore (Riscaldamento/Raffreddamento) ACCESSORIO DXD
#define STS_DEFROST_ON      0x10  // bit.4: Ci dice che il modulo DXD sta andando in defrost.


//------------------ Definizione Camandi accesorio ------------------------
#define CMD_ON              0x01  // bit.1: Byte command
#define DISABLE_SLAVE       0x02
#define RESTART_SLAVE		0x04

#define WRITE_CMD           0x80  // se attivo il bit[7], devo iniviare un comando di scrittura allo slave


//------------------------------------------------------------------------
// Definizione della struttarA dei dati in RAM e relativi indirizzi
//------------------------------------------------------------------------
typedef struct SDATA_TAG {
	// informazioni
	Byte KTS_Connect;
	Byte level_link_CTRLFAN;
	unsigned short size_unit;                     // 2= 200mq/h  12= 1200mq/h
	
	Byte level_link_ModBus;
	Byte Status_RemoteCTRL;                      //    b7. 1= Modbus Active, b6 = RFM Install
	
	S_DIG_ACCESSORY   AccI2C[TOT_ACCESSORY_I2C]; // dati degli Accessory collegati tramite I2C
	S_REGULATE_DXD    RegDXD;
	Byte info_ProbesAD;                     // 7.P2CO2, 6.P1CO2, 5.PICO2, 4.P2RH, 3.P1RH, 2.PIRH, 1.P1VOC, 0.PIVOC   (bit: 0 = Ambiente, 1 = Duct)
	
	// setting par.
	Byte speed_motors_R;                   // comando ai motori Returns, 0 -> 255 , il dato si pu√≤ discostare dal valore in eeprom se vi √® in corso un Defrost, oppure una Co2 Alarm,...
	Byte speed_motors_F;                   // comando ai motori Returns, 0 -> 255 , il dato si pu√≤ discostare dal valore in eeprom se vi √® in corso un Defrost, oppure una Co2 Alarm,...
	
	Byte increase_speed_RH_CO2;            // valore da 0% a 100% di incremento della velocit‡ rispetto al set-poin a causa: RH alta, o CO2 alta, o VOCs alta.
	
	Byte mem_sel_idxStepMotors;            // questi 3 var. servono per far cambiare stato alla eeprom dopo 12 sec., memorizzando
	unsigned short mem_Set_StepMotors;
	unsigned short mem_Enab_FuncStepMotors;
	
	// misure 2 Byte
	short feedbk_v_speed_motors_R;           // tensione letta sul comando dei motori ramo RETURN (0.00V a 11.00V) il valore √® moltipl. per 100, quindi 1100 = 11.00V
	short feedbk_v_speed_motors_F;           // tensione letta sul comando dei motori ramo FRESH  (0.00V a 11.00V) il valore √® moltipl. per 100, quindi 1100 = 11.00V
	short measure_CO2_max;                   // valore massimo di Co2 tra i vari sensori: espresso in PPM ( da 0 a 2000 PPM)
	short measure_RH_max;                    // valore massimo di Umidit√† tra i vari sensori: da 0% a 100%
	short measure_VOC_max;                   // valore massimo di VOC tra i vari sensori: da 0  a 10000 ug/M3
	short measure_AWP_max;                   // valore massimo di Temperatura tra i vari sensori AWP
	short measure_pressure;                  // valore espresso in Pascal da 0 a 1000Pa
	short measure_airflow;                   // portata  m3/h
	short measure_RH_Analogic;               // Misura della sonda RH interna all'Unit√
	short measure_VOC_Analogic;              // Misura della sonda VOC interna all'Unit√
	short measure_CO2_Analogic;              // Misura della sonda CO2 interna all'Unit√
	short measure_Temp[4];                   // 0=T_Fresh, 1=T_Return, 2=T_Supply, 3=T_Exhaust  (i valori temp. sono espressi per 10, es: 102 = 10.2 ∞C)
	short measure_ADC_input[2];              // i valori sono grezzi da 0 a 1023
	short measure_I_KTS;                     // valori espresi in mA: FS=1024 = 250mA
	short rpm_motors_R[3];                   // 0 = Motore R1, 1= Motore R2,...
	short rpm_motors_F[3];                   // 0 = Motore F1, 1= Motore F2,...
	
	short absorption_bypass_min;             // Assorbimento minimo mA del motore Bypass
	short absorption_bypass_max;             // Assorbimento massimo mA del motore Bypass
	
	// stati
	unsigned short status_unit;            // 2 Byte:  bit[15,14]
	//          bit[13]: Calibrazione/verifica in corso
	//          bit[12]: Test ON
	//          bit[11]: 1= VOC_MAX, bit[10]: 1= CO2_MAX, bit[9]: 1= RH_MAX
	//          bit[8]: 1= CMD_FAN_INPUT
	//          bit[7,6]: b10=BypassClose, b01=BypassRun, b00=BypassOpen
	//          bit[5,...,0]: b10000= * none *,  b010000=BOOST ON, b001000=IMBALANCE ON, b000100=POST_VENT ON, b000010=DeFROST ON, b00001=RUN FANS, b000000=STANDBY
	Byte status_weekly;                    // 1 Byte:  bit[7,6]:1,0=SPEED3/ 0,1=SPEED2/ 0,0=SPEED1 | bit[5,4]: 0,1=IMBAL_1_ON/ 0,0=IMBAL_OFF | bit[3,2]:0,1=RIF_TEMP2/ 0,0=RIF_TEMP1 | bit[1]: WEEKLY_RUN, bit[0]:WEEKLY_ENABLE
	Byte status_Leds_ctrlFan;              // 1 Byte:   [1,0] LED BYPASS    (11=none, 10=ON, 01=Toggle, 00=OFF)
	//           [3,2] LED ALARM     (11=none, 10=ON, 01=Toggle, 00=OFF)
	//           [5,4] LED MOTORS R  (11=none, 10=ON, 01=Toggle, 00=OFF)
	//           [7,6] LED MOTORS F  (11=none, 10=ON, 01=Toggle, 00=OFF)
	Byte StatusMotorsR;                    // 1 Byte:   bit:  7.none | 6.AlarmM3 | 5.AlarmM2 | 4.AlarmM1 | 3.none | 2.PowerON_M3 | 1.PowerON_M2 | 0.PowerON_M1
	Byte StatusMotorsF;                    // 1 Byte:   bit:  7.none | 6.AlarmM3 | 5.AlarmM2 | 4.AlarmM1 | 3.none | 2.PowerON_M3 | 1.PowerON_M2 | 0.PowerON_M1

	Byte StatusOutput;                     // 1 Byte:   bit:  7..  ..|  1.Out1  | 0.Out0
	
	unsigned short status_test;            // 2 Byte:   bit[15,..., ]: * none *
	//           bit[4]: 1 = TEST IPHED
	//           bit[3]: 1 = TEST BUZZER
	//           bit[2]: 1 = TEST OUTPUT ON
	//           bit[1]: 1 = TEST BYPASS ON
	//           bit[0]: 1 = TEST ON  / 0 = TEST OFF
	unsigned short timeout_test;
	Byte data_test;                        // 1 Byte:  per memorizzare dei dati per i test
	
	// Allarmi/ Eventi
	Byte Events[14];                       // 14 Byte:  vedi tabella ALARM.H in testa al file, 'Definizione Byte eventi'
	
}_PACK SDATA;

/*enum Base {
	FLOAT = 0,
	FLOAT1,
	FLOAT2,
	FLOAT3,
	FLOAT4,
	DEC,
	HEX,
	BYTE
	};*/
	
extern Byte DigitAccessoryOn(int nAcc_I2C);
extern Byte DigitAccessoryOperating(int nAcc_I2C);

#endif /* DATA_H_ */