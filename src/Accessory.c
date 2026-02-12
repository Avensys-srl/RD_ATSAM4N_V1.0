/*
 * Accessory.cpp
 *
 * Data creazione: 27/01/2014
 
 * Autore: Maurizio Colmone.
 
 * Descrizione: Tutte le funzioni per gestire i 13 accessori digitali (I2C)
 */ 

#include "asf.h"
#include "common/build_config.h"

#include "data.h"
#include "eeprom_map.h"
#include "alarm.h"
#include <math.h>
#include "MASTER_I2C.h"
#include "Protocol_Ser1.h"

#define byte	Byte

#ifdef VIEW_DEBUG_CODE_NO_KTS  //''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' x_dbg
//#define VIEW_ACCESSORY
#endif


//--------------- MACRO -------------------------
#define NUM_CONSECUTIVE_CALLS_MISSING   36 // numero di chiamate consecutive senza risposta 
#define word uint16_t

//--------------- gloabl Vars -------------------------
static Byte idxSeq = 0;
extern SDATA  sData;

//-----------------------------------------------------
// Variabili di appoggio per TEST IPEHD
//-----------------------------------------------------

extern bool flag_ipehd;
extern int counter_ipehd;

//-----------------------------------------------------
// Variabili di appoggio per calcolo portata se presenti
// FLW1 e FLW 2.
//-----------------------------------------------------

#define K_FLOW1          1
#define K_FLOW2          2
#define MAX_KF_SAMPLE    1

extern int   contaflow_1;
extern int   contaflow_2;
extern float sumpress_1;
extern float sumpress_2;
extern short flow_motor_1;  //FLW 1
extern short flow_motor_2;  //FLW 2

//-----------------------------------------------------
// Variabili di appoggio per EBPD e SSR
//-----------------------------------------------------

extern Byte info_SSR;
extern Byte info_EBPD; // definisco un valore iniziale fuori dal range in modo tale da obbligare la sincronizzazione sin dal primo ciclo.
extern Byte change_SSR;
extern Byte change_EBPD;
extern Byte change_EBP2;

extern Byte   countdown_Update_EepromToShadow;

extern short cnt_no_Pressure_for_Electrical_Heater;

short Pressure_CAPS = 0;
short Pressure_CAPR = 0;

short Airflow_CAPS = 0;
short Airflow_CAPR = 0;

//-----------------------------------------------------
// Definizione timer di sistema.
//-----------------------------------------------------
extern S_CLOCK_WEEK    sClockWeek;

extern unsigned long countHoursFunz;

short CO2_1_old_Value = 0;
short CO2_2_old_Value = 0;

extern volatile uint32_t CO2_1_counter;
extern volatile uint32_t CO2_2_counter;

long CO2_1_Raw_Gas_Value = 0;
long CO2_2_Raw_Gas_Value = 0;

short CO2_1_array[48] = {0};
short CO2_2_array[48] = {0};
	
extern volatile uint32_t CO2_1_Average_Counter;
extern volatile uint32_t CO2_2_Average_Counter;

Byte Index_CO2_1 = 0;
Byte Index_CO2_2 = 0;

bool Clean_Event_Flag1 = false;
bool Clean_Event_Flag2 = false;

uint8_t BPD_FW = 0;

extern Bool Buzzer_Off;

uint32_t Zaki3 = 0;

//----------------------------------------------------
//        CONFIGURAZIONI   HARDWARE
//----------------------------------------------------

extern void ActiveBuzzer(byte numBeepBeep, unsigned int freq_100ms);
byte write_func_compressor_Dxd(void);
byte write_regulate_Dxd(void);
Byte read_Access_clima(byte addrSlave, byte iAcc, byte byte_event, byte bit_event, byte msk_clear_event, byte msk_event_poll);
Byte read_Access_Dxd(byte addrSlave, byte iAcc, byte byte_event, byte bit_event, byte msk_clear_event, byte msk_event_poll);
Byte read_Sensor_Air(byte addrSlave, byte iAcc, byte byte_event, byte bit_event, byte msk_clear_event);
Byte read_ext_Sensor(byte addrSlave, byte iAcc, byte byte_event, byte bit_event, byte msk_clear_event);
byte read_Sensor_Pressure(byte addrSlave, byte iAcc, byte byte_event, byte bit_event, byte msk_clear_event);
byte read_DSC(byte addrSlave, byte iAcc, byte byte_event, byte bit_event, byte msk_clear_event);
short calcola_portata(float pressione, int flow);
extern unsigned long millis( void );

//----------------------------------------------------
// Ritorna lo stato dell'accessorio se è operativo 
// o no (link KO / Diabilitato).
//----------------------------------------------------
Byte DigitAccessoryOperating(int nAcc_I2C)
{  
   if ((sData.AccI2C[nAcc_I2C].sts & STS_ACC_OPERATIVE) && (nAcc_I2C < TOT_ACCESSORY_I2C))
     return 1;
  
   return 0; 
}  

//----------------------------------------------------
// Ritorna lo stato dell'acccessorio Acceso/ Spento
//----------------------------------------------------
Byte DigitAccessoryOn(int nAcc_I2C)
{  
   if ((sData.AccI2C[nAcc_I2C].sts & STS_ACC_ON) && (nAcc_I2C < TOT_ACCESSORY_I2C))
     return 1;
  
   return 0; 
}  

//----------------------------------------------------------
// Ritorna lo stato dell'acccessorio se è inserito nella eeprom
//
// input: 
//  nAcc = ACC_EEP_PEHD,  ACC_EEP_PHWD,  ACC_EEP_HWD,..
//----------------------------------------------------------
Byte AccessoryPresent_EepList(int nAccEep)
{
   Byte nb, msk, val;
   
   nb = (nAccEep >> 3);
   msk  = 1 << (nAccEep & 0x07);  
   
   val = read_byte_eeprom(ADDR_EEP(AccessoyHW[nb]));
     
   if(val & msk)
      return 1;
   
   return 0;   
}


//----------------------------------------------------------
//  Accendi l'accessorio, inviando il comando tramte I2C.
//----------------------------------------------------------
void TAG_DigitAccessoryOn(int nAcc_I2C)
{ 
    if(nAcc_I2C < TOT_ACCESSORY_I2C) {
       if(!DigitAccessoryOn(nAcc_I2C)) // non è acceso invia il comando di accensione
         sData.AccI2C[nAcc_I2C].comand = (WRITE_CMD | CMD_ON);
              
       sData.AccI2C[nAcc_I2C].sts |= STS_CMD_ACC_ON;   
    }   
}  

//----------------------------------------------------------
// Spegni l'accessorio, inviando il comando tramte I2C.
//----------------------------------------------------------
void TAG_DigitAccessoryOff(int nAcc_I2C)
{ 
   if(nAcc_I2C < TOT_ACCESSORY_I2C) {
     if(DigitAccessoryOn(nAcc_I2C))   // se è acceso invia il comando di spegnimento.
       sData.AccI2C[nAcc_I2C].comand = WRITE_CMD; 

     
     sData.AccI2C[nAcc_I2C].sts &= ~STS_CMD_ACC_ON; 
     
  }   
} 

//----------------------------------------------------------
// Spegni l'accessorio, inviando il comando tramte I2C.
//----------------------------------------------------------
void TAG_DigitDisable(int nAcc_I2C)
{ 
   if(nAcc_I2C < TOT_ACCESSORY_I2C) 
      sData.AccI2C[nAcc_I2C].comand = (WRITE_CMD | DISABLE_SLAVE);   
}  

//----------------------------------------------------------
// Segna che l'accessorio Digitale è operativo.
//----------------------------------------------------------
void TAG_DigitAccessoryOperating(int nAcc_I2C)
{ 
    if(nAcc_I2C < TOT_ACCESSORY_I2C)  
       sData.AccI2C[nAcc_I2C].sts |= (STS_ACC_OPERATIVE | STS_ACC_CONNECT);   
}  

//----------------------------------------------------------
// Togli l'operatitività dell'Accessorio Digitale 
//----------------------------------------------------------
void TAG_DigitAccessoryNotOperating(int nAcc_I2C)
{ 
   if(nAcc_I2C < TOT_ACCESSORY_I2C) 
    // sData.AccI2C[nAcc_I2C].sts &= ~STS_ACC_OPERATIVE;
      sData.AccI2C[nAcc_I2C].sts &= 0xFD;    
}  

//------------------------------------------------------------------------------
// In base all'accessorio digitale che gira sul Bus I2C, ritorna il byte ed il 
// bit dell'array <AccessoyHW[4]>  in eeprom.
//------------------------------------------------------------------------------
void located_pos_AccessoyToEep(byte nAcc_I2C, byte sts, byte *nbyte_eep, byte *nbit_eep)
{ 
   switch(nAcc_I2C)
   {
        case ACC_I2C_HEATER:          
          if(sts & STS_ACC_ELECT){
            *nbyte_eep = (ACC_EEP_EHD >> 3);
            *nbit_eep  = (ACC_EEP_EHD & 0x07); 
          }else{
            *nbit_eep  = (ACC_EEP_HWD >> 3); 
            *nbit_eep  = (ACC_EEP_HWD & 0x07);  
          }  
        break;
        
        case ACC_I2C_COOLER:
          *nbyte_eep = (ACC_EEP_CWD  >> 3);
          *nbit_eep  = (ACC_EEP_CWD  & 0x07);           
        break;
             
        case ACC_I2C_PREHEATER: 
          if(sts & STS_ACC_ELECT) {
             *nbyte_eep = (ACC_EEP_PEHD  >> 3);
             *nbit_eep  = (ACC_EEP_PEHD  & 0x07);   
          }else{
             *nbyte_eep = (ACC_EEP_PHWD  >> 3);
             *nbit_eep  = (ACC_EEP_PHWD  & 0x07);   
          }  
        break;
                
        case ACC_I2C_CO2_1:
           *nbyte_eep = (ACC_EEP_P1CO2 >> 3);
           *nbit_eep  = (ACC_EEP_P1CO2 & 0x07);  
        break;
        
        case ACC_I2C_CO2_2:
           *nbyte_eep = (ACC_EEP_P2CO2 >> 3);
           *nbit_eep  = (ACC_EEP_P2CO2 & 0x07);   
        break; 
        
        case ACC_I2C_RH_1:
           *nbyte_eep = (ACC_EEP_P1RH >> 3);
           *nbit_eep  = (ACC_EEP_P1RH & 0x07);  
        break; 
        
        case ACC_I2C_RH_2:
           *nbyte_eep = (ACC_EEP_P2RH >> 3);
           *nbit_eep  = (ACC_EEP_P2RH & 0x07);  
        break; 
       
        case ACC_I2C_VOC:
           *nbyte_eep = (ACC_EEP_P1VOC >> 3);
           *nbit_eep  = (ACC_EEP_P1VOC & 0x07);  
        break; 
        
        case ACC_I2C_AWP:
           *nbyte_eep = (ACC_EEP_AWP >> 3);
           *nbit_eep  = (ACC_EEP_AWP & 0x07);  
        break; 
       
        case ACC_I2C_PCAP:
           *nbyte_eep = (ACC_EEP_PCAP >> 3);
           *nbit_eep  = (ACC_EEP_PCAP & 0x07);  
        break; 
         
        case ACC_I2C_PCAF:
           *nbyte_eep = (ACC_EEP_PCAF >> 3);
           *nbit_eep  = (ACC_EEP_PCAF & 0x07);  
        break;
        
        case ACC_I2C_DPP:
           *nbyte_eep = (ACC_EEP_DPP >> 3);
           *nbit_eep  = (ACC_EEP_DPP & 0x07);  
        break;
        
         case ACC_I2C_DXD:
           *nbyte_eep = (ACC_EEP_DXD >> 3);
           *nbit_eep  = (ACC_EEP_DXD & 0x07);  
        break; 
 
         case ACC_I2C_EXT1:
           *nbyte_eep = (ACC_EEP_EXT1 >> 3);
           *nbit_eep  = (ACC_EEP_EXT1 & 0x07);  
        break; 

         case ACC_I2C_EXT2:
           *nbyte_eep = (ACC_EEP_EXT2 >> 3);
           *nbit_eep  = (ACC_EEP_EXT2 & 0x07);  
        break; 

         case ACC_I2C_EXT3:
           *nbyte_eep = (ACC_EEP_EXT3 >> 3);
           *nbit_eep  = (ACC_EEP_EXT3 & 0x07);  
        break; 

         case ACC_I2C_EXT4:
           *nbyte_eep = (ACC_EEP_EXT4 >> 3);
           *nbit_eep  = (ACC_EEP_EXT4 & 0x07);  
        break;         
          
        case ACC_I2C_FLW1:
           *nbyte_eep = (ACC_EEP_FLW1 >> 3);
           *nbit_eep  = (ACC_EEP_FLW1 & 0x07);  
        break;         		  
		
	case ACC_I2C_FLW2:
           *nbyte_eep = (ACC_EEP_FLW2 >> 3);
           *nbit_eep  = (ACC_EEP_FLW2 & 0x07);  
        break;  
		
	case ACC_I2C_EBPD:
           *nbyte_eep = (ACC_EEP_EBPD >> 3);
           *nbit_eep  = (ACC_EEP_EBPD & 0x07);  
        break;  

	case ACC_I2C_SSR:
           *nbyte_eep = (ACC_EEP_SSR >> 3);
           *nbit_eep  = (ACC_EEP_SSR & 0x07);  
        break;  
        
        case ACC_I2C_EBP2:
           *nbyte_eep = (ACC_EEP_EBP2 >> 3);
           *nbit_eep  = (ACC_EEP_EBP2 & 0x07);  
        break;
        
        case ACC_I2C_DSC:
           *nbyte_eep = (ACC_EEP_DSC >> 3);
           *nbit_eep  = (ACC_EEP_DSC & 0x07);  
        break;
		
     }                
}  

//------------------------------------------------------------------------------
// Inserisci l'accessorio digitale (bus I2C) nella lista in eeprom e 
// nell'unita'. Vedi tabella riassuntiva.
//
//  --------------------------+--------------------------
//          nAcc_I2C          |      AccessoyHW  
//           (Ram)            | (eeprom : [byte].bits)
//  --------------------------+--------------------------
//    ACC_I2C_HEATER    =  0     EHD, HWD  = [0].3/2
//    ACC_I2C_COOLER    =  1          CWD  = [0].4
//    ACC_I2C_PREHEATER =  2    PHWD, PEHD = [0].1/0  
//    ACC_I2C_CO2_1     =  3         P1CO2 = [2].6
//    ACC_I2C_CO2_2     =  4         P2CO2 = [2].7
//    ACC_I2C_RH_1      =  5          P1RH = [2].3
//    ACC_I2C_RH_2      =  6          P2RH = [2].4
//    ACC_I2C_VOC       =  7         P1VOC = [2].1
//    ACC_I2C_AWP       =  8          AWP  = [0].5
//    ACC_I2C_PCAP      =  9          PCAP = [1].5
//    ACC_I2C_PCAF      = 10          PCAF = [1].6
//    ACC_I2C_DPP       = 11           DPP = [1].7
//    ACC_I2C_DXD       = 12           DXD = [0].7
//  --------------------------+--------------------------
//------------------------------------------------------------------------------
void InsertDigitAccessory(int nAcc_I2C) 
{
  byte nbit, nbyte, val;
  word addr;
  
  if ( Buzzer_Off )
	return;
	
  if(nAcc_I2C < TOT_ACCESSORY_I2C) {
     TAG_DigitAccessoryOperating(nAcc_I2C);
     sData.AccI2C[nAcc_I2C].level_link  = 10;        // livello della comunicazione (valore espresso in 10/10) 
     sData.AccI2C[nAcc_I2C].cnt_link_ok = 1;         // Contatore Link buoni
     sData.AccI2C[nAcc_I2C].cnt_link_ko = 0;         // Contatore Link falliti
     sData.AccI2C[nAcc_I2C].cnt_persist_link_ko = 0; // contatore di persistenza di collegamenti Falliti
     
     located_pos_AccessoyToEep(nAcc_I2C, sData.AccI2C[nAcc_I2C].sts,  &nbyte, &nbit);
      
     addr = ADDR_EEP(AccessoyHW[nbyte]);     
     val = read_byte_eeprom(addr);                
     
     val |= (1 << nbit);    
     write_byte_eeprom(addr, val);
	 
	 Serial_println1(" ");
	 Serial_print1("addr: ");
	 Serial_print(addr, DEC);
	 Serial_print1("  val: ");
	 Serial_print(val, HEX);
	 Serial_println1(" ");
	 
     // aggiorniamo anche il contatore dell'area delle info.
     addr = ADDR_EEP(cntUpdate_info);
     val = read_byte_eeprom(addr);
     val++;
     write_byte_eeprom(addr, val); 
     
     // attiva il buzzer per segnalare che è stato rilevato un  nuovo HW
     ActiveBuzzer(2, 3); // 2beep, 0,3 sec.
	 Buzzer_Off = true;
     
     // Verifichiamo le maschere dei bit di controllo motori, in base alla nuova config.
	 // Let's check the motor control bit masks, based on the new config.
     if((nAcc_I2C == ACC_I2C_PCAP) || (nAcc_I2C == ACC_I2C_PCAF))
       check_control_speed_func();
	   
	 if(nAcc_I2C == ACC_I2C_FLW1) 
		check_control_speed_func1();

#ifdef VIEW_ACCESSORY  //''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' x_dbg
    Serial_println1(" ");
    Serial_print1("Insert Slave: ");
    Serial_print(nAcc_I2C, DEC);
    Serial_print1("  EEP_Acc: ");
    val = read_byte_eeprom(ADDR_EEP(AccessoyHW[0]));  
    Serial_print(val, HEX);
    Serial_print1(" ");
    val = read_byte_eeprom(ADDR_EEP(AccessoyHW[1])); 
    Serial_print(val, HEX);
    Serial_print1(" ");
    val = read_byte_eeprom(ADDR_EEP(AccessoyHW[2]));
    Serial_println(val, HEX);
#endif   //'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' x_dbg     

  }  
  
}  

//------------------------------------------------------------------------------
// Rimuove l'accessorio digitale dall'unità e dalla lista in eeprom
//------------------------------------------------------------------------------
void RemoveDigitAccessory(int nAcc_I2C) 
{
  byte nbit= 0, nbyte=3, val;
  int addr;
  
  if ( Buzzer_Off )
	return;
  
  if(nAcc_I2C < TOT_ACCESSORY_I2C) { 
     located_pos_AccessoyToEep(nAcc_I2C, sData.AccI2C[nAcc_I2C].sts,  &nbyte, &nbit);
     
     sData.AccI2C[nAcc_I2C].numCmdDisable = 0;
     
     sData.AccI2C[nAcc_I2C].sts = 0;
     sData.AccI2C[nAcc_I2C].comand   = 0;
     sData.AccI2C[nAcc_I2C].measure1 = 0;            // misura 1
     sData.AccI2C[nAcc_I2C].measure2 = 0;            // misura 2
     sData.AccI2C[nAcc_I2C].measure3 = 0;            // misura 3
     sData.AccI2C[nAcc_I2C].measure4 = 0;            // misura 4
     sData.AccI2C[nAcc_I2C].data     = 0;            // per CWD, EHD è la taglia, per le sonde è la tipologia es: da canale o d'ambiente 
     sData.AccI2C[nAcc_I2C].status_comand = 0;
     // aggiorniamo anche il contatore dell'area delel info.
     addr = ADDR_EEP(AccessoyHW[nbyte]);     
     val = read_byte_eeprom(addr);                
     
     val &= ~(1 << nbit);    
     write_byte_eeprom(addr, val); 
     addr = ADDR_EEP(cntUpdate_info);
     val = read_byte_eeprom(addr);
     val++;
     write_byte_eeprom(addr, val);  
    
     // Verifichiamo le maschere dei bit di controllo motori, in base alla nuova config.
     if((nAcc_I2C == ACC_I2C_PCAP) || (nAcc_I2C == ACC_I2C_PCAF) || (nAcc_I2C == ACC_I2C_FLW1) || (nAcc_I2C == ACC_I2C_FLW2)) {
          word enab_func, msk_enab_func;
          byte cnt_info;
          
          enab_func     = read_word_eeprom(ADDR_EEP(Enab_Fuction));
          msk_enab_func = read_word_eeprom(ADDR_EEP(msk_Enab_Fuction)); 
          cnt_info      = read_byte_eeprom(ADDR_EEP(cntUpdate_info));
          //  (LOW)  bit[7]:CAF  | bit[6]:CAP  | bit[5]:CSF ...
          //  (HIGH) ...
          enab_func  &= ~0x00E0;
          enab_func  |= (1 << ENAB_CSF);
		  /*if (( msk_enab_func & 0x0080) == 0x0080 ) // CAF enabled
			  {
				enab_func  |= (1 << ENAB_CAF);
				msk_enab_func &= ~0x0040; // 0x00C0	
			  }
		  else
		    msk_enab_func &= ~0x00C0; // 0x00C0*/
          cnt_info++;
          
          write_word_eeprom(ADDR_EEP(Enab_Fuction),          enab_func);
          write_word_eeprom(ADDR_EEP(msk_Enab_Fuction),  msk_enab_func);
          write_byte_eeprom(ADDR_EEP(cntUpdate_info),         cnt_info);   
          
          countdown_Update_EepromToShadow = 0;  // aggiorniamo velocemente la eeprom.            
     }  
     
	 if((nAcc_I2C == ACC_I2C_EBPD) || (nAcc_I2C == ACC_I2C_EBP2))
		BPD_FW = 0;
		 
     // attiva il buzzer per segnalare che è stato cancellato un accessorio
     ActiveBuzzer(1, 6); // 1 beep, 0,6 sec.
	 Buzzer_Off = true;   

#ifdef VIEW_ACCESSORY  //''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' x_dbg
    Serial_println1(" ");
    Serial_print1("Remove Slave: ");
    Serial_print(nAcc_I2C, DEC);
    Serial_print1("  EEP_Acc: ");
    val = read_byte_eeprom(ADDR_EEP(AccessoyHW[0]));  
    Serial_print(val, HEX);
    Serial_print1(" ");
    val = read_byte_eeprom(ADDR_EEP(AccessoyHW[1])); 
    Serial_print(val, HEX);
    Serial_print1(" ");
    val = read_byte_eeprom(ADDR_EEP(AccessoyHW[2]));
    Serial_println(val, HEX);
#endif   //'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' x_dbg            
  }  
      
}  


//==============================================================================
// Function:  pollingDigitAccessory
// 
// Description:
//   La funzione gira sotto scheduler, sample (0,4 sec) e va a pollare tutti gli 
//   accessori Digitali, secondo una certa sequenza. Precedenza ai comandi 
//   di scrittura (ON/OFF, DISABLE). 
//   il Giro completo lo fa ogni 5 secondi =(0,4 * 12 = 4.8 sec). 
//==============================================================================
static char flag_poll;

int pollingDigitAccessory(void)
{
   byte addrSlave, risp, idxAcc = 0xff, AccPres_eepList, info;
   byte byte_event, bit_event, msk_event_poll, msk_clear_event = 0, nAlmLinkKO;
   byte temp_8;
   unsigned short temp;   
  
   //if ( Buzzer_Off )
		//return;
  
   if ((sData.status_test & MSK_TEST_LEDS) || (counter_ipehd > 0) ) 
     //digitalWrite(pStatusLedI2C,  HIGH);  // led giallo sempre acceso ACCESO  
	 ioport_set_pin_level(pStatusLedI2C, IOPORT_PIN_LEVEL_HIGH); 
   else
     //digitalWrite(pStatusLedI2C,  LOW);  // led SPENTO
	 ioport_set_pin_level(pStatusLedI2C, IOPORT_PIN_LEVEL_LOW);       
   
   // serve per far spegnere il leds 0,4 secondi prima.
   if(flag_poll++ & 0x01)
     return 1;
         
   // andiamo pollare l'access.
   idxAcc = idxSeq;
   
   // incrementiamo per il giro successivo
   idxSeq++;  
   if(idxSeq >= TOT_ACCESSORY_I2C)
     idxSeq = ACC_I2C_HEATER;    
   
   if(change_SSR == 1)
   {
     info = read_byte_eeprom(ADDR_EEP(Set_EHD_mod));   
     if (info_SSR != info)
      {
      change_SSR = 0;
      info_SSR = info;  
      }
   }   

   if(change_EBPD == 1)
   {    
     info = read_byte_eeprom(ADDR_EEP(Set_BPD_mod));  
     if (info_EBPD != info)
      {
      change_EBPD = 0;
	  change_EBP2 = 0;
      info_EBPD = info;  
      }
   }

 
  wdt_restart(WATCH_DOG);    
#ifdef WORK_WDT    
//   ClearWDT();
#endif

   switch (idxAcc) 
   {    
        case ACC_I2C_HEATER: // accessorio di Riscald. Post-ventilazione           
           addrSlave       = ADDR_HEATER; 
           // BYTE[00]: 7.HD_LinkEr, 6.Proximity  , 5.ErrorSize , 4.Ta_OvRange , 3.FaultElect  , 2.Tw_OvRange, 1.NTCwaterKo, 0.NTCair_KO
           byte_event      = (ALM_EHD_NTC_KO >> 3);      // troviamo il byte degli allarmi associati all'acc.
           bit_event       = (ALM_EHD_NTC_KO & 0x07);    // calcoliamo il primo bit degli allarmi dell'accessorio
           msk_clear_event = 0;                          // cancella tutto il byte 
           msk_event_poll  = 0x5F;                       //  i bit 7, 5, vengono assegnati dal modulo rd 
           nAlmLinkKO      = ALM_EHD_LINK;
           
           AccPres_eepList = AccessoryPresent_EepList(ACC_EEP_HWD) + AccessoryPresent_EepList(ACC_EEP_EHD);
        break;
        
        case ACC_I2C_PREHEATER: // accessorio di Riscald. Pre-ventilazione           
           addrSlave       = ADDR_PREHEATER;  
           // BYTE[02]: 7.PHD_LinkEr, 6.Proximity  , 5.ErrorSize , 4.Ta_OvRange , 3.FaultElect  , 2.Tw_OvRange, 1.NTCwaterKo, 0.NTCair_KO 
           byte_event      = (ALM_PEH_NTC_KO >> 3);     // troviamo il byte degli allarmi associati all'acc.  
           bit_event       = (ALM_PEH_NTC_KO & 0x07);   // calcoliamo il primo bit degli allarmi dell'accessorio
           msk_clear_event = 0;                         // cancella tutto il byte 
           msk_event_poll  = 0x5F;                      //  i bit 7, 5, vengono assegnati dal modulo rd 
           nAlmLinkKO      = ALM_PEH_LINK;     
     
           AccPres_eepList = AccessoryPresent_EepList(ACC_EEP_PHWD) + AccessoryPresent_EepList(ACC_EEP_PEHD);
        break;  
    
        case ACC_I2C_COOLER: // accessorio di Raffeddamento
           addrSlave       = ADDR_COOLER;
           // BYTE[01]: 7.CWD_LinkEr, 6.- - - - -  , 5.ErrorSize , 4.Ta_OvRange , 3.FaultElect  , 2.Tw_OvRange, 1.NTCwaterKo, 0.NTCair_KO  
           byte_event      = (ALM_CWD_NTC_KO >> 3);     // troviamo il byte degli allarmi associati all'acc. 
           bit_event       = (ALM_CWD_NTC_KO & 0x07);   // calcoliamo il primo bit degli allarmi dell'accessorio
           msk_clear_event = 0;                         // cancella tutto il byte 
           msk_event_poll  = 0x1F;                      // i bits 7, 5, vengono assegnati dal modulo rd, mentre il bit 6 non gestito
           nAlmLinkKO      = ALM_CWD_LINK;
            
           AccPres_eepList = AccessoryPresent_EepList(ACC_EEP_CWD);
        break;    
     
        case ACC_I2C_CO2_1:   // Sonda CO2, num.1
           addrSlave       = ADDR_PROBE1_CO2;
           // BYTE[03]: ..., 3.S1_CO2LinkEr, 2.S1_CO2_KO, ...   
           byte_event      = (ALM_PCO2_EX1_KO >> 3);    // troviamo il byte degli allarmi associati all'acc. 
           bit_event       = (ALM_PCO2_EX1_KO & 0x07);  // troviamo il primo bit degli allarmi dell'accessorio
           msk_clear_event = 0xF3;                      // cancella i bits 3, 2 
           msk_event_poll  = 0x04;                      // il bit 3 viene assegnato dal modulo RD, riceviamo solo il bit 2 dall'acc.
           nAlmLinkKO      = ALM_PCO2_EX1_LINK;           
           
           AccPres_eepList = AccessoryPresent_EepList(ACC_EEP_P1CO2);    
        break;
         
        case ACC_I2C_CO2_2:   // Sonda CO2, num.2
           addrSlave       = ADDR_PROBE2_CO2; 
           // BYTE[03]: 7.- - - - , 6.S2_CO2LinkEr, 5.S2_CO2_KO, ...
           byte_event      = (ALM_PCO2_EX2_KO >> 3);    // troviamo il byte degli allarmi associati all'acc. 
           bit_event       = (ALM_PCO2_EX2_KO & 0x07);  // troviamo il primo bit degli allarmi dell'accessorio
           msk_clear_event = 0x1F;                      // cancella i bits 7, 6, 5
           msk_event_poll  = 0x10;                      // il bit 6, viene gestito dal modulo RD, riceviamo solo il bit 5 dall'acc.
           nAlmLinkKO      = ALM_PCO2_EX2_LINK;
           
           AccPres_eepList = AccessoryPresent_EepList(ACC_EEP_P2CO2); 
        break;
                          
        case ACC_I2C_RH_1:  // Sonda Umidita', num.1
           addrSlave       = ADDR_PROBE1_RH;
           // BYTE[04]: ... , 4.- - - - , 3.S1_RHLinkEr , 2.S1_RH_KO, ...
           byte_event      = (ALM_PRH_EX1_KO >> 3);     // troviamo il byte degli allarmi associati all'acc. 
           bit_event       = (ALM_PRH_EX1_KO & 0x07);   // troviamo il primo bit degli allarmi dell'accessorio
           msk_clear_event = 0xE3;                      // cancella i bits 4, 3, 2 
           msk_event_poll  = 0x04;                      // il bit 3, viene gestito dal modulo RD, riceviamo solo il bit 2 dall'acc.
           nAlmLinkKO      = ALM_PRH_EX1_LINK;
                       
           AccPres_eepList = AccessoryPresent_EepList(ACC_EEP_P1RH);   
        break;
         
        case ACC_I2C_RH_2:  // Sonda Umidita', num.1
           addrSlave       = ADDR_PROBE2_RH;
            // BYTE[04]: 7.- - - -, 6.S2_RHLinkEr, 5.S2_RH_KO, ...
           byte_event      = (ALM_PRH_EX2_KO >> 3);     // troviamo il byte degli allarmi associati all'acc. 
           bit_event       = (ALM_PRH_EX2_KO & 0x07);   // troviamo il primo bit degli allarmi dell'accessorio
           msk_clear_event = 0x1F;                      // cancella i bits 7, 6, 5
           msk_event_poll  = 0x10;                      // il bit 6, viene gestito dal modulo RD, riceviamo solo il bit 5 dall'acc.
           nAlmLinkKO      = ALM_PRH_EX2_LINK;
                    
           AccPres_eepList = AccessoryPresent_EepList(ACC_EEP_P2RH);       
        break;
           
        case ACC_I2C_VOC: // Sonda Polveri sottili, num.1
           addrSlave       = ADDR_PROBE1_VOC;
           // BYTE[05]:  ... , 4.- - - - , 3.S1_VOCLink, 2.S1_VOC_KO, ...
           byte_event      = (ALM_VOC_EX1_KO >> 3);      // troviamo il byte degli allarmi associati all'acc. 
           bit_event       = (ALM_VOC_EX1_KO & 0x07);    // troviamo il primo bit degli allarmi dell'accessorio
           msk_clear_event = 0xF3;                       // cancella i bits 3, 2 
           msk_event_poll  = 0x04;                       // il bit 3, viene gestito dal modulo RD, riceviamo solo il bit 2 dall'acc.
           nAlmLinkKO      = ALM_VOC_EX1_LINK;
           
           AccPres_eepList = AccessoryPresent_EepList(ACC_EEP_P1VOC);  
        break; 
        
        case ACC_I2C_AWP:  // Sonda Temperatura ambientale interna
           addrSlave       = ADDR_AWP;
           // BYTE[07]: 7.AWP_LinkEr, 6.AWP_KO, ... 
           byte_event      = (ALM_AWP_KO >> 3);       // troviamo il byte degli allarmi associati all'acc. 
           bit_event       = (ALM_AWP_KO & 0x07);     // troviamo il primo bit degli allarmi dell'accessorio
           msk_clear_event = 0x3F;                    // cancella i bits 7, 6 
           msk_event_poll  = 0x40;                    // il bit 7, viene gestito dal modulo RD.
           nAlmLinkKO      = ALM_AWP_LINK;
           
           AccPres_eepList  = AccessoryPresent_EepList(ACC_EEP_AWP);   
        break;    
   
        case ACC_I2C_PCAF: // SENSORS_PRESSURE PORTATA
           addrSlave        = ADDR_PROBE_CAF;
           // BYTE[06]: ..., 5.- - - -, 4.PCAF_LinkEr, 3.PCAF_KO, ...
           byte_event       = (ALM_PCAF_KO >> 3);      // troviamo il byte degli allarmi associati all'acc. 
           bit_event        = (ALM_PCAF_KO & 0x07);    // troviamo il primo bit degli allarmi dell'accessorio
           msk_clear_event  = 0xC7;                    // cancella i bits 5, 4, 3
           msk_event_poll   = 0x08;                    // il bit 4, viene gestito dal modulo RD.
           nAlmLinkKO       = ALM_PCAF_LINK;
            
           AccPres_eepList   = AccessoryPresent_EepList(ACC_EEP_PCAF);  
       break;
        
        case ACC_I2C_PCAP:
           addrSlave       = ADDR_PROBE_CAP;
           // BYTE[06]: ..., 2.- - - - , 1.PCAP_LinkE, 0.PCAP_KO
           byte_event      = (ALM_PCAP_KO >> 3);      // troviamo il byte degli allarmi associati all'acc. 
           bit_event       = (ALM_PCAP_KO & 0x07);    // troviamo il primo bit degli allarmi dell'accessorio
           msk_clear_event = 0xF8;                    // cancella i bits 2, 1, 0
           msk_event_poll  = 0x01;                    // il bit 1, viene gestito dal modulo RD.
           nAlmLinkKO      = ALM_PCAP_LINK;
          
           AccPres_eepList = AccessoryPresent_EepList(ACC_EEP_PCAP);  
        break;
        
        case ACC_I2C_DPP:
           addrSlave       = ADDR_PROBE_DSP;
           // BYTE[06]:  7.DPP_LinkEr, 6.DPP_KO, ...   
           byte_event      = (ALM_DPP_KO >> 3);      // troviamo il byte degli allarmi associati all'acc. 
           bit_event       = (ALM_DPP_KO & 0x07);    // troviamo il primo bit degli allarmi dell'accessorio 
           msk_clear_event = 0x3F;                   // cancella i bits 7, 6 
           msk_event_poll  = 0x40;                   // il bit 7, viene gestito dal modulo RD.
           nAlmLinkKO      = ALM_DPP_LINK;
          
           AccPres_eepList = AccessoryPresent_EepList(ACC_EEP_DPP);  
        break;
        
        case ACC_I2C_DXD:
           addrSlave       = ADDR_DXD;
           // BYTE[06]:  7.DXD_LinkEr, 6.DXD_COMPRES, ...   
           byte_event      = (ALM_DXD_COMPRESS >> 3);      // troviamo il byte degli allarmi associati all'acc. 
           bit_event       = (ALM_DXD_COMPRESS & 0x07);    // troviamo il primo bit degli allarmi dell'accessorio 
           msk_clear_event = 0x3F;                         // cancella i bits 7, 6, 
           msk_event_poll  = 0x40;                         // il bit 7, viene gestito dal modulo RD.
           nAlmLinkKO      = ALM_DXD_LINK;
          
           AccPres_eepList = AccessoryPresent_EepList(ACC_EEP_DXD);  
        break;   

        case ACC_I2C_EXT1: // Multisensore 1
           addrSlave       = ADDR_EXT1;
           
           byte_event      = (ALM_EXT1_KO >> 3);         // troviamo il byte degli allarmi associati all'acc. 
           bit_event       = (ALM_EXT1_KO & 0x07);       // troviamo il primo bit degli allarmi dell'accessorio
           msk_clear_event = 0xCF;                       // cancella i bits 4, 5 
           msk_event_poll  = 0x20;                       // il bit 4, viene gestito dal modulo RD, riceviamo solo il bit 5 dall'acc.
           nAlmLinkKO      = ALM_EXT1_LINK;
           
           AccPres_eepList = AccessoryPresent_EepList(ACC_EEP_EXT1);
        break;   

        case ACC_I2C_EXT2: // Multisensore 2
           addrSlave       = ADDR_EXT2;
           
           byte_event      = (ALM_EXT2_KO >> 3);         // troviamo il byte degli allarmi associati all'acc. 
           bit_event       = (ALM_EXT2_KO & 0x07);       // troviamo il primo bit degli allarmi dell'accessorio
           msk_clear_event = 0x3F;                       // cancella i bits 6, 7 
           msk_event_poll  = 0x80;                       // il bit 6, viene gestito dal modulo RD, riceviamo solo il bit 7 dall'acc.
           nAlmLinkKO      = ALM_EXT2_LINK;

           AccPres_eepList = AccessoryPresent_EepList(ACC_EEP_EXT2);
        break;
        
        case ACC_I2C_EXT3: // Multisensore 3
           addrSlave       = ADDR_EXT3;
           
           byte_event      = (ALM_EXT3_KO >> 3);         // troviamo il byte degli allarmi associati all'acc. 
           bit_event       = (ALM_EXT3_KO & 0x07);       // troviamo il primo bit degli allarmi dell'accessorio
           msk_clear_event = 0xFC;                       // cancella i bits 0, 1 
           msk_event_poll  = 0x02;                       // il bit 0, viene gestito dal modulo RD, riceviamo solo il bit 1 dall'acc.
           nAlmLinkKO      = ALM_EXT3_LINK;

           AccPres_eepList = AccessoryPresent_EepList(ACC_EEP_EXT3);
        break;
        
        case ACC_I2C_EXT4: // Multisensore 4
           addrSlave       = ADDR_EXT4;
           
           byte_event      = (ALM_EXT4_KO >> 3);         // troviamo il byte degli allarmi associati all'acc. 
           bit_event       = (ALM_EXT4_KO & 0x07);       // troviamo il primo bit degli allarmi dell'accessorio
           msk_clear_event = 0xF3;                       // cancella i bits 2, 3 
           msk_event_poll  = 0x04;                       // il bit 2, viene gestito dal modulo RD, riceviamo solo il bit 3 dall'acc.
           nAlmLinkKO      = ALM_EXT4_LINK;

           AccPres_eepList = AccessoryPresent_EepList(ACC_EEP_EXT4);
        break;
		
		//---------------------------------------------
         case ACC_I2C_FLW1: // FLOWMETER 1
           addrSlave       = ADDR_FLW1;
           
           byte_event      = (ALM_FLW1_KO >> 3);         // troviamo il byte degli allarmi associati all'acc. 
           bit_event       = (ALM_FLW1_KO & 0x07);       // troviamo il primo bit degli allarmi dell'accessorio
           msk_clear_event = 0xCF;                       // cancella i bits 4, 5 
           msk_event_poll  = 0x20;                       // il bit 4, viene gestito dal modulo RD, riceviamo solo il bit 5 dall'acc.
           nAlmLinkKO      = ALM_FLW1_LINK;
           
           AccPres_eepList = AccessoryPresent_EepList(ACC_EEP_FLW1);
        break;   
 
         case ACC_I2C_FLW2: // FLOWMETER 2
           addrSlave       = ADDR_FLW2;
           
           byte_event      = (ALM_FLW2_KO >> 3);         // troviamo il byte degli allarmi associati all'acc. 
           bit_event       = (ALM_FLW2_KO & 0x07);       // troviamo il primo bit degli allarmi dell'accessorio
           msk_clear_event = 0x3F;                       // cancella i bits 6, 7 
           msk_event_poll  = 0x80;                       // il bit 6, viene gestito dal modulo RD, riceviamo solo il bit 7 dall'acc.
           nAlmLinkKO      = ALM_FLW2_LINK;
           
           AccPres_eepList = AccessoryPresent_EepList(ACC_EEP_FLW2);
        break; 

 
         case ACC_I2C_EBPD: // EBPD 
           addrSlave       = ADDR_EBPD;
           
           byte_event      = (ALM_EBPD_KO >> 3);         // troviamo il byte degli allarmi associati all'acc. 
           bit_event       = (ALM_EBPD_KO & 0x07);       // troviamo il primo bit degli allarmi dell'accessorio
           msk_clear_event = 0xFC;                      // cancella i bits 0, 1 
           msk_event_poll  = 0x02;                      // il bit 0, viene gestito dal modulo RD, riceviamo solo il bit 1 dall'acc.
           nAlmLinkKO      = ALM_EBPD_LINK;
           
           AccPres_eepList = AccessoryPresent_EepList(ACC_EEP_EBPD);
        break; 

  
         case ACC_I2C_SSR: // SSR
           addrSlave       = ADDR_SSR;
           
           byte_event      = (ALM_SSR_KO >> 3);         // troviamo il byte degli allarmi associati all'acc. 
           bit_event       = (ALM_SSR_KO & 0x07);       // troviamo il primo bit degli allarmi dell'accessorio
           msk_clear_event = 0xF3;                      // cancella i bits 2, 3 
           msk_event_poll  = 0x04;                      // il bit 2, viene gestito dal modulo RD, riceviamo solo il bit 3 dall'acc.
           nAlmLinkKO      = ALM_SSR_LINK;
           
           AccPres_eepList = AccessoryPresent_EepList(ACC_EEP_SSR);
        break; 

         case ACC_I2C_EBP2: // EBP2 
           addrSlave       = ADDR_EBP2;
           
           byte_event      = (ALM_EBP2_KO >> 3);         // troviamo il byte degli allarmi associati all'acc. 
           bit_event       = (ALM_EBP2_KO & 0x07);       // troviamo il primo bit degli allarmi dell'accessorio
           msk_clear_event = 0xCF;                       // cancella i bits 4, 5 
           msk_event_poll  = 0x20;                       // il bit 4, viene gestito dal modulo RD, riceviamo solo il bit 5 dall'acc.
           nAlmLinkKO      = ALM_EBP2_LINK;
           
           AccPres_eepList = AccessoryPresent_EepList(ACC_EEP_EBP2);
        break; 		
        
         case ACC_I2C_DSC: // DSC
           addrSlave       = ADDR_DSC;
           
           byte_event      = (ALM_DSC_KO >> 3);         // troviamo il byte degli allarmi associati all'acc. 
           bit_event       = (ALM_DSC_KO & 0x07);       // troviamo il primo bit degli allarmi dell'accessorio
           msk_clear_event = 0xFC;                       // cancella i bits 0, 1 
           msk_event_poll  = 0x02;                       // il bit 1, viene gestito dal modulo RD, riceviamo solo il bit 5 dall'acc.
           nAlmLinkKO      = ALM_DSC_LINK;
           
           AccPres_eepList = AccessoryPresent_EepList(ACC_EEP_DSC);
        break; 	
        
           if(!AccPres_eepList){
             sData.RegDXD.processor_I2C = 0;
             sData.RegDXD.defrost_on    = 0;
           }  
        break;

   }    
   
   risp = 0;
   // Se non è presente nella lista, ma è connesso, inviamo il comando di cancellazione
   if(!AccPres_eepList && (sData.AccI2C[idxAcc].sts & STS_ACC_CONNECT))
       sData.AccI2C[idxAcc].comand = (WRITE_CMD | DISABLE_SLAVE);
   
   if(sData.AccI2C[idxAcc].comand & WRITE_CMD) 
   {    
     // ---------- SCRITTURA DATI -----------
     //
     //  Addr.Data |Idx Buff.|   Description
     // -----------+---------+--------------------------
     //    0x00        0        ..., b[1] 1=DISCONECT_SLAVE_I2C, b[0] (1=Power_ON /0=Power SAVE))
     //          
     sData.AccI2C[idxAcc].status_comand = (sData.AccI2C[idxAcc].comand & 0x3F); 
     

     
     
     if(WriteSlave(addrSlave, 0x00, sData.AccI2C[idxAcc].comand) == 1) 
     {
        risp = 1;
        
       // per evitare di continuare a forzare solo il comando di write, inseriamo qui l'ipotetico stato, 
       // al giro successivo lo correggiamo dalla lettura  
       if(sData.AccI2C[idxAcc].comand & CMD_ON) {
         TAG_DigitAccessoryOn(idxAcc);
         sData.AccI2C[idxAcc].comand &= ~CMD_ON;
       }else {
         TAG_DigitAccessoryOff(idxAcc);
         sData.AccI2C[idxAcc].sts &= ~STS_ACC_ON;
       }  
       
       if(sData.AccI2C[idxAcc].comand &  DISABLE_SLAVE) {
         sData.Events[byte_event]  &=  msk_clear_event;
         sData.AccI2C[idxAcc].comand = 0;  
         RemoveDigitAccessory(idxAcc);
         return 1;
       }else {
           // dopo il comando andiamo a rileggere lo stato
           if(idxSeq)
             idxSeq--; 
           else 
             idxSeq = (TOT_ACCESSORY_I2C-1);                
       }  
         
       sData.AccI2C[idxAcc].comand &= ~WRITE_CMD;  
       
     }else {
       // comunicazione fallita
        if(sData.AccI2C[idxAcc].comand &  DISABLE_SLAVE) {   
          sData.AccI2C[idxAcc].numCmdDisable++;   
          sData.AccI2C[idxAcc].comand = (WRITE_CMD | DISABLE_SLAVE);
          
          if(idxSeq)
            idxSeq--;  // riesegui almeno 3 tentativi prima di forzare la cancellazione.
          else 
            idxSeq = (TOT_ACCESSORY_I2C-1);  
       
          if (sData.AccI2C[idxAcc].numCmdDisable > 3) {
             RemoveDigitAccessory(idxAcc);  
             sData.AccI2C[idxAcc].comand = 0;  
             return 1;
          }                  
        }
     }    
  }else if(sData.RegDXD.processor_I2C && (sData.AccI2C[ACC_I2C_DXD].sts & STS_ACC_CONNECT)) {
    //
    // ---------- SCRITTURA VALORI ACC. DXD -----------
    //
    if(sData.RegDXD.processor_I2C & CHANGE_FUNC_COMPRESS)        // Cambia modo di funzionare da Caldo a Freddo e viceversa 
      write_func_compressor_Dxd();
    else if (sData.RegDXD.processor_I2C & CHANGE_VAL_0_10V)  // Cambia il valore della potenza del Compressore (da 0.0V a 10.0V).
      write_regulate_Dxd();
    else 
      sData.RegDXD.processor_I2C = 0;  
  
  } else if ((idxAcc == ACC_I2C_EBPD) && (change_EBPD == 0))
      {
      //info = read_byte_eeprom(ADDR_EEP(Set_BPD_mod));
      
      if(WriteSlave(ADDR_EBPD, 0x0B, (byte)info_EBPD)==1)
      change_EBPD = 1;
      
  }else if ((idxAcc == ACC_I2C_EBP2) && (change_EBP2 == 0))
      {
		if(WriteSlave(ADDR_EBP2, 0x0B, (byte)(100-info_EBPD))==1)
			   change_EBP2 = 1;  
		  
	  }

  else if ((idxAcc == ACC_I2C_SSR) && (change_SSR == 0))
     {
      //info = read_byte_eeprom(ADDR_EEP(Set_EHD_mod));
      if(WriteSlave(ADDR_SSR, 0x0B, (byte)info_SSR)==1)
        change_SSR = 1;
 
  } else  {
     //  ----------- LETTURA DATI -----------
     //         
     switch (idxAcc) 
     {  
         //  ---- Slave:   EHD, PHD, HWD, CWD ----  
         //
         case ACC_I2C_HEATER:           
         case ACC_I2C_PREHEATER: // accessorio di Riscald. Pre-ventilazione             
         case ACC_I2C_COOLER: 
           risp = read_Access_clima(addrSlave, idxAcc, byte_event, bit_event, msk_clear_event, msk_event_poll); 
         break;
        
         case ACC_I2C_DXD:
           risp = read_Access_Dxd(addrSlave, idxAcc, byte_event, bit_event, msk_clear_event, msk_event_poll); 
         break;
        
         //  ---- Slave:   P1CO2, P2CO2, P1RH, P2RH, P1VOC, AWP  ----  
         //
         case ACC_I2C_CO2_1:  // 3
         case ACC_I2C_CO2_2:  // 4
         case ACC_I2C_RH_1:   // 5
         case ACC_I2C_RH_2:   // 6
         case ACC_I2C_VOC:    // 7
         case ACC_I2C_AWP:    // 8        
            risp = read_Sensor_Air(addrSlave, idxAcc, byte_event, bit_event, msk_clear_event);   
         break;
         
         // Multisensore
         case ACC_I2C_EXT1:
            risp = read_ext_Sensor(addrSlave, idxAcc, byte_event, bit_event, msk_clear_event);   
         break;
         case ACC_I2C_EXT2:
            risp = read_ext_Sensor(addrSlave, idxAcc, byte_event, bit_event, msk_clear_event);   
         break;         
         case ACC_I2C_EXT3:
            risp = read_ext_Sensor(addrSlave, idxAcc, byte_event, bit_event, msk_clear_event);   
         break;
         case ACC_I2C_EXT4:
            risp = read_ext_Sensor(addrSlave, idxAcc, byte_event, bit_event, msk_clear_event);   
         break;
		 
		 // FLOWMETER
	 case ACC_I2C_FLW1:
            risp = read_ext_Sensor(addrSlave, idxAcc, byte_event, bit_event, msk_clear_event);   
         break;
         case ACC_I2C_FLW2:
            risp = read_ext_Sensor(addrSlave, idxAcc, byte_event, bit_event, msk_clear_event);   
         break;         

		 // BPD EXTCTRL
         case ACC_I2C_EBPD:
         case ACC_I2C_EBP2:
            risp = read_ext_Sensor(addrSlave, idxAcc, byte_event, bit_event, msk_clear_event);   
         break;
		 
		 // EHD MODULATOR
         case ACC_I2C_SSR:
            risp = read_ext_Sensor(addrSlave, idxAcc, byte_event, bit_event, msk_clear_event);   
         break;
		 
         
         case ACC_I2C_PCAP:
            risp = read_Sensor_Pressure(addrSlave, idxAcc, byte_event, bit_event, msk_clear_event); 
         break;
         
         case ACC_I2C_PCAF:
            risp = read_Sensor_Pressure(addrSlave, idxAcc, byte_event, bit_event, msk_clear_event);
         break;
         
         case ACC_I2C_DSC:
            risp = read_DSC(addrSlave, idxAcc, byte_event, bit_event, msk_clear_event);
         break;
         
         case ACC_I2C_DPP:  
           // TODO: da implementare il polling degli accesssori DPP
           risp = 0;
           // ?!?! non sono mai pollati, evitiamo allarmi fasulli
           sData.Events[byte_event] &= msk_clear_event; 
         break;
        
     }     
  }  
      
  // controlliamo lo stato della comunicazione 
  if(risp) {
   
     //digitalWrite(pStatusLedI2C,  HIGH);  // led ACCESO, Messaggio Buono
	  ioport_set_pin_level(pStatusLedI2C, IOPORT_PIN_LEVEL_HIGH);  
     
      if (!AccPres_eepList)  {
        InsertDigitAccessory(idxAcc); 
        sData.AccI2C[idxAcc].level_link = 10;
      }else
        RemoveAlarm(nAlmLinkKO); 
      
      sData.AccI2C[idxAcc].cnt_persist_link_ko = 0;
      sData.AccI2C[idxAcc].cnt_link_ok++; 
         
  }else {
      if (!AccPres_eepList) {
         sData.Events[byte_event] &= msk_clear_event; // se non è creato cancelliamo gli allarmi
         RemoveAlarm(nAlmLinkKO); 
         sData.AccI2C[idxAcc].cnt_persist_link_ko = 0;
      }else {
         sData.AccI2C[idxAcc].cnt_link_ko++; 
         sData.AccI2C[idxAcc].cnt_persist_link_ko++;
         
         if(sData.AccI2C[idxAcc].cnt_persist_link_ko > NUM_CONSECUTIVE_CALLS_MISSING)
            sData.AccI2C[idxAcc].cnt_persist_link_ko = NUM_CONSECUTIVE_CALLS_MISSING;
      }  
  }  

  // monitoriamo lo stato del bus
  if (AccPres_eepList) 
  { 
  
      if(sData.AccI2C[idxAcc].cnt_persist_link_ko >= NUM_CONSECUTIVE_CALLS_MISSING)  
      {
         // segnaliamo solo l'allarme di Link_KO  
         sData.Events[byte_event] &= msk_clear_event; 
         sData.AccI2C[idxAcc].measure1 = 0;
         sData.AccI2C[idxAcc].measure2 = 0;
         sData.AccI2C[idxAcc].measure3 = 0;
         sData.AccI2C[idxAcc].measure4 = 0;
		 sData.AccI2C[idxAcc].measure5 = 0;
         InsertAlarm(nAlmLinkKO);  
         TAG_DigitAccessoryNotOperating(idxAcc);
         
         if(idxAcc == ACC_I2C_DXD)
           sData.RegDXD.defrost_on = 0;
      }  
        
      temp = (sData.AccI2C[idxAcc].cnt_link_ok + sData.AccI2C[idxAcc].cnt_link_ko);
      
      if(temp >= 4) 
          sData.AccI2C[idxAcc].level_link = ((temp  - sData.AccI2C[idxAcc].cnt_link_ko) * 10) / temp;

      if(temp >= 10) {        
         sData.AccI2C[idxAcc].cnt_link_ko = 0;
         sData.AccI2C[idxAcc].cnt_link_ok = 0;
      }      
		
      /*if(idxAcc == ACC_I2C_DSC) {
          Serial_println1(" ");
          Serial_print1("Link OK: ");
          Serial_print(sData.AccI2C[idxAcc].cnt_link_ok, DEC);     
          Serial_print1("  Link faild: ");
          Serial_print(sData.AccI2C[idxAcc].cnt_link_ko, DEC);  
          Serial_print1("  temp: ");
          Serial_println(temp, DEC);           
          Serial_print1("Livel Link: ");
          Serial_println(sData.AccI2C[idxAcc].level_link, DEC);
          Serial_println1(" ");          
       } */ 
      
   }
          
   return 1;  // tieni attiva la funzione nello scheduler..
}  


//---------------------------------------------------------------------------------------------
//                 Read data Slave:   EHD, PHD, HWD, CWD
// 
//   Addr.Data |Idx Buff.|             Description
//  -----------+---------+---------------------------------------------------------------------
//      0x00       W      comand Write, no read  
//      0x01       0      Status: .., bit[3]:1=Elect/0=Water, bit[2]:1=ON/0=OFF,
//                                    bit[1]:1=OPEARATIVO, bit[0]:1=Collegato             
//      0x02       1      Alarms: 7.Link, .., 5.ErrSize, ...
//      0x03       2      byte_low Measures1 (Tair) 
//      0x04       3      byte_hi  Measures1
//      0x05       4      byte_low Measures2 (HWD: Twater / EHD:proximity / ....)
//      0x06       5      byte_hi  Measures2  
//      0x07       6      byte_low Measures1 (Tair) 
//      0x08       7      byte_hi  Measures1
//      0x09       8      byte_low Measures2 (HWD: Twater / EHD:proximity / ....)
//      0x10       9      byte_hi  Measures2  
//      0x11      10      byte size: NULL= Nessuno controllo sul size, 2= 200mq/h,  ....
//      0x12      11      byte info:  6=(SONDA_EXHAUST-7.0 °C), 5=SONDA_EXHAUST, 4=SONDA_SUPPLY, 3=SONDA_RETURN, 2=SONDA_FRESH, 1=SONDA_ACCESSORIO 
//
//    -------- Alarms --------
//  [00]: 7.HD_LinkErr, 6.NO_Press, 5.ErrorSize, 4.Ta_OvRange, 3.FaultElect, 2.Tw_OvRange, 1.NTCwaterKO, 0.NTCair_KO 
//  [01]: 7.CWD_LinkEr, 6.- - - - , 5.ErrorSize, 4.Ta_OvRange, 3.FaultElect, 2.Tw_OvRange, 1.NTCwaterKo, 0.NTCair_KO 
//  [02]: 7.PHD_LinkEr, 6.NO_Press, 5.ErrorSize, 4.Ta_OvRange, 3.FaultElect, 2.Tw_OvRange, 1.NTCwaterKo, 0.NTCair_KO 
//----------------------------------------------------------------------------------------------
Byte read_Access_clima(byte addrSlave, byte iAcc, byte byte_event, byte bit_event, byte msk_clear_event, byte msk_event_poll)
{
   byte risp = 0, byte_alarm = 0;
   byte buff_Data[20];
   int status_read_slave;
  
   status_read_slave = ReadSlave(addrSlave, 0x01, &buff_Data[0], 15); // we put 15 bytes as a standard instead of 8
  
   if(status_read_slave == 1) 
   {
      risp = 1;
	  
	  // extract EHD08 or EHD07 firmware version
	  // if high version number is between 1 and 3 the board is EHD08, and if it is between 4 and 6 the board is EHD07
	  sData.AccI2C[iAcc].measure3 = 0;
	  //sData.AccI2C[iAcc].measure3 = ( buff_Data[6] & 0xF0 ) >> 4;
	  sData.AccI2C[iAcc].measure3 = buff_Data[12];
	  sData.AccI2C[iAcc].measure3 = sData.AccI2C[iAcc].measure3 | ( buff_Data[7] & 0xF0 );
	  sData.AccI2C[iAcc].measure3 = sData.AccI2C[iAcc].measure3 | (((short)buff_Data[0] & 0xE0 ) << 3);
	  //buff_Data[6] =  buff_Data[6] & 0x0F;
	  buff_Data[7] =  buff_Data[7] & 0x0F;
	  buff_Data[0] =  buff_Data[0] & 0x1F;
	  
      sData.AccI2C[iAcc].cnt_persist_link_ko = 0;
      sData.AccI2C[iAcc].cnt_link_ok++; 
      // Status: ...., bit[4]:Ctrl_Water_Ice, bit[3]:1=Elect/0=Water, bit[2]:Stato Acc. 1=ON/0=OFF, bit[1]:1=OPEARATIVO, bit[0]:1=Collegato   
      sData.AccI2C[iAcc].sts = (buff_Data[0] | STS_ACC_CONNECT);
    
      if(sData.AccI2C[iAcc].sts  & STS_ACC_OPERATIVE) 
         TAG_DigitAccessoryOperating(iAcc);                 // segnaliamo che è connesso e operativo     
      
      byte_alarm  = 0;
      
#ifdef _CLEAR_ALARMS_PREHEATER
      if(addrSlave != ACC_I2C_PREHEATER) 
         byte_alarm |= ((buff_Data[1] << bit_event) & msk_event_poll);
#else
      byte_alarm |= ((buff_Data[1] << bit_event) & msk_event_poll);
#endif
      
      // se l'accessorio è elettrico, mascheriamo per circa 1 minuto l'allarme di NO_press, 
      // in modo che i motori aumentino la velocità fino a superare la pressione neccessaria al corretto funzionamento.
      if ((sData.AccI2C[iAcc].sts & STS_ACC_ELECT) && (sData.AccI2C[iAcc].status_comand & CMD_ON)) 
      {
          if (byte_alarm & 0x40) 
          {  // 7.HD_LinkErr, 6.NO_Press, ..
             if (cnt_no_Pressure_for_Electrical_Heater < 9) {
               cnt_no_Pressure_for_Electrical_Heater++; 
               byte_alarm &= ~0x40;
             }
			 else
				sData.AccI2C[iAcc].status_comand &= ~ CMD_ON;
          }else
             cnt_no_Pressure_for_Electrical_Heater = 0;       
      }else 
        cnt_no_Pressure_for_Electrical_Heater = 0; 
        
      
      sData.Events[byte_event] = byte_alarm;
       
      sData.AccI2C[iAcc].measure1 = (word)(((word)buff_Data[3] << 8) | buff_Data[2]);   // misura: T_Air
      sData.AccI2C[iAcc].measure2 = (word)(((word)buff_Data[5] << 8) | buff_Data[4]);   // misura: (HWD: Twater / EHD:proximity)
	  if(iAcc == ACC_I2C_HEATER) {
		  sData.AccI2C[iAcc].measure4 = (word)(((word)buff_Data[9] << 8) | buff_Data[8]);   // misura: Voltage
		  sData.AccI2C[iAcc].measure5 = (word)(((word)buff_Data[11] << 8) | buff_Data[10]);   // misura: Current
	  }

      // byte info:  6=(SONDA_EXHAUST-5.0 °C), 5=SONDA_EXHAUST, 4=SONDA_SUPPLY, 3=SONDA_RETURN, 2=SONDA_FRESH, 1=SONDA_ACCESSORIO 
      sData.AccI2C[iAcc].info = buff_Data[7] & 0x0F;
      
      if(iAcc == ACC_I2C_PREHEATER) {
          if (sData.AccI2C[iAcc].info == 6)
             sData.AccI2C[iAcc].measure1 = sData.measure_Temp[I_PROBE_EXHAUST];
          else if (sData.AccI2C[iAcc].info == 5) 
             sData.AccI2C[iAcc].measure1 = sData.measure_Temp[I_PROBE_EXHAUST]; 
          else if (sData.AccI2C[iAcc].info == 4)  
             sData.AccI2C[iAcc].measure1 = sData.measure_Temp[I_PROBE_SUPPLY];   
          else if (sData.AccI2C[iAcc].info == 3) 
             sData.AccI2C[iAcc].measure1 = sData.measure_Temp[I_PROBE_RETURN];             
          else if (sData.AccI2C[iAcc].info == 2)
             sData.AccI2C[iAcc].measure1 = sData.measure_Temp[I_PROBE_FRESH];
       }  
      // size: 2= 0023, 5= 0053, 22= 0223, 
      //       NULL= Nessuno controllo sul size   
      //       3   = Nessuno controllo sul size, perchè sono usciti i primi PEHD con valore 3 :-((. 
      if ((buff_Data[6] != 0xFF) && (buff_Data[6] != 3)) {     
         sData.AccI2C[iAcc].data = buff_Data[6];  
        
         if(sData.AccI2C[iAcc].data != sData.size_unit) 
            InsertAlarm(ALM_EHD_SIZE + (iAcc * 8));
         else
            RemoveAlarm(ALM_EHD_SIZE + (iAcc * 8)); 
      }else     
         RemoveAlarm(ALM_EHD_SIZE + (iAcc * 8)); 
              
   }  

//--------- x_dbg
//  Serial_println1(" ");
//  Serial_print1("ADDR_SL: ");
//  Serial_println(addrSlave, DEC);
//  Serial_print1("iAcc: ");
//  Serial_println(iAcc, DEC);
//  
//  if(status_read_slave == 1) {
//        Serial_print1("M1: ");
//        Serial_println(sData.AccI2C[iAcc].measure1, DEC);
//        Serial_print1("M2: ");
//        Serial_println(sData.AccI2C[iAcc].measure2, DEC);
//        Serial_print1("Comando: ");
//        Serial_println(sData.AccI2C[iAcc].status_comand, DEC);
//        Serial_print1("Stato ");
//        Serial_println(sData.AccI2C[iAcc].sts, DEC);
//     }    
//  else
//      Serial_println1("Not install!!"); 
//--------- x_dbg      
  
   return risp;
}  


/* -------- Funzione Pronta per i 12 byte (tutte le misure) ZH
byte read_Access_clima(byte addrSlave, byte iAcc, byte byte_event, byte bit_event, byte msk_clear_event, byte msk_event_poll)
{
   byte risp = 0, byte_alarm = 0;
   byte buff_Data[12];
   int status_read_slave;
  
   status_read_slave = ReadSlave(addrSlave, 0x01, &buff_Data[0], 12);
  
   if(status_read_slave == 1) 
   {
      risp = 1; 
      sData.AccI2C[iAcc].cnt_persist_link_ko = 0;
      sData.AccI2C[iAcc].cnt_link_ok++; 
      // Status: ...., bit[4]:Ctrl_Water_Ice, bit[3]:1=Elect/0=Water, bit[2]:Stato Acc. 1=ON/0=OFF, bit[1]:1=OPEARATIVO, bit[0]:1=Collegato   
      sData.AccI2C[iAcc].sts = (buff_Data[0] | STS_ACC_CONNECT);
    
      if(sData.AccI2C[iAcc].sts  & STS_ACC_OPERATIVE) 
         TAG_DigitAccessoryOperating(iAcc);                 // segnaliamo che è connesso e operativo     
      
      byte_alarm  = 0;
#ifdef _CLEAR_ALARMS_PREHEATER
      if(addrSlave != ACC_I2C_PREHEATER) 
         byte_alarm |= ((buff_Data[1] << bit_event) & msk_event_poll);
#else
      byte_alarm |= ((buff_Data[1] << bit_event) & msk_event_poll);
#endif
      
      // se l'accessorio è elettrico, mascheriamo per circa 1 minuto l'allarme di NO_press, 
      // in modo che i motori aumentino la velocità fino a superare la pressione neccessaria al corretto funzionamento.
      if ((sData.AccI2C[iAcc].sts & STS_ACC_ELECT) && (sData.AccI2C[iAcc].status_comand & CMD_ON)) 
      {
          if (byte_alarm & 0x40) 
          {  // 7.HD_LinkErr, 6.NO_Press, ..
             if (cnt_no_Pressure_for_Electrical_Heater < 9) {
               cnt_no_Pressure_for_Electrical_Heater++; 
               byte_alarm &= ~0x40;
             }
          }else
             cnt_no_Pressure_for_Electrical_Heater = 0;       
      }else 
        cnt_no_Pressure_for_Electrical_Heater = 0; 
      
      sData.Events[byte_event] = byte_alarm;
       
      sData.AccI2C[iAcc].measure1 = (word)(((word)buff_Data[3] << 8) | buff_Data[2]);   // misura: T_Air
      sData.AccI2C[iAcc].measure2 = (word)(((word)buff_Data[5] << 8) | buff_Data[4]);   // misura: Twater
      sData.AccI2C[iAcc].measure3 = (word)(((word)buff_Data[7] << 8) | buff_Data[6]);   // misura: Pressure
      sData.AccI2C[iAcc].measure4 = (word)(((word)buff_Data[9] << 8) | buff_Data[8]);   // misura: Analog Input

      // byte info:  6=(SONDA_EXHAUST-5.0 °C), 5=SONDA_EXHAUST, 4=SONDA_SUPPLY, 3=SONDA_RETURN, 2=SONDA_FRESH, 1=SONDA_ACCESSORIO 
      sData.AccI2C[iAcc].info = buff_Data[11] & 0x0F;
 
// #ifdef VIEW_ACCESSORY //-------------------------------------- x_dbg
  Serial_println1(" ");
  Serial_print1("ADDR_SL: ");
  Serial_println(addrSlave, DEC);
  Serial_print1("iAcc: ");
  Serial_println(iAcc, DEC);
  
  if(status_read_slave == 1) {
        Serial_print1("TA: ");
        Serial_println(sData.AccI2C[iAcc].measure1, DEC);
        Serial_print1("TW: ");
        Serial_println(sData.AccI2C[iAcc].measure2, DEC);
        Serial_print1("PRE: ");
        Serial_println(sData.AccI2C[iAcc].measure3, DEC);
        Serial_print1("A/I: ");
        Serial_println(sData.AccI2C[iAcc].measure4, DEC);
     }    
  else
      Serial_println("Not install!!"); 
//#endif   //-------------------------------------- x_dbg 
 
 
 
      
      if(iAcc == ACC_I2C_PREHEATER) {
          if (sData.AccI2C[iAcc].info == 6)
             sData.AccI2C[iAcc].measure1 = sData.measure_Temp[I_PROBE_EXHAUST] - 50; // -5.0°C per far lavorare meglio il deFrost con il PEHD
          else if (sData.AccI2C[iAcc].info == 5) 
             sData.AccI2C[iAcc].measure1 = sData.measure_Temp[I_PROBE_EXHAUST]; 
          else if (sData.AccI2C[iAcc].info == 4)  
             sData.AccI2C[iAcc].measure1 = sData.measure_Temp[I_PROBE_SUPPLY];   
          else if (sData.AccI2C[iAcc].info == 3) 
             sData.AccI2C[iAcc].measure1 = sData.measure_Temp[I_PROBE_RETURN];             
          else if (sData.AccI2C[iAcc].info == 2)
             sData.AccI2C[iAcc].measure1 = sData.measure_Temp[I_PROBE_FRESH];
       }  
      // size: 2= 0023, 5= 0053, 22= 0223, 
      //       NULL= Nessuno controllo sul size   
      //       3   = Nessuno controllo sul size, perchè sono usciti i primi PEHD con valore 3 :-((. 
      if ((buff_Data[10] != 0xFF) && (buff_Data[10] != 3)) {     
         sData.AccI2C[iAcc].data = buff_Data[10];  
        
         if(sData.AccI2C[iAcc].data != sData.size_unit) 
            InsertAlarm(ALM_EHD_SIZE + (iAcc * 8));
         else
            RemoveAlarm(ALM_EHD_SIZE + (iAcc * 8)); 
      }else     
         RemoveAlarm(ALM_EHD_SIZE + (iAcc * 8)); 
              
   }  
  
   return risp;
}  
--------  FINE */


//---------------------------------------------------------------------------------------------  
//     Dati per la gestione accessorio
//
// Address |Read/Write| N.Byte |     Descrizione
// --------+----------+--------+---------------------------------------------------------------
//   000         W         1      <comand>   ..., bit[1]=(1= DISCONNECT), bit[0]=(1=Switch_ON /0=Switch_OFF)     
//   001         R         1      <sts>      Stato accessorio:  bit[0]: 1=Collegato
//                                                              bit[1]: 1=OPEARATIVO,  
//                                                              bit[2]: 1=ACCESO/0=SPENTO, 
//                                                              bit[3]: 1=FUNC_HEATER(winter)/0=FUNC_COOLER(summer) 
//                                                              bit[4]: 1=DEFROST ON
//   002         R         1      <Allarmi>   ...,bit[1]= Link_KO, bit[0]=Allarme Compressore                                                                    
//   003         R         2      <misura_1>  ** non usata **
//   005         R         2      <misura_2>  ** non usata **
//   007        R/W        1      <info>      'W' = FUNZ_RISCALDO, 'S' = FUNZ_RAFFREDDO
//   008        R/W        1      <data>      valore regolatore 0-10V ,  100 = 10.0V  0 = 0.0V        
//
//---------------------------------------------------------------------------------------------              
Byte read_Access_Dxd(byte addrSlave, byte iAcc, byte byte_event, byte bit_event, byte msk_clear_event, byte msk_event_poll) 
{
   byte risp = 0, byte_alarm = 0;
   byte buff_Data[10];
   byte status_read_slave = 0;

      
   // leggiamo: sts, alarms, Measures 1 & 2, info 
   status_read_slave = ReadSlave(addrSlave, 0x01, &buff_Data[1], 8); // il byte 0 è dedicato ai comandi
  
   if(status_read_slave == 1) 
   {
      risp = 1; 
      sData.AccI2C[iAcc].cnt_persist_link_ko = 0;
      sData.AccI2C[iAcc].cnt_link_ok++; 
      // Status: ...., bit[3]: 1=WORK_HEATER(winter)/0=WORK_COOLER(summer), bit[2]:Stato Acc. 1=ON/0=OFF, bit[1]:1=OPEARATIVO, bit[0]:1=Collegato   
      sData.AccI2C[iAcc].sts = (buff_Data[1] | STS_ACC_CONNECT);
      
      if(buff_Data[1] & STS_DEFROST_ON)
        sData.RegDXD.defrost_on = 1;
      else 
        sData.RegDXD.defrost_on = 0;  
    
      if(sData.AccI2C[iAcc].sts  & STS_ACC_OPERATIVE) 
         TAG_DigitAccessoryOperating(iAcc);                 // segnaliamo che è connesso e operativo     
      
      byte_alarm  = 0;
      byte_alarm  = ((buff_Data[2] << bit_event) & msk_event_poll);
      sData.Events[byte_event] &= msk_clear_event;
      sData.Events[byte_event] |= byte_alarm;
      
      sData.AccI2C[iAcc].info  = buff_Data[7];
      sData.AccI2C[iAcc].data  = buff_Data[8];
      
//---------------------------------------x_dbg
/*
      Serial_println1(" ");
      Serial_println1("--- Read DXD ---");
      Serial_print1("Alarm: ");
      Serial_println(sData.Events[byte_event], HEX);
      Serial_print1("Status: ");
      Serial_println(sData.AccI2C[iAcc].sts, HEX);
      Serial_print1("Func: ");
      Serial_println(sData.AccI2C[iAcc].info, BYTE);
      Serial_print1("Val 0-10V: ");
      Serial_println(sData.AccI2C[iAcc].data, DEC);
      Serial_println1("----------------");
 */     
//---------------------------------------x_dbg
      
      if(sData.RegDXD.val_0_10V != sData.AccI2C[iAcc].data) 
      {
          if(sData.RegDXD.countdown_aligns_val++ >= 2) {
            sData.RegDXD.countdown_aligns_val = 0;
            sData.RegDXD.val_0_10V = sData.AccI2C[iAcc].data;
//---------------------------------------x_dbg            
//            Serial_println1(" * * Alignes value * * ");    
//---------------------------------------x_dbg            
          }  
        
      }else  
          sData.RegDXD.countdown_aligns_val = 0;

    }
    
    return risp;
}


//--------------------------------------------------------------------------------------------
// Invia al DXD il tipo di funzione che deve svolgere il compressore:
//  'W' = winter (Riscaldamento)
//  'S' = summer (Raffredamento)
// 
// Address |Read/Write| N.Byte |     Descrizione
// --------+----------+--------+---------------------------------------------------------------
//   ...
//   007        R/W        1      <info>      'W' = FUNZ_RISCALDO, 'S' = FUNZ_RAFFREDDO
//   ...
//
//  Return: = 0 -> link KO
//          = 1 -> Scrittura riuscita
//--------------------------------------------------------------------------------------------
byte write_func_compressor_Dxd(void) 
{
   byte risp = 0;
   
   if((sData.RegDXD.func_compressor != 'W') && (sData.RegDXD.func_compressor != 'S'))
      return 0; 

//---------------------------------------x_dbg
//   Serial_println1(" ");
//   Serial_println1("** WR func.Cmp **");
//---------------------------------------x_dbg
   
   if(WriteSlave(ADDR_DXD, 0x07, sData.RegDXD.func_compressor) == 1) {
      risp = 1; // tutto ok
      sData.AccI2C[ACC_I2C_DXD].info = sData.RegDXD.func_compressor;
      sData.RegDXD.processor_I2C &= ~CHANGE_FUNC_COMPRESS;
      sData.RegDXD.cnt_link_err_write = 0;
   }else {
      // dopo 3 tentativi di scrittura, cancella la pendenza.
      if(++sData.RegDXD.cnt_link_err_write > 3) {
        sData.RegDXD.processor_I2C &= ~CHANGE_FUNC_COMPRESS;
        sData.RegDXD.cnt_link_err_write = 0;
      }        
   }  
     
   return risp;  
}   


//--------------------------------------------------------------------------------------------
// Invia al DXD il valore di spinta del compressore:
// Nel momento in cui il valore è 0.0V spengo il compressore    
// 
// Address |Read/Write| N.Byte |     Descrizione
// --------+----------+--------+---------------------------------------------------------------
//   ...
//   008        R/W        1      <data>  valore regolatore 0-10V ,  100 = 10.0V  0 = 0.0V         
//   ...
//
//  Return: = 0 -> link KO
//          = 1 -> Scrittura riuscita
//--------------------------------------------------------------------------------------------
byte write_regulate_Dxd(void) 
{
   byte risp = 0;
   
//---------------------------------------x_dbg
//   Serial_println1(" ");
//   Serial_println1("** WR Regulate Cmp **");
//---------------------------------------x_dbg
 
   if(WriteSlave(ADDR_DXD, 0x08, sData.RegDXD.val_0_10V) == 1) {
     risp = 1; // tutto ok
     sData.AccI2C[ACC_I2C_DXD].data = sData.RegDXD.val_0_10V;
     
     // diamo il comando anche di accensione oltre che il livello 0-10V
     if(DigitAccessoryOn(ACC_I2C_DXD) == 0) 
       TAG_DigitAccessoryOn(ACC_I2C_DXD);        
     
     sData.RegDXD.processor_I2C &= ~CHANGE_VAL_0_10V;
     sData.RegDXD.cnt_link_err_write = 0;
                
   }else {
      if(++sData.RegDXD.cnt_link_err_write > 3) {
        sData.RegDXD.processor_I2C &= ~CHANGE_VAL_0_10V;
        sData.RegDXD.cnt_link_err_write = 0;
      }  
   }  
     
   return risp;  
}   

int Dsc_Data_Size = 0;
byte Write_Buffer[770];
byte Write_To_Dsc = 0;

byte read_DSC(byte addrSlave, byte iAcc, byte byte_event, byte bit_event, byte msk_clear_event)
{
  byte risp = 0, alarm=0;
  byte buff_Data[12];
  int statusBus;
  byte Byte1;
  byte Byte2;
  byte *Pointer1 = NULL;
  int Index;
  
    statusBus = ReadSlave(addrSlave, 0x01, &buff_Data[0], 1);
    
    if(statusBus == 1) 
    {
        risp = 1;
        sData.AccI2C[iAcc].cnt_persist_link_ko = 0;
        sData.AccI2C[iAcc].cnt_link_ok++;               
        TAG_DigitAccessoryOperating(iAcc);                 // we point out that it is connected and operational     
        sData.AccI2C[iAcc].sts &= 0xF3; 
        sData.AccI2C[iAcc].sts |= (buff_Data[0] & 0x04);   // Let's add the status of: b [2] 1 = Power_ON / 0 = Power_SAVE
    }
    else{
      Dsc_Data_Size = 0;
      return risp = 0;
    }
  
  Byte1 = sData.measure_CO2_max >> 8;
  Byte2 = sData.measure_CO2_max & 0xFF;
  
  statusBus = WriteSlave(addrSlave, 0, Byte1);
  if(statusBus != 1)
   {
     Dsc_Data_Size = 0;
     Write_To_Dsc = 0;
     return risp = 0;
   }
  else
     risp = 1;
  
  statusBus = WriteSlave(addrSlave, 1, Byte2);
  if(statusBus != 1)
   {
     Dsc_Data_Size = 0;
     Write_To_Dsc = 0;
     return risp = 0;
   }
  else
     risp = 1;
     
  Pointer1 = (byte *)&sClockWeek;
  for ( Index = 0; Index < sizeof (S_CLOCK_WEEK); Index++ )
       {
         statusBus = WriteSlave(addrSlave, Index, *Pointer1);
         Pointer1++;
         if(statusBus != 1)
           {
             Dsc_Data_Size = 0;
             Write_To_Dsc = 0;
             return risp = 0;
           }
         else
           risp = 1;      
       } 
   
  return risp;
}

//-------------------------------------- x_dbg
//int debug_rx_ok = 0;
//-------------------------------------- x_dbg

//---------------------------------------------------------------------------------------------
//                ----  Read Slave:  P1CO2, P2CO2, P1RH, P2RH, P1VOC, AWP  ----  
//
//   Address |Data[#]|       Description
// ----------+-------+-------------------------------------------------------------------
//    0x01      [0]   Byte Status (..., b[2] 1=Power_ON /0=Power_SAVE, ...., ....)
//    0x02      [1]   Byte Alarms ( .., b[2] Vin_Low, b[1] Sens.Temp_KO, b[0] Sens. RH/CO2/VOC_KO)
//    0x03      [2]   byte Low Measure Sens. RH/CO2/VOC 
//    0x04      [3]   byte HI Measure Sens.  RH/CO2/VOC
//    0x05      [4]   byte Low Measure Sens. Temp (value x10)
//    0x06      [5]   byte Low Measure Sens. Temp (value x10)
//    0x07      [6]   byte info (probe: 0x01= Duct, 0x00= Ambient)
//
//    -------- Alarms --------
//   [03]: 7.- - - -, 6.S2_CO2LinkEr,5.S2_CO2_KO, 4.- - - -, 3.S1_CO2LinkEr, 2.S1_CO2_KO , 1.- - - -, 0.SInt_CO2_KO
//   [04]: 7.- - - -, 6.S2_RHLinkEr, 5.S2_RH_KO , 4.- - - -, 3.S1_RHLinkEr , 2.S1_RH_KO  , 1.- - - -, 0.SInt_RH_KO
//   [05]: 7.- - - -, 6.- - - - -  , 5.- - - - -, 4.- - - -, 3.S1_VOCLink  , 2.S1_VOC_KO , 1.- - - -, 0.SInt_VOC_KO                                         
//   [06]: 7.DPP_LinkE, 6.DPP_KO   , 5.- - - - -, 4.PCAF_LinkEr, 3.PCAF_KO , 2.- - - - - , 1.PCAP_LinkE, 0.PCAP_KO
//   [07]: 7.AWP_LinkE, 6.AWP_KO   , 5.- - - - -, 4.- - - - -  , 3.- - - - , 2.- - - - - , 1.PIR_LinkEr, 0.PIR_KO 
//---------------------------------------------------------------------------------------------
Byte read_Sensor_Air(byte addrSlave, byte iAcc, byte byte_event, byte bit_event, byte msk_clear_event)
{
  byte risp = 0, alarm=0;
  byte buff_Data[12];
  int statusBus;
   
  statusBus = ReadSlave(addrSlave, 0x01, &buff_Data[0], 11); // we put 15 bytes as a standard instead of 11
  
  if(statusBus == 1) 
  {
      risp = 1; 
      sData.AccI2C[iAcc].cnt_persist_link_ko = 0;
      sData.AccI2C[iAcc].cnt_link_ok++;               
      TAG_DigitAccessoryOperating(iAcc);                 // segnaliamo che è connesso e operativo     
      sData.AccI2C[iAcc].sts &= 0xF3; 
      sData.AccI2C[iAcc].sts |= (buff_Data[0] & 0x04);   // Aggiungiamo lo stato di : b[2] 1=Power_ON /0=Power_SAVE
            
      
      sData.AccI2C[iAcc].measure1 = (word)(((word)buff_Data[3] << 8) | buff_Data[2]);   // misura: Sens Co2, RH, Voc
      sData.AccI2C[iAcc].measure2 = (word)(((word)buff_Data[5] << 8) | buff_Data[4]);   // misura: Temperature
	  sData.AccI2C[iAcc].measure3 = (word)(((word)buff_Data[7] << 8) | buff_Data[6]);   // misura: RAW GAS
	  sData.AccI2C[iAcc].measure4 = (word)(((word)buff_Data[9] << 8) | buff_Data[8]);   // misura: RAW GAS
      sData.AccI2C[iAcc].data     = buff_Data[10];      // info (probe: 0x01= Duct, 0x00= Ambient)
      
      if (iAcc == ACC_I2C_CO2_1)
	  {
		  if ( CO2_1_Average_Counter >= 1800000 ) // strore Co2 value every 30 min
			  {
				CO2_1_Average_Counter = 0;
				if ( Index_CO2_1 < 48 ) // we have not yet reached 24 hours
				{
					CO2_1_array[Index_CO2_1] =  sData.AccI2C[iAcc].measure1; // store CO2 value
					Index_CO2_1++;
				}
				else // we have reached 24 hours
				{
					// Found CO2 MIN and MAX value in the last 24 hours
					byte i;
					short max, min;
					
					Index_CO2_1 = 0; // restart 24hour index to 0
					max = CO2_1_array[0];
					min = CO2_1_array[0];
					for (i = 1; i < 48; i++)
					{
						// Update mx if the current element is greater
						if (CO2_1_array[i] > max)
						{
							max = CO2_1_array[i];
						}

						// Update mn if the current element is smaller
						if (CO2_1_array[i] < min)
						{
							min = CO2_1_array[i];
						}
					}
					if ( (max - min) <= 500 )
						Clean_Event_Flag1 = true;
					else
						Clean_Event_Flag1 = false;		
				}
			  }
			  
		  CO2_1_Raw_Gas_Value = ( sData.AccI2C[iAcc].measure3 & 0xFFFF) | (((long)sData.AccI2C[iAcc].measure4 << 16) & 0xFFFF0000);
		  
		  /*if ( CO2_1_old_Value == 0 )
		  {
			  CO2_1_counter = 0;
			  CO2_1_old_Value = sData.AccI2C[iAcc].measure1;
		  }
		  else
		  {
			  if ( CO2_1_old_Value != sData.AccI2C[iAcc].measure1 )
				{
					CO2_1_counter = 0;
					CO2_1_old_Value = sData.AccI2C[iAcc].measure1;
				}
			else
				{
					if ( CO2_1_counter >= 86400000 )
					{
						CO2_1_counter = 0;
						CO2_1_old_Value = 0;
						// Restart Sens05 board.
						sData.AccI2C[iAcc].comand = (WRITE_CMD | RESTART_SLAVE);
						WriteSlave(addrSlave, 0x00, sData.AccI2C[iAcc].comand);
					}
				}	
		  }*/
	  }
	  
	  if (iAcc == ACC_I2C_CO2_2)
	  {
		  if ( CO2_2_Average_Counter >= 1800000 ) // strore Co2 value every 30 min
		  {
			  CO2_2_Average_Counter = 0;
			  if ( Index_CO2_2 < 48 ) // we have not yet reached 24 hours
			  {
				  CO2_2_array[Index_CO2_2] =  sData.AccI2C[iAcc].measure1; // store CO2 value
				  Index_CO2_2++;
			  }
			  else // we have reached 24 hours
			  {
				  // Found CO2 MIN and MAX value in the last 24 hours
				  byte i1;
				  short max1, min1;
				  
				  Index_CO2_2 = 0; // restart 24hour index to 0
				  max1 = CO2_2_array[0];
				  min1 = CO2_2_array[0];
				  for (i1 = 1; i1 < 48; i1++)
				  {
					  // Update mx if the current element is greater
					  if (CO2_2_array[i1] > max1)
					  {
						  max1 = CO2_2_array[i1];
					  }

					  // Update mn if the current element is smaller
					  if (CO2_2_array[i1] < min1)
					  {
						  min1 = CO2_2_array[i1];
					  }
				  }
				  if ( (max1 - min1) <= 500 )
					Clean_Event_Flag2 = true;
				  else
					Clean_Event_Flag2 = false;
			  }
		  }
		  
		  CO2_2_Raw_Gas_Value = ( sData.AccI2C[iAcc].measure3 & 0xFFFF) | (((long)sData.AccI2C[iAcc].measure4 << 16) & 0xFFFF0000);
		  
		  /*if ( CO2_2_old_Value == 0 )
		  {
			  CO2_2_counter = 0;
			  CO2_2_old_Value = sData.AccI2C[iAcc].measure1;
		  }
		  else
		  {
			  if ( CO2_2_old_Value != sData.AccI2C[iAcc].measure1 )
			  {
				  CO2_2_counter = 0;
				  CO2_2_old_Value = sData.AccI2C[iAcc].measure1;
			  }
			  else
			  {
				  if ( CO2_2_counter >= 86400000 )
				  {
					  CO2_2_counter = 0;
					  CO2_2_old_Value = 0;
					  // Restart Sens05 board.
					  sData.AccI2C[iAcc].comand = (WRITE_CMD | RESTART_SLAVE);
					  WriteSlave(addrSlave, 0x00, sData.AccI2C[iAcc].comand);
				  }
			  }
		  }*/
	  }
	  
      if(iAcc == ACC_I2C_AWP) {
         // il sensore AWP è un sensore RH, che però viene utilizzato per la lettura della temp. 
         // Byte Alarms dall'AWP:  .., b[2] Vin_Low,< b[1] Sens.Temp_KO >, b[0] Sens. RH/CO2/VOC_KO
         // dverso L'unità BYTE : 7.AWP_LinkEr, 6.AWP_KO,  ...
         alarm = ((buff_Data[1] >> 1) & 0x01);
         if(!alarm) 
           sData.measure_Temp[I_PROBE_RETURN] = (int)sData.AccI2C[iAcc].measure2;    // Loop Chiuso: misura della temperatura del Sensore AWP  
//---------------------------------       
/*
         Serial_println1(" ");
         Serial_print1("Tamb:");
         Serial_print(sData.AccI2C[iAcc].measure2, DEC);
         Serial_print1("  TR:");
         Serial_print(sData.measure_Temp[I_PROBE_RETURN], DEC);
         Serial_print1("  Alm:");
         Serial_println(alarm, DEC);
*/         
//-----------------------------------         
 
      }else {
         alarm =  buff_Data[1] & 0x01;  // gestiamo dal sensore solo l'allarme di Sensore fuori campo. 
      }

      
      sData.Events[byte_event]  &= msk_clear_event;  
      sData.Events[byte_event]  |= (alarm << bit_event); 
	  
	  sData.AccI2C[iAcc].measure3 = ( sData.AccI2C[iAcc].data >> 1 ) & 0x7F;
	  
       // 7.P2CO2, 6.P1CO2, 5.PICO2, 4.P2RH, 3.P1RH, 2.PIRH, 1.P1VOC, 0.PIVOC   (bit: 0 = Ambiente, 1 = Duct)
      if(sData.AccI2C[iAcc].data & 0x01) {         
         if (iAcc == ACC_I2C_CO2_2) 
            sData.info_ProbesAD |= 0x80;
         else if (iAcc == ACC_I2C_CO2_1) 
            sData.info_ProbesAD |= 0x40;    
         else if (iAcc == ACC_I2C_RH_2) 
            sData.info_ProbesAD |= 0x20;   
         else if (iAcc == ACC_I2C_RH_1) 
            sData.info_ProbesAD |= 0x10;        
         else if (iAcc == ACC_I2C_VOC) 
            sData.info_ProbesAD |= 0x08;  
      }else {   
         if (iAcc == ACC_I2C_CO2_2) 
            sData.info_ProbesAD &= ~0x80;
         else if (iAcc == ACC_I2C_CO2_1) 
            sData.info_ProbesAD &= ~0x40;    
         else if (iAcc == ACC_I2C_RH_2) 
            sData.info_ProbesAD &= ~0x20;   
         else if (iAcc == ACC_I2C_RH_1) 
            sData.info_ProbesAD &= ~0x10;        
         else if (iAcc == ACC_I2C_VOC) 
            sData.info_ProbesAD &= ~0x08;  
      }       
   }
   
 
#ifdef VIEW_ACCESSORY //-------------------------------------- x_dbg
  Serial_println1(" ");
  Serial_print1("ADDR_SL: ");
  Serial_println(addrSlave, DEC);
  
  if(statusBus == 1) {
     if ((iAcc == ACC_I2C_CO2_1) || (iAcc == ACC_I2C_CO2_2)){
        Serial_print1("CO2: ");
        Serial_println(sData.AccI2C[iAcc].measure1, DEC);
     }else if ((iAcc == ACC_I2C_RH_1) || (iAcc == ACC_I2C_RH_2)) {
        Serial_print1("RH: ");
        Serial_println(sData.AccI2C[iAcc].measure1, DEC);
     }else if (iAcc == ACC_I2C_VOC) {
        Serial_print1("VOCS: ");
        Serial_println(sData.AccI2C[iAcc].measure1, DEC);
     }else { 
         Serial_print(iAcc, DEC);
         Serial_println1(" : ?? ");
     }    
  }else
      Serial_println1(" Not install!!"); 
#endif   //-------------------------------------- x_dbg    
 
  
   return risp;  
  
}  
//---------------------------------------------------------------------------------------------
//                ----  Read Slave:  EXT1, EXT2, EXT3, EXT4  ----  
//
//   Address |Data[#]|       Description
// ----------+-------+-------------------------------------------------------------------
//    0x01      [0]   Byte Status (..., b[2] 1=Power_ON /0=Power_SAVE, ...., ....)
//    0x02      [1]   Byte Alarms ( .., b[2] Vin_Low, b[1] Sens.Temp_KO, b[0] Sens. RH/CO2/VOC_KO)
//    0x03      [2]   byte Low Measure Sens. RH
//    0x04      [3]   byte HI Measure Sens.  RH
//    0x03      [4]   byte Low Measure Sens. CO2
//    0x04      [5]   byte HI Measure Sens.  CO2
//    0x03      [6]   byte Low Measure Sens. VOC
//    0x04      [7]   byte HI Measure Sens.  VOC
//    0x05      [8]   byte Low Measure Sens. Temp (value x10)
//    0x06      [9]   byte Low Measure Sens. Temp (value x10)
//    0x07      [10]   byte info (probe: 0x01= Duct, 0x00= Ambient)
//
//    -------- Alarms --------
//   byte[11]: 7.EXT2_KO   , 6.EXT2_Link   , 5.EXT1_KO  , 4.EXT1_Link  , 3.TExhaus_KO  , 2.TSupply_KO, 1.TReturn_KO, 0.TFresh_KO  
//   byte[12]: 7.- - - - - , 6.- - - - -  , 5.- - - - - , 4.- - - - -  , 3.EXT4_KO     , 2.EXT4_Link , 1.EXT3_KO   , 0.EXT3_Link
//---------------------------------------------------------------------------------------------
Byte read_ext_Sensor(byte addrSlave, byte iAcc, byte byte_event, byte bit_event, byte msk_clear_event)
{
  byte risp = 0, alarm=0, info=0;
  byte buff_Data[20];
  int  statusBus;
  uint16_t CAPS_CAPR_FIRM_VER;
   
  statusBus = ReadSlave(addrSlave, 0x01, &buff_Data[0], 15);  // we put 15 bytes as a standard instead of 11
  
  if(statusBus == 1) 
  {
	  CAPS_CAPR_FIRM_VER = 0;
	  CAPS_CAPR_FIRM_VER = ( buff_Data[6] & 0xF0 ) >> 4;
	  CAPS_CAPR_FIRM_VER = CAPS_CAPR_FIRM_VER | ( buff_Data[7] & 0xF0 );
	  CAPS_CAPR_FIRM_VER = CAPS_CAPR_FIRM_VER | (((short)buff_Data[0] & 0xE0 ) << 3);
	  
      risp = 1; 
      sData.AccI2C[iAcc].cnt_persist_link_ko = 0;
      sData.AccI2C[iAcc].cnt_link_ok++;               
      TAG_DigitAccessoryOperating(iAcc);                 // segnaliamo che è connesso e operativo     
      sData.AccI2C[iAcc].sts &= 0xF3; 
      sData.AccI2C[iAcc].sts |= (buff_Data[0] & 0x04);   // Aggiungiamo lo stato di : b[2] 1=Power_ON /0=Power_SAVE
            
                                                                                        //      EXT_1/2/3/4     //       EBPD           //        SSR           //    FLW 1/2       //  
      sData.AccI2C[iAcc].measure1 = (word)(((word)buff_Data[3] << 8) | buff_Data[2]);   // misura: Temperature  // misura: Tempo CW     // misura: T1           // misura: T1       //
      sData.AccI2C[iAcc].measure2 = (word)(((word)buff_Data[5] << 8) | buff_Data[4]);   // misura: RH           // misura: Tempo CCW    // misura: T2           // misura: T1       //
      sData.AccI2C[iAcc].measure3 = (word)(((word)buff_Data[7] << 8) | buff_Data[6]);   // misura: CO2          // misura: Corrente CW  // misura: T3           // misura: T1       //
      sData.AccI2C[iAcc].measure4 = (word)(((word)buff_Data[9] << 8) | buff_Data[8]);   // misura: VOC          // misura: Corrente CCW // misura: Pressure     // misura: Pressure //
      sData.AccI2C[iAcc].data     = buff_Data[10];                                      // info  : TBD          // info:  0-100 comando // info:  0-100 comando // info:  TBD       //
        
      sData.Events[byte_event]  &= msk_clear_event;  
      sData.Events[byte_event]  |= (alarm << bit_event);    
     
   if ( addrSlave == ADDR_SSR)
   {
       flow_motor_1 = calcola_portata(sData.AccI2C[ACC_I2C_SSR].measure4, K_FLOW1);
       sData.AccI2C[ACC_I2C_SSR].measure1=sData.AccI2C[ACC_I2C_SSR].measure1+100;
       sData.AccI2C[ACC_I2C_SSR].measure2=sData.AccI2C[ACC_I2C_SSR].measure2+100;
       sData.AccI2C[ACC_I2C_SSR].measure3=sData.AccI2C[ACC_I2C_SSR].measure3+100;
       
   }
   
   if ( addrSlave == ADDR_FLW1)
   {
	  Pressure_CAPS =  sData.AccI2C[iAcc].measure4;
	  sData.AccI2C[iAcc].measure3 = CAPS_CAPR_FIRM_VER;
	  buff_Data[6] =  buff_Data[6] & 0x0F;
	  buff_Data[7] =  buff_Data[7] & 0x0F;
	  buff_Data[0] =  buff_Data[0] & 0x1F;
      //flow_motor_1 = calcola_portata(sData.AccI2C[ACC_I2C_FLW1].measure4, K_FLOW1);
   }
   
   if ( addrSlave == ADDR_FLW2)
   {
	  Pressure_CAPR =  sData.AccI2C[iAcc].measure4;
	  sData.AccI2C[iAcc].measure3 = CAPS_CAPR_FIRM_VER;
	  buff_Data[6] =  buff_Data[6] & 0x0F;
	  buff_Data[7] =  buff_Data[7] & 0x0F;
	  buff_Data[0] =  buff_Data[0] & 0x1F;
      //flow_motor_2 = calcola_portata(sData.AccI2C[ACC_I2C_FLW2].measure4, K_FLOW2);
   }
   
   if (( addrSlave == ADDR_EBPD ) || ( addrSlave == ADDR_EBP2 ))
   {
		BPD_FW = 0;
		BPD_FW = BPD_FW | ( buff_Data[7] & 0xF0);
		buff_Data[7] = buff_Data[7] & 0xF;
		BPD_FW = BPD_FW | (( buff_Data[9] & 0xF0) >> 4);
		buff_Data[9] = buff_Data[9] & 0xF;
   }
	
   }

 
#ifdef VIEW_ACCESSORY //-------------------------------------- x_dbg
if(addrSlave==63)
{
  Serial_println1(" ");
  Serial_print1("ADDR_SL: ");
  Serial_println(addrSlave, DEC);
  Serial_print1("iAcc: ");
  Serial_println(iAcc, DEC);
  
  if(statusBus == 1) {
        Serial_print1("M1: ");
        Serial_println(sData.AccI2C[iAcc].measure1, DEC);
        Serial_print1("M2: ");
        Serial_println(sData.AccI2C[iAcc].measure2, DEC);
        Serial_print1("M3: ");
        Serial_println(sData.AccI2C[iAcc].measure3, DEC);
        Serial_print1("M4: ");
        Serial_println(sData.AccI2C[iAcc].measure4, DEC);
        Serial_print1("info: ");
        Serial_println(sData.AccI2C[iAcc].data, DEC);
     }    
  else
      Serial_println1("Not install!!"); 
}
#endif   //-------------------------------------- x_dbg    
 
  
   return risp;  
  
}  

//---------------------------------------------------------------------------------------------
//                ----  Read Slave:  CAP ----  
//
//   Address | R/W |       Description
// ----------+-----+-------------------------------------------------------------------
//    0x00      W    Cmd_by_Unit;  ..., bit[1]:1= DISCONNECT, ...
//    0x01      R    byte_Status;  ...,     bit[0]:1=Collegato
//    0x02      R    byte Alarms;  ...,     bit[0]:1=Sensore Guasto  
//    0x03      R    Measure1;     (05:Lo + 06:Hi)  pressure (Pa)  
//    0x05      R    Measure2;     non usato
//    0x07      R    size_Acc;     non usato
//    0x08      R    info;         non utilizzato 
//
//    -------- Alarms --------                                 
//   [06]: 7.DPP_LinkE, 6.DPP_KO   , 5.- - - - -, 4.PCAF_LinkEr, 3.PCAF_KO , 2.- - - - - , 1.PCAP_LinkE, 0.PCAP_KO
//---------------------------------------------------------------------------------------------
byte read_Sensor_Pressure(byte addrSlave, byte iAcc, byte byte_event, byte bit_event, byte msk_clear_event)
{
  byte risp = 0, alarm;
  byte buff_Data[20];
  int statusBus;
   
  // leggiamo solo la misura e il byte allarmi. 
  statusBus = ReadSlave(addrSlave, 0x02, &buff_Data[0], 12); // we put 12 bytes as a standard instead of 5
  
  
  if(statusBus == 1) 
  {
      risp = 1; 
      sData.AccI2C[iAcc].cnt_persist_link_ko = 0;
      sData.AccI2C[iAcc].cnt_link_ok++;               
      TAG_DigitAccessoryOperating(iAcc);                 // segnaliamo che è connesso e operativo   
      
      alarm =  buff_Data[0]; 
      sData.AccI2C[iAcc].measure1 = (word)(((word)buff_Data[2] << 8) | buff_Data[1]);   // misura: pressione
	  sData.AccI2C[iAcc].measure3 = (word)(((word)buff_Data[4] << 8) | buff_Data[3]);   // Firmware serial
	  
      sData.AccI2C[iAcc].measure2 = 1; // la seconda misura la uso per segnalare una nuova lettura del sensore.  
      
      alarm &= 0x00; // gestiamo dal sensore solo l'allarme di Sensore fuori campo o guasto. ---> annullo tutto nicola 27/04/2016
      
      sData.Events[byte_event]  &= msk_clear_event;  
      sData.Events[byte_event]  |= (alarm << bit_event); 
	  
	  // leggiamo le funzioni abilitate
	  unsigned short enab_func = read_word_eeprom(ADDR_EEP(Enab_Fuction)); 
	  
	  //if(enab_func & (1 << ENAB_WINTER))
		sData.measure_pressure = sData.AccI2C[iAcc].measure1;
      
//------------------------- x_dbg
/*
      Serial_println1(" ");
      Serial_print1("ADDR_SL: ");
      Serial_println(addrSlave, DEC);
      Serial_print1("Alarm: ");
      Serial_print(alarm, HEX);
      Serial_print1("  Pressure: ");
      Serial_println(sData.AccI2C[iAcc].measure1, DEC);
*/      
//-------------------------
  }
  
   
  sData.AccI2C[iAcc].data  = 0;
  
  return risp;
}  



//----------------------------------------------------------------
// Ritorna l'OR dello stato del tipo di sonda richiesto, e Calcola
// il valore + alto per tipo di sonda. 
//
// Input: ProbeType = PROBES_RH / PROBES_CO2 / PROBES_VOC
//          
//----------------------------------------------------------------
/*void probeQAirActive()
{

    int temp_measure_CO2_max = 0;                  
    int temp_measure_RH_max  = 0;                   
    int temp_measure_VOC_max = 0;  
    int temp_measure_AWP_max = 0;  

    // Sonde RH digitali
    if(DigitAccessoryOperating(ACC_I2C_RH_1) || DigitAccessoryOperating(ACC_I2C_RH_2))
    {
       if(sData.AccI2C[ACC_I2C_RH_1].measure1 > sData.AccI2C[ACC_I2C_RH_2].measure1)     
         temp_measure_RH_max = sData.AccI2C[ACC_I2C_RH_1].measure1;
       else  
         temp_measure_RH_max = sData.AccI2C[ACC_I2C_RH_2].measure1;
         
    }  
         
    sData.measure_RH_max = temp_measure_RH_max;    

      
    // Sonde CO2 digitali         
    if(DigitAccessoryOperating(ACC_I2C_CO2_1) || DigitAccessoryOperating(ACC_I2C_CO2_2))
    {   
       if(sData.AccI2C[ACC_I2C_CO2_1].measure1 > sData.AccI2C[ACC_I2C_CO2_2].measure1)     
         temp_measure_CO2_max = sData.AccI2C[ACC_I2C_CO2_1].measure1;
       else  
         temp_measure_CO2_max = sData.AccI2C[ACC_I2C_CO2_2].measure1;             
    }  
    
    sData.measure_CO2_max = temp_measure_CO2_max;  
 
    // Sonde VOC digitali
    if(DigitAccessoryOperating(ACC_I2C_VOC))  
      sData.measure_VOC_max = sData.AccI2C[ACC_I2C_VOC].measure1;
        
    if(DigitAccessoryOperating(ACC_I2C_AWP))
      sData.measure_AWP_max = sData.AccI2C[ACC_I2C_AWP].measure2;
}*/

void probeQAirActive()
{

    int temp_measure_CO2_max = 0;                  
    int temp_measure_RH_max  = 0;                   
    int temp_measure_VOC_max = 0;  
    int temp_measure_AWP_max = 0;  

    // Sonde RH digitali
    if(DigitAccessoryOperating(ACC_I2C_RH_1) || DigitAccessoryOperating(ACC_I2C_RH_2))
    {
       if(sData.AccI2C[ACC_I2C_RH_1].measure1 > sData.AccI2C[ACC_I2C_RH_2].measure1)     
         temp_measure_RH_max = sData.AccI2C[ACC_I2C_RH_1].measure1;
       else  
         temp_measure_RH_max = sData.AccI2C[ACC_I2C_RH_2].measure1;
         
    }  
         
    sData.measure_RH_max = temp_measure_RH_max;    

      
    // Sonde CO2 digitali         
    if(DigitAccessoryOperating(ACC_I2C_CO2_1) || DigitAccessoryOperating(ACC_I2C_CO2_2))
    {   
       if(sData.AccI2C[ACC_I2C_CO2_1].measure1 > sData.AccI2C[ACC_I2C_CO2_2].measure1)     
         temp_measure_CO2_max = sData.AccI2C[ACC_I2C_CO2_1].measure1;
       else  
         temp_measure_CO2_max = sData.AccI2C[ACC_I2C_CO2_2].measure1;             
    }  
    
	sData.measure_CO2_max = temp_measure_CO2_max;
	
    /*if (temp_measure_CO2_max< 1000)
        sData.measure_CO2_max = (short)(float)293.88*log(temp_measure_CO2_max)-1367.3;  
    else
        sData.measure_CO2_max = (short)(float)140.65*log(temp_measure_CO2_max)-321.98;*/
 
    // Sonde VOC digitali
    if(DigitAccessoryOperating(ACC_I2C_VOC))  
      sData.measure_VOC_max = sData.AccI2C[ACC_I2C_VOC].measure1;
        
    if(DigitAccessoryOperating(ACC_I2C_AWP))
      sData.measure_AWP_max = sData.AccI2C[ACC_I2C_AWP].measure2;
}

short calcola_portata(float pressione, int flow)

{
  float portata, avg_press;
  
  if (pressione <= 0)
  pressione = 0;
  
if (flow ==K_FLOW1)
{ 
	if (contaflow_1 < MAX_KF_SAMPLE )
	{
		sumpress_1 += pressione;
		contaflow_1++;
		portata = flow_motor_1;
	}
	else
	{
		avg_press = (float) sumpress_1 / MAX_KF_SAMPLE;
        portata = (float)((float)((float)(KF1_1 * avg_press) * avg_press)*avg_press) + (float)((float)(KF1_2 * avg_press) * avg_press)  + (float)(KF1_3 * avg_press) +  KF1_4;
		if (portata <=20)
        portata = 0;
	    else if (portata >= 908)
		portata = 908;	
	    sumpress_1  = 0;
		contaflow_1 = 0;
	}
}		
else if (flow ==K_FLOW2)
{
	if (contaflow_2 < MAX_KF_SAMPLE )
	{
		sumpress_2 += pressione;
		contaflow_2++;
		portata = flow_motor_2;
	}
	else
	{	
		avg_press = (float) sumpress_2 / MAX_KF_SAMPLE;
		portata = (float)((float)((float)(KF1_1 * avg_press) * avg_press)*avg_press) + (float)((float)(KF1_2 * avg_press) * avg_press)  + (float)(KF1_3 * avg_press) +  KF1_4;   
		if (portata <=20)
        portata = 0;
	    else if (portata >= 908)
		portata = 908;
	    sumpress_2  = 0;
		contaflow_2 = 0;
	}
  
}
  
  
  return portata;

}
