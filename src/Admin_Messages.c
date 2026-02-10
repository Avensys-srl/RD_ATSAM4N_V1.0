/*
 * Admin_Messages.cpp
 *
 * Descrizione:
 *   Il file è dedicato alla gestione delle 2 seriale, e con relative messaggistiche
 *
 *   Seriale1 ( 9600 Baud): ModuloRD <-> KTS
 *
 *   Seriale2 (38400 Baud): ModuloRD <-> Modulo CTRL_FAN
 *                          ModuloRd <-> Modulo ModBus
 */ 

#include "asf.h"
#include "common/build_config.h"

 #include "Protocol_Ser1.h"
 #include "Protocol_Ser2.h"
 #include "eeprom_map.h"
 #include "alarm.h"
 #include "data.h"
 #include "string.h"

//------------------------- Macro -----------------------------
#define byte	Byte

#define lowByte(i)    ( (char) i )
#define highByte(i)    ( (char) ( ((int) i) >> 8) )

//------- Led 5 Rosso, E' montato sulla scheda RD2 --------
#define LED5_BLINK  /*(PORTFINV = 0x0001)*/

// stato del Rele' e delle tensione dell'elettronica
#define POWER_SAVE     0
#define POWER_ON      'O'

#define DIM_MAX_BUFF_SER1         128  // Attenzione il driver della uart1 HA DIMENSIONE massima di 128 Byte
#define DIM_MAX_BUFF_SER2         128

//---- STATI RICEZIONE SERIALI --------
#define  RUN_DOWNLOAD              1
#define  BUFFER_RX_EMPY            0
#define  ERROR_HEAD_MESSAGE       -1
#define  ERROR_LEN_MESSAGE        -2
#define  ERROR_CKSUM_16           -3

//------------------ Stati BYPASS ---------------------
#define ANTIORARIO       0xFF
#define ORARIO           0
#define CLOSE_BPD        1   // bypass chiuso, il flusso dell'aria passa dallo scambiatore
#define OPEN_BPD         0   // bypass aperto 100%, il flusso dell'aria passa solo dal canale Bypass.

#define NO_TACHO			 // Se Abilitato gli rpm sono calcolati in proporzione al segnale in uscita su un fondo scala di 3200 rpm 
  
//--------------------- GLOBAL VARS --------------------------- 
unsigned long int preview_3_Mills   = 0;  
  
int cnt_resp_poll_CTRL_FAN  = 0;
int cnt_req_poll_CTRL_FAN   = 0; 

byte upgrade_eep_for_Modbus = 0;     
 
byte stsMotorsR_buff[3] = {0, 0, 0};
byte stsMotorsF_buff[3] = {0, 0, 0};
byte cnt_status_motors  = 0;

byte  cnt_no_link_Modbus = 0;

extern Byte connect_KTS;
extern int  cnt_sec_no_link_KTS;
extern Byte buff_ser1[DIM_MAX_BUFF_SER1];     // buffer di appoggio
extern SDATA  sData;
extern S_CLOCK_WEEK    sClockWeek;
extern Byte buff_ser2[DIM_MAX_BUFF_SER2];   // buffer di appoggio

//-----------  stato rele' unita ---------------------
extern Byte PowerMotors;

extern unsigned long countHoursFunz;
extern short  Temp_modbus[4];

extern Bool Tcheck;

extern short flow_motor_1;  //FLW 1
extern short flow_motor_2;  //FLW 2

extern short S1_T;
extern short S1_RH;
extern short S2_T;
extern short S2_RH;
extern short S4_T;
extern short S4_RH;

extern short T1;
extern short T2;
extern short T3;
extern short T4;
extern short T5;

char test_buff[60];

extern short Pressure_CAPS;
extern short Pressure_CAPR;
extern short Airflow_CAPS;
extern short Airflow_CAPR;

extern uint8_t BPD_FW;

bool RD_Clock_Set = false;
extern uint8_t Reset_Log_Buffer[512];
uint8_t Log_Index = 0;
uint32_t year1;

extern volatile uint32_t KTS_10min_Counter;
extern float BPD_Current;
extern Byte RotaBypass;

extern volatile int count_blink_led5;

extern struct {
	uint16_t CH0;
	uint16_t CH1;
	float CH0_Value;
	float CH1_Value;
	bool State;
	bool Start_Conversion;
} ADS7142_IC9;

extern int Read_Message_Ser1();
extern void ActiveBuzzer(byte numBeepBeep, unsigned int freq_100ms);
extern void Active_Procedure_Bypass_OpCl(Byte direct_bpd, int SecondsWait);
extern void TAG_DigitAccessoryOn(int nAcc_I2C);
extern void Write_Message_Ser1(byte *buff);
extern int update_data_eeprom(unsigned short start_addr, unsigned short end_addr, Byte *buff);
extern unsigned long millis( void );
extern unsigned short GetMaxAirFlow();
extern void TAG_DigitAccessoryOff(int nAcc_I2C);

//#ifdef __cplusplus
//extern "C" {
//#endif
	extern int Read_Message_Ser2(void);
	extern void Send_Message_Ser2(Byte *pBuff, int len);
	extern void free_serial_2();
//#ifdef __cplusplus
//}
//#endif

//---------------------------------------------------------------
//  intercepts messages from the serial 1 (only KTS)
//---------------------------------------------------------------
int intercepts_messages_serial_1()
{
   byte sts, buff_temp[18];
   int val_ret, i, c, nb;
   unsigned short addr_start[4], addr_end[4], addr1, addr2, val_16;
   signed short stemp; 
   unsigned int freq_100ms;
   byte numBeepBeep;
   int flag;
   
   wdt_restart(WATCH_DOG);
   
   val_ret = Read_Message_Ser1();
        
   if(val_ret > RUN_DOWNLOAD) 
   {
     
      if(!connect_KTS) {
        ActiveBuzzer(2, 3); // 2 beep-beep con freq. 300m
        connect_KTS = 1;
        RemoveAlarm(ALM_KTS_FAULT);  
      }  
      
      // è stato ricevuto un messaggio sul bus del KTS azzeriamo il contatore del reset del 12V
      cnt_sec_no_link_KTS = 0;  
      
      // Messaggi Buoni
      switch (buff_ser1[IHM1_TYPE_COMAND])
      {
         case  COMAND_TEST_UNIT:
             sData.timeout_test = 1860;   // al massimo stai 31 minuti
         
             switch(buff_ser1[IRQT_TYPE_TEST]) 
             {               
               case START_TEST:
                 //  messaggio di inizio test: 's', 'm', 8, 'T', 'S' ,'t', 'a', CK_16
                 sData.status_unit |= (1 << POS_BIT_IN_TESTING);
                 sData.status_test  = MSK_TEST_ENABLE; 
               break;               
               
               case END_TEST:                 
                 //  messaggio di   fine test: 's', 'm', 8, 'T', 'E' ,'n', 'd', CK_16
                 // inviamo il comando di chiusura test unità
                 sData.status_unit &= ~(1 << POS_BIT_IN_TESTING);
                 sData.status_test  = 0;                  
               break;
                             
               case TEST_BYPASS:
                 sData.status_unit |= (1 << POS_BIT_IN_TESTING); 
                 sData.status_test  |= (MSK_TEST_ENABLE | MSK_TEST_BYPASS); 
                 if(buff_ser1[IRQT_DATA_2_TEST] < 2)
                   buff_ser1[IRQT_DATA_2_TEST] = 2; // vengono passati i secondi di attesa
                 
                 if(buff_ser1[IRQT_DATA_1_TEST] == 'o')       // Open
                   Active_Procedure_Bypass_OpCl(OPEN_BPD,  buff_ser1[IRQT_DATA_2_TEST]);
                 else if (buff_ser1[IRQT_DATA_1_TEST] == 'c') // Close 
                   Active_Procedure_Bypass_OpCl(CLOSE_BPD, buff_ser1[IRQT_DATA_2_TEST]); 
                 else
                   sData.status_test  &= ~MSK_TEST_BYPASS; // disabilita lo stato di test Bypass                    
               break;
               
               case TEST_OUTPUT: 
                 sData.status_unit |= (1 << POS_BIT_IN_TESTING); 
                 sData.status_test  |= (MSK_TEST_ENABLE | MSK_TEST_OUTPUT); 
                 if(buff_ser1[IRQT_DATA_1_TEST] == 'A'){ // Relè in attrazione 
                    //digitalWrite(pOut1,   HIGH);
					ioport_set_pin_level(pOut1, IOPORT_PIN_LEVEL_HIGH);
                    //digitalWrite(pOut2,   HIGH);
					ioport_set_pin_level(pOut2, IOPORT_PIN_LEVEL_HIGH);
                    sData.StatusOutput |= 0x03;       
                 }else if (buff_ser1[IRQT_DATA_1_TEST] == 'S'){ // Spegni i Relè
                    //digitalWrite(pOut1,   LOW); 
					ioport_set_pin_level(pOut1, IOPORT_PIN_LEVEL_LOW);
                    //digitalWrite(pOut2,   LOW);
					ioport_set_pin_level(pOut2, IOPORT_PIN_LEVEL_LOW);
                    sData.StatusOutput &= ~0x03;  
                 }else
                    sData.status_test  &= ~MSK_TEST_OUTPUT; // disabilita lo stato di test OUTPUT  
               break;
               
               case TEST_BUZZER:
                  
                  sData.status_unit |= (1 << POS_BIT_IN_TESTING);
                  sData.status_test |= (MSK_TEST_ENABLE | MSK_TEST_BUZZER);
                  //   (8bit) numBeepBeep , (32bit) freq_100ms
                  numBeepBeep = buff_ser1[IRQT_DATA_1_TEST];
                  freq_100ms  = buff_ser1[IRQT_DATA_2_TEST];
                  
                  ActiveBuzzer(numBeepBeep, freq_100ms);
               break;
               
               case TEST_IPEHD: 
                  sData.status_unit |= (1 << POS_BIT_IN_TESTING);
                 
                  if(buff_ser1[IRQT_DATA_1_TEST] == 'A'){        
                    sData.status_test  |= (MSK_TEST_ENABLE | MSK_TEST_IPEHD);
                    TAG_DigitAccessoryOn(ACC_I2C_PREHEATER);   
                  }else {
                     // SPEGNI
                    sData.status_test  &= ~MSK_TEST_IPEHD; // disabilita lo stato di test OUTPUT
                    TAG_DigitAccessoryOff(ACC_I2C_PREHEATER);                      
                  }   
               break;
               
               case TEST_LEDS:  
                  sData.status_unit |= (1 << POS_BIT_IN_TESTING);
                  
                  if(buff_ser1[IRQT_DATA_1_TEST] == 'C') {         // CAMBIA stato Ai LEDS
                    sData.status_test |= (MSK_TEST_ENABLE | MSK_TEST_LEDS);  
                    sData.data_test    = buff_ser1[IRQT_DATA_2_TEST];                  
                  }else 
                    sData.status_test  &= ~MSK_TEST_LEDS; // disabilita lo stato di test OUTPUT                                          
               break;     
       
               case TEST_BURN_IN:
               case TEST_RESULT:
                   sData.status_test |= MSK_TEST_ENABLE;
               break;         
             
             }  
           
            // il messaggio di risposta è identico a quello della richiesta 
            if(sData.status_test)
              Write_Message_Ser1(buff_ser1); 
         break;
         
         case  COMAND_DATA_DEBUG:
            buff_ser1[IHM1_POS_CRC_LO]   = IRSD_CRC_LO;
            buff_ser1[IHM1_TYPE_COMAND]  = COMAND_DATA_DEBUG;
            // Debug Accessory CAP
            buff_ser1[IRSD_LEV_LINK_CAP] = sData.AccI2C[ACC_I2C_PCAP].level_link;   // livello di comunicazione tra UnitÃ  e Accesorio CAP 0/10 d
            buff_ser1[IRSD_MEASUR_PA_LO] = lowByte(sData.measure_pressure);
            buff_ser1[IRSD_MEASUR_PA_HI] = highByte(sData.measure_pressure);           
            // Debug Motori: Vengono visionati lo status, le velocità e il pilotaggio 0-10V
			buff_ser1[IRSD_LEV_LINK_CTRL_FAN] = sData.level_link_CTRLFAN;                       // livello di comunicazione tra modulo rd2. e CTRL_FAN 0/10
            buff_ser1[IRSD_STATUS_MOTORS_R]  =  sData.StatusMotorsR;                            // bit.. 7.none | 6.AlarmMR3 | 5.AlarmMR2 | 4.AlarmMR1 | 3.Err_V_Speed_R | 2.PowerMR3 | 1.PowerMR2 | 0.PowerMR1 
            buff_ser1[IRSD_STATUS_MOTORS_F]  =  sData.StatusMotorsF;                            // bit.. 7.none | 6.AlarmMF3 | 5.AlarmMF2 | 4.AlarmMF1 | 3.Err_V_Speed_F  | 2.PowerMF3 | 1.PowerMF2 | 0.PowerMF1 
            buff_ser1[IRSD_FEEDBK_VOUT_R_LO] = lowByte(sData.feedbk_v_speed_motors_R);          //  valore di tensione letto in uscita del pilota ventole F: da 0.0V a 11.00V
            buff_ser1[IRSD_FEEDBK_VOUT_R_HI] = highByte(sData.feedbk_v_speed_motors_R);               
            buff_ser1[IRSD_FEEDBK_VOUT_F_LO] = lowByte(sData.feedbk_v_speed_motors_F);                
            buff_ser1[IRSD_FEEDBK_VOUT_F_HI] = highByte(sData.feedbk_v_speed_motors_F);
           
            buff_ser1[IRSD_RPM_MOTOR_R1_LO] = lowByte(sData.rpm_motors_R[0]);
            buff_ser1[IRSD_RPM_MOTOR_R1_HI] = highByte(sData.rpm_motors_R[0]);             
            buff_ser1[IRSD_RPM_MOTOR_F1_LO] = lowByte(sData.rpm_motors_F[0]);
            buff_ser1[IRSD_RPM_MOTOR_F1_HI] = highByte(sData.rpm_motors_F[0]);  
            
            buff_ser1[IRSD_RPM_MOTOR_R2_LO] = lowByte(sData.rpm_motors_R[1]);
            buff_ser1[IRSD_RPM_MOTOR_R2_HI] = highByte(sData.rpm_motors_R[1]);             
            buff_ser1[IRSD_RPM_MOTOR_F2_LO] = lowByte(sData.rpm_motors_F[1]);
            buff_ser1[IRSD_RPM_MOTOR_F2_HI] = highByte(sData.rpm_motors_F[1]); 
            
            buff_ser1[IRSD_RPM_MOTOR_R3_LO] = lowByte(sData.rpm_motors_R[2]);
            buff_ser1[IRSD_RPM_MOTOR_R3_HI] = highByte(sData.rpm_motors_R[2]);             
            buff_ser1[IRSD_RPM_MOTOR_F3_LO] = lowByte(sData.rpm_motors_F[2]);
            buff_ser1[IRSD_RPM_MOTOR_F3_HI] = highByte(sData.rpm_motors_F[2]);             
            
                        
            // debug CAF
            buff_ser1[IRSD_MEASURE_AIRFLOW_LO] = lowByte(sData.measure_airflow);
            buff_ser1[IRSD_MEASURE_AIRFLOW_HI] = highByte(sData.measure_airflow);
            
            // debug Accessory clima
            buff_ser1[IRSD_LEV_LINK_PREHEATER] = sData.AccI2C[ACC_I2C_PREHEATER].level_link;   // 0/10 Level
            buff_ser1[IRSD_LEV_LINK_HEATER]    = sData.AccI2C[ACC_I2C_HEATER].level_link;      // 0/10 Level
            buff_ser1[IRSD_LEV_LINK_COOLER]    = sData.AccI2C[ACC_I2C_COOLER].level_link;      // 0/10 Level
                          
            /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - *
               byte STATUS da inviare al KTS:   
                                 bit.4: 1=IN_ALLARME, 0=NO ALLARMI,  
                                 bit.3: [Comando da Unità(1 ACCENDITI/SPEGNITI)]  
                                 bit.2: ON=1/OFF=0, 
                                 bit.1: AIR=1/WATER=0, 
                                 bit.0: CONNECT       
             * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  */    
            sts = 0;
            if(sData.AccI2C[ACC_I2C_HEATER].sts & STS_ACC_CONNECT)   sts |= 0x01;                    
            if(sData.AccI2C[ACC_I2C_HEATER].sts & STS_ACC_ELECT)     sts |= 0x02;                      
            if((sData.AccI2C[ACC_I2C_HEATER].sts & STS_ACC_ON) /*|| (read_byte_eeprom(ADDR_EEP(Set_EHD_mod)) > 0) */) sts |= 0x04;   // nuovo SSR monitorato
            if(sData.AccI2C[ACC_I2C_HEATER].status_comand & CMD_ON)  sts |= 0x08;               
            if(sData.Events[0])                                      sts |= 0x10;
            buff_ser1[IRSD_STATUS_HEATER]    = sts;        
     
            sts = 0;               
            if(sData.AccI2C[ACC_I2C_COOLER].sts & STS_ACC_CONNECT)    sts |= 0x01;                                       
            if(sData.AccI2C[ACC_I2C_COOLER].sts & STS_ACC_ON)         sts |= 0x04;                 
            if(sData.AccI2C[ACC_I2C_COOLER].status_comand & CMD_ON)   sts |= 0x08;
            if(sData.Events[1])                                       sts |= 0x10;
            buff_ser1[IRSD_STATUS_COOLER]    = sts;
          
            sts = 0;               
            if(sData.AccI2C[ACC_I2C_PREHEATER].sts & STS_ACC_CONNECT)   sts |= 0x01;                    
            if(sData.AccI2C[ACC_I2C_PREHEATER].sts & STS_ACC_ELECT)     sts |= 0x02;                      
            if(sData.AccI2C[ACC_I2C_PREHEATER].sts & STS_ACC_ON)        sts |= 0x04;                 
            if(sData.AccI2C[ACC_I2C_PREHEATER].status_comand & CMD_ON)  sts |= 0x08;
            if(sData.Events[2])                                         sts |= 0x10;
            buff_ser1[IRSD_STATUS_PREHEATER]  = sts;
            
            sts = 0;               
            if(sData.AccI2C[ACC_I2C_DSC].sts & STS_ACC_CONNECT)              sts |= 0x01;                                       
            if(sData.AccI2C[ACC_I2C_DSC].sts & STS_ACC_OPERATIVE)            sts |= 0x02;
            if(sData.AccI2C[ACC_I2C_DSC].sts & STS_ACC_ON)                   sts |= 0x04;
            if(sData.AccI2C[ACC_I2C_DSC].status_comand & CMD_ON)             sts |= 0x08;
            if(sData.Events[3])                                              sts |= 0x10;
            buff_ser1[IRSD_STATUS_DSC]    = sts;
            
            buff_ser1[IRSD_MEASURE_TW_HEAT_LO] = lowByte(sData.AccI2C[ACC_I2C_HEATER].measure2);   // 2 byte:  Temperature water se  l'accessory è HWD altrimenti dato inutile
            buff_ser1[IRSD_MEASURE_TW_HEAT_HI] = highByte(sData.AccI2C[ACC_I2C_HEATER].measure2);
            // Se non è configurata la sonda TempAir, passa il valore della TRet.
            if(sData.AccI2C[ACC_I2C_HEATER].info == 1){           
               buff_ser1[IRSD_MEASURE_TA_HEAT_LO] = lowByte(sData.AccI2C[ACC_I2C_HEATER].measure1);   // 2 byte:  Temperature air sia per HWD che EHD
               buff_ser1[IRSD_MEASURE_TA_HEAT_HI] = highByte(sData.AccI2C[ACC_I2C_HEATER].measure1);
            }else {
               buff_ser1[IRSD_MEASURE_TA_HEAT_LO] = lowByte(sData.measure_Temp[I_PROBE_RETURN]);
               buff_ser1[IRSD_MEASURE_TA_HEAT_HI] = highByte(sData.measure_Temp[I_PROBE_RETURN]);
            }  
            buff_ser1[IRSD_MEASURE_TW_COOL_LO] = lowByte(sData.AccI2C[ACC_I2C_COOLER].measure2);   // 2 byte:  Temperature water accessoy
            buff_ser1[IRSD_MEASURE_TW_COOL_HI] = highByte(sData.AccI2C[ACC_I2C_COOLER].measure2);
            
            buff_ser1[IRSD_MEASURE_TA_COOL_LO] = lowByte(sData.AccI2C[ACC_I2C_COOLER].measure1);   // 2 byte:  Temperature air CWD
            buff_ser1[IRSD_MEASURE_TA_COOL_HI] = highByte(sData.AccI2C[ACC_I2C_COOLER].measure1);
           
            buff_ser1[IRSD_MEASURE_TW_PREHEAT_LO] = lowByte(sData.AccI2C[ACC_I2C_PREHEATER].measure2);   // 2 byte:  Temperature water se  l'accessory è HWD altrimenti dato inutile
            buff_ser1[IRSD_MEASURE_TW_PREHEAT_HI] = highByte(sData.AccI2C[ACC_I2C_PREHEATER].measure2);            
            buff_ser1[IRSD_MEASURE_TA_PREHEAT_LO] = lowByte(sData.AccI2C[ACC_I2C_PREHEATER].measure1);   // 2 byte:  Temperature air sia per HWD che EHD
            buff_ser1[IRSD_MEASURE_TA_PREHEAT_HI] = highByte(sData.AccI2C[ACC_I2C_PREHEATER].measure1);
                          
            buff_ser1[IRSD_MEASURE_ABSOR_BYPASS_MIN]    = lowByte(sData.absorption_bypass_min);
            buff_ser1[IRSD_MEASURE_ABSOR_BYPASS_MAX_LO] = lowByte(sData.absorption_bypass_max);
            buff_ser1[IRSD_MEASURE_ABSOR_BYPASS_MAX_HI] = highByte(sData.absorption_bypass_max);
            
            // Valore di comunicazione del sensore RH/CO2/VOC  
            if (sData.AccI2C[ACC_I2C_CO2_1].sts & STS_ACC_CONNECT) 
                buff_ser1[IRSD_LEV_LINK_I_SENSOR_AMB]   =  sData.AccI2C[ACC_I2C_CO2_1].level_link;  
            else if (sData.AccI2C[ACC_I2C_CO2_2].sts & STS_ACC_CONNECT)  
                buff_ser1[IRSD_LEV_LINK_I_SENSOR_AMB]   =  sData.AccI2C[ACC_I2C_CO2_2].level_link;    
            else if (sData.AccI2C[ACC_I2C_RH_1].sts & STS_ACC_CONNECT)  
                buff_ser1[IRSD_LEV_LINK_I_SENSOR_AMB]   =  sData.AccI2C[ACC_I2C_RH_1].level_link;    
            else if (sData.AccI2C[ACC_I2C_RH_2].sts & STS_ACC_CONNECT)  
                buff_ser1[IRSD_LEV_LINK_I_SENSOR_AMB]   =  sData.AccI2C[ACC_I2C_RH_2].level_link;  
            else if (sData.AccI2C[ ACC_I2C_VOC].sts & STS_ACC_CONNECT)  
                buff_ser1[IRSD_LEV_LINK_I_SENSOR_AMB]   =  sData.AccI2C[ ACC_I2C_VOC].level_link;    
                
            buff_ser1[IRSD_LEV_LINK_MODBUS]  =  sData.level_link_ModBus;      // Valore di comunicazione del Modbus
			
			//debug Caps Capr
			buff_ser1[IRSD_MEASUR_PA_CAPS_LO] = lowByte(Pressure_CAPS);
			buff_ser1[IRSD_MEASUR_PA_CAPS_HI] = highByte(Pressure_CAPS);
			buff_ser1[IRSD_MEASUR_PA_CAPR_LO] = lowByte(Pressure_CAPR);
			buff_ser1[IRSD_MEASUR_PA_CAPR_HI] = highByte(Pressure_CAPR);
			buff_ser1[IRSD_MEASUR_AF_CAPS_LO] = lowByte(Airflow_CAPS);
			buff_ser1[IRSD_MEASUR_AF_CAPS_HI] = highByte(Airflow_CAPS);
			buff_ser1[IRSD_MEASUR_AF_CAPR_LO] = lowByte(Airflow_CAPR);
			buff_ser1[IRSD_MEASUR_AF_CAPR_HI] = highByte(Airflow_CAPR);
			buff_ser1[IRSD_LEV_LINK_CAPR] = sData.AccI2C[ACC_I2C_FLW2].level_link;       // 0/10 Level
			buff_ser1[IRSD_LEV_LINK_CAPS] = sData.AccI2C[ACC_I2C_FLW1].level_link;      // 0/10 Level
			
			sts = 0;
			if(sData.AccI2C[ACC_I2C_FLW1].sts & STS_ACC_CONNECT)    sts |= 0x01;
			if(sData.AccI2C[ACC_I2C_FLW1].sts & STS_ACC_ON)         sts |= 0x04;
			if(sData.AccI2C[ACC_I2C_FLW1].status_comand & CMD_ON)   sts |= 0x08;
			if(sData.Events[12])                                    sts |= 0x10;
			buff_ser1[IRSD_STATUS_CAPS]    = sts;
			
			sts = 0;
			if(sData.AccI2C[ACC_I2C_FLW2].sts & STS_ACC_CONNECT)    sts |= 0x01;
			if(sData.AccI2C[ACC_I2C_FLW2].sts & STS_ACC_ON)         sts |= 0x04;
			if(sData.AccI2C[ACC_I2C_FLW2].status_comand & CMD_ON)   sts |= 0x08;
			if(sData.Events[12])                                    sts |= 0x10;
			buff_ser1[IRSD_STATUS_CAPR]    = sts;
			
			//debug AIR SENSOR 1 an 2
			buff_ser1[IRSD_MEASUR_CO2_1_LO] = lowByte(sData.AccI2C[ACC_I2C_CO2_1].measure1);
			buff_ser1[IRSD_MEASUR_CO2_1_HI] = highByte(sData.AccI2C[ACC_I2C_CO2_1].measure1);
			buff_ser1[IRSD_MEASUR_VOC_1_LO] = lowByte(sData.AccI2C[ACC_I2C_VOC].measure1);
			buff_ser1[IRSD_MEASUR_VOC_1_HI] = highByte(sData.AccI2C[ACC_I2C_VOC].measure1);
			buff_ser1[IRSD_MEASUR_RH_1] = lowByte(sData.AccI2C[ACC_I2C_RH_1].measure1);
			buff_ser1[IRSD_MEASUR_CO2_2_LO] = lowByte(sData.AccI2C[ACC_I2C_CO2_2].measure1);
			buff_ser1[IRSD_MEASUR_CO2_2_HI] = highByte(sData.AccI2C[ACC_I2C_CO2_2].measure1);
			buff_ser1[IRSD_MEASUR_RH_2] = lowByte(sData.AccI2C[ACC_I2C_RH_2].measure1);
			buff_ser1[IRSD_FIRM_VER_CO2_1] = lowByte(sData.AccI2C[ACC_I2C_CO2_1].measure3);
			buff_ser1[IRSD_FIRM_VER_CO2_2] = lowByte(sData.AccI2C[ACC_I2C_CO2_2].measure3);
			buff_ser1[IRSD_LEV_LINK_CO2_1] = sData.AccI2C[ACC_I2C_CO2_1].level_link;
			buff_ser1[IRSD_LEV_LINK_CO2_2] = sData.AccI2C[ACC_I2C_CO2_2].level_link;
			buff_ser1[IRSD_MEASUR_IAQ_1_LO] = (sData.AccI2C[ACC_I2C_CO2_1].measure2 >> 2) & 0XFF;
			buff_ser1[IRSD_MEASUR_IAQ_1_HI] = (sData.AccI2C[ACC_I2C_CO2_1].measure2 >> 10) & 0x3F;
			buff_ser1[IRSD_MEASUR_IAQ_2_LO] = (sData.AccI2C[ACC_I2C_CO2_2].measure2 >> 2) & 0XFF;
			buff_ser1[IRSD_MEASUR_IAQ_2_HI] = (sData.AccI2C[ACC_I2C_CO2_2].measure2 >> 10) & 0x3F;
			
			sts = 0;
			if(sData.AccI2C[ACC_I2C_CO2_1].sts & STS_ACC_CONNECT)    sts |= 0x01;
			if(sData.AccI2C[ACC_I2C_CO2_1].sts & STS_ACC_OPERATIVE)  sts |= 0x02;
			if(sData.AccI2C[ACC_I2C_CO2_1].sts & STS_ACC_ON)         sts |= 0x04;
			if(sData.AccI2C[ACC_I2C_CO2_1].status_comand & CMD_ON)   sts |= 0x08;
			if(sData.Events[3])                                      sts |= 0x10;
			buff_ser1[IRSD_STATUS_CO2_1]    = sts;
			
			sts = 0;
			if(sData.AccI2C[ACC_I2C_CO2_2].sts & STS_ACC_CONNECT)    sts |= 0x01;
			if(sData.AccI2C[ACC_I2C_CO2_2].sts & STS_ACC_OPERATIVE)  sts |= 0x02;
			if(sData.AccI2C[ACC_I2C_CO2_2].sts & STS_ACC_ON)         sts |= 0x04;
			if(sData.AccI2C[ACC_I2C_CO2_2].status_comand & CMD_ON)   sts |= 0x08;
			if(sData.Events[3])                                      sts |= 0x10;
			buff_ser1[IRSD_STATUS_CO2_2]    = sts;
			
			sts = 0;
			if(sData.AccI2C[ACC_I2C_RH_1].sts & STS_ACC_CONNECT)    sts |= 0x01;
			if(sData.AccI2C[ACC_I2C_RH_1].sts & STS_ACC_OPERATIVE)  sts |= 0x02;
			if(sData.AccI2C[ACC_I2C_RH_1].sts & STS_ACC_ON)         sts |= 0x04;
			if(sData.AccI2C[ACC_I2C_RH_1].status_comand & CMD_ON)   sts |= 0x08;
			if(sData.Events[4])                                     sts |= 0x10;
			buff_ser1[IRSD_STATUS_RH1]    = sts;
			
			sts = 0;
			if(sData.AccI2C[ACC_I2C_RH_2].sts & STS_ACC_CONNECT)    sts |= 0x01;
			if(sData.AccI2C[ACC_I2C_RH_2].sts & STS_ACC_OPERATIVE)  sts |= 0x02;
			if(sData.AccI2C[ACC_I2C_RH_2].sts & STS_ACC_ON)         sts |= 0x04;
			if(sData.AccI2C[ACC_I2C_RH_2].status_comand & CMD_ON)   sts |= 0x08;
			if(sData.Events[4])                                     sts |= 0x10;
			buff_ser1[IRSD_STATUS_RH2]    = sts;
			
			sts = 0;
			if(sData.AccI2C[ACC_I2C_VOC].sts & STS_ACC_CONNECT)    sts |= 0x01;
			if(sData.AccI2C[ACC_I2C_VOC].sts & STS_ACC_OPERATIVE)  sts |= 0x02;
			if(sData.AccI2C[ACC_I2C_VOC].sts & STS_ACC_ON)         sts |= 0x04;
			if(sData.AccI2C[ACC_I2C_VOC].status_comand & CMD_ON)   sts |= 0x08;
			if(sData.Events[5])                                    sts |= 0x10;
			buff_ser1[IRSD_STATUS_VOC]    = sts;
			
			buff_ser1[IRSD_ACCURACY_CO2_1] = (sData.AccI2C[ACC_I2C_CO2_1].measure2) & 0x3; // CO2 accuracy
			buff_ser1[IRSD_ACCURACY_CO2_1] |= ((sData.AccI2C[ACC_I2C_VOC].measure2) << 2) & 0xC; // VOC accuracy
			buff_ser1[IRSD_ACCURACY_CO2_1] |= ((sData.AccI2C[ACC_I2C_RH_1].measure2) << 4) & 0x30; // Humidity Accuracy
			
			buff_ser1[IRSD_ACCURACY_CO2_2] = (sData.AccI2C[ACC_I2C_CO2_2].measure2) & 0x3; // CO2 accuracy
			buff_ser1[IRSD_ACCURACY_CO2_2] |= ((sData.AccI2C[ACC_I2C_RH_2].measure2) << 4) & 0x30; // Humidity Accuracy
			
			buff_ser1[IRSD_FIRM_VER_CAPS_LO] = sData.AccI2C[ACC_I2C_FLW1].measure3 & 0xFF;
			buff_ser1[IRSD_FIRM_VER_CAPS_HI] = (sData.AccI2C[ACC_I2C_FLW1].measure3 >> 8) & 0xFF;
			buff_ser1[IRSD_FIRM_VER_CAPR_LO] = sData.AccI2C[ACC_I2C_FLW2].measure3 & 0xFF;
			buff_ser1[IRSD_FIRM_VER_CAPR_HI] = (sData.AccI2C[ACC_I2C_FLW2].measure3 >> 8) & 0xFF;
			buff_ser1[IRSD_FIRM_VER_PREHEATER_LO] = sData.AccI2C[ACC_I2C_PREHEATER].measure3 & 0xFF;
			buff_ser1[IRSD_FIRM_VER_PREHEATER_HI] = (sData.AccI2C[ACC_I2C_PREHEATER].measure3 >> 8) & 0xFF;
			buff_ser1[IRSD_FIRM_VER_HEATER_LO] = sData.AccI2C[ACC_I2C_HEATER].measure3 & 0xFF;
			buff_ser1[IRSD_FIRM_VER_HEATER_HI] = (sData.AccI2C[ACC_I2C_HEATER].measure3 >> 8) & 0xFF;
			buff_ser1[IRSD_FIRM_VER_COOLER_LO] = sData.AccI2C[ACC_I2C_COOLER].measure3 & 0xFF;
			buff_ser1[IRSD_FIRM_VER_COOLER_HI] = (sData.AccI2C[ACC_I2C_COOLER].measure3 >> 8) & 0xFF;
			buff_ser1[IRSD_FIRM_VER_CAP_LO] = sData.AccI2C[ACC_I2C_PCAP].measure3 & 0xFF;
			buff_ser1[IRSD_FIRM_VER_CAP_HI] = (sData.AccI2C[ACC_I2C_PCAP].measure3 >> 8) & 0xFF;
			buff_ser1[IRSD_FIRM_VER_CAF_LO] = sData.AccI2C[ACC_I2C_PCAF].measure3 & 0xFF;
			buff_ser1[IRSD_FIRM_VER_CAF_HI] = (sData.AccI2C[ACC_I2C_PCAF].measure3 >> 8) & 0xFF;
			buff_ser1[IRSD_FIRM_VER_BPD] = BPD_FW;
			
			buff_ser1[IRSD_HEATER_VOLTAGE_LO] = sData.AccI2C[ACC_I2C_HEATER].measure4 & 0xFF;
			buff_ser1[IRSD_HEATER_VOLTAGE_HI] = (sData.AccI2C[ACC_I2C_HEATER].measure4 >> 8) & 0xFF;
			buff_ser1[IRSD_HEATER_CURRENT_LO] = sData.AccI2C[ACC_I2C_HEATER].measure5 & 0xFF;
			buff_ser1[IRSD_HEATER_CURRENT_HI] = (sData.AccI2C[ACC_I2C_HEATER].measure5 >> 8) & 0xFF;
			
			buff_ser1[IRSD_HEATER_INFO] = sData.AccI2C[ACC_I2C_HEATER].info;
			
            // invia il messaggio al KTS
            Write_Message_Ser1(buff_ser1);
         break;
         
         case  COMAND_POLLING_BASE:
             // Sincronizza l'orologio Settimanale per il WEEKLY, una diff. di N secondi non sono importanti.
             if((sClockWeek.minutes != buff_ser1[IRQP_DATA_MIN])     ||  
                (sClockWeek.weekday != buff_ser1[IRQP_DATA_WEEKDAY]) || 
                (sClockWeek.hours  != buff_ser1[IRQP_DATA_HOURS])) 
             {
                 sClockWeek.seconds = buff_ser1[IRQP_DATA_SEC];        
                 sClockWeek.minutes = buff_ser1[IRQP_DATA_MIN];      
                 sClockWeek.hours   = buff_ser1[IRQP_DATA_HOURS];
                 sClockWeek.weekday = buff_ser1[IRQP_DATA_WEEKDAY];    // 0=Lunedi, 1= Martedi,....,6=Domenica
                 sClockWeek.Day = buff_ser1[IRQP_DATA_DAY];
                 sClockWeek.Mounth = buff_ser1[IRQP_DATA_MOUNTH];
                 sClockWeek.Year = buff_ser1[IRQP_DATA_YEAR];
                 sClockWeek.Century = buff_ser1[IRQP_DATA_CENTURY];
				 
             }
			 
             if ( RD_Clock_Set == false ) // set RD RTC
			 {
				 //rtc_clear_status(RTC, 0);
				 RD_Clock_Set = true;
				 rtc_set_time(RTC, sClockWeek.hours, sClockWeek.minutes, sClockWeek.seconds);
				 sClockWeek.Century = 20;
				 year1 = 0;
				 year1 = sClockWeek.Year;
				 year1 = year1 + (sClockWeek.Century * 100);
				 rtc_set_date(RTC, year1, sClockWeek.Mounth, sClockWeek.Day, sClockWeek.weekday);
			 }
			 
			 KTS_10min_Counter = 0;
			             
             //------------------ Costruiamo il messaggio di risposta -----------------------------------
             buff_ser1[IHM1_POS_CRC_LO]        = IRSP_CRC_LO;
             buff_ser1[IHM1_TYPE_COMAND]       = COMAND_POLLING_BASE;   
              
             buff_ser1[IRSP_MEASURE_TEMP_1_LO] = lowByte(sData.measure_Temp[I_PROBE_FRESH]);
             buff_ser1[IRSP_MEASURE_TEMP_1_HI] = highByte(sData.measure_Temp[I_PROBE_FRESH]);

             
             // se presente AWP passo al KTS non la TReturn, ma la temperatura dell'ambiente interno
             if(DigitAccessoryOperating(ACC_I2C_AWP)) {
                buff_ser1[IRSP_MEASURE_TEMP_2_LO] = lowByte(sData.AccI2C[ACC_I2C_AWP].measure2);
                buff_ser1[IRSP_MEASURE_TEMP_2_HI] = highByte(sData.AccI2C[ACC_I2C_AWP].measure2); 
             }else {
                buff_ser1[IRSP_MEASURE_TEMP_2_LO] = lowByte(sData.measure_Temp[I_PROBE_RETURN]);
                buff_ser1[IRSP_MEASURE_TEMP_2_HI] = highByte(sData.measure_Temp[I_PROBE_RETURN]);
             }  
                                
             buff_ser1[IRSP_MEASURE_TEMP_3_LO] = lowByte(sData.measure_Temp[I_PROBE_SUPPLY]);
             buff_ser1[IRSP_MEASURE_TEMP_3_HI] = highByte(sData.measure_Temp[I_PROBE_SUPPLY]);
             
             buff_ser1[IRSP_MEASURE_TEMP_4_LO] = lowByte(sData.measure_Temp[I_PROBE_EXHAUST]);
             buff_ser1[IRSP_MEASURE_TEMP_4_HI] = highByte(sData.measure_Temp[I_PROBE_EXHAUST]);
        
             // valore espresso in % da 0% a 100%  
             for(i=0; i <2; i++){
               val_16 = sData.measure_ADC_input[i] / 10;
               if(val_16 > 100)
                 val_16 = 100;
               buff_ser1[IRSP_MEASURE_IN1 + i]  = (byte)val_16;
             }

             // Passo il tipo di sonda: se vi sono sonde a canale o ambiente
             buff_ser1[IRSP_INFO_PROBE_AD]       = sData.info_ProbesAD;               //  7.2CO2  6.P1CO2  5.----  4.P2RH  3.P1RH  2.----  1.P1VOC  0.----
                                                                                      // 0 = Ambiente, 1 = Duct
             buff_ser1[IRSP_MEASURE_RH_SENS]     = sData.measure_RH_max;              // da 1% a 99%    
             buff_ser1[IRSP_MEASURE_CO2_SENS_LO] = lowByte(sData.measure_CO2_max);    // valori da 0 a 2000 PPM    
             buff_ser1[IRSP_MEASURE_CO2_SENS_HI] = highByte(sData.measure_CO2_max);                   
             buff_ser1[IRSP_MEASURE_VOC_SENS_LO] = lowByte(sData.measure_VOC_max);    // da 0 a 10000 ug/mq
             buff_ser1[IRSP_MEASURE_VOC_SENS_HI] = highByte(sData.measure_VOC_max);    
             // stati    
             buff_ser1[IRSP_STATUS_UNIT_LO]      = lowByte(sData.status_unit);
             buff_ser1[IRSP_STATUS_UNIT_HI]      = highByte(sData.status_unit);                                        
             buff_ser1[IRSP_STATUS_WEEKLY]       = sData.status_weekly;
             
             for(i=0; i <13; i++) 
                 buff_ser1[IRSP_EVENT_BYTE_00 + i] = sData.Events[i];
             
             // 2 byte: a disposizione, mettiamo a zero il livello
             sts = 0;               
             if(sData.AccI2C[ACC_I2C_DSC].sts & STS_ACC_CONNECT)              sts |= 0x01;                                       
             if(sData.AccI2C[ACC_I2C_DSC].sts & STS_ACC_OPERATIVE)            sts |= 0x02;
             if(sData.AccI2C[ACC_I2C_DSC].sts & STS_ACC_ON)                   sts |= 0x04;
             if(sData.AccI2C[ACC_I2C_DSC].status_comand & CMD_ON)             sts |= 0x08;
             if(sData.Events[3])                                              sts |= 0x10;
             buff_ser1[IRSP_NONE_0] = sts;                    
             buff_ser1[IRSP_INCREASE_SPEED_RH_CO2] = sData.increase_speed_RH_CO2;  
             
             // passiamo lo stato di aggiornamento delle varie arie di eeprom. 
             buff_ser1[IRSP_CNT_UPDATE_EEP_INFO]        = read_byte_eeprom(ADDR_EEP(cntUpdate_info));
             buff_ser1[IRSP_CNT_UPDATE_EEP_SETTING_PAR] = read_byte_eeprom(ADDR_EEP(cntUpdate_SettingPar));
             buff_ser1[IRSP_CNT_UPDATE_EEP_SETP_TEMP]   = read_byte_eeprom(ADDR_EEP(cntUpdate_SetTemp));
             buff_ser1[IRSP_CNT_UPDATE_EEP_WEEKLY]      = read_byte_eeprom(ADDR_EEP(cntUpdate_dayProg));  
             buff_ser1[IRSP_MEASURE_AWP_SENS_LO]        = lowByte(sData.measure_AWP_max);    
             buff_ser1[IRSP_MEASURE_AWP_SENS_HI]        = highByte(sData.measure_AWP_max);
			 
			 buff_ser1[IRSP_KTS_CURRENT] = sData.measure_I_KTS;
			 buff_ser1[IRSP_SPEED_MOTOR_R] = sData.speed_motors_R;
			 buff_ser1[IRSP_SPPED_MOTOR_F] = sData.speed_motors_F;
			 buff_ser1[IRSP_STATUS_LED_CTRL_FAN] = sData.status_Leds_ctrlFan;
			 buff_ser1[IRSP_BPD_CURRENT_LO] = (ushort)BPD_Current & 0xFF;
			 buff_ser1[IRSP_BPD_CURRENT_HI] = ((ushort)BPD_Current >> 8) & 0xFF;
			 buff_ser1[IRSP_BPD_ROTATE] = RotaBypass;
			 
			 uint16_t Temp_Voltage;
			 
			 if ( ADS7142_IC9.State)
			 {
				 Temp_Voltage = (uint16_t)( ADS7142_IC9.CH0_Value * 10 );
				 buff_ser1[IRSP_12V_LO] = lowByte(Temp_Voltage);
				 buff_ser1[IRSP_12V_HI] = highByte(Temp_Voltage);
				 Temp_Voltage = (uint16_t)( ADS7142_IC9.CH1_Value * 10 );
				 buff_ser1[IRSP_5V_LO] = lowByte(Temp_Voltage);
				 buff_ser1[IRSP_5V_HI] = highByte(Temp_Voltage);
			 }
			 else
			 {
				 buff_ser1[IRSP_12V_LO] = 0;
				 buff_ser1[IRSP_12V_HI] = 0;
				 buff_ser1[IRSP_5V_LO] = 0;
				 buff_ser1[IRSP_5V_HI] = 0;
			 }
			 
			 if ( Reset_Log_Buffer[0] ) // LOG not Empty
				 {
					 if ( Log_Index < Reset_Log_Buffer[0] )
					 {
						 buff_ser1[IRSP_NUM_RECORDS] = Reset_Log_Buffer[0];
						 buff_ser1[IRSP_INDEX_RECORD] = Log_Index;
						 buff_ser1[IRSP_TYPE_OF_RESET] = Reset_Log_Buffer[Log_Index*9 + 1];
						 buff_ser1[IRSP_LOG_YEAR_HIGH] = Reset_Log_Buffer[Log_Index*9 + 2];
						 buff_ser1[IRSP_LOG_YEAR_LOW] = Reset_Log_Buffer[Log_Index*9 + 3];
						 buff_ser1[IRSP_LOG_MONTH] = Reset_Log_Buffer[Log_Index*9 + 4];
						 buff_ser1[IRSP_LOG_DAY] = Reset_Log_Buffer[Log_Index*9 + 5];
						 buff_ser1[IRSP_LOG_HOURS_1] = Reset_Log_Buffer[Log_Index*9 + 6];
						 buff_ser1[IRSP_LOG_HOURS_2] = Reset_Log_Buffer[Log_Index*9 + 7];
						 buff_ser1[IRSP_LOG_HOURS_3] = Reset_Log_Buffer[Log_Index*9 + 8];
						 buff_ser1[IRSP_LOG_HOURS_4] = Reset_Log_Buffer[Log_Index*9 + 9];
						 Log_Index++;
					 }
					 else
					 {
						 Log_Index = 0;
						 buff_ser1[IRSP_NUM_RECORDS] = Reset_Log_Buffer[0];
						 buff_ser1[IRSP_INDEX_RECORD] = Log_Index;
						 buff_ser1[IRSP_TYPE_OF_RESET] = Reset_Log_Buffer[1];
						 buff_ser1[IRSP_LOG_YEAR_HIGH] = Reset_Log_Buffer[2];
						 buff_ser1[IRSP_LOG_YEAR_LOW] = Reset_Log_Buffer[3];
						 buff_ser1[IRSP_LOG_MONTH] = Reset_Log_Buffer[4];
						 buff_ser1[IRSP_LOG_DAY] = Reset_Log_Buffer[5];
						 buff_ser1[IRSP_LOG_HOURS_1] = Reset_Log_Buffer[6];
						 buff_ser1[IRSP_LOG_HOURS_2] = Reset_Log_Buffer[7];
						 buff_ser1[IRSP_LOG_HOURS_3] = Reset_Log_Buffer[8];
						 buff_ser1[IRSP_LOG_HOURS_4] = Reset_Log_Buffer[9];
						 Log_Index++;
					 }
				 }
			 else // LOG empty
				 {
					  buff_ser1[IRSP_NUM_RECORDS] = 0;
					  buff_ser1[IRSP_INDEX_RECORD] = 0;
					  buff_ser1[IRSP_TYPE_OF_RESET] = 0;
					  buff_ser1[IRSP_LOG_YEAR_HIGH] = 0;
					  buff_ser1[IRSP_LOG_YEAR_LOW] = 0;
					  buff_ser1[IRSP_LOG_MONTH] = 0;
					  buff_ser1[IRSP_LOG_DAY] = 0;
					  buff_ser1[IRSP_LOG_HOURS_1] = 0;
					  buff_ser1[IRSP_LOG_HOURS_2] = 0;
					  buff_ser1[IRSP_LOG_HOURS_3] = 0;
					  buff_ser1[IRSP_LOG_HOURS_4] = 0;
					  Log_Index = 0;
				 }
             
             // invia il messaggio al KTS
             Write_Message_Ser1(&buff_ser1[0]);
         break;
         
         
         case  COMAND_READ_EEPROM:
             addr1 = buff_ser1[IRQR_ADDR_BYTE_START_EEP];            // Indirizzo INIZIALE di memoria delle eeprom  che si vuole leggere  
             addr2 = addr1 + buff_ser1[IRQR_ADDR_NUM_BYTE_EEP] - 1;  // Ultimo Indirizzo di memoria delle eeprom  che si vuole leggere  
  
             //------- Costruiamo il messaggio di risposta ---------          
             buff_ser1[IHM1_POS_CRC_LO]  = IRSR_START_DATA_EEPROM + (addr2 - addr1) +1;
             buff_ser1[IHM1_TYPE_COMAND] = COMAND_READ_EEPROM;    
             
             for(i=0;  addr1 <= addr2; addr1++, i++) 
                 buff_ser1[IRSR_START_DATA_EEPROM + i] = read_byte_eeprom(addr1);
             
             // invia il messaggio al KTS
             Write_Message_Ser1(buff_ser1);           
         break;  
         
         case  COMAND_WRITE_EEPROM:
             
             addr1 = buff_ser1[IRQW_ADDR_BYTE_START_EEP];           // Indirizzo INIZIALE di memoria delle eeprom  che si vuole scrivere 
             addr2 = addr1 + buff_ser1[IRQW_ADDR_NUM_BYTE_EEP] -1;  // Ultimo Indirizzo di memoria delle eeprom che si vuole leggere
		
			 //memset ( test_buff, 0, sizeof(test_buff));
			 //sprintf ( test_buff, "addr1 : %d , addr2 : %d\r\n", addr1, addr2);
			 //Serial_write(test_buff, strlen ( test_buff )); 
             
             // prima di scrivere andiamo a verificare che l'area di memoria non sia protetta da scrittura, i byte protetti sono pochi e sono:       
             addr_start[0] = 0; 
             addr_end[0]   = ADDR_EEP(size2_free[1]);     
             
             addr_start[1] = ADDR_EEP(Set_Power_ON);       
             addr_end[1]   = ADDR_EEP(cntUpdate_SettingPar);
             
             addr_start[2] = ADDR_EEP(Bypass_minTempExt);
             addr_end[2]   = ADDR_EEP(cntUpdate_SetTemp);
             
             addr_start[3] = addr_end[2] + 1;          
             addr_end[3]   = ADDR_EEP(cntUpdate_dayProg);
             
			 c='1';
             for(i=0 ; i < 4; i++) {
                if((addr1 >= addr_start[i]) && (addr2 <= addr_end[i])) {
                   c = '0'; //Zona di scrittura valida
                   break;
                }   
             } 
             
            wdt_restart(WATCH_DOG);
			
            if(c == '0') {
               // Scriviamo in eeprom, ma solo dopo aver verificato che i valori inseriti non sono fuori limiti.
               flag = update_data_eeprom(addr1, addr2, &buff_ser1[IRQW_START_DATA_EEPROM]);   
               if(flag == -1) 
                 c ='2'; // '2': errore dati non validi
            }
			
			wdt_restart(WATCH_DOG);
			
              //------- Costruiamo il messaggio di risposta ---------          
            buff_ser1[IHM1_POS_CRC_LO]                 = IRSW_CRC_LO;
            buff_ser1[IHM1_TYPE_COMAND]                = COMAND_WRITE_EEPROM;  
            // '0': scrittura OK
            // '1': errore zona eeprom non valida
            // '2': errore dati non validi
            // '3': dispositivo occupato.  
            buff_ser1[IRSW_RESULT_W]                   = c;
            buff_ser1[IRSW_CNT_UPDATE_EEP_INFO]        = read_byte_eeprom(ADDR_EEP(cntUpdate_info));
            buff_ser1[IRSW_CNT_UPDATE_EEP_SETTING_PAR] = read_byte_eeprom(ADDR_EEP(cntUpdate_SettingPar));
            buff_ser1[IRSW_CNT_UPDATE_EEP_SETP_TEMP]   = read_byte_eeprom(ADDR_EEP(cntUpdate_SetTemp));
            buff_ser1[IRSW_CNT_UPDATE_EEP_WEEKLY]      = read_byte_eeprom(ADDR_EEP(cntUpdate_dayProg));  
          
            // invia il messaggio al KTS
            Write_Message_Ser1(buff_ser1);    
         break;
      }  
   }
   
   return val_ret;
}  

//---------------------------------------------------------------
//  intercepts messages from the serial 2
//---------------------------------------------------------------
void update_EB_RemoteCTRL(byte val_new_status)
{
    // Se c'è un cambiamento registra in eeprom
    byte val;
    byte nbyte = (ACC_EEP_MBUS >> 3);
    // List AccessoyHW[1]:    DPS, PCAF,   PCAP, -----,  ----,  ----,   RFM,  MBUS  (LSB) 
    val   = read_byte_eeprom(ADDR_EEP(AccessoyHW[nbyte]));
  
    if(val_new_status & 0x80) // modbus Fuction: Active
      val |= (1 << (ACC_EEP_MBUS & 0x07));
    else 
      val &= ~(1 << (ACC_EEP_MBUS & 0x07));  
    
    if(val_new_status & 0x40) // RF Module: Install
      val |= (1 << (ACC_EEP_RFM & 0x07));
    else 
      val &= ~(1 << (ACC_EEP_RFM & 0x07));  
    
    write_byte_eeprom(ADDR_EEP(AccessoyHW[nbyte]),  val);
    sData.Status_RemoteCTRL = val_new_status;  
  
}  


//---------------------------------------------------------------
//  intercepts messages from the serial 2
//---------------------------------------------------------------
#ifndef NO_TACHO
int intercepts_messages_serial_2()
{
   int val_ret = 0, len;
   unsigned short enab_func;
   byte i, n;
   byte msk_motors = 0x11;
   byte buff[128];
   byte addr_eep;
   unsigned short addr_start[4], addr_end[4], addr1, addr2, esitoWR;
   int flag;
      
   // 1: Andiamo ad analizzare se vi sono messaggi nel buffer
   val_ret = Read_Message_Ser2();
   
    
   if(val_ret > RUN_DOWNLOAD )  // if(val_ret > 0)
   {     
          
       switch (buff_ser2[IHM2_TYPE_COMAND])
       {        
           case  COMAND_POLLING_CTRL_FAN:             
              n = read_byte_eeprom(ADDR_EEP(numMotors));
              if(n== 4)
                msk_motors = 0x33;
              else if(n==6)
                msk_motors = 0x77;
              
              // err_V_speed_#    
              msk_motors |= 0x08;  
            
              // Status Motors R & F:  bit.. 7.none | 6.AlarmM3 | 5.AlarmM2 | 4.AlarmM1 | 3.Err_VSpeed | 2.PowerON_M3 | 1.PowerON_M2 | 0.PowerON_M1
              stsMotorsR_buff[0] = buff_ser2[IRS_CMD_F_STATUS_FAN_R] & msk_motors;                   
              stsMotorsF_buff[0] = buff_ser2[IRS_CMD_F_STATUS_FAN_F] & msk_motors;    
             
              // valore di tensioni lette in uscita che comandono le ventole R, F: da 0.00V a 11.00V il valore Ã¨ moltiplicato x 100. es: 500 = 5.00
              sData.feedbk_v_speed_motors_R =  buff_ser2[IRS_CMD_F_FEEDBK_VOUT_R_LO]; 
              sData.feedbk_v_speed_motors_R |= (unsigned short)(buff_ser2[IRS_CMD_F_FEEDBK_VOUT_R_HI] << 8);  
              
              sData.feedbk_v_speed_motors_F = buff_ser2[IRS_CMD_F_FEEDBK_VOUT_F_LO];
              sData.feedbk_v_speed_motors_F |= (unsigned short)(buff_ser2[IRS_CMD_F_FEEDBK_VOUT_F_HI] << 8);               
           
              // Lettura della 6 Speed Motors (rpm)
              sData.rpm_motors_R[0] = buff_ser2[IRS_CMD_F_RPM_FAN_R1_LO]; 
              sData.rpm_motors_R[0] |= (unsigned short)(buff_ser2[IRS_CMD_F_RPM_FAN_R1_HI] << 8);
              
              sData.rpm_motors_F[0] = buff_ser2[IRS_CMD_F_RPM_FAN_F1_LO]; 
              sData.rpm_motors_F[0] |= (unsigned short)(buff_ser2[IRS_CMD_F_RPM_FAN_F1_HI] << 8);  
             
              sData.rpm_motors_R[1] = buff_ser2[IRS_CMD_F_RPM_FAN_R2_LO];
              sData.rpm_motors_R[1] |= (unsigned short)(buff_ser2[IRS_CMD_F_RPM_FAN_R2_HI] << 8);
              
              sData.rpm_motors_F[1] = buff_ser2[IRS_CMD_F_RPM_FAN_F2_LO];
              sData.rpm_motors_F[1] |= (unsigned short)(buff_ser2[IRS_CMD_F_RPM_FAN_F2_HI] << 8);  
              
              sData.rpm_motors_R[2] =  buff_ser2[IRS_CMD_F_RPM_FAN_R3_LO]; 
              sData.rpm_motors_R[2] |= (unsigned short)(buff_ser2[IRS_CMD_F_RPM_FAN_R3_HI] << 8);
              
              sData.rpm_motors_F[2] = buff_ser2[IRS_CMD_F_RPM_FAN_F3_LO];
              sData.rpm_motors_F[2] |= (unsigned short)(buff_ser2[IRS_CMD_F_RPM_FAN_F3_HI] << 8);  

        
              cnt_resp_poll_CTRL_FAN++;   // incrementa conatatore di risposte ok di polling

              RemoveAlarm(ALM_EB_CTRL_FAN);
                                           
              // il cambiamento di stato deve essere = per 2 pollate consecutive
              if(n==2)
                  msk_motors = 0x01; 
              else if(n==4)
                  msk_motors = 0x03;  
              else
                  msk_motors = 0x07; 
                  
              // occorre confermare il dato per 3 volte prima di segnalare errori o altro.
              if ((stsMotorsR_buff[0] == stsMotorsR_buff[1]) && (stsMotorsR_buff[0] == stsMotorsR_buff[2])) 
                 sData.StatusMotorsR = stsMotorsR_buff[0] & 0xF7;         //(tolto err_V_speed_R --> 056 da problemi di impedenza)  06/06/2016
              else {
                 i = (ALM_MR1_NO_PWR >> 3);
                 sData.Events[i] = stsMotorsR_buff[0] & 0xF0;             // allarmi di no rotazione motore  (tolto err_V_speed_R --> 056 da problemi di impedenza)  06/06/2016
                 sData.Events[i] |= (~stsMotorsR_buff[0] & msk_motors);   // allarmi no POWER_MOTORS
              }   
                
              if ((stsMotorsF_buff[0] == stsMotorsF_buff[1]) && (stsMotorsF_buff[0] == stsMotorsF_buff[2])) 
                 sData.StatusMotorsF = stsMotorsF_buff[0] & 0xF7;          //(tolto err_V_speed_F --> 056 da problemi di impedenza)  06/06/2016
              else  {
                 i = (ALM_MF1_NO_PWR >> 3);
                 sData.Events[i] = stsMotorsF_buff[0] & 0xF0;              // allarmi di no rotazione motore (tolto err_V_speed_F --> 056 da problemi di impedenza)  06/06/2016 
                 sData.Events[i] |= (~stsMotorsF_buff[0] & msk_motors);    // allarmi no POWER_MOTORS
              }  
              
#ifdef _CLEAR_ALARMS_POWER_MOTORS
              sData.StatusMotorsF &= 0xF0;
              sData.StatusMotorsR &= 0xF0;              
#endif
  
			   enab_func = read_word_eeprom(ADDR_EEP(Enab_Fuction)); 
			   if (enab_func & (1 << ENAB_MBF))
				{
				 sData.StatusMotorsF &= 0xF0;
                 sData.StatusMotorsR &= 0xF0;
				}
  
              stsMotorsR_buff[2] = stsMotorsR_buff[1]; 
              stsMotorsR_buff[1] = stsMotorsR_buff[0]; 
              stsMotorsF_buff[2] = stsMotorsF_buff[1]; 
              stsMotorsF_buff[1] = stsMotorsF_buff[0]; 
           break;
           
                   
          case  COMAND_PACKET_1_DATA:    
              //    b7. 1= Modbus Active, b6 = RFM Install, ... b1              
              if(sData.Status_RemoteCTRL != buff_ser2[IPK1M_STATUS_REMOTE_CTRL]) 
              {
                  update_EB_RemoteCTRL(buff_ser2[IPK1M_STATUS_REMOTE_CTRL]);                 
              }  
         
              if(buff_ser2[IPK1M_SET_CLOCK] == 'S') {
                  sClockWeek.seconds = buff_ser2[IPK1M_CLOCK_HOURS];
                  sClockWeek.minutes = buff_ser2[IPK1M_CLOCK_MIN];
                  sClockWeek.hours   = buff_ser2[IPK1M_CLOCK_SEC];
                  sClockWeek.weekday = buff_ser2[IPK1M_CLOCK_DAYWEEK]; 
              }
            
              if(buff_ser2[IPK1M_CLEAN_FILTERS] == 'C') {
                  RemoveAlarm(ALM_FILTER);
                  for(i=0;  i < 4;  i++) 
                      write_byte_eeprom(ADDR_EEP(time_lastCloggedFilters[i]), read_byte_eeprom(ADDR_EEP(hour_runnig[i])));                
              }  
              
              if(read_byte_eeprom(ADDR_EEP(SW_Vers_ModBus[0])) != buff_ser2[IPK1M_VER_FW_1_CAR])
                  write_byte_eeprom(ADDR_EEP(SW_Vers_ModBus[0]),   buff_ser2[IPK1M_VER_FW_1_CAR]);
                  
              if(read_byte_eeprom(ADDR_EEP(SW_Vers_ModBus[1])) != buff_ser2[IPK1M_VER_FW_2_CAR])
                  write_byte_eeprom(ADDR_EEP(SW_Vers_ModBus[1]),   buff_ser2[IPK1M_VER_FW_2_CAR]);
                  
              cnt_no_link_Modbus = 0;  
                         
              RemoveAlarm(ALM_EB_REM_CTRL);      
          break;
                    
          
          case  COMAND_READ_EEPROM:
              // rispondiamo subito al modulo.
              if (buff_ser2[IRQ_RDE_NUM_BYTE] < 120) {
                  addr_eep = buff_ser2[IRQ_RDE_ADDR_EEP];               // Indirizzo INIZIALE di memoria delle eeprom  che si vuole leggere     
                  len = IRS_RDE_START_DATA_EEP + buff_ser2[IRQ_RDE_NUM_BYTE] + 2; // Conteggiamo anche i 2 Byte dei Cksum                  
                  //------- Costruiamo il messaggio di risposta ---------    
                  buff[IHM2_POS_CRC_LO]     = (len - 2);
                  buff[IHM2_ADDR_RECEIVER]  = ADDR_REM_CTRL;
                  buff[IHM2_TYPE_COMAND]    = COMAND_READ_EEPROM;    
                  buff[IRS_RDE_ADDR_EEP]    = addr_eep;
                  buff[IRS_RDE_NUM_BYTE]    = buff_ser2[IRQ_RDE_NUM_BYTE];
             
                  for(i=0;  i < buff_ser2[IRQ_RDE_NUM_BYTE]; addr_eep++, i++) 
                     buff[IRS_RDE_START_DATA_EEP + i] = read_byte_eeprom(addr_eep);
                     
                  Send_Message_Ser2(&buff[0], len);   
                               
             }
             
             cnt_no_link_Modbus = 0;  
             
             RemoveAlarm(ALM_EB_REM_CTRL);   
          break;
          
          case  COMAND_WRITE_EEPROM:              
                          
             addr1 = buff_ser2[IRQ_WRE_ADDR_EEP];           // Indirizzo INIZIALE di memoria delle eeprom  che si vuole scrivere 
             addr2 = addr1 + buff_ser2[IRQ_WRE_NUM_BYTE];   // Ultimo Indirizzo di memoria dell'eeprom che si vuole leggere 
             
             // prima di scrivere andiamo a verificare che l'area di memoria non sia protetta da scrittura, i byte protetti sono pochi e sono:   
             addr_start[0] = 0; 
             addr_end[0]   = ADDR_EEP(size2_free[1]);     
             
             addr_start[1] = ADDR_EEP(Set_Power_ON);       
             addr_end[1]   = ADDR_EEP(cntUpdate_SettingPar);
             
             addr_start[2] = ADDR_EEP(Bypass_minTempExt);
             addr_end[2]   = ADDR_EEP(cntUpdate_SetTemp);
             
             addr_start[3] = addr_end[2] + 1;          
             addr_end[3]   = ADDR_EEP(cntUpdate_dayProg);

             for(i=0, esitoWR='1'; i < 4; i++) {
                if((addr1 >= addr_start[i]) && (addr2 <= addr_end[i])) {
                   esitoWR= '0'; //Zona di scrittura valida
                   break;
                }   
             } 
 
             if(esitoWR == '0') {
               // Scriviamo in eeprom, ma solo dopo aver verificato che i valori inseriti non sono fuori limiti.
               flag = update_data_eeprom(addr1, addr2, &buff_ser2[IRQ_WRE_START_DATA_EEP]);  
               
               if(flag == -1) 
                  esitoWR= '2';         
            }         

            //------- Costruiamo il messaggio di risposta ---------                
            buff[IHM2_POS_CRC_LO]     = IRS_WRE_CKS_LO;
            buff[IHM2_ADDR_RECEIVER]  = ADDR_REM_CTRL;
            buff[IHM2_TYPE_COMAND]    = COMAND_WRITE_EEPROM;          

            buff[IRS_WRE_ADDR_EEP]    = buff_ser2[IRQ_WRE_ADDR_EEP];
            buff[IRS_WRE_NUM_BYTE]    = buff_ser2[IRQ_WRE_NUM_BYTE];
            // '0': scrittura OK
            // '1': errore zona eeprom non valida
            // '2': errore dati non validi
            // '3': dispositivo occupato.
            buff[IRS_WRE_ESITO] = esitoWR;
            
            len = IRS_WRE_CKS_LO + 2;
            
            // invia il messaggio di rispota al modbus
            Send_Message_Ser2(&buff[0], len);     
            upgrade_eep_for_Modbus = 1;        
          break;
                 
          default:
            free_serial_2();
          break; 
       }  
             
   }else if (val_ret < 0) {
       // -- Gestione messaggi di errore -- 
       //   ERROR_HEAD_MESSAGE =  -1          
       //   ERROR_LEN_MESSAGE  =  -2   
       //   ERROR_CKSUM_16     =  -3
       free_serial_2();
   } 


   return val_ret;
}

#else
int intercepts_messages_serial_2()
{
	int val_ret = 0, len;
	unsigned short enab_func;
	byte i, n;
	byte msk_motors = 0x11;
	byte buff[128];
	byte addr_eep;
	unsigned short addr_start[4], addr_end[4], addr1, addr2, esitoWR;
	int flag;
	
	// 1: Andiamo ad analizzare se vi sono messaggi nel buffer
	val_ret = Read_Message_Ser2();
	
	
	if(val_ret > RUN_DOWNLOAD )  // if(val_ret > 0)
	{
		
		switch (buff_ser2[IHM2_TYPE_COMAND])
		{
			case  COMAND_POLLING_CTRL_FAN:
			n = read_byte_eeprom(ADDR_EEP(numMotors));
			if(n== 4)
			msk_motors = 0x33;
			else if(n==6)
			msk_motors = 0x77;
			
			// err_V_speed_#
			msk_motors |= 0x08;
			
			// Status Motors R & F:  bit.. 7.none | 6.AlarmM3 | 5.AlarmM2 | 4.AlarmM1 | 3.Err_VSpeed | 2.PowerON_M3 | 1.PowerON_M2 | 0.PowerON_M1
			stsMotorsR_buff[0] = buff_ser2[IRS_CMD_F_STATUS_FAN_R] & msk_motors;
			stsMotorsF_buff[0] = buff_ser2[IRS_CMD_F_STATUS_FAN_F] & msk_motors;
			
			// ==========  MOTORI ZA SENZA RPM CHE SONO PERO' PROPORZIONALI AL VOLTAGGIO DI REGOLAZIONE
			
			
			// valore di tensioni lette in uscita che comandono le ventole R, F: da 0.00V a 11.00V il valore Ã¨ moltiplicato x 100. es: 500 = 5.00
			sData.feedbk_v_speed_motors_R =  buff_ser2[IRS_CMD_F_FEEDBK_VOUT_R_LO];
			sData.feedbk_v_speed_motors_R |= (unsigned short)(buff_ser2[IRS_CMD_F_FEEDBK_VOUT_R_HI] << 8);
			
			
			sData.feedbk_v_speed_motors_F = buff_ser2[IRS_CMD_F_FEEDBK_VOUT_F_LO];
			sData.feedbk_v_speed_motors_F |= (unsigned short)(buff_ser2[IRS_CMD_F_FEEDBK_VOUT_F_HI] << 8);
			
			
			
			// CALCOLO DELLA PROPORZIONALITA' DEGLI RPM
			
			//  RPM = 3.200 *  V_speed_motors
			
			// Lettura della 6 Speed Motors (rpm)
			sData.rpm_motors_R[0] = buff_ser2[IRS_CMD_F_RPM_FAN_R1_LO];
			sData.rpm_motors_R[0] |= (unsigned short)(buff_ser2[IRS_CMD_F_RPM_FAN_R1_HI] << 8);
			
			if(sData.feedbk_v_speed_motors_R)
			{
				sData.rpm_motors_R[0] = (unsigned int) ((float)(sData.feedbk_v_speed_motors_R*3.200));
			}
			else
			sData.rpm_motors_R[0] = 0;
			
			
			sData.rpm_motors_F[0] = buff_ser2[IRS_CMD_F_RPM_FAN_F1_LO];
			sData.rpm_motors_F[0] |= (unsigned short)(buff_ser2[IRS_CMD_F_RPM_FAN_F1_HI] << 8);
			
			if(sData.feedbk_v_speed_motors_F)
			{
				sData.rpm_motors_F[0] = (unsigned int) ((float)(sData.feedbk_v_speed_motors_F*3.200));
			}else
			sData.rpm_motors_F[0] = 0;
			
			sData.rpm_motors_R[1] = buff_ser2[IRS_CMD_F_RPM_FAN_R2_LO];
			sData.rpm_motors_R[1] |= (unsigned short)(buff_ser2[IRS_CMD_F_RPM_FAN_R2_HI] << 8);
			
			sData.rpm_motors_F[1] = buff_ser2[IRS_CMD_F_RPM_FAN_F2_LO];
			sData.rpm_motors_F[1] |= (unsigned short)(buff_ser2[IRS_CMD_F_RPM_FAN_F2_HI] << 8);
			
			sData.rpm_motors_R[2] =  buff_ser2[IRS_CMD_F_RPM_FAN_R3_LO];
			sData.rpm_motors_R[2] |= (unsigned short)(buff_ser2[IRS_CMD_F_RPM_FAN_R3_HI] << 8);
			
			sData.rpm_motors_F[2] = buff_ser2[IRS_CMD_F_RPM_FAN_F3_LO];
			sData.rpm_motors_F[2] |= (unsigned short)(buff_ser2[IRS_CMD_F_RPM_FAN_F3_HI] << 8);

			
			cnt_resp_poll_CTRL_FAN++;   // incrementa conatatore di risposte ok di polling

			RemoveAlarm(ALM_EB_CTRL_FAN);
			
			// il cambiamento di stato deve essere = per 2 pollate consecutive
			if(n==2)
			msk_motors = 0x01;
			else if(n==4)
			msk_motors = 0x03;
			else
			msk_motors = 0x07;
			
			// occorre confermare il dato per 3 volte prima di segnalare errori o altro.
			if ((stsMotorsR_buff[0] == stsMotorsR_buff[1]) && (stsMotorsR_buff[0] == stsMotorsR_buff[2]))
			sData.StatusMotorsR = stsMotorsR_buff[0] & 0xF7;         //(tolto err_V_speed_R --> 056 da problemi di impedenza)  06/06/2016
			else {
				i = (ALM_MR1_NO_PWR >> 3);
				sData.Events[i] = stsMotorsR_buff[0] & 0xF0;             // allarmi di no rotazione motore  (tolto err_V_speed_R --> 056 da problemi di impedenza)  06/06/2016
				sData.Events[i] |= (~stsMotorsR_buff[0] & msk_motors);   // allarmi no POWER_MOTORS
			}
			
			if ((stsMotorsF_buff[0] == stsMotorsF_buff[1]) && (stsMotorsF_buff[0] == stsMotorsF_buff[2]))
			sData.StatusMotorsF = stsMotorsF_buff[0] & 0xF7;          //(tolto err_V_speed_F --> 056 da problemi di impedenza)  06/06/2016
			else  {
				i = (ALM_MF1_NO_PWR >> 3);
				sData.Events[i] = stsMotorsF_buff[0] & 0xF0;              // allarmi di no rotazione motore (tolto err_V_speed_F --> 056 da problemi di impedenza)  06/06/2016
				sData.Events[i] |= (~stsMotorsF_buff[0] & msk_motors);    // allarmi no POWER_MOTORS
			}
			
			#ifdef _CLEAR_ALARMS_POWER_MOTORS
			sData.StatusMotorsF &= 0xF0;
			sData.StatusMotorsR &= 0xF0;
			#endif

			// Tolgo gli errori di rotazione visto che non leggo la tacho.
			
			sData.StatusMotorsF &= 0x0F;
			sData.StatusMotorsR &= 0x0F;
			
			RemoveAlarm(ALM_MR1_NO_PULSE);
			RemoveAlarm(ALM_MF1_NO_PULSE);
			
			
			enab_func = read_word_eeprom(ADDR_EEP(Enab_Fuction));
			if (enab_func & (1 << ENAB_MBF))
			{
				sData.StatusMotorsF &= 0xF0;
				sData.StatusMotorsR &= 0xF0;
			}
			
			stsMotorsR_buff[2] = stsMotorsR_buff[1];
			stsMotorsR_buff[1] = stsMotorsR_buff[0];
			stsMotorsF_buff[2] = stsMotorsF_buff[1];
			stsMotorsF_buff[1] = stsMotorsF_buff[0];
			break;
			
			
			case  COMAND_PACKET_1_DATA:
			//    b7. 1= Modbus Active, b6 = RFM Install, ... b1
			if(sData.Status_RemoteCTRL != buff_ser2[IPK1M_STATUS_REMOTE_CTRL])
			{
				update_EB_RemoteCTRL(buff_ser2[IPK1M_STATUS_REMOTE_CTRL]);
			}
			
			if(buff_ser2[IPK1M_SET_CLOCK] == 'S') {
				sClockWeek.seconds = buff_ser2[IPK1M_CLOCK_HOURS];
				sClockWeek.minutes = buff_ser2[IPK1M_CLOCK_MIN];
				sClockWeek.hours   = buff_ser2[IPK1M_CLOCK_SEC];
				sClockWeek.weekday = buff_ser2[IPK1M_CLOCK_DAYWEEK];
			}
			
			if(buff_ser2[IPK1M_CLEAN_FILTERS] == 'C') {
				RemoveAlarm(ALM_FILTER);
				for(i=0;  i < 4;  i++)
				write_byte_eeprom(ADDR_EEP(time_lastCloggedFilters[i]), read_byte_eeprom(ADDR_EEP(hour_runnig[i])));
			}
			
			if(read_byte_eeprom(ADDR_EEP(SW_Vers_ModBus[0])) != buff_ser2[IPK1M_VER_FW_1_CAR])
			write_byte_eeprom(ADDR_EEP(SW_Vers_ModBus[0]),   buff_ser2[IPK1M_VER_FW_1_CAR]);
			
			if(read_byte_eeprom(ADDR_EEP(SW_Vers_ModBus[1])) != buff_ser2[IPK1M_VER_FW_2_CAR])
			write_byte_eeprom(ADDR_EEP(SW_Vers_ModBus[1]),   buff_ser2[IPK1M_VER_FW_2_CAR]);
			
			cnt_no_link_Modbus = 0;
			
			RemoveAlarm(ALM_EB_REM_CTRL);
			break;
			
			
			case  COMAND_READ_EEPROM:
			// rispondiamo subito al modulo.
			if (buff_ser2[IRQ_RDE_NUM_BYTE] < 120) {
				addr_eep = buff_ser2[IRQ_RDE_ADDR_EEP];               // Indirizzo INIZIALE di memoria delle eeprom  che si vuole leggere
				len = IRS_RDE_START_DATA_EEP + buff_ser2[IRQ_RDE_NUM_BYTE] + 2; // Conteggiamo anche i 2 Byte dei Cksum
				//------- Costruiamo il messaggio di risposta ---------
				buff[IHM2_POS_CRC_LO]     = (len - 2);
				buff[IHM2_ADDR_RECEIVER]  = ADDR_REM_CTRL;
				buff[IHM2_TYPE_COMAND]    = COMAND_READ_EEPROM;
				buff[IRS_RDE_ADDR_EEP]    = addr_eep;
				buff[IRS_RDE_NUM_BYTE]    = buff_ser2[IRQ_RDE_NUM_BYTE];
				
				for(i=0;  i < buff_ser2[IRQ_RDE_NUM_BYTE]; addr_eep++, i++)
				buff[IRS_RDE_START_DATA_EEP + i] = read_byte_eeprom(addr_eep);
				
				Send_Message_Ser2(&buff[0], len);
				
			}
			
			cnt_no_link_Modbus = 0;
			
			RemoveAlarm(ALM_EB_REM_CTRL);
			break;
			
			case  COMAND_WRITE_EEPROM:
			
			addr1 = buff_ser2[IRQ_WRE_ADDR_EEP];           // Indirizzo INIZIALE di memoria delle eeprom  che si vuole scrivere
			addr2 = addr1 + buff_ser2[IRQ_WRE_NUM_BYTE];   // Ultimo Indirizzo di memoria dell'eeprom che si vuole leggere
			
			// prima di scrivere andiamo a verificare che l'area di memoria non sia protetta da scrittura, i byte protetti sono pochi e sono:
			addr_start[0] = 0;
			addr_end[0]   = ADDR_EEP(size2_free[1]);
			
			addr_start[1] = ADDR_EEP(Set_Power_ON);
			addr_end[1]   = ADDR_EEP(cntUpdate_SettingPar);
			
			addr_start[2] = ADDR_EEP(Bypass_minTempExt);
			addr_end[2]   = ADDR_EEP(cntUpdate_SetTemp);
			
			addr_start[3] = addr_end[2] + 1;
			addr_end[3]   = ADDR_EEP(cntUpdate_dayProg);

			for(i=0, esitoWR='1'; i < 4; i++) {
				if((addr1 >= addr_start[i]) && (addr2 <= addr_end[i])) {
					esitoWR= '0'; //Zona di scrittura valida
					break;
				}
			}
			
			if(esitoWR == '0') {
				// Scriviamo in eeprom, ma solo dopo aver verificato che i valori inseriti non sono fuori limiti.
				flag = update_data_eeprom(addr1, addr2, &buff_ser2[IRQ_WRE_START_DATA_EEP]);
				
				if(flag == -1)
				esitoWR= '2';
			}

			//------- Costruiamo il messaggio di risposta ---------
			buff[IHM2_POS_CRC_LO]     = IRS_WRE_CKS_LO;
			buff[IHM2_ADDR_RECEIVER]  = ADDR_REM_CTRL;
			buff[IHM2_TYPE_COMAND]    = COMAND_WRITE_EEPROM;

			buff[IRS_WRE_ADDR_EEP]    = buff_ser2[IRQ_WRE_ADDR_EEP];
			buff[IRS_WRE_NUM_BYTE]    = buff_ser2[IRQ_WRE_NUM_BYTE];
			// '0': scrittura OK
			// '1': errore zona eeprom non valida
			// '2': errore dati non validi
			// '3': dispositivo occupato.
			buff[IRS_WRE_ESITO] = esitoWR;
			
			len = IRS_WRE_CKS_LO + 2;
			
			// invia il messaggio di rispota al modbus
			Send_Message_Ser2(&buff[0], len);
			upgrade_eep_for_Modbus = 1;
			break;
			
			default:
			free_serial_2();
			break;
		}
		
		}else if (val_ret < 0) {
		// -- Gestione messaggi di errore --
		//   ERROR_HEAD_MESSAGE =  -1
		//   ERROR_LEN_MESSAGE  =  -2
		//   ERROR_CKSUM_16     =  -3
		free_serial_2();
	}


	return val_ret;
}

#endif

//----------------------------------------------------------------
// Func: 
//  processes_messages
//
// Decription:
//  Campiona le due seriali ogni 5 millsec. Questo serve per 
//  determinare la fine di un messaggio ( se la lunghezza del 
//  messaggio non cambia dopo 5 mills allora vuol dire che è 
//  terminato).
//----------------------------------------------------------------
void processes_messages()
{
  unsigned long int timerMills;
  
  timerMills = millis(); 
  
  if (((timerMills -  preview_3_Mills) >= 3) 
    || (preview_3_Mills > timerMills))          // situazione di overflow.
  {
    
    if (preview_3_Mills < timerMills) {
              
       //* * * * * * * * * * * * * * * * * * * 
       //          Seriale 1
       //* * * * * * * * * * * * * * * * * * * 
       intercepts_messages_serial_1();
            
       // * * * * * * * * * * * * * * * * * * * 
       //          Seriale 2
       // * * * * * * * * * * * * * * * * * * * 
       intercepts_messages_serial_2();
     }
     
     preview_3_Mills = timerMills;
  }      
}  



//-------------------------------------------------------
// Invio comando di polling verso la scheda CTRL_FAN
//-------------------------------------------------------
void polling_CTRL_FAN(void)
{
    byte buff[32];
    int len, i;
    byte msk_pwr, msk_alm, msk_alm_motors, numMotors, b, sts_leds;

    // Andiamo a comporre lo stato dei 4 Leds della scheda CTRL_FAN: 
    sts_leds = 0; 
    
    if(sData.status_test & MSK_TEST_LEDS) {
      // (11=none, 10=ON FIXED, 01=Toggle, 00=OFF) 
      sts_leds = sData.data_test;  
    }else {        
      //---------- LED 4 ROSSO ---------------   
      //  bit:[3,2]=(11=none, 10=ON FIXED, 01=Toggle, 00=OFF) 
      b = (ALM_FILTER >> 3);
      msk_alm = ~(1 << (ALM_FILTER & 0x07));
    
      // byte 08:  ALM_MR1_NO_PWR,  ALM_MR2_NO_PWR,  ALM_MR3_NO_PWR,  ALM_NONE_67, ...   
      // byte 09:  ALM_MF1_NO_PWR,  ALM_MF2_NO_PWR,  ALM_MF3_NO_PWR,  ALM_NONE_75, ...
      if(PowerMotors == POWER_SAVE) 
        msk_alm_motors &= 0xF0; // maschera gli allarmi di NO POWER    
      else 
        msk_alm_motors = 0xF7; //non controllo errore tensione di comando --> motore EC056 da problemi di impedenza
     
      for(i=0; i < 13; i++) 
      {
         if(i==b) {      
           if (sData.Events[b] & msk_alm)
              sts_leds |= (1 << 3);  // Led ROSSO ON
         }else if((i == 8) || (i == 9)) {
            if (sData.Events[i] & msk_alm_motors)
              sts_leds |= (1 << 3);  // Led ROSSO ON
         }else {   
            if(sData.Events[i])
              sts_leds |= (1 << 3);  // Led ROSSO ON
         }   
      }  
   
      //---------- LED 3 GIALLO ---------------  
      // Filtro sporco
      //  bit:[1,0]=(11=none, 10=ON FIXED, 01=Toggle, 00=OFF) 
      if(CkAlarm(ALM_FILTER))           
        sts_leds |= 0x02; // LED GIALLO FIXED.                 
  
      //--------------- LED_2_VERDE -----------------   
      // WEEKLY ENAB
      //  bit:[7,6]=(11=none, 10=ON FIXED, 01=Toggle, 00=OFF) 
      if (sData.status_weekly & MSK_BIT_WEEKLY_ENAB)
        sts_leds |= (1 << 7); // FIXED
   
      //--------------- LED_1_VERDE -----------------  
      // RUN = FIXED, MOTORS FAILED = TOGGLE 
      // bit:[5,4]=(11=none, 10=ON FIXED, 01=Toggle, 00=OFF) 
      numMotors = read_byte_eeprom(ADDR_EEP(numMotors));
    
      // StatusMotorsR:   7.none | 6.AlarmNoRotM3 | 5.AlarmNoRotM2 | 4.AlarmNoRotM1 | 3.none | 2.PowerON_M3 | 1.PowerON_M2 | 0.PowerON_M1 
      // StatusMotorsF:   7.none | 6.AlarmNoRotM3 | 5.AlarmNoRotM2 | 4.AlarmNoRotM1 | 3.none | 2.PowerON_M3 | 1.PowerON_M2 | 0.PowerON_M1    
      if(numMotors == 2) {
         msk_pwr = 0x01;
         msk_alm = 0x10;
      }else if (numMotors == 4) {
         msk_pwr = 0x03;
         msk_alm = 0x30;
      }else {
         msk_pwr = 0x07;
         msk_alm = 0x70;      
      }   
  
      if(PowerMotors == POWER_ON)
      {
        if(cnt_status_motors++ > 4) { 
          cnt_status_motors = 5;   
          
          // 1. se manca l'alimentazione a un motore segnalalo con led lampeggiante
          if (((sData.StatusMotorsF & msk_pwr) != msk_pwr) || ((sData.StatusMotorsR & msk_pwr) != msk_pwr)) {        
              sts_leds |= (1 << 4); // toggle   
          }else if ((sData.StatusMotorsF & msk_alm) || (sData.StatusMotorsR & msk_alm)) { 
             // c'è il power ma uno o + motori non girano
             sts_leds |= (1 << 4); // toggle
          }else {
             sts_leds |= (1 << 5);  // FIXED 
          } 
        }else
           sts_leds |= (1 << 5);  // FIXED 
              
      }else {
        if ((sData.StatusMotorsF & msk_alm) || (sData.StatusMotorsR & msk_alm)) { 
           // c'è il power ma uno o + motori non girano
           sts_leds |= (1 << 4); // toggle
        }      
        cnt_status_motors = 0; 
      }  
    }
    
    sData.status_Leds_ctrlFan     = sts_leds;
    
    // secondo Protocol_Ser2, completiamo il buffer prima di inviare
    buff[IHM2_START_MSG_0]        = PROTO_START_MSG_0;            // byte_0: 's'
    buff[IHM2_START_MSG_1]        = PROTO_START_MSG_1;            // byte_0: 'm'
    buff[IHM2_POS_CRC_LO]         = IRQ_CMD_F_CRC_LO;             // byte_2: posizione cksum_16 (byte LO) che è anche il penultimo byte
    buff[IHM2_ADDR_RECEIVER]      = ADDR_CTRL_FAN;                // byte_3: Indirizzo del destinatorio del messaggio
    buff[IHM2_TYPE_COMAND]        = COMAND_POLLING_CTRL_FAN;      // byte_4: Tipo di comando/richiesta
    buff[IRQ_CMD_F_SPEED_FANS_R]  = sData.speed_motors_R;         // byte_5: valori SPEED_FANS_R da 0 -> 255 
    buff[IRQ_CMD_F_SPEED_FANS_F]  = sData.speed_motors_F;         // byte_6: valori SPEED_FANS_F da 0 -> 255
    buff[IRQ_CMD_F_STATUS_LEDS]   = sData.status_Leds_ctrlFan;    // byte_7:
    buff[IRQ_CMD_F_NPULSE_XROUND] = read_byte_eeprom(ADDR_EEP(numPulseMotors));   // byte_8: numero di pulse per giro. ( 2, 3, 4)
    buff[IRQ_CMD_F_NUM_MOTORS]    = read_byte_eeprom(ADDR_EEP(numMotors));        // byte_9: numero di motori da gestire
    buff[IRQ_CMD_F_BYTE_1_DONE]   = 0x00;                         // done
    buff[IRQ_CMD_F_BYTE_2_DONE]   = 0x00;                         // done 
    
    len = IRQ_CMD_F_CRC_LO + 2;
    
    //------ calcola il livello di comunicazione tra le due scheda 10/10 -----
    cnt_req_poll_CTRL_FAN++; // incrementa contatore di richiesta Polling
     
    if ((cnt_req_poll_CTRL_FAN % 4) == 0) {
       sData.level_link_CTRLFAN = (cnt_resp_poll_CTRL_FAN * 10) / (cnt_req_poll_CTRL_FAN -1);

       if (cnt_req_poll_CTRL_FAN >= 24) 
       {      
          if (cnt_resp_poll_CTRL_FAN == 0) 
               InsertAlarm(ALM_EB_CTRL_FAN);  
               
          cnt_resp_poll_CTRL_FAN >>= 1; 
          cnt_req_poll_CTRL_FAN  >>= 1;
       }
    }     
          
    // inviamo il comando
    Send_Message_Ser2(&buff[0], len);     

}  

//----------------------------------------------------------------------------------
// La comunicazione tra il modulo Rd ed il Modulo Modbus, avviene diversificando
//   il messsaggio tra domanda e risposta, ovvero dato un tipo di messaggio inviato 
//   dal modulo RD (esempio il pacchetto _PACKET1_DATA ), la risposta può essere ad
//   esempio una richiesta di lettura di una area di eeprom. Questo trasforma 
//   il Modbus in un Master temporaneo.
//----------------------------------------------------------------------------------
byte index_packet_data =0;

void polling_ModBus()
{
    byte byte_alarm, b, i;
    byte buff[102];
    unsigned short size_unit, Caf_Max;
    short  stemp_16 =0;
    int len;
    
    
    if(upgrade_eep_for_Modbus) {
      upgrade_eep_for_Modbus = 0;
      return;
    }  
    
    // Testata del Messaggio           
    buff[IHM2_ADDR_RECEIVER]     = ADDR_REM_CTRL;               // byte_3: Indirizzo del destinatorio del messaggio ModBUS, RFM
     
    if ((index_packet_data & 0x01)==0) {
        /**********************
         * Pachetto dati 1
         **********************/        
        buff[IHM2_POS_CRC_LO]    = IPK1U_CKS_LO;                 // posizione cksum_16 (byte LO) che è anche il penultimo byte
        buff[IHM2_TYPE_COMAND]   = COMAND_PACKET_1_DATA;          // byte_4: Tipo di comando/richiesta
    
        buff[IPK1U_STATUS_UNIT_LO]  = lowByte(sData.status_unit);   
        buff[IPK1U_STATUS_UNIT_HI]  = highByte(sData.status_unit);   
        
        /*                            0x80          0x40           0x20              0x10            0x08            0x04            0x02            0x01
          BYTE_ALARM_LO:  (MSB)  CAF Acces.KO, CAP Acces.KO, Temper.Sensors KO, VOC Sensors KO,  RH Sensors KO, CO2 Sensors KO,  Motor Fresh KO,  Motor Return KO   */
        b = 0;
        byte_alarm = (ALM_MR1_NO_PWR >> 3);
        if(sData.Events[byte_alarm])           b  = 0x01;    // tutti gli allarmi motorsR
                        
        byte_alarm = (ALM_MF1_NO_PWR >> 3);
        if(sData.Events[byte_alarm])           b |= 0x02;  // tutti gli allarmi motorsF           
        
//-----  BEGIN -----  VANNO RIVISTI QUESTI ERRORI, SONO GESTITI POCO E MALE

//        byte_alarm = (ALM_PCO2_EX1_KO >> 3);
//        if(sData.Events[byte_alarm])           b |= 0x04;  // tutti gli allarmi Sensori CO2
//                     
//        byte_alarm = (ALM_PRH_EX1_KO >> 3);
//        if(sData.Events[byte_alarm])           b |= 0x08;  // tutti gli allarmi Sensori RH                
//            
//        byte_alarm = (ALM_VOC_EX1_KO >> 3);
//        if(sData.Events[byte_alarm])           b |= 0x10; // tutti gli allarmi Sensori VOC
//                         
//        byte_alarm = (ALM_PTFRESH_KO >> 3);
//        if(sData.Events[byte_alarm] || 
//           CkAlarm(ALM_AWP_KO)      || 
//           CkAlarm(ALM_AWP_LINK))              b |= 0x20;  // tutti gli allarmi NTC + AWP
//
//   
//
//
//        //   Events[06]:  SENSORS_PRESSURE: 7.DPP_LinkEr, 6.DPP_KO     , 5.- - - - - , 4.PCAF_LinkEr, 3.PCAF_KO     , 2.- - - - - , 1.PCAP_LinkE, 0.PCAP_KO    
//        byte_alarm = ALM_PCAP_KO >> 3;     
//        if(sData.Events[byte_alarm] & 0x03)    b |= 0x40;                    
//        if(sData.Events[byte_alarm] & 0x18)    b |= 0x80;
//                               
         buff[IPK1U_BYTE_ALARM_LO]  = b;     
         b=0;
//        
//        /*                            0x80          0x40           0x20              0x10            0x08            0x04            0x02            0x01
//          BYTE_ALARM_HI:  (MSB)        - - - ,        - - -,   Electr.Board KO,   PreHeater KO,      Cooler KO,      Heater KO,    Bypass Fails,  DSP Access.KO     */  
//        b=0;
//        if(sData.Events[byte_alarm] & 0xC0)   b  = 0x01;                        
//        if(CkAlarm(ALM_BYPASS_KO))            b |= 0x02;    
//        
//        byte_alarm = ALM_EHD_NTC_KO >> 3;  
//        if(sData.Events[byte_alarm])          b |= 0x04;   
//        
//        byte_alarm = ALM_CWD_NTC_KO >> 3;  
//        if(sData.Events[byte_alarm])          b |= 0x08;  
//        
//        byte_alarm = ALM_PEH_NTC_KO >> 3;  
//        if(sData.Events[byte_alarm])          b |= 0x10;      
//
//
//------ END ------    
        
             
        //   Events[10] : 7.- - - - - , 6.KTS_FAULT  , 5.----, 4.EepFault   , 3.----    , 2.BattFault , 1.EB_RemCtrl, 0.EB_CtrlFan_KO 
        byte_alarm = ALM_EB_CTRL_FAN >> 3; 
        if(sData.Events[byte_alarm] & 0x57)   b |= 0x20;  // Electr.Board KO
                    
        buff[IPK1U_BYTE_ALARM_HI]      = b;              
        buff[IPK1U_STATUS_OUTPUT]      = (sData.StatusOutput & 0x03);    // bits: ..., [1]STS_OUT2, [0]STS_OUT1
        buff[IPK1U_STATUS_FILTER]      = CkAlarm(ALM_FILTER);
        
        buff[IPK1U_MEASURE_V_INPUT_1]  = (sData.measure_ADC_input[0] /10);     // Misura input 1 da 0V a 10.0V (=100)
        buff[IPK1U_MEASURE_V_INPUT_2]  = (sData.measure_ADC_input[1] /10);
        buff[IPK1U_NUM_MOTORS]         = read_byte_eeprom(ADDR_EEP(numMotors)); 
        buff[IPK1U_FEEDBK_VR_LO]       = (byte)(sData.feedbk_v_speed_motors_R & 0x0FF);         // Byte 2: FeedBack tensione, Comando gruppo Return
        buff[IPK1U_FEEDBK_VR_HI]       = (byte)(sData.feedbk_v_speed_motors_R >> 8);
        buff[IPK1U_FEEDBK_VF_LO]       = (byte)(sData.feedbk_v_speed_motors_F & 0x0FF);         // Byte 2: FeedBack tensione, Comando gruppo Fresh
        buff[IPK1U_FEEDBK_VF_HI]       = (byte)(sData.feedbk_v_speed_motors_F >> 8); 
       

        for(i=IPK1U_RPM_FAN_R1_LO; i <=IPK1U_RPM_FAN_F5_HI; i++)
            buff[i] = 0;
      
        buff[IPK1U_RPM_FAN_R1_LO]  = (byte)(sData.rpm_motors_R[0] & 0x0FF);      
        buff[IPK1U_RPM_FAN_R1_HI]  = (byte)(sData.rpm_motors_R[0] >> 8); 
        buff[IPK1U_RPM_FAN_F1_LO]  = (byte)(sData.rpm_motors_F[0] & 0x0FF);  
        buff[IPK1U_RPM_FAN_F1_HI]  = (byte)(sData.rpm_motors_F[0] >> 8); 
        
        if(buff[IPK1U_NUM_MOTORS] > 2) {
           buff[IPK1U_RPM_FAN_R2_LO]  = (unsigned char)(sData.rpm_motors_R[1] & 0x0FF);        
           buff[IPK1U_RPM_FAN_R2_HI]  = (unsigned char)(sData.rpm_motors_R[1] >> 8); 
           buff[IPK1U_RPM_FAN_F2_LO]  = (unsigned char)(sData.rpm_motors_F[1] & 0x0FF);    
           buff[IPK1U_RPM_FAN_F2_HI]  = (unsigned char)(sData.rpm_motors_F[1] >> 8); 
           
           if(buff[IPK1U_NUM_MOTORS] > 4) {
              buff[IPK1U_RPM_FAN_R3_LO]  = (unsigned char)(sData.rpm_motors_R[2] & 0x0FF);      
              buff[IPK1U_RPM_FAN_R3_HI]  = (unsigned char)(sData.rpm_motors_R[2] >> 8);  
              buff[IPK1U_RPM_FAN_F3_LO]  = (unsigned char)(sData.rpm_motors_F[2] & 0x0FF);  
              buff[IPK1U_RPM_FAN_F3_HI]  = (unsigned char)(sData.rpm_motors_F[2] >> 8);  
           }          
        }  
    
        buff[IPK1U_CNT_UPDATE_INFO]     = read_byte_eeprom(ADDR_EEP(cntUpdate_info));
        buff[IPK1U_CNT_UPDATE_SET_PAR]  = read_byte_eeprom(ADDR_EEP(cntUpdate_SettingPar));   
        buff[IPK1U_CNT_UPDATE_SET_TEMP] = read_byte_eeprom(ADDR_EEP(cntUpdate_SetTemp));
        buff[IPK1U_CNT_UPDATE_WEEKLY]   = read_byte_eeprom(ADDR_EEP(cntUpdate_dayProg));
        
        //  aggiunta con la RF (10/04/15)
        buff[IPK1U_STATUS_MOTORS_R]  = sData.StatusMotorsR;
        buff[IPK1U_STATUS_MOTORS_F]  = sData.StatusMotorsF;
        
        for(int a=0; a <= 11; a++)
           buff[IPK1U_EVENT_BYTE_0 + a] = sData.Events[a];

        
        len = IPK1U_CKS_LO + 2;
   
    }else {
       /**********************
        * Pachetto dati 2
        **********************/
        
        buff[IHM2_POS_CRC_LO]     = IPK2U_CKS_LO;             // posizione cksum_16 (byte LO) che è anche il penultimo byte
        buff[IHM2_TYPE_COMAND]    = COMAND_PACKET_2_DATA;     // byte_4: Tipo di comando/richiesta

        buff[IPK2U_CLOCK_DAYWEEK]   = sClockWeek.weekday;
        buff[IPK2U_CLOCK_HOURS]     = sClockWeek.hours;
        buff[IPK2U_CLOCK_MIN]       = sClockWeek.minutes;
        buff[IPK2U_CLOCK_SEC]       = sClockWeek.seconds;
        //  4 Byte: hour run
        buff[IPK2U_HOURS_RUN_BYT0]  = (byte)(countHoursFunz & 0x0FF);              
        buff[IPK2U_HOURS_RUN_BYT1]  = (byte)((countHoursFunz >> 8) & 0x0FF);       
        buff[IPK2U_HOURS_RUN_BYT2]  = (byte)((countHoursFunz >> 16) & 0x0FF);    
        buff[IPK2U_HOURS_RUN_BYT3]  = (byte)((countHoursFunz >> 24) & 0x0FF);        
        

        buff[IPK2U_NUM_NTC]         = read_byte_eeprom(ADDR_EEP(numNTC)) & 0x07;
        buff[IPK2U_SET_MBF_FAN_F]   = read_byte_eeprom(ADDR_EEP(Set_MBF_fresh));   // Controllo motore Fresh se siamo in MBF
        buff[IPK2U_SET_MBF_FAN_R]   = read_byte_eeprom(ADDR_EEP(Set_MBF_return));  // Controllo motore Return se siamo in MBF
		
        buff[IPK2U_SET_EHD_MOD]   = read_byte_eeprom(ADDR_EEP(Set_EHD_mod));   // Controllo motore Fresh se siamo in MBF
        buff[IPK2U_SET_BPD_MOD]   = read_byte_eeprom(ADDR_EEP(Set_BPD_mod));  // Controllo motore Return se siamo in MBF
		
        
        buff[IPK2U_INFO_PROBE_AD]   = sData.info_ProbesAD;          //  7.2CO2  6.P1CO2  5.----  4.P2RH  3.P1RH  2.----  1.P1VOC  0.----
      
        buff[IPK2U_MEASURE_TF_LO]   =  (byte)(Temp_modbus[I_PROBE_FRESH] & 0x0FF);     
        buff[IPK2U_MEASURE_TF_HI]   =  (byte)(Temp_modbus[I_PROBE_FRESH] >> 8);
        
        if(DigitAccessoryOperating(ACC_I2C_AWP)) {
            buff[IPK2U_MEASURE_TR_LO]  = (byte)(sData.AccI2C[ACC_I2C_AWP].measure1 & 0x0FF);
            buff[IPK2U_MEASURE_TR_HI]  = (byte)(sData.AccI2C[ACC_I2C_AWP].measure1 >> 8); 
        }else {  
            buff[IPK2U_MEASURE_TR_LO]  = (byte)(Temp_modbus[I_PROBE_RETURN] & 0x0FF);
            buff[IPK2U_MEASURE_TR_HI]  = (byte)(Temp_modbus[I_PROBE_RETURN] >> 8);
        }
        
        
        if(Tcheck==0)
        {
        buff[IPK2U_MEASURE_TS_LO]  = (byte)(Temp_modbus[I_PROBE_SUPPLY] & 0x0FF);         //  Byte 2 -> Temp. Supply
        buff[IPK2U_MEASURE_TS_HI]  = (byte)(Temp_modbus[I_PROBE_SUPPLY] >> 8);
        buff[IPK2U_MEASURE_TE_LO]  = (byte)(Temp_modbus[I_PROBE_EXHAUST] & 0x0FF);         
        buff[IPK2U_MEASURE_TE_HI]  = (byte)(Temp_modbus[I_PROBE_EXHAUST] >> 8);  
        }        
        
        // bits: [7]......., [6]CMD_PreHeat, [5]CMD_Cooler, [4]CMD_Heather, [3]........., [2]STS_PreHeat, [1]STS_Cooler, [0]STS_Heather     
        // con RFM aggiunto 2 bit
        // bits: [7]HWD_EHD, [6]CMD_PreHeat, [5]CMD_Cooler, [4]CMD_Heather, [3]PHWD_PEHD, [2]STS_PreHeat, [1]STS_Cooler, [0]STS_Heather
        b=0;
        if(DigitAccessoryOn(ACC_I2C_HEATER))                     b |= 0x01;
        if(DigitAccessoryOn(ACC_I2C_COOLER))                     b |= 0x02;
        if(DigitAccessoryOn(ACC_I2C_PREHEATER))                  b |= 0x04;
        if(sData.AccI2C[ACC_I2C_PREHEATER].sts & STS_ACC_ELECT)  b |= 0x08; // bit=1 Elettrico
        if(sData.AccI2C[ACC_I2C_HEATER].comand & CMD_ON)         b |= 0x10;
        if(sData.AccI2C[ACC_I2C_COOLER].comand & CMD_ON)         b |= 0x20;
        if(sData.AccI2C[ACC_I2C_PREHEATER].comand & CMD_ON)      b |= 0x40;
        if(sData.AccI2C[ACC_I2C_HEATER].sts & STS_ACC_ELECT)     b |= 0x80; // bit=1 Elettrico
        
        buff[IPK2U_STATUS_ACCESS_CLIMA] = b;   
              
        buff[IPK2U_MEASURE_ACC_TA_HEATER_LO]  = (byte)(sData.AccI2C[ACC_I2C_HEATER].measure1 & 0x0FF);  // 2 byte:  Temperature air sia per HWD che EHD
        buff[IPK2U_MEASURE_ACC_TA_HEATER_HI]  = (byte)(sData.AccI2C[ACC_I2C_HEATER].measure1 >> 8);
     
        buff[IPK2U_MEASURE_ACC_TW_HEATER_LO]  = (byte)(sData.AccI2C[ACC_I2C_HEATER].measure2 & 0x0FF);   // 2 byte:  Temperature acqua se HWD oppure lettura pressione se EHD
        buff[IPK2U_MEASURE_ACC_TW_HEATER_HI]  = (byte)(sData.AccI2C[ACC_I2C_HEATER].measure2 >> 8);

        buff[IPK2U_MEASURE_ACC_TA_COOLER_LO]  = (byte)(sData.AccI2C[ACC_I2C_COOLER].measure1 & 0x0FF);  // 2 byte:  Temperature air CWD
        buff[IPK2U_MEASURE_ACC_TA_COOLER_HI]  = (byte)(sData.AccI2C[ACC_I2C_COOLER].measure1 >> 8);
               
        buff[IPK2U_MEASURE_ACC_TW_COOLER_LO]  = (byte)(sData.AccI2C[ACC_I2C_COOLER].measure2 & 0x0FF);  // 2 byte:  Temperature water accessoy
        buff[IPK2U_MEASURE_ACC_TW_COOLER_HI]  = (byte)(sData.AccI2C[ACC_I2C_COOLER].measure2 >> 8);
        
        buff[IPK2U_MEASURE_ACC_TA_PREHEATER_LO]  = (byte)(sData.AccI2C[ACC_I2C_PREHEATER].measure1 & 0x0FF);  // 2 byte:  Temperature air PHWD che PEHD
        buff[IPK2U_MEASURE_ACC_TA_PREHEATER_HI]  = (byte)(sData.AccI2C[ACC_I2C_PREHEATER].measure1 >> 8);
               
        buff[IPK2U_MEASURE_ACC_TW_PREHEATER_LO]  = (byte)(sData.AccI2C[ACC_I2C_PREHEATER].measure2 & 0x0FF);  // 2 byte:  Temperature acqua se PHWD oppure lettura pressione se PEHD
        buff[IPK2U_MEASURE_ACC_TW_PREHEATER_HI]  = (byte)(sData.AccI2C[ACC_I2C_PREHEATER].measure2 >> 8);
        
        if(Tcheck ==0)
        {

        buff[IPK2U_EXT1_M1_LO]  = (byte)(sData.AccI2C[ACC_I2C_EXT1].measure1 & 0x0FF);  // 2 byte:  EXT1 Temperature
        buff[IPK2U_EXT1_M1_HI]  = (byte)(sData.AccI2C[ACC_I2C_EXT1].measure1 >> 8);
               
        buff[IPK2U_EXT1_M2_LO]  = (byte)(sData.AccI2C[ACC_I2C_EXT1].measure2 & 0x0FF);  // 2 byte:  EXT1 RH
        buff[IPK2U_EXT1_M2_HI]  = (byte)(sData.AccI2C[ACC_I2C_EXT1].measure2 >> 8);
        }

        buff[IPK2U_EXT1_M3_LO]  = (byte)(sData.AccI2C[ACC_I2C_EXT1].measure3 & 0x0FF);  // 2 byte:  EXT1 CO2
        buff[IPK2U_EXT1_M3_HI]  = (byte)(sData.AccI2C[ACC_I2C_EXT1].measure3 >> 8);
               
        buff[IPK2U_EXT1_M4_LO]  = (byte)(sData.AccI2C[ACC_I2C_EXT1].measure4 & 0x0FF);  // 2 byte:  EXT1 VOC
        buff[IPK2U_EXT1_M4_HI]  = (byte)(sData.AccI2C[ACC_I2C_EXT1].measure4 >> 8);
        
        if(Tcheck ==0)
        {

        buff[IPK2U_EXT2_M1_LO]  = (byte)(sData.AccI2C[ACC_I2C_EXT2].measure1 & 0x0FF);  // 2 byte:  EXT2 Temperature
        buff[IPK2U_EXT2_M1_HI]  = (byte)(sData.AccI2C[ACC_I2C_EXT2].measure1 >> 8);
               
        buff[IPK2U_EXT2_M2_LO]  = (byte)(sData.AccI2C[ACC_I2C_EXT2].measure2 & 0x0FF);  // 2 byte:  EXT2 RH
        buff[IPK2U_EXT2_M2_HI]  = (byte)(sData.AccI2C[ACC_I2C_EXT2].measure2 >> 8);
        }
        
        buff[IPK2U_EXT2_M3_LO]  = (byte)(sData.AccI2C[ACC_I2C_EXT2].measure3 & 0x0FF);  // 2 byte:  EXT2 CO2
        buff[IPK2U_EXT2_M3_HI]  = (byte)(sData.AccI2C[ACC_I2C_EXT2].measure3 >> 8);

        buff[IPK2U_EXT2_M4_LO]  = (byte)(sData.AccI2C[ACC_I2C_EXT2].measure4 & 0x0FF);  // 2 byte:  EXT2 VOC
        buff[IPK2U_EXT2_M4_HI]  = (byte)(sData.AccI2C[ACC_I2C_EXT2].measure4 >> 8);
        
        buff[IPK2U_EXT3_M1_LO]  = (byte)(sData.AccI2C[ACC_I2C_EXT3].measure1 & 0x0FF);  // 2 byte:  EXT3 Temperature
        buff[IPK2U_EXT3_M1_HI]  = (byte)(sData.AccI2C[ACC_I2C_EXT3].measure1 >> 8);
               
        buff[IPK2U_EXT3_M2_LO]  = (byte)(sData.AccI2C[ACC_I2C_EXT3].measure2 & 0x0FF);  // 2 byte:  EXT3 RH
        buff[IPK2U_EXT3_M2_HI]  = (byte)(sData.AccI2C[ACC_I2C_EXT3].measure2 >> 8);
        
        buff[IPK2U_EXT3_M3_LO]  = (byte)(sData.AccI2C[ACC_I2C_EXT3].measure3 & 0x0FF);  // 2 byte:  EXT3 CO2
        buff[IPK2U_EXT3_M3_HI]  = (byte)(sData.AccI2C[ACC_I2C_EXT3].measure3 >> 8);
               
        buff[IPK2U_EXT3_M4_LO]  = (byte)(sData.AccI2C[ACC_I2C_EXT3].measure4 & 0x0FF);  // 2 byte:  EXT3 VOC
        buff[IPK2U_EXT3_M4_HI]  = (byte)(sData.AccI2C[ACC_I2C_EXT3].measure4 >> 8);
        
        if(Tcheck==0)
        {
        buff[IPK2U_EXT4_M1_LO]  = (byte)(sData.AccI2C[ACC_I2C_EXT4].measure1 & 0x0FF);  // 2 byte:  EXT4 Temperature
        buff[IPK2U_EXT4_M1_HI]  = (byte)(sData.AccI2C[ACC_I2C_EXT4].measure1 >> 8);
               
        buff[IPK2U_EXT4_M2_LO]  = (byte)(sData.AccI2C[ACC_I2C_EXT4].measure2 & 0x0FF);  // 2 byte:  EXT4 RH
        buff[IPK2U_EXT4_M2_HI]  = (byte)(sData.AccI2C[ACC_I2C_EXT4].measure2 >> 8);
        }
        buff[IPK2U_EXT4_M3_LO]  = (byte)(sData.AccI2C[ACC_I2C_EXT4].measure3 & 0x0FF);  // 2 byte:  EXT4 CO2
        buff[IPK2U_EXT4_M3_HI]  = (byte)(sData.AccI2C[ACC_I2C_EXT4].measure3 >> 8);
               
        buff[IPK2U_EXT4_M4_LO]  = (byte)(sData.AccI2C[ACC_I2C_EXT4].measure4 & 0x0FF);  // 2 byte:  EXT4 VOC
        buff[IPK2U_EXT4_M4_HI]  = (byte)(sData.AccI2C[ACC_I2C_EXT4].measure4 >> 8);

        //buff[IPK2U_PFAN_1_LO]  = (byte)(flow_motor_1 & 0x0FF);  // 2 byte:  P FAN 1
        //buff[IPK2U_PFAN_1_HI]  = (byte)(flow_motor_1 >> 8);   

        //buff[IPK2U_PFAN_2_LO]  = (byte)(flow_motor_2 & 0x0FF);  // 2 byte:  P FAN 2
        //buff[IPK2U_PFAN_2_HI]  = (byte)(flow_motor_2 >> 8);
        buff[IPK2U_PFAN_1_LO]  = (byte)(Pressure_CAPS & 0x0FF);  // 2 byte:  P FAN 1
        buff[IPK2U_PFAN_1_HI]  = (byte)(Pressure_CAPS >> 8);   

        buff[IPK2U_PFAN_2_LO]  = (byte)(Pressure_CAPR & 0x0FF);  // 2 byte:  P FAN 2
        buff[IPK2U_PFAN_2_HI]  = (byte)(Pressure_CAPR >> 8);
        

        if(Tcheck==0)
        {
        buff[IPK2U_TEMP_1_LO]  = (byte)(sData.AccI2C[ACC_I2C_SSR].measure1 & 0x0FF);  // 2 byte:  TEMP T1 test value = 14 °C
        buff[IPK2U_TEMP_1_HI]  = (byte)(sData.AccI2C[ACC_I2C_SSR].measure1 >> 8);
        buff[IPK2U_TEMP_2_LO]  = (byte)(sData.AccI2C[ACC_I2C_SSR].measure2 & 0x0FF);  // 2 byte:  TEMP T2 test value = 15 °C
        buff[IPK2U_TEMP_2_HI]  = (byte)(sData.AccI2C[ACC_I2C_SSR].measure2 >> 8);
		buff[IPK2U_TEMP_3_LO]  = (byte)(sData.AccI2C[ACC_I2C_SSR].measure3 & 0x0FF);  // 2 byte:  TEMP T3 test value = 16.57 °C
        buff[IPK2U_TEMP_3_HI]  = (byte)(sData.AccI2C[ACC_I2C_SSR].measure3 >> 8);
        }     
//        //  elettrico: temperatura della resistenza, Acqua, temperatura acqua.
//        if(sData.AccI2C[ACC_I2C_PREHEATER].sts & STS_ACC_ELECT){
//            // ad aria
//           buff[IPK2U_MEASURE_ACC_TW_PREHEATER_LO] = (byte)(sData.AccI2C[ACC_I2C_PREHEATER].measure1 & 0x0FF);  
//           buff[IPK2U_MEASURE_ACC_TW_PREHEATER_HI] = (byte)(sData.AccI2C[ACC_I2C_PREHEATER].measure1 >> 8); // Temperatura dell'aria
//        }else {
//           // ad acqua
//           buff[IPK2U_MEASURE_ACC_TW_PREHEATER_LO] = (byte)(sData.AccI2C[ACC_I2C_PREHEATER].measure2 & 0x0FF);  
//           buff[IPK2U_MEASURE_ACC_TW_PREHEATER_HI] = (byte)(sData.AccI2C[ACC_I2C_PREHEATER].measure2 >> 8); // Temperatura dell'acqua         
//        }  
          
        buff[IPK2U_MEASURE_CO2_LO]       =  (byte)(sData.measure_CO2_max & 0x0FF);
        buff[IPK2U_MEASURE_CO2_HI]       =  (byte)(sData.measure_CO2_max >> 8);
        buff[IPK2U_MEASURE_RH]           =  (byte)(sData.measure_RH_max);         // 1 solo Byte da 0 a 100%
        buff[IPK2U_MEASURE_VOC]          =  (byte)(sData.measure_VOC_max);        // 1 solo byte da 0 a 100PPM
        buff[IPK2U_MEASURE_PRESSURE_LO]  =  (byte)(sData.measure_pressure & 0x0FF); 
        buff[IPK2U_MEASURE_PRESSURE_HI]  =  (byte)(sData.measure_pressure >> 8);
        buff[IPK2U_MEASURE_AIRFLOW_LO]   =  (byte)(sData.measure_airflow & 0x0FF);
        buff[IPK2U_MEASURE_AIRFLOW_HI]   =  (byte)(sData.measure_airflow >> 8);
        
        buff[IPK2U_MEASURE_ACC_DSP_LO]   =  (byte)(sData.AccI2C[ACC_I2C_DPP].measure1 & 0x0FF);
        buff[IPK2U_MEASURE_ACC_DSP_HI]   =  (byte)(sData.AccI2C[ACC_I2C_DPP].measure1 >> 8);
        // da 0/10 a 10/10
        buff[IPK2U_LEVEL_LINK_HEATER]    =  sData.AccI2C[ACC_I2C_HEATER].level_link;  
        buff[IPK2U_LEVEL_LINK_COOLER]    =  sData.AccI2C[ACC_I2C_COOLER].level_link;  
        buff[IPK2U_LEVEL_LINK_PREHEATER] =  sData.AccI2C[ACC_I2C_PREHEATER].level_link;
        
        if(DigitAccessoryOperating(ACC_I2C_PCAF)) // la contemporaneità dei 2 accessori è impossibile.
          buff[IPK2U_LEVEL_LINK_CAP_CAF] =  sData.AccI2C[ACC_I2C_PCAF].level_link; 
        else
          buff[IPK2U_LEVEL_LINK_CAP_CAF] =  sData.AccI2C[ACC_I2C_PCAP].level_link;    
          
        buff[IPK2U_LEVEL_LINK_CTRL_FAN]  =  sData.level_link_CTRLFAN;    
       // 2431       
        size_unit = 0;
        b = (read_byte_eeprom(ADDR_EEP(SerialString[4])) - '0');
        if(b <=2)
          size_unit = b * 10000; 
        b = (read_byte_eeprom(ADDR_EEP(SerialString[5])) - '0'); 
          size_unit += (b * 1000);  
        b = (read_byte_eeprom(ADDR_EEP(SerialString[6])) - '0'); 
          size_unit += (b * 100);    
      
        Caf_Max = GetMaxAirFlow();
        
        buff[IPK2U_SET_MAX_VAL_CAF]      = (Caf_Max/100);
      
        if(size_unit <= 200)
          buff[IPK2U_MAX_SETP_CAP]  = 25; 
        else if (size_unit <= 1200) 
          buff[IPK2U_MAX_SETP_CAP]  = 35;
        else  
          buff[IPK2U_MAX_SETP_CAP]  = 45;   
          
        buff[IPK2U_SET_MAX_SETP_CAF_LO] = (byte)(Caf_Max & 0x0FF);    // Nuovo valore intero es: 450mq/h -> 450
        buff[IPK2U_SET_MAX_SETP_CAF_HI] = (byte)(Caf_Max >> 8);  
       
        buff[IPK2U_INCREASE_SPEED_RH_CO2] = sData.increase_speed_RH_CO2; // valore da 0% a 100% di incremento della velocità rispetto al set-poin a causa: RH alta, o CO2 alta, o VOCs alta.
        buff[IPK2U_STATUS_WEEKLY]         = sData.status_weekly; 
       
//////// nel caso di controllo di coerenza, riscrivo alcuni valori

        if(Tcheck ==1)
        {
        buff[IPK2U_EXT2_M1_LO]  = (byte)(S2_T & 0x0FF);
        buff[IPK2U_EXT2_M1_HI]  = (byte)(S2_T >> 8);
        buff[IPK2U_EXT2_M2_LO]  = (byte)(S2_RH & 0x0FF);
        buff[IPK2U_EXT2_M2_HI]  = (byte)(S2_RH >> 8);                
          
        buff[IPK2U_TEMP_1_LO]  = (byte)(T1 & 0x0FF);
        buff[IPK2U_TEMP_1_HI]  = (byte)(T1 >> 8);
        buff[IPK2U_TEMP_2_LO]  = (byte)(T2 & 0x0FF);
        buff[IPK2U_TEMP_2_HI]  = (byte)(T2 >> 8);
		buff[IPK2U_TEMP_3_LO]  = (byte)(T3 & 0x0FF);
        buff[IPK2U_TEMP_3_HI]  = (byte)(T3 >> 8);
        buff[IPK2U_MEASURE_TS_LO]  = (byte)(T5 & 0x0FF);
        buff[IPK2U_MEASURE_TS_HI]  = (byte)(T5 >> 8);
        buff[IPK2U_MEASURE_TE_LO]  = (byte)(T4 & 0x0FF);
        buff[IPK2U_MEASURE_TE_HI]  = (byte)(T4 >> 8); 
        
        buff[IPK2U_EXT1_M1_LO]  = (byte)(S1_T & 0x0FF);
        buff[IPK2U_EXT1_M1_HI]  = (byte)(S1_T >> 8);
        buff[IPK2U_EXT1_M2_LO]  = (byte)(S1_RH & 0x0FF);
        buff[IPK2U_EXT1_M2_HI]  = (byte)(S1_RH >> 8);
        
        buff[IPK2U_EXT4_M1_LO]  = (byte)(S4_T & 0x0FF);
        buff[IPK2U_EXT4_M1_HI]  = (byte)(S4_T >> 8);
        buff[IPK2U_EXT4_M2_LO]  = (byte)(S4_RH & 0x0FF);
        buff[IPK2U_EXT4_M2_HI]  = (byte)(S4_RH >> 8);
        
        
        }
              
        len =IPK2U_CKS_LO + 2;
    }   
  
    index_packet_data++;

   
    sData.level_link_ModBus = (22 - cnt_no_link_Modbus) / 2; 
    
    if(cnt_no_link_Modbus < 22)
      cnt_no_link_Modbus++;
    else
      InsertAlarm(ALM_EB_REM_CTRL);  
          
    // inviamo il comando
    Send_Message_Ser2(&buff[0], len);     
}  
  
//----------------------------------------------------------------------------------
// Func: polling_serial_2
//
// Description:
//  La funzione gira nello sheduler 0.3 sec. 
//  Le schede da pollare sono 2, la Control_Fan e il modbus e è confermata
//  la presenza dal bit D8.
//-----------------------------------------------------------------------------------
byte index_poll_ser2 =0;

int polling_serial_2(void)
{

wdt_restart(WATCH_DOG);       
#ifdef WORK_WDT       
  //ClearWDT();  // teniamo vivo il WacthDog
#endif   

  LED5_BLINK;
  
  if ( count_blink_led5 >= 500)
  {
	  ioport_toggle_pin_level(pLED5);
	  count_blink_led5 = 0;
  }
  
  if(/*digitalRead(pInsertEB_ModBus)*/ ioport_get_pin_level(pInsertEB_ModBus) )
     index_poll_ser2++; // se viene rilevata la scheda, allora pollala
  else { 
     index_poll_ser2 = 0; 
     cnt_no_link_Modbus = 0;                      
     RemoveAlarm(ALM_EB_REM_CTRL);  
     if(sData.Status_RemoteCTRL)
       update_EB_RemoteCTRL(0);
  }   
   
  if(index_poll_ser2 & 0x01) 
     polling_ModBus();   
  else 
     polling_CTRL_FAN(); // la scheda FANS ogni 1,2 sec   
        
    
  
  return 1; // rimaniamo nello scheduler
}
