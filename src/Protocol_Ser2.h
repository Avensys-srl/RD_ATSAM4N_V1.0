/*
 * Protocol_Ser2.h
 *
 * Author: Marketing
 *
 * Created on 14 dicembre 2013, 9.57
 *
 * Descriptions:
 *  Protocollo Messaggio su canale uart 2. Vi sono + dispositivi su questo canale,
 *  RD2, CTRL_FAN,  SK_MODBUS.
 */ 


#ifndef PROTOCOL_SER2_H_
#define PROTOCOL_SER2_H_

	//-------------------------------------------------------
	//              caratteri speciali protocollo
	//-------------------------------------------------------
	// inizio messaggio
	#ifndef PROTO_START_MSG_0
	#define  PROTO_START_MSG_0        's'
	#endif
	#ifndef PROTO_START_MSG_1
	#define  PROTO_START_MSG_1        'm'
	#endif
	// TYPE_COMAND
	#define COMAND_PACKET_1_DATA      '1'
	#define COMAND_PACKET_2_DATA      '2'
	#define COMAND_POLLING_CTRL_FAN   'F'
	#define COMAND_READ_EEPROM        'R'
	#define COMAND_WRITE_EEPROM       'W'
	#define COMAND_POLLING_CTRL_FAN   'F'


	//---------------  Indirizzi RECEIVER  ------------------
	#define ADDR_RD_UNIT            0x01
	#define ADDR_CTRL_FAN           0x04
	#define ADDR_REM_CTRL           0x07


	//------------- Codici risposte Eeprom  -----------------
	#define WR_EEP_OK                 '0'
	#define WR_EEP_ADDR_NOT_VALID     '1'
	#define WR_EEP_DATA_NOT_AVLID     '2'
	#define WR_SLAVE_BUSY             '3'


	//--------------- macro per: mask_rx_data_unit  -------------------
	#define RX_DATA_1_OK            0x01
	#define RX_DATA_2_OK            0x02
	#define RX_EEP_INFO_OK          0x04
	#define RX_EEP_SET_PAR_OK       0x08
	#define RX_EEP_SET_TEMP_OK      0x10
	#define RX_EEP_SET_WEEK_OK      0x20
	#define RX_EEP_CONFIG_HW_OK     0x40    // aggiunta con RFM


	//-------------------------------------------------------
	//  Testa dei Messaggi del protocollo 1.0
	//-------------------------------------------------------
	enum _index_Head_Message_Ser2 {
		IHM2_START_MSG_0 = 0,    // byte_0: start message 's'
		IHM2_START_MSG_1,        // byte_1: start message 'm'
		IHM2_POS_CRC_LO,         // byte_2: posizione cksum_16 (byte LO) che Ã¨ anche il penultimo byte
		IHM2_ADDR_RECEIVER,      // byte_3: Indirizzo del destinatorio del messaggio
		IHM2_TYPE_COMAND,        // byte_4: Tipo di comando/richiesta
		IHM2_START_DATA          // byte_5: Inizio BYTE Dati, lo sviluppo dei dati Ã¨ in funzione del TYPE_COMAND
	};
	

	//-----------------------------------------------------
	//  Richesta di Polling da Modulo RD a Modulo CTRL_FAN
	//  LEN: 6 + 9 = 15 Byte
	//-----------------------------------------------------
	enum index_message_req_polling_ctrlFan {
		IRQ_CMD_F_SPEED_FANS_R = IHM2_START_DATA, // valori SPEED_FANS_R da 0 -> 255
		IRQ_CMD_F_SPEED_FANS_F,                   // valori SPEED_FANS_F da 0 -> 255
		IRQ_CMD_F_STATUS_LEDS,                    //  [1,0] LED 3 GIALLO (11= none, 10= ON FIXED, 01=Toggle, 00 = OFF)
		//  [3,2] LED 4 ROSS0  (11= none, 10= ON FIXED, 01=Toggle, 00 = OFF)
		//  [5,4] LED 1 VERDE  (11= none, 10= ON FIXED, 01=Toggle, 00 = OFF)
		//  [7,6] LED 2 VERDE  (11= none, 10= ON FIXED, 01=Toggle, 00 = OFF)
		IRQ_CMD_F_NPULSE_XROUND,                  // numero di pulse per giro. ( 2, 3, 4)
		IRQ_CMD_F_NUM_MOTORS,                     // numero di motori da gestire
		IRQ_CMD_F_BYTE_1_DONE,                    // done, per future implementazioni
		IRQ_CMD_F_BYTE_2_DONE,                    // done, per future implementazioni
		IRQ_CMD_F_CRC_LO,                         // byte_13
		IRQ_CMD_F_CRC_HI                          // byte_14
	};


	//-----------------------------------------------------
	//  Risposta Polling da Modulo CTRL_FAN a Modulo RD
	//  LEN : 6 + 22 = 28 Byte
	//-----------------------------------------------------
	enum index_message_resp_polling_ctrlFan {
		IRS_CMD_F_STATUS_FAN_R = IHM2_START_DATA,  // bit.. 7.none | 6.AlarmMR3 | 5.AlarmMR2 | 4.AlarmMR1 | 3.none | 2.PowerMR3 | 1.PowerMR2 | 0.PowerMR1
		IRS_CMD_F_STATUS_FAN_F,                    // bit.. 7.none | 6.AlarmMF3 | 5.AlarmMF2 | 4.AlarmMF1 | 3.none | 2.PowerMF3 | 1.PowerMF2 | 0.PowerMF1
		IRS_CMD_F_FEEDBK_VOUT_R_LO,                // valore di tensione letto in uscita del pilota ventole R: da 0.00V a 11.00V
		IRS_CMD_F_FEEDBK_VOUT_R_HI,
		IRS_CMD_F_FEEDBK_VOUT_F_LO,                // valore di tensione letto in uscita del pilota ventole F: da 0.0V a 11.00V
		IRS_CMD_F_FEEDBK_VOUT_F_HI,
		IRS_CMD_F_RPM_FAN_R1_LO,                   // RMP Motore R1
		IRS_CMD_F_RPM_FAN_R1_HI,
		IRS_CMD_F_RPM_FAN_R2_LO,
		IRS_CMD_F_RPM_FAN_R2_HI,
		IRS_CMD_F_RPM_FAN_R3_LO,
		IRS_CMD_F_RPM_FAN_R3_HI,
		IRS_CMD_F_RPM_FAN_F1_LO,
		IRS_CMD_F_RPM_FAN_F1_HI,
		IRS_CMD_F_RPM_FAN_F2_LO,
		IRS_CMD_F_RPM_FAN_F2_HI,
		IRS_CMD_F_RPM_FAN_F3_LO,
		IRS_CMD_F_RPM_FAN_F3_HI,
		IRS_CMD_F_RPM_VERSION_FW_H,              // versione FW 1,... Cifra + significativa
		IRS_CMD_F_RPM_VERSION_FW_L,              // versione FW 0,... Cifra - significativa
		IRS_CMD_F_BYTE_0_DONE,                   // done
		IRS_CMD_F_BYTE_1_DONE,                   // done
		IRS_CMD_F_CRC_LO,
		IRS_CMD_F_CRC_HI
	};
	
	//-----------------------------------------------------
	//  Packet1 Data dall'Unità al Modbus.
	//-----------------------------------------------------
	enum index_message_packet1_data_unit {
		/*05*/IPK1U_STATUS_UNIT_LO = IHM2_START_DATA,
		IPK1U_STATUS_UNIT_HI,
		IPK1U_BYTE_ALARM_LO,       // vengono raggruppati gli allarmi
		IPK1U_BYTE_ALARM_HI,
		IPK1U_STATUS_OUTPUT,       // bits: ..., [1]STS_OUT2, [0]STS_OUT1
		/*10*/IPK1U_STATUS_FILTER,
		IPK1U_MEASURE_V_INPUT_1,   // Misura input 1 da 0V a 10.0V (=100)
		IPK1U_MEASURE_V_INPUT_2,
		IPK1U_NUM_MOTORS,
		IPK1U_FEEDBK_VR_LO,        // Byte 2: FeedBack tensione, Comando gruppo Return
		/*15*/IPK1U_FEEDBK_VR_HI,
		IPK1U_FEEDBK_VF_LO,        // Byte 2: FeedBack tensione, Comando gruppo Fresh
		IPK1U_FEEDBK_VF_HI,
		IPK1U_RPM_FAN_R1_LO,       // Byte 2: RMP Motore R1
		IPK1U_RPM_FAN_R1_HI,
		/*20*/IPK1U_RPM_FAN_F1_LO,
		IPK1U_RPM_FAN_F1_HI,
		IPK1U_RPM_FAN_R2_LO,
		IPK1U_RPM_FAN_R2_HI,
		IPK1U_RPM_FAN_F2_LO,
		/*25*/IPK1U_RPM_FAN_F2_HI,
		IPK1U_RPM_FAN_R3_LO,
		IPK1U_RPM_FAN_R3_HI,
		IPK1U_RPM_FAN_F3_LO,
		IPK1U_RPM_FAN_F3_HI,
		/*30*/IPK1U_RPM_FAN_R4_LO,
		IPK1U_RPM_FAN_R4_HI,
		IPK1U_RPM_FAN_F4_LO,
		IPK1U_RPM_FAN_F4_HI,
		IPK1U_RPM_FAN_R5_LO,
		/*35*/IPK1U_RPM_FAN_R5_HI,
		IPK1U_RPM_FAN_F5_LO,
		IPK1U_RPM_FAN_F5_HI,
		IPK1U_CNT_UPDATE_INFO,       // contatore per allineare la aree di eeprom
		IPK1U_CNT_UPDATE_SET_PAR,    // contatore per allineare la aree di eeprom
		/*40*/IPK1U_CNT_UPDATE_SET_TEMP,   // contatore per allineare la aree di eeprom
		IPK1U_CNT_UPDATE_WEEKLY,     // contatore per allineare la aree di eeprom
		//  aggiunta con la RF (10/04/15)
		IPK1U_STATUS_MOTORS_R,
		IPK1U_STATUS_MOTORS_F,
		IPK1U_EVENT_BYTE_0,           // [00]: ACC. EHD/HWD
		/*45*/IPK1U_EVENT_BYTE_1,           // [01]: ACC. CWD
		IPK1U_EVENT_BYTE_2,           // [02]: ACC. PEHD/PHWD
		IPK1U_EVENT_BYTE_3,           // [03]: PROBES_CO2
		IPK1U_EVENT_BYTE_4,           // [04]: PROBES_RH
		IPK1U_EVENT_BYTE_5,           // [05]: PROBES_VOC
		/*50*/IPK1U_EVENT_BYTE_6,           // [06]: SENSORS_PRESSURE
		IPK1U_EVENT_BYTE_7,           // [07]: PROBES_OTHERS
		IPK1U_EVENT_BYTE_8,           // [08]: MOTORS_R
		IPK1U_EVENT_BYTE_9,           // [09]: MOTORS_F
		IPK1U_EVENT_BYTE_10,          // [10]: MIX_ALM_1
		/*55*/IPK1U_EVENT_BYTE_11,          // [11]: MIX_ALM_2.
		IPK1U_CKS_LO,
		IPK1U_CKS_HI
	}; // Totale byte 58 -> ~15.5ms

	//-----------------------------------------------------
	//  Packet2 Data dall'Unità al Modbus .
	//-----------------------------------------------------
	enum index_message_packet2_data_unit {
		/*05*/IPK2U_CLOCK_DAYWEEK = IHM2_START_DATA,
		IPK2U_CLOCK_HOURS,
		IPK2U_CLOCK_MIN,
		IPK2U_CLOCK_SEC,
		IPK2U_HOURS_RUN_BYT0,         //  Byte 0: hour run
		/*10*/IPK2U_HOURS_RUN_BYT1,         //  Byte 1
		IPK2U_HOURS_RUN_BYT2,         //  Byte 2
		IPK2U_HOURS_RUN_BYT3,         //  Byte 3: hour run
		IPK2U_NUM_NTC,                //  num NTC Enable
		IPK2U_MEASURE_TF_LO,          //  Byte 2 -> Temp. Fresh
		/*15*/IPK2U_MEASURE_TF_HI,
		IPK2U_MEASURE_TR_LO,          //  Byte 2 ->  Temp. Return
		IPK2U_MEASURE_TR_HI,
		IPK2U_MEASURE_TS_LO,          //  Byte 2 -> Temp. Supply
		IPK2U_MEASURE_TS_HI,
		/*20*/IPK2U_MEASURE_TE_LO,          // Temp. Exhaust
		IPK2U_MEASURE_TE_HI,          // Temp. Exhaust
		IPK2U_STATUS_ACCESS_CLIMA,    // (Da rivedere sul RD2)  bits: [7]Heater_Air_Water, [6]CMD_PreHeat, [5]CMD_Cooler, [4]CMD_Heather, [3]PreHeat_Air_Water, [2]STS_PreHeat, [1]STS_Cooler, [0]STS_Heather
		IPK2U_MEASURE_ACC_TA_HEATER_LO,     //(Da rivedere sul RD2)  T.air Heater
		IPK2U_MEASURE_ACC_TA_HEATER_HI,     //(Da rivedere sul RD2)  T.air Heater
		/*25*/IPK2U_MEASURE_ACC_TW_COOLER_LO,     // T.water Cooler
		IPK2U_MEASURE_ACC_TW_COOLER_HI,     // T.water Cooler
		IPK2U_MEASURE_ACC_TW_PREHEATER_LO,
		IPK2U_MEASURE_ACC_TW_PREHEATER_HI,
		IPK2U_MEASURE_CO2_LO,
		/*30*/IPK2U_MEASURE_CO2_HI,
		IPK2U_MEASURE_RH,             // 1 solo Byte da 0 a 100%
		IPK2U_MEASURE_VOC,            // 1 solo byte da 0 a 100PPM
		IPK2U_MEASURE_PRESSURE_LO,
		IPK2U_MEASURE_PRESSURE_HI,
		/*35*/IPK2U_MEASURE_AIRFLOW_LO,
		IPK2U_MEASURE_AIRFLOW_HI,
		IPK2U_MEASURE_ACC_DSP_LO,
		IPK2U_MEASURE_ACC_DSP_HI,
		IPK2U_LEVEL_LINK_HEATER,      // da 0/10 a 10/10
		/*40*/IPK2U_LEVEL_LINK_COOLER,      // da 0/10 a 10/10
		IPK2U_LEVEL_LINK_PREHEATER,   // da 0/10 a 10/10
		IPK2U_LEVEL_LINK_CAP_CAF,      // da 0/10 a 10/10
		IPK2U_LEVEL_LINK_CTRL_FAN,    // da 0/10 a 10/10
		IPK2U_SET_MAX_VAL_CAF,        // Vecchio valore CAF_MAX: da molt. x 100
		/*45*/IPK2U_MAX_SETP_CAP,           // valore da molt. x 10
		IPK2U_SET_MAX_SETP_CAF_LO,    // Nuovo valore CAF_MAX: intero es: 450mq/h -> 450
		IPK2U_SET_MAX_SETP_CAF_HI,
		//  aggiunta con la RF (10/04/15)
		IPK2U_MEASURE_ACC_TW_HEATER_LO,     //(Da implemetare sul RD2)   T.water Heater
		IPK2U_MEASURE_ACC_TW_HEATER_HI,     //(Da implementare sul RD2)  T.water Heater
		/*50*/IPK2U_INCREASE_SPEED_RH_CO2,        //(Da implementare sul RD2)
		IPK2U_STATUS_WEEKLY,                //(Da implementare sul RD2)
		IPK2U_INFO_PROBE_AD,                //  7.2CO2  6.P1CO2  5.----  4.P2RH  3.P1RH  2.----  1.P1VOC  0.----
		//aggiunta con ZH (14/07/2018)
		IPK2U_SET_MBF_FAN_F,               // 1 solo Byte da 0 a 100% - Controllo MBF motore di fresh
		IPK2U_SET_MBF_FAN_R,               // 1 solo Byte da 0 a 100% - Controllo MBF motore di return
		/*55*/IPK2U_MEASURE_ACC_TA_COOLER_LO,     // T.water Cooler
		IPK2U_MEASURE_ACC_TA_COOLER_HI,     // T.water Cooler
		IPK2U_MEASURE_ACC_TA_PREHEATER_LO,
		IPK2U_MEASURE_ACC_TA_PREHEATER_HI,
		IPK2U_EXT1_M1_LO,
		/*60*/IPK2U_EXT1_M1_HI,
		IPK2U_EXT1_M2_LO,
		IPK2U_EXT1_M2_HI,
		IPK2U_EXT1_M3_LO,
		IPK2U_EXT1_M3_HI,
		IPK2U_EXT1_M4_LO,
		IPK2U_EXT1_M4_HI,
		IPK2U_EXT2_M1_LO,
		IPK2U_EXT2_M1_HI,
		IPK2U_EXT2_M2_LO,
		/*70*/IPK2U_EXT2_M2_HI,
		IPK2U_EXT2_M3_LO,
		IPK2U_EXT2_M3_HI,
		IPK2U_EXT2_M4_LO,
		IPK2U_EXT2_M4_HI,
		IPK2U_EXT3_M1_LO,
		IPK2U_EXT3_M1_HI,
		IPK2U_EXT3_M2_LO,
		IPK2U_EXT3_M2_HI,
		IPK2U_EXT3_M3_LO,
		/*80*/IPK2U_EXT3_M3_HI,
		IPK2U_EXT3_M4_LO,
		IPK2U_EXT3_M4_HI,
		IPK2U_EXT4_M1_LO,
		IPK2U_EXT4_M1_HI,
		IPK2U_EXT4_M2_LO,
		IPK2U_EXT4_M2_HI,
		IPK2U_EXT4_M3_LO,
		IPK2U_EXT4_M3_HI,
		IPK2U_EXT4_M4_LO,
		/*90*/IPK2U_EXT4_M4_HI,
		IPK2U_PFAN_1_LO,
		IPK2U_PFAN_1_HI,
		IPK2U_PFAN_2_LO,
		IPK2U_PFAN_2_HI,
		IPK2U_TEMP_1_LO,
		IPK2U_TEMP_1_HI,
		IPK2U_TEMP_2_LO,
		IPK2U_TEMP_2_HI,
		IPK2U_TEMP_3_LO,
		/*100*/ IPK2U_TEMP_3_HI,
		IPK2U_SET_EHD_MOD,
		IPK2U_SET_BPD_MOD,
		IPK2U_CKS_LO,
		/*102*/ IPK2U_CKS_HI
	};    // Totale byte 102

	//-----------------------------------------------------
	//  Packet1 Data dal Modbus all'Unità
	//-----------------------------------------------------
	enum index_message_packet1_data_modbus {
		/*05*/IPK1M_STATUS_REMOTE_CTRL = IHM2_START_DATA,  // bit7: 1= Funzione modbus Attiva, bit6: 1= Modulo Installato, bit5,..,bit0:---
		IPK1M_VER_FW_1_CAR,      // Versione FW è un numero esempio 101 = 1.01
		IPK1M_VER_FW_2_CAR,
		IPK1M_SET_CLOCK,         // se il byte è  'S' reimposta l'orologio.
		IPK1M_CLOCK_DAYWEEK,
		IPK1M_CLOCK_HOURS,
		IPK1M_CLOCK_MIN,
		IPK1M_CLOCK_SEC,
		IPK1M_CLEAN_FILTERS,     // se byte = 'C', cancella stato filtro
		IPK1M_NONE_0,            // * future implementations *
		IPK1M_NONE_1,            // * future implementations *
		IPK1M_CKS_LO,
		IPK1M_CKS_HI
	}; // totale: 18 byte

	//-----------------------------------------------------
	//  Richiesta lettura N byte eeprom.
	//  Tale richiesta viene fatta dal Modbus verso RD
	//-----------------------------------------------------
	enum index_message_req_read_eeprom {
		/*05*/IRQ_RDE_ADDR_EEP = IHM2_START_DATA,
		IRQ_RDE_NUM_BYTE,
		IRQ_RDE_CKS_LO,
		IRQ_RDE_CKS_HI
	}; // totale: 15 byte

	//-----------------------------------------------------
	//  Risposta lettura N byte eeprom.
	//-----------------------------------------------------
	enum index_message_resp_read_eeprom {
		/*05*/IRS_RDE_ADDR_EEP  = IHM2_START_DATA,
		IRS_RDE_NUM_BYTE,
		IRS_RDE_START_DATA_EEP
		//........
		// IRS_RDE_CKS_LO = IRS_RDE_NUM_BYTE + NUM_BYTE
		// IRS_RDE_CKS_HI = IRS_RDE_NUM_BYTE + NUM_BYTE + 1
	};

	//-----------------------------------------------------
	//  Richiesta di scrittura N byte eeprom.
	//-----------------------------------------------------
	enum index_message_req_write_eeprom {
		/*05*/IRQ_WRE_ADDR_EEP = IHM2_START_DATA,
		IRQ_WRE_NUM_BYTE,
		IRQ_WRE_START_DATA_EEP
		//........
		// IRQ_WRE_CKS_LO,
		// IRQ_WRE_CKS_HI
	};


	//-----------------------------------------------------
	//  Risposta di scrittura N byte eeprom.
	//-----------------------------------------------------
	enum index_message_resp_write_eeprom {
		/*05*/IRS_WRE_ADDR_EEP = IHM2_START_DATA,
		IRS_WRE_NUM_BYTE,
		IRS_WRE_ESITO,   // '0': scrittura OK
		// '1': errore zona eeprom non valida
		// '2': errore dati non validi
		// '3': dispositivo occupato.
		IRS_WRE_CKS_LO,
		IRS_WRE_CKS_HI
	};

extern void Serial2_begin( void );
extern void Serial2_end( void );
extern int Serial2_available(void);
extern int Serial2_peek(void);
extern int Serial2_read(void);
extern void Serial2_flush(void);
extern void Serial2_write(uint8_t* b, uint16_t len);

#endif /* PROTOCOL_SER2_H_ */