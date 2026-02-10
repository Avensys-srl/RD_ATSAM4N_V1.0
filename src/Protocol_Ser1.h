/*
 * Protocol_Ser1.h
 *
 *  Author/Date: Maurizio Colmone Dic.2013
 *
 *  Descrizione:   Protocollo di comunicazione sulla seriale 1.
 *                 La seguente seriale √® dedicata per la comunicazione tra KTS e RD2.0
 *
 *  Esempio:
 */ 


#ifndef PROTOCOL_SER1_H_
#define PROTOCOL_SER1_H_

//=====================================================================================
// Protocollo Messaggio di debug del Descrittore
//
// Chiamata
//       KTS: 's','m', 6, 'D', ..... ,  CRC_LO, CRC_HI
//   RD_UNIT: 's','m', 6, 'D', ..... ,  CRC_LO, CRC_HI      ( TOT: (IDD_POS_CRC_HI + 1) Byte))
//
//=====================================================================================

#include "asf.h"


// ---- definizioni comuni al Protocol_Ser2.h ----
#ifndef PROTO_START_MSG_0
#define  PROTO_START_MSG_0       's'
#endif
#ifndef PROTO_START_MSG_1
#define  PROTO_START_MSG_1       'm'
#endif

//---- TYPE_COMAND ----
#define COMAND_TEST_UNIT         'T'    // Richiesta di esecuzione di un determinato test
#define COMAND_DATA_DEBUG        'D'    // comando per la lettura dei dati per il debug, misure velocit√† motori, etc...
#define COMAND_POLLING_BASE      'P'    // comando di polling base di dati
#define COMAND_READ_EEPROM       'R'    // comando di lettura N Byte eeprom
#define COMAND_WRITE_EEPROM      'W'    // comando di Scrittura N Byte eeprom


//---- TYPE TEST ----
#define  START_TEST              'S'
#define  END_TEST                'E'
#define  TEST_BYPASS             '0'
#define  TEST_OUTPUT             '1'
#define  TEST_BUZZER             '2'
#define  TEST_IPEHD              '3'
#define  TEST_LEDS               '4'
#define  TEST_BURN_IN            '5'   // data_1 = minuti di burn-in
#define  TEST_CHANGE_SPEED       '6'
#define  TEST_RESULT             '9'   // data_1 = 'S' succesfull

//---------------------------------------------------------------
//  Testa dei Messaggi del protocollo 1.0
//---------------------------------------------------------------
enum _index_Head_Message_Ser1 {
	IHM1_START_MSG_0 = 0,          // byte_0: start message 's'
	IHM1_START_MSG_1,              // byte_1: start message 'm'
	IHM1_POS_CRC_LO,               // byte_2: posizione cksum_16 (byte LO) che √® anche il penultimo byte
	IHM1_TYPE_COMAND,              // byte_3: Tipo di comando/richiesta
	IHM1_START_DATA                // byte_4: Inizio BYTE Dati, lo sviluppo dei dati √® in funzione del TYPE_COMAND
};

//----------------------------------------------------------
// 'T': Richiesta di esecuzione di un determinato test
// messaggio di inizio test: 's', 'm', 8, 'T', 'S', ...
// messaggio di fine   test: 's', 'm', 8, 'T', 'E', ...
//----------------------------------------------------------
enum index_message_request_test {
	IRQT_TYPE_TEST = IHM1_START_DATA,       // byte_04:
	IRQT_DATA_1_TEST,                       // byte_05:
	IRQT_DATA_2_TEST,                       // byte_06:
	IRQT_CRC_LO,                            // byte_07:
	IRQT_CRC_HI                             // byte_08:
};  // Totale: 10  Byte

//----------------------------------------------------------
// 'D': Indici array messaggio Request DATA_DEBUG
//----------------------------------------------------------
enum index_message_request_data_debug {
	IRQD_NONE_0 = IHM1_START_DATA,           // byte_04:
	IRQD_NONE_1,                             // byte_05:
	IRQD_CRC_LO,                             // byte_06:
	IRQD_CRC_HI                              // byte_07:
};    // Totale: 9  Byte

//--  Indici array messaggio Respone DATA_DEBUG --
enum index_message_resp_data_debug {
	// PAGINA 4: Pressione
	IRSD_LEV_LINK_CAP = IHM1_START_DATA ,   // byte_04: livello di comunicazione tra Unita'† e Accesorio CAP/CAF 0/10
	IRSD_MEASUR_PA_LO,                      // byte_05: byte LO: differential_Pa
	IRSD_MEASUR_PA_HI,                      // byte_06: byte HI: differential_Pa
	// PAGINA 1 : Motors
	IRSD_LEV_LINK_CTRL_FAN,                 // byte_07: livello di comunicazione tra modulo rd2. e CTRL_FAN 0/10
	IRSD_STATUS_MOTORS_R,                   // byte_08: bit.. 7.none | 6.AlarmMR3 | 5.AlarmMR2 | 4.AlarmMR1 | 3.none | 2.PowerMR3 | 1.PowerMR2 | 0.PowerMR1
	IRSD_STATUS_MOTORS_F,                   // byte_09: bit.. 7.none | 6.AlarmMF3 | 5.AlarmMF2 | 4.AlarmMF1 | 3.none | 2.PowerMF3 | 1.PowerMF2 | 0.PowerMF1
	IRSD_FEEDBK_VOUT_R_LO,                  // byte_10:  valore di tensione letto in uscita del pilota ventole F: da 0.0V a 11.00V
	IRSD_FEEDBK_VOUT_R_HI,                  // byte_11:
	IRSD_FEEDBK_VOUT_F_LO,                  // byte_12:
	IRSD_FEEDBK_VOUT_F_HI,                  // byte_13:
	IRSD_RPM_MOTOR_R1_LO,                   // byte_14: byte LO: RPM motors R1
	IRSD_RPM_MOTOR_R1_HI,                   // byte_15: byte HI: RPM motors R1
	IRSD_RPM_MOTOR_R2_LO,                   // byte_16: byte LO: RPM motors R2
	IRSD_RPM_MOTOR_R2_HI,                   // byte_17: byte HI: RPM motors R2
	IRSD_RPM_MOTOR_R3_LO,                   // byte_18: byte LO: RPM motors R3
	IRSD_RPM_MOTOR_R3_HI,                   // byte_19: byte HI: RPM motors R4
	IRSD_RPM_MOTOR_F1_LO,                   // byte_20: byte LO: RPM motors F1
	IRSD_RPM_MOTOR_F1_HI,                   // byte_21: byte HI: RPM motors F1
	IRSD_RPM_MOTOR_F2_LO,                   // byte_22: byte LO: RPM motors F2
	IRSD_RPM_MOTOR_F2_HI,                   // byte_23: byte HI: RPM motors F2
	IRSD_RPM_MOTOR_F3_LO,                   // byte_24: byte LO: RPM motors F3
	IRSD_RPM_MOTOR_F3_HI,                   // byte_25: byte HI: RPM motors F3
	// PAGINA 2,3 : Accessory CLIMATE (le informazioni sono in eeprom sugli accessori configurati)
	IRSD_LEV_LINK_PREHEATER,                // 0/10 Level
	IRSD_LEV_LINK_HEATER,                   // 0/10 Level
	IRSD_LEV_LINK_COOLER,                   // 0/10 Level
	IRSD_STATUS_PREHEATER,                  // byte STATUS:    bit.4: 1=IN_ALLARME, 0= no allarme,
	//                 bit.3: Comando da Unit‡(1 ACCENDITI/SPEGNITI)
	//                 bit.2: ON=1/OFF=0,
	//                 bit.1: AIR=1/WATER=0,
	//                 bit.0: CONNECT
	IRSD_STATUS_HEATER,                     //  come sopra
	IRSD_STATUS_COOLER,                     //  come sopra
	IRSD_MEASURE_TW_HEAT_LO,                // 2 byte:  Temperature water se  l'accessory Ë HWD altrimenti dato inutile
	IRSD_MEASURE_TW_HEAT_HI,
	IRSD_MEASURE_TA_HEAT_LO,                // 2 byte:  Temperature air sia per HWD che EHD
	IRSD_MEASURE_TA_HEAT_HI,
	IRSD_MEASURE_TW_COOL_LO,                // 2 byte:  Temperature water accessoy
	IRSD_MEASURE_TW_COOL_HI,
	IRSD_MEASURE_AIRFLOW_LO,                //
	IRSD_MEASURE_AIRFLOW_HI,                //
	
	IRSD_MEASURE_ABSOR_BYPASS_MIN,          // valore espresso in mA da 0 a 255mA
	IRSD_MEASURE_ABSOR_BYPASS_MAX_LO,       // valore espresso in mA da 0 a 280mA
	IRSD_MEASURE_ABSOR_BYPASS_MAX_HI,
	IRSD_LEV_LINK_I_SENSOR_AMB,             // Valore di comunicazione del sensore RH/CO2/VOC
	IRSD_LEV_LINK_MODBUS,                   // Valore di comunicazione del Modbus
	IRSD_MEASURE_TW_PREHEAT_LO,             // 2 byte:  Temperature water se  l'accessory Ë PHWD
	IRSD_MEASURE_TW_PREHEAT_HI,
	IRSD_MEASURE_TA_PREHEAT_LO,             // 2 byte:  Temperature air sia per PHWD che PEHD
	IRSD_MEASURE_TA_PREHEAT_HI,
	IRSD_MEASURE_TA_COOL_LO,                // 2 byte:  Temperature air per CWD
	IRSD_MEASURE_TA_COOL_HI,
	IRSD_STATUS_DSC,

	// CAPS - CAPF
	IRSD_MEASUR_PA_CAPS_LO,                 //
	IRSD_MEASUR_PA_CAPS_HI,                 //
	IRSD_MEASUR_PA_CAPR_LO,                 //
	IRSD_MEASUR_PA_CAPR_HI,                 //
	IRSD_MEASUR_AF_CAPS_LO,                 //
	IRSD_MEASUR_AF_CAPS_HI,                 //
	IRSD_MEASUR_AF_CAPR_LO,                 //
	IRSD_MEASUR_AF_CAPR_HI,                 //
	IRSD_LEV_LINK_CAPR,                     // 0/10 Level
	IRSD_LEV_LINK_CAPS,                     // 0/10 Level
	IRSD_STATUS_CAPS,                      // 1 byte: status
	IRSD_STATUS_CAPR,                      // 1 byte: status
	
	// AIR SENSOR
	IRSD_MEASUR_CO2_1_LO,                 //
	IRSD_MEASUR_CO2_1_HI,                 //
	IRSD_MEASUR_VOC_1_LO,                 //
	IRSD_MEASUR_VOC_1_HI,                 //
	IRSD_MEASUR_RH_1,                     //
	IRSD_MEASUR_CO2_2_LO,                 //
	IRSD_MEASUR_CO2_2_HI,                 //
	IRSD_MEASUR_RH_2,                     //
	IRSD_FIRM_VER_CO2_1,                  //
	IRSD_FIRM_VER_CO2_2,                  //
	IRSD_LEV_LINK_CO2_1,                  // 0/10 Level
	IRSD_LEV_LINK_CO2_2,                  // 0/10 Level
	IRSD_MEASUR_IAQ_1_LO,                 // byte 76
	IRSD_MEASUR_IAQ_1_HI,                 // byte 77
	IRSD_MEASUR_IAQ_2_LO,                 //
	IRSD_MEASUR_IAQ_2_HI,                 //
	IRSD_STATUS_CO2_1,                    // 1 byte: status
	IRSD_STATUS_CO2_2,                    // 1 byte: status
	IRSD_ACCURACY_CO2_1,                  // 
	IRSD_ACCURACY_CO2_2,                  // 
	
	IRSD_FIRM_VER_CAPS_LO,                  // byte 84
	IRSD_FIRM_VER_CAPS_HI,                  // byte 85
	IRSD_FIRM_VER_CAPR_LO,                  // byte 86
	IRSD_FIRM_VER_CAPR_HI,                  // byte 87
	IRSD_FIRM_VER_PREHEATER_LO,             // byte 88
	IRSD_FIRM_VER_PREHEATER_HI,             // byte 89
	IRSD_FIRM_VER_HEATER_LO,                // byte 90
	IRSD_FIRM_VER_HEATER_HI,                // byte 91
	IRSD_FIRM_VER_COOLER_LO,                // byte 92
	IRSD_FIRM_VER_COOLER_HI,                // byte 93
	IRSD_FIRM_VER_CAP_LO,                   // byte 94
	IRSD_FIRM_VER_CAP_HI,                   // byte 95
	IRSD_FIRM_VER_CAF_LO,                   // byte 96
	IRSD_FIRM_VER_CAF_HI,                   // byte 97
	IRSD_FIRM_VER_BPD,                      // byte 98
	
	IRSD_HEATER_VOLTAGE_LO,                 // byte 99
	IRSD_HEATER_VOLTAGE_HI,                 // byte 100
	IRSD_HEATER_CURRENT_LO,                 // byte 101
	IRSD_HEATER_CURRENT_HI,                 // byte 102
	
	IRSD_STATUS_RH1,						// byte 103: status  from version 2.65
	IRSD_STATUS_RH2,						// byte 104: status
	IRSD_STATUS_VOC,						// byte 105: status
	IRSD_HEATER_INFO,						// byte 106
	
	IRSD_CRC_LO,                            // byte_107: CRC : Low
	IRSD_CRC_HI                             // byte_108: CRC : high
}; // 5 + 100 byte

//-------------------------------------------------------------
// 'R': Indici array messaggio Request Read/write eeprom
//-------------------------------------------------------------
enum index_message_request_read_eeprom {
	IRQR_ADDR_BYTE_START_EEP = IHM1_START_DATA,        // byte_04:  Indirizzo INIZIALE di memoria delle eeprom  che si vuole scrivere o leggere
	IRQR_ADDR_NUM_BYTE_EEP,                            // byte_05:  Numero di Byte da leggere
	IRQR_CRC_LO,                                       // byte_06:
	IRQR_CRC_HI                                        // byte_07:
};

//------------------ response ----------------------------------
enum index_message_response_read_eeprom {
	IRSR_ADDR_BYTE_START_EEP = IHM1_START_DATA,        // byte_04:
	IRSR_ADDR_NUM_BYTE_EEP,                            // byte_05:
	IRSR_START_DATA_EEPROM                             // byte_06:
	// ..... N BYTE EEPROM di Lettura
	// IRSR_CRC_LO, l'indice CRC Ë dinamico e varia in funzione della lunghezza del numero di byte
	// IRSR_CRC_HI
};

//-------------------------------------------------------------
// 'W': Indici array messaggio Request Information
//-------------------------------------------------------------
enum index_message_request_write_eeprom {
	IRQW_ADDR_BYTE_START_EEP = IHM1_START_DATA,   // byte_04: Indirizzo di partenza di scrittura eeprom
	IRQW_ADDR_NUM_BYTE_EEP,                       // byte_05: Numero di Byte da leggere
	IRQW_START_DATA_EEPROM                        // byte_06: Inizio area dati da scrivere in eeprom Max 116 byte
	//.............. N BYTE EEPROM di Scrittura
	// IRQW_CRC_LO , l'indice CRC Ë dinamico e varia in funzione della lunghezza del numero di byte
	// IRQW_CRC_HI
};

//------------------ response ----------------------------------
enum index_message_response_write_eeprom {
	IRSW_ADDR_BYTE_START_EEP = IHM1_START_DATA,   // byte_04: Indirizzo di partenza di scrittura eeprom
	IRSW_ADDR_NUM_BYTE_EEP,                       // byte_05: Numero di Byte da leggere
	IRSW_RESULT_W,                                // byte_06: '0': scrittura OK,  '1': errore zona eeprom non valida,  '2': errore dati non validi,  '3': dispositivo occupato.
	IRSW_CNT_UPDATE_EEP_INFO,                     // byte_07:
	IRSW_CNT_UPDATE_EEP_SETTING_PAR,              // byte_08:
	IRSW_CNT_UPDATE_EEP_SETP_TEMP,                // byte_09:
	IRSW_CNT_UPDATE_EEP_WEEKLY,                   // byte_10:
	IRSW_CRC_LO,                                  // byte_11:
	IRSW_CRC_HI                                   // byte_12:
};

//-------------------------------------------------------------
// 'P': Indici array messaggio Req. Polling BASE
//-------------------------------------------------------------
enum index_message_request_polling_base {
	IRQP_DATA_SEC = IHM1_START_DATA,          // byte_04:
	IRQP_DATA_MIN,                            // byte_05:
	IRQP_DATA_HOURS,                          // byte_06:
	IRQP_DATA_WEEKDAY,                        // byte_07:
	IRQP_DATA_DAY,                            // byte_08:
	IRQP_DATA_MOUNTH,                         // byte_09:
	IRQP_DATA_YEAR,                           // byte_10:
	IRQP_DATA_CENTURY,                        // byte_11:
	IRQP_CRC_LO,                              // byte_12:
	IRQP_CRC_HI                               // byte_13:
};    // Totale: 9  Byte

//------------------ response ----------------------------------
enum index_message_resp_polling_base {
	IRSP_MEASURE_TEMP_1_LO  = IHM1_START_DATA,  // Temp. Fresh
	IRSP_MEASURE_TEMP_1_HI,
	IRSP_MEASURE_TEMP_2_LO,                     // Temp. Return
	IRSP_MEASURE_TEMP_2_HI,
	IRSP_MEASURE_TEMP_3_LO,                     // Temp. Supply
	IRSP_MEASURE_TEMP_3_HI,
	IRSP_MEASURE_TEMP_4_LO,                     // Temp. Exhaust
	IRSP_MEASURE_TEMP_4_HI,
	IRSP_MEASURE_IN1,               // valore espresso x 10 ovvero 95 = 9.5V
	IRSP_MEASURE_IN2,
	IRSP_INFO_PROBE_AD,             // 7.P2CO2, 6.P1CO2, 5.----, 4.P2RH, 3.P1RH, 2.----, 1.P1VOC, 0.----  (0 = Ambiente, 1 = Duct)
	IRSP_MEASURE_RH_SENS,           // 1 byte:  da 0% a 99%
	IRSP_MEASURE_CO2_SENS_LO,       // 2 byte:  da 0 a 2000 PPM
	IRSP_MEASURE_CO2_SENS_HI,       //
	IRSP_MEASURE_VOC_SENS_LO,       // 2 byte:  da 0 a 10000 ug/mq
	IRSP_MEASURE_VOC_SENS_HI,
	IRSP_STATUS_UNIT_LO,            // 2 byte:  bit[15,14,13,12]: * none *
	IRSP_STATUS_UNIT_HI,            //          bit[11]: 1= VOC_MAX, bit[10]: 1= CO2_MAX,  bit[9]: 1= RH_MAX
	//          bit[8]: 1= CMD_FAN_INPUT
	//          bit[7,6]: b10=BypassClose, b01=BypassRun, b00=BypassOpen
	//          bit[5,...,0]:  b10000= none b010000= RUN BOOST, b001000= RUN IMBALANCE, b000100=RUN POST_VENT, b000010=RUN DeFROST, b00001=RUN NORMAL, b000000=STANDBY
	
	IRSP_STATUS_WEEKLY,             // 1 byte:  bit[7,6]:1,0=STEP_SPEED_3/ 0,1=STEP_SPEED_2/ 0,0=STEP_SPEED_1 | bit[5,4]: 0,1=IMBAL_1_ON/ 0,0=IMBAL_OFF | bit[3,2]:0,1=RIF_TEMP2/ 0,0=RIF_TEMP1 | bit[1]: WEEKLY_RUN, bit[0]:ENABLE
	
	IRSP_EVENT_BYTE_00,             //13 byte: vedi descrizione allarmi nel file: data.h
	IRSP_EVENT_BYTE_01,
	IRSP_EVENT_BYTE_02,
	IRSP_EVENT_BYTE_03,
	IRSP_EVENT_BYTE_04,
	IRSP_EVENT_BYTE_05,
	IRSP_EVENT_BYTE_06,
	IRSP_EVENT_BYTE_07,
	IRSP_EVENT_BYTE_08,
	IRSP_EVENT_BYTE_09,
	IRSP_EVENT_BYTE_10,
	IRSP_EVENT_BYTE_11,
	IRSP_EVENT_BYTE_12,
	
	IRSP_NONE_0,                    // 1 byte: a disposizione
	IRSP_INCREASE_SPEED_RH_CO2,     // 1 byte: Coefficiente moltiplicativo per 10,  10= 1.0 ed Ë l'incremento della velocit‡ rispetto al set-poin a causa: RH alta, o CO2 alta, o VOCs alta.
	// dati utili a capire se vi √® stata fatta una variazione di eeprom da parte di altri.
	IRSP_CNT_UPDATE_EEP_INFO,
	IRSP_CNT_UPDATE_EEP_SETTING_PAR,
	IRSP_CNT_UPDATE_EEP_SETP_TEMP,
	IRSP_CNT_UPDATE_EEP_WEEKLY,
	IRSP_MEASURE_AWP_SENS_LO,       // 2 byte:  Temperatura x 10
	IRSP_MEASURE_AWP_SENS_HI,
	
	IRSP_NUM_RECORDS, // Number of Records Saved
	IRSP_INDEX_RECORD, // Index of Record to be sent
	IRSP_TYPE_OF_RESET,
	IRSP_LOG_YEAR_HIGH,
	IRSP_LOG_YEAR_LOW,
	IRSP_LOG_MONTH,
	IRSP_LOG_DAY,
	IRSP_LOG_HOURS_1,
	IRSP_LOG_HOURS_2,
	IRSP_LOG_HOURS_3,
	IRSP_LOG_HOURS_4,
	
	IRSP_KTS_CURRENT,  // from version 2.65
	IRSP_SPEED_MOTOR_R,
	IRSP_SPPED_MOTOR_F,
	IRSP_STATUS_LED_CTRL_FAN,
	IRSP_BPD_CURRENT_LO,
	IRSP_BPD_CURRENT_HI,
	IRSP_BPD_ROTATE,
	
	IRSP_12V_LO,  // from version 2.66
	IRSP_12V_HI,
	IRSP_5V_LO,
	IRSP_5V_HI,
	
	IRSP_CRC_LO,
	IRSP_CRC_HI
};    // Totale: 5 + 57 Byte

extern void Serial_begin( void );
extern void Serial_end( void );
extern int Serial_available(void);
extern int Serial_peek(void);
extern int Serial_read(void);
extern void Serial_flush(void);
extern void Serial_write(uint8_t* b, uint16_t len);
extern void Serial_print1( const char* buff);
extern void Serial_print( Byte b, int base);	
extern void Serial_println(Byte b, int base);
extern void Serial_println1(const char* buff);	

#endif /* PROTOCOL_SER1_H_ */