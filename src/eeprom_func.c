/*
 * eeprom_func.cpp
 *
 * Author/Date: Maurizio Colmone dic.2013
 *
 * Description: Vi sono tutte le funzioni legate alla gestione dell'eeprom
 */ 


#include "asf.h"

#include "eeprom_map.h"
#include "data.h"
#include "alarm.h"
#include "Protocol_Ser1.h"



// ------------- Funzioni Abilitate in base alla config. ---------------
//  bit[7]:CAF] | bit[6]:CAP  | bit[5]:CSF | bit[4]:ImbalanON | bit[3]:StepLess | bit[2]:WeeklyENAB | bit[1]:BoostON | bit[0]:DeFrostENAB.
#define ENAB_FUNC_EXTRA    0xFF  //      0            0             1              1                  1                   1                   1               1
#define ENAB_FUNC_BASIC    0x29  //      0            0             1              0                  1                   0                   0               1

 //---------------- vars Global ------------------------------------  
 Byte Shadow_eep[256];
 Byte Shadow_eep_preview[256];
 extern Byte   countdown_Update_EepromToShadow;
 
 //-----------------------------------------------------------------
 // Dati di defualt dell'eeprom al primo start-up
 //-----------------------------------------------------------------
 
 const Byte vect_default_eeprom[] = {
    //----------  Information (59 Byte) ------------------
    1,                        //  1 Byte: Address Unit: da 1 a 32, lo 0 è utilizzato per i comandi di broadcast.     
    1,                        //  1 Byte: Type Fuctions:  configurazione funzionalita' Unità:  0= BASIC, 1= PLUS, 2= EXTRA 

    ' ','4','.','0',          //  4 Byte:  Ascii: versione hardware MB     Es: " 4.0"
    ' ',            //  5 Byte:  Ascii: versione software MB     Es: " 2.01" oppure " 2.10"
    '2', '.',
    '7', '0', 
    
#ifdef _EEP_SERIAL_NUMBER
    SN[0],  SN[1],  SN[2],  SN[3],
    SN[4],  SN[5],  SN[6],  SN[7],
    SN[8],  SN[9],  SN[10], SN[11],
    SN[12], SN[13], SN[14], SN[15],
    SN[16], SN[17], 
#else
    /* Serial Number Default */
    '0','0','0','0',          // 18 Byte: SerialNumber in Ascii:  "OrVe SIZE CFG DATA PRG",  es:  "1565 0023 115 1351 002"
    '0','0','0','0',          //          SIZE:  0023, 0043, 0053,... 0423                          
    '0','0','0',              //          CFG:   [8]: '0'= OSC, '1'= SSC, '2'= EOS, '3'= FOS, .. | [9]: '0'='A', '1'='B',  ... | [10]: '1', '2', ... , '9'    
    '0','0','0','0',          //          DATA:  [0,1]: '13' = Anno | [2,3]: '51' = Settimana                
    '0','0','0',              //          PROG:  Progress. Number    
#endif    
    0, 0,                     //  2 Byte: Number Versione FW Modbus  
    '-','-',                  //  2 Byte: Ascii SignTEST           
    0xFF,                     //  1 Byte: 0= esito positivo, != 0 codice di errore durante il test.    
    0x00,0x00,0x00,0x00,      //  4 Byte(L,..,H): hour_runnig, ogni 8 ore memorizza in eeprom l'avanzamento.  
    0x00,0x00,0x00,0x00,      //  4 Byte(L,..,H): hour_maintFilter, memorizza l'ultima pulizia filtri utilizzando il dato hour_runnig.       
    0x40,                     //  5 Byte:  parte 1 list Acc.HW: (MSB)    PIR,   BPD,   AWP,   CWD,   EHD,   HWD,  PHWD,  PEHD  (LSB) 
    0x1C,                     //           parte 2 list Acc.HW: (MSB)    DPS,   CAF,   CAP,   INP,   OUT, DDPV2,   RFM,  MBUS  (LSB) 
    0x00,                     //           parte 3 list Acc.HW: (MSB)  P2CO2, P1CO2,  EBPD,  P2RH,  P1RH,   SSR, P1VOC,  EBP2  (LSB) 
    0x00,                     //           parte 4 list Acc.HW: (MSB)  -----, -----,  FLW2,  FLW1,  EXT4,  EXT3,  EXT2,  EXT1  (LSB)    
   
    0x29,                     //  2 Byte:  (LOW)   bit[7]:CAF  | bit[6]:CAP  | bit[5]:CSF  | bit[4]:ImbalanON | bit[3]:StepLess | bit[2]:WeeklyENAB | bit[1]:BoostON   | bit[0]:DeFrostENAB. 
    0x13,                     //           (HIGH)  bit[15]:--- | bit[14]:MBF | bit[13]:DPP | bit[12]:PREHEATER| bit[11]:EnSummer| bit[10]EnWinter   | bit[9]:CtrlFilter| bit[8]:PASSWORD

   
    ENAB_FUNC_EXTRA,          // se il bit =0, Toglie la possibilit� di abilitatre la funzione. 
    0xFF,

    
    
    0x3,
    0x3,
	0,0,                      //  2 Byte calibra CAP 1               // 6 Byte: liberi per ampliamenti futuri
	30,						  //  1 Byte Clean Event Delay
    0x00,					  //  1 Byte KTS_Watchdog	
	0xDC,0x00,				  //  0xDC, 0x00,               //  2 byte SetPointTemp_Heater, Set-Point Temperatura 1 (Range: 12.0 °C a 35.0 °C),   (default: 22.0)

       
    0x00,                     // 1 Byte:  cntUpdate_info   
    
    //--- Configurazioni HW di fabbrica non modificabili (17 Byte) ---
    2,                        //  1 Byte: numMotors: valori: 2, 4, 6. 
    4,                        //  1 Byte: numPulseMotors, numero di pulse per giro. 1, 2, 3, 4,..., 8                               
    'F',                      //  1 Byte: typeMotors, ('F' = Forward , con le mappature CAP e CAF), 'B' = Backward 
    1,                        //  1 Byte: chWireless,  numero canale wireless da 1 a 16 (era  progMotors = 'S'  mai usato e direi pure inutile)   
    100,                      //  1 Byte: Depotenz dei motori : da 100% a 40%            
     4,                       //  1 Byte: Numero di sonde NTC da 2 a 4.     
    0xE4,                     //  1 Byte: Posiz_NTC => bit 7,6 = Exaust(3), bit 5,4 = Supply(2), bit 3,2 = Return(1), bit 1,0 = Fresh(0)
    0xFF,                     //  1 Byte: RotazioneBypass: 0xFF = bypass chiuso in senso AntiOrario(default), 0= bypass chiuso in senso Orario
     
    //0xFF,0xFF,0xFF,0xFF,      //  9 Byte: liberi per ampliamenti futuri   
    //0xFF,0xFF,0xFF,0xFF,
	0x28, 0x00,               //  8 Byte(LO, HI):  4 Soglie di velocità dei motori in modalità: CAP (Range da 30Pa a 250/350/450Pa), soglia[3] = se è impostato il set in Stepless.
	0x5A, 0x00,
	0x96, 0x00,
	0x28, 0x00,
    0xFF, //  1 Byte: liberi per ampliamenti futuri
    
    //---------  Setting Param.(51 Byte) ---------------   
    1,                        //  1 Byte: Set_Power_ON,  1= UNIT RUN (power_on), 0 = UNIT STANDBY (power_off)    
    BPD_AUTOMATIC,            //  1 Byte: Set_Bypass,   0= Automatic;  1= Ext Ctrl 1;  2= Manual CLOSE;  3= Manual OPEN   4= Automatic + Standby
    INP_DISABLE,              //  2 Byte: Set_Input[2],  0= Disable                
    INP_DISABLE,              //                         1= 12V->Unit RUN , 0V->Unit STOP
                              //                         2= 0V->Unit STOP , 12V->Unit RUN
                              //                         3= 0-10V Air flow regulation 
                              //                         4= 12V->Bypass Open ,  0V->Bypass Closed 
                              //                         5=  0V->Bypass Open , 12V->Bypass Closed		
    0, 0,                     //   0= Disable                   (il rele rimane non alimentato)
                              //   1= Bypass Status Open        (il rele passa in attrazione se: Bypass Open)	
                              //   2= Common Fault Status       (il rele passa in attrazione se: Unit� in allarme)		
                              //   3= Unit is Run (ex: SDD)     (il rele passa in attrazione se: Unita � in funz.)	
                              // 128= Disable                   (il rele rimane in attrazione)
                              // 129= Bypass Status Open        (il rele passa in Diseccitaz. se: Bypass Open)
                              // 130= Common Fault Status       (il rele passa in Diseccitaz. se: Unit� in allarme)
                              // 131= Unit is Run (ex: SDD)     (il rele passa in Diseccitaz. se: Unita � in funz.)      
    3,                        //  1 Byte: Selettore dell'indice degli array Set_StepMotors*[4]: 0, 1, 2, 3=Steepless                                
    0xFA, 0x00,               //  8 Byte(LO, HI):  4 Soglie di velocità dei motori in modalità: CFS/ CAF  (Range: da 25.0%,  a 100.0%), soglia[3] = se è impostato il set in Stepless.
    0x90, 0x01,               //                   se CAF [Range da 20.0%SIZE a 110.0%SIZE (es:SIZE=0043: 80mq/h a 440mq/h)]  (3.6 mq/h = 1 L/s)  
    0xBC, 0x02,
    0xFA, 0x00,                       
    0x28, 0x00,               //  8 Byte(LO, HI):  4 Soglie di velocità dei motori in modalità: CAP (Range da 30Pa a 250/350/450Pa), soglia[3] = se è impostato il set in Stepless.
    0x5A, 0x00,
    0x96, 0x00,
    0x28, 0x00,               
    0,                        //  1 Byte: Set_imbalance_1, values max: +/-70 %
    0,                        //  1 Byte: Set_imbalance_2, values max: +/-70 %  
    60,                       //  1 Byte: Set_TimeBoost, valore in minunti (min:15, max:240) se abilitato va al massimo della velocita'� per il tempo indicato. 
//   LO,  HI    
    0xEE, 0x02,               //  2 Byte(LO, HI): SetPoint_CO2 da 500 a 1500 PPM  (default 750 PPM)
    50,                       //  1 Byte: SetPoint_RH,  da 20% a 99% (default 50%)
    0x14, 0x00,               //  2 Byte(LO, HI): SetPoint_VOC, da 2  a 100 PPM   (default 20PPM)
    0xB4, 0x00,               //  2 Byte: gg funz., trascorso il quale occorre fare manutenzione ai Filter (0xB4 0x00)
    '1','2','3','4','5',      //  5 Byte password service 
    '0','0','0','0','0',      //  5 Byte password user 
    0,0,                      //  2 Byte calibra CAP
    0,                        //  1 Byte Manual Reset impostato a 0 di fabbrica 
    0,                        //  1 Byte: valore di default, DPP non calibrato
    20,                       //  1 Byte: valore di default, MBF fresh 20% 2.0V
    20,                       //  1 Byte: valore di default, MBF fresh 20% 2.0V
    100,                      //  1 Byte: valore di default, CO2 airflow 100%
        
    0x00,                     //  1 Byte: cntUpdate_SettingPar
        
    //---------- Calibrazione Sonde ( 6 Byte) -----------------
    0,0,0,0,0,0,              //  6 Byte: Calibr[6], 4 sonde temp + 2 Qual_Air
              
    //--------- Soglie Temperature (20 Byte)----------------
    0xA0, 0x00,               //  2 Byte(Lo, Hi): Bypass_minTempExt, il valore è moltiplicato x10.(Range: 11.0 �C a 32.0 �C),   (default: 16.0)
    0xDC, 0x00,               //  4 Byte(Lo, Hi): SetPointTemp[0], Set-Point Temperatura 1 (Range: 12.0 °C a 35.0 °C),         (default: 22.0)         
    0xBE, 0x00,               //                  SetPointTemp[1], Set-Point Temperatura 2 (Range: 12.0 °C a 35.0 °C),         (default: 19.0)
    0x00,                     //  1 Byte:  ...., bit[0]: 1=Rif.Temp2, 0=Rif.Temp1 
     15,                      //  2 Byte: [0] isteresi temperature per disattivare HWD/EHD, con AWP presente  (es: 22.0 + 1.5 = 23.5 ACC_OFF)
     (uint8_t)-20,                     //          [1] isteresi temperature per  attivare HWD/EHD,   con AWP presente  (es: 22.0 - 2.0 = 20.0 ACC_ON )
     10,                      //  2 Byte: [0] isteresi temperature per disattivare CWD,      con AWP presente. (es: 22.0 + 1.0 = 23.0 ACC_OFF) 
     40,                      //          [1] isteresi temperature per attivare CWD,        con AWP presente. (es: 22.0 + 4.0 = 26.0 ACC_ON)
     50,                      //  2 Byte: [0] isteresi temperature per disattivare HWD/EHD, loop aperto   (es: 22.0 +5.0 = 27.0 ACC_OFF)
     (uint8_t)-20,                     //          [1] isteresi temperature per  attivare HWD/EHD,   loop aperto   (es: 22.0 -2.0 = 20.0 ACC_ON )
     (uint8_t)-10,                     //  2 Byte: [0] isteresi temperature per disattivare CWD,     loop aperto   (es: 22.0 -1.0 = 21.0 ACC_OFF) 
     40,                      //          [1] isteresi temperature per attivare CWD,        loop aperto   (es: 22.0 +4.0 = 27.0 ACC_ON) 
 // dalla 2.16
    0x0E,                     //  1 Byte: ...bit[6]: 1=START test IPEHD 0=STOP test IPEHD, bit[5,4,3,2]: speed_regulator , bit[1]: 1= TempAmb=TR / TempAmb=TS, bit[0]: 1= summer(Acqua Fredda) / 0 winter(Acqua calda)
    120,                      //  1 Byte: TS_hist_temp_supply => Set-Point_Temp -/- 12.0 (Max -18.0)
    0,					      //  1 Byte:
    0,    					  //  1 Byte:
    
    0x00,                     //  1 Byte: cntUpdate_SetTemp;
        
        
    //--------- weekly program (85 Byte) -------------------
    // LUNEDI
    0,                        //  1 Byte: numbers of ranges (from 0=disable to 4)
    0x00, 0x00, 0x00, 0x00,   //  4 Byte: timeON,  Values from 0 to 48 (step di 30')
    0x00, 0x00, 0x00, 0x00,   //  4 Byte: timeOFF, Values from 0 to 48 (step di 30')
    0x00,                     //  1 Byte: bit[7,6]:Step Speed range 4 | bit[5,4]:Step Speed range 3 | bit[3,2]:Step Speed range 2 | bit[1,0]:Step Speed range 1. 
    0x00,                     //  1 Byte: bit[7,6]:Set Imbal. range 4 | bit[5,4]:Set Imbal. range 3 | bit[3,2]:Set Imbal. range 2 | bit[1,0]:Set Imbal. range 1.
    0x00,                     //  1 Byte: bit[7,6]:Rif. Temp. range 4 | bit[5,4]:Rif. Temp. range 3 | bit[3,2]:Rif. Temp. range 2 | bit[1,0]:SRif. Temp.range 1.
    // MARTEDI
     0,                       //  1 Byte: numbers of ranges (from 0=disable to 4)
    0x00, 0x00, 0x00, 0x00,   //  4 Byte: timeON,  Values from 0 to 48 (step di 30')
    0x00, 0x00, 0x00, 0x00,   //  4 Byte: timeOFF, Values from 0 to 48 (step di 30')
    0x00,                     //  1 Byte: bit[7,6]:Step Speed range 4 | bit[5,4]:Step Speed range 3 | bit[3,2]:Step Speed range 2 | bit[1,0]:Step Speed range 1. 
    0x00,                     //  1 Byte: bit[7,6]:Set Imbal. range 4 | bit[5,4]:Set Imbal. range 3 | bit[3,2]:Set Imbal. range 2 | bit[1,0]:Set Imbal. range 1.
    0x00,                     //  1 Byte: bit[7,6]:Rif. Temp. range 4 | bit[5,4]:Rif. Temp. range 3 | bit[3,2]:Rif. Temp. range 2 | bit[1,0]:SRif. Temp.range 1.
    // MERCOLEDI
    0,                        //  1 Byte: numbers of ranges (from 0=disable to 4)
    0x00, 0x00, 0x00, 0x00,   //  4 Byte: timeON,  Values from 0 to 48 (step di 30')
    0x00, 0x00, 0x00, 0x00,   //  4 Byte: timeOFF, Values from 0 to 48 (step di 30')
    0x00,                     //  1 Byte: bit[7,6]:Step Speed range 4 | bit[5,4]:Step Speed range 3 | bit[3,2]:Step Speed range 2 | bit[1,0]:Step Speed range 1. 
    0x00,                     //  1 Byte: bit[7,6]:Set Imbal. range 4 | bit[5,4]:Set Imbal. range 3 | bit[3,2]:Set Imbal. range 2 | bit[1,0]:Set Imbal. range 1.
    0x00,                     //  1 Byte: bit[7,6]:Rif. Temp. range 4 | bit[5,4]:Rif. Temp. range 3 | bit[3,2]:Rif. Temp. range 2 | bit[1,0]:SRif. Temp.range 1.
    // GIOVEDI
    0,                        //  1 Byte: numbers of ranges (from 0= disable to 4)
    0x00, 0x00, 0x00, 0x00,   //  4 Byte: timeON,  Values from 0 to 48 (step di 30')
    0x00, 0x00, 0x00, 0x00,   //  4 Byte: timeOFF, Values from 0 to 48 (step di 30')
    0x00,                     //  1 Byte: bit[7,6]:Step Speed range 4 | bit[5,4]:Step Speed range 3 | bit[3,2]:Step Speed range 2 | bit[1,0]:Step Speed range 1. 
    0x00,                     //  1 Byte: bit[7,6]:Set Imbal. range 4 | bit[5,4]:Set Imbal. range 3 | bit[3,2]:Set Imbal. range 2 | bit[1,0]:Set Imbal. range 1.
    0x00,                     //  1 Byte: bit[7,6]:Rif. Temp. range 4 | bit[5,4]:Rif. Temp. range 3 | bit[3,2]:Rif. Temp. range 2 | bit[1,0]:SRif. Temp.range 1.
    // VENERDI
    0,                        //  1 Byte: numbers of ranges (from 0=disable to 4)
    0x00, 0x00, 0x00, 0x00,   //  4 Byte: timeON,  Values from 0 to 48 (step di 30')
    0x00, 0x00, 0x00, 0x00,   //  4 Byte: timeOFF, Values from 0 to 48 (step di 30')
    0x00,                     //  1 Byte: bit[7,6]:Step Speed range 4 | bit[5,4]:Step Speed range 3 | bit[3,2]:Step Speed range 2 | bit[1,0]:Step Speed range 1. 
    0x00,                     //  1 Byte: bit[7,6]:Set Imbal. range 4 | bit[5,4]:Set Imbal. range 3 | bit[3,2]:Set Imbal. range 2 | bit[1,0]:Set Imbal. range 1.
    0x00,                     //  1 Byte: bit[7,6]:Rif. Temp. range 4 | bit[5,4]:Rif. Temp. range 3 | bit[3,2]:Rif. Temp. range 2 | bit[1,0]:SRif. Temp.range 1.
    // SABATO
    0,                        //  1 Byte: numbers of ranges (from 0=disable to 4)
    0x00, 0x00, 0x00, 0x00,   //  4 Byte: timeON,  Values from 0 to 48 (step di 30')
    0x00, 0x00, 0x00, 0x00,   //  4 Byte: timeOFF, Values from 0 to 48 (step di 30')
    0x00,                     //  1 Byte: bit[7,6]:Step Speed range 4 | bit[5,4]:Step Speed range 3 | bit[3,2]:Step Speed range 2 | bit[1,0]:Step Speed range 1. 
    0x00,                     //  1 Byte: bit[7,6]:Set Imbal. range 4 | bit[5,4]:Set Imbal. range 3 | bit[3,2]:Set Imbal. range 2 | bit[1,0]:Set Imbal. range 1.
    0x00,                     //  1 Byte: bit[7,6]:Rif. Temp. range 4 | bit[5,4]:Rif. Temp. range 3 | bit[3,2]:Rif. Temp. range 2 | bit[1,0]:SRif. Temp.range 1.
    // DOMENICA
    0,                        //  1 Byte: numbers of ranges (from 0=disable to 4)
    0x00, 0x00, 0x00, 0x00,   //  4 Byte: timeON,  Values from 0 to 48 (step di 30')
    0x00, 0x00, 0x00, 0x00,   //  4 Byte: timeOFF, Values from 0 to 48 (step di 30')
    0x00,                     //  1 Byte: bit[7,6]:Step Speed range 4 | bit[5,4]:Step Speed range 3 | bit[3,2]:Step Speed range 2 | bit[1,0]:Step Speed range 1. 
    0x00,                     //  1 Byte: bit[7,6]:Set Imbal. range 4 | bit[5,4]:Set Imbal. range 3 | bit[3,2]:Set Imbal. range 2 | bit[1,0]:Set Imbal. range 1.
    0x00,                     //  1 Byte: bit[7,6]:Rif. Temp. range 4 | bit[5,4]:Rif. Temp. range 3 | bit[3,2]:Rif. Temp. range 2 | bit[1,0]:SRif. Temp.range 1.

       
    0x00,                     //  1 Byte: cntUpdate_dayProg;
        
   //-------------------- (Info EEPROM 2 Byte) ------------------- 
   VERSION_EEP,               // 1 Byte: version_eeprom; 
   CHECK_EEP, CHECK_EEP       // 2 Byte: none   
 };  // totale  Byte: 242
 
extern Byte constrain(Byte x, Byte a, Byte b);
extern Byte eep_read_byte(unsigned int addr);
extern void eep_write_byte(unsigned int addr, Byte data_to_send);

//-----------------------------------------------------------------
// Dopo 12 secondi aggiorna i dati in eeprom. dalla shadow.
//-----------------------------------------------------------------
int CkUpdate_EepromToShadow (void) {
  unsigned int i=0;
  
  if(countdown_Update_EepromToShadow != 0xFF) {
    if(countdown_Update_EepromToShadow)
      countdown_Update_EepromToShadow--;
    else {
      // timer scaduto, andiamo ad aggiornare l'eeprom
      for(i=0; i < 255; i++) {
        if(Shadow_eep_preview[i] != Shadow_eep[i]) {
           eep_write_byte(i,  Shadow_eep[i]); 
           Shadow_eep_preview[i]  = Shadow_eep[i];
        }         
      }  
        
      countdown_Update_EepromToShadow = 0xFF;  
    }  
  }  
  
  return 1;
}

//-----------------------------------------------------------------
// Scrivi 1 byte in eeprom
//-----------------------------------------------------------------
void write_byte_eeprom(unsigned short addr_eep, Byte val)
{
    addr_eep &= 0x0FF; 
    Shadow_eep[addr_eep] = val;
    // dopo 6 secondi aggiorniamo anche la eeprom.
    countdown_Update_EepromToShadow  = 6; // 6 
}  

//-----------------------------------------------------------------
// Scrivi 2 byte in eeprom
//-----------------------------------------------------------------
 void write_word_eeprom(unsigned short addr_eep, unsigned short val_16)
 {
    addr_eep &= 0x0FF; 
    write_byte_eeprom(addr_eep, (Byte)(val_16 & 0x0FF));       // byte LOW
    write_byte_eeprom((addr_eep + 1), (Byte)(val_16 >> 8));    // byte  HI
 }  
 
//-----------------------------------------------------------------
// Scrivi 4 byte in eeprom
//----------------------------------------------------------------- 
void write_Dword_eeprom(unsigned short addr_eep, unsigned long val_32)
{
    addr_eep &= 0x0FF; 
    write_byte_eeprom(addr_eep, (Byte)(val_32 & 0x00000FF));       // byte LOW
    write_byte_eeprom((addr_eep + 1), (Byte)(val_32 >> 8));
    write_byte_eeprom((addr_eep + 2), (Byte)(val_32 >> 16));  
    write_byte_eeprom((addr_eep + 3), (Byte)(val_32 >> 24));  
}

void Erase_EEprom ( void )
{
	int i;
	
	for ( i=0; i<256; i++ )
	{
		//write_byte_eeprom( i, 0xFF);
		eep_write_byte ( i, 0xFF);
	}
}

//-----------------------------------------------------------------
// Leggi 1 byte dalla eeprom
//-----------------------------------------------------------------
 Byte read_byte_eeprom(unsigned short addr_eep)
 {
    addr_eep &= 0x0FF; 
    return Shadow_eep[addr_eep]; 
 }   
 
//-----------------------------------------------------------------
// Leggi 2 byte dalla eeprom
//-----------------------------------------------------------------
 unsigned short read_word_eeprom(unsigned short addr_eep)
 {
   unsigned short val_16 = 0;
   
   addr_eep &= 0x0FF; 
   val_16 = read_byte_eeprom(addr_eep);
   val_16 |= ((unsigned short)read_byte_eeprom(addr_eep+1) << 8);
   
   return val_16; 
 }   
 
 
//-----------------------------------------------------------------
// Leggi 4 byte dalla eeprom, il primo � quello LOW, per poi salire
//-----------------------------------------------------------------
 unsigned long int read_Dword_eeprom(unsigned short addr_eep)
 {
   unsigned long int val_32 = 0;
   
   addr_eep &= 0x0FF; 
   val_32 = (unsigned long)read_byte_eeprom(addr_eep) & 0x0FF;
   val_32 |= (((unsigned long)read_byte_eeprom(addr_eep+1) <<  8) & 0x0000FF00);
   val_32 |= (((unsigned long)read_byte_eeprom(addr_eep+2) << 16) & 0x00FF0000);
   val_32 |= (((unsigned long)read_byte_eeprom(addr_eep+2) << 24) & 0xFF000000);
   
   return val_32; 
 }   
 
//-----------------------------------------------------------------
// Viene upgradata l'unit� ad EXTRA, in base al size si abilitano le
// funzioni CAP & CAF.
//-----------------------------------------------------------------
void upgrade_type_func()
{
   Byte i, msk, size_unit[4];
   
   msk = ENAB_FUNC_EXTRA;
   
   // Leggiamo dal SN  il size della unita'es: 
   // SN: 1565 0023 115 1351 002" 
   // size: 0023
   for(i=0; i <= 3; i++)
      size_unit[i] = read_byte_eeprom(ADDR_EEP(SerialString[4+i]));
   
   //  bit[7]:CAF | bit[6]:CAP | bit[5]:CSF | ...
   // al momento solo le unit� serie 5 utilizzano la funz. CAP/CAF. 
   if(size_unit[3] != '5')
     msk &= 0x3F;
           
   write_byte_eeprom(ADDR_EEP(msk_Enab_Fuction), msk); 
} 
 
//------------------------------------------------------------------
// Func: check_control_speed_func
// 
// Description:
//   Andiamo a vedere la funzionalita dell' unit� (CAP/CAF/CSF) e, 
//   facciamo le varie considerazioni in base all'accessorio configuato,
//   al tipo di motore e alle mappature inserite.
//------------------------------------------------------------------
 void check_control_speed_func()
 {
    unsigned short enab_func, msk_enab_func, new_enab_func, new_msk_enab_func, i;
    Byte acc_cap, acc_caf, temp, msk, type_func, serie_unit;
   
    // -------------- bit: Enab_Fuction ----------------
    //  (LOW)  bit[7]:CAF  | bit[6]:CAP  | bit[5]:FSC    | bit[4]:ImbalanON   | bit[3]:StepLess | bit[2]:WeeklyON | bit[1]:BoostON   | bit[0]:DeFrostON. 
    //  (HIGH) bit[15]:--- | bit[14]:--- | bit[13]:---   | bit[12]:EnPreHeater| bit[11]:EnSummer| bit[10]:EnWinter| bit[9]:CtrlFilter| bit[8]:Password    
    enab_func     = read_word_eeprom(ADDR_EEP(Enab_Fuction));
    msk_enab_func = read_word_eeprom(ADDR_EEP(msk_Enab_Fuction));
    type_func     = read_byte_eeprom(ADDR_EEP(Type_func));
    serie_unit    = read_byte_eeprom(ADDR_EEP(SerialString[7]));  //es:  1565 0023 ...
         
    new_enab_func     = enab_func; 
    new_msk_enab_func = msk_enab_func;

 
    if(type_func == 0) 
    {
        //*************   
        //**  BASIC  **
        //*************   
        // Funz. BASIC, non � possibile in alcun modo abilitare le funz. CAP/CAF
        new_enab_func &= ~0x00C0; // tolgo la possibilit� di far funzione CAP e CAF
        new_msk_enab_func &= ~0x00C0; // tolgo la possibilit� di far funzione CAP e CAF
        new_msk_enab_func |= 0x0020;
    }else {  
        //*************       
        //**  EXTRA  **    
        //*************     
        acc_caf = 0;
        acc_cap = 0;  
        
        // List AccessoyHW[1]:    DPS, PCAF, PCAP, INPUT, OUT, DDPV2, RFM, MBUS  (LSB)
		// vogliamo abilitare CAP anche se connessi FLW1 o FLW2 (in AccessoyHW[3])
		{
		      Byte hw1 = read_byte_eeprom(ADDR_EEP(AccessoyHW[1]));
		      Byte hw3 = read_byte_eeprom(ADDR_EEP(AccessoyHW[3]));
		      // maschera PCAP, FLW1, FLW2
		      Byte m_pcap = 1 << (ACC_EEP_PCAP & 0x07);
		      Byte m_flw1 = 1 << (ACC_EEP_FLW1 & 0x07);
		      Byte m_flw2 = 1 << (ACC_EEP_FLW2 & 0x07);
		
		      if ((hw1 & m_pcap)   // PCAP
		       || (hw3 & m_flw1)   // FLW1
		       || (hw3 & m_flw2))  // FLW2
		      {
		            acc_cap = 1;
		            new_msk_enab_func |= 0x0040;  // abilita bit CAP nella maschera
		      }
		}
        msk   = (1 << (ACC_EEP_PCAF & 0x07));          
        if(temp & msk){
          acc_caf = 1;
          //(LOW) eep:  bit[7]:CAF  | bit[6]:CAP  | bit[5]:CSF    | bit[4]:ImbalanON | bit[3]:STPL | bit[2]:WeeklyON | bit[1]:BoostON   | bit[0]:DeFrostON. 
          new_msk_enab_func |= 0x0080;
        }  
                  
        // Se non ho l'accessorio CAP esterno, e l'unit� non � una serie 5, disabilita la funzione CAP perch� non ho le mappature dei motori
		// Si je n'ai pas l'accessoire CAP externe et que l'appareil n'est pas une s�rie 5, d�sactivez la fonction CAP car je n'ai pas les cartographies moteur
        if(!acc_cap && (serie_unit != '5') && (serie_unit != '8'))
           new_enab_func &= ~0x0040; // NO CAP in eep 
      
        
        // Se non ho l'accessorio CAF o CAP esterno, e l'unit� non � una serie 5, disabilita la funzione CAF perch� non ho le mappature dei motori
		// Si je n'ai pas l'accessoire externe CAF ou CAP, et que l'appareil n'est pas une s�rie 5, d�sactiver la fonction CAF car je n'ai pas les cartographies moteur
        if(!acc_caf && (serie_unit != '5') && (serie_unit != '8')) 
           new_enab_func &= ~0x0080; // NO CAF in eep
      
        
        // Se ho abilitato la Funz. CAP, ma non � abilitata la maschera, Disabilitiamo la funzione 
		// If I have enabled the Function.CAP, but the mask is not enabled. We disable the function          
        if (enab_func & (1 << ENAB_CAP)) 
        {
           //(LOW) eep:  bit[7]:CAF  | bit[6]:CAP  | bit[5]:CSF    | bit[4]:ImbalanON | bit[3]:STPL | bit[2]:WeeklyON | bit[1]:BoostON   | bit[0]:DeFrostON. 
           new_enab_func &= ~0x0080; // NO CAF in eep
           
           if((new_msk_enab_func  & (1 << ENAB_CAP)) == 0)
             new_enab_func &= ~0x0040; // NO CAP in eep  
        }   
           
        if (enab_func & (1 << ENAB_CAF)) 
        {
          new_enab_func &= ~0x0040; // NO CAP in eep
          
          if((new_msk_enab_func  & (1 << ENAB_CAF)) == 0)
             new_enab_func &= ~0x0080;
        }     
         
    }
    
    // Se dopo tutte le varie masturbazioni mentali, risulta che nessun controllo � stato inserito, andiamo con quello di default: FSC
	// If after all the various mental masturbations, it turns out that no controls have been inserted, we go with the default one: FSC
    if((new_enab_func & 0x00C0) == 0)
       new_enab_func |= (1 << ENAB_CSF);
       
    temp =0;
    
    if(new_msk_enab_func != msk_enab_func) { 
       i = ADDR_EEP(msk_Enab_Fuction);
       eep_write_byte(i,  (Byte)new_msk_enab_func); 
       Shadow_eep[i] = (Byte)new_msk_enab_func;
       Shadow_eep_preview[i] = (Byte)new_msk_enab_func;    
       temp =1;          
    }   
    
    if(new_enab_func != enab_func) {
       i = ADDR_EEP(Enab_Fuction);
       eep_write_byte(i,  (Byte)new_enab_func); 
       Shadow_eep[i] = (Byte)new_enab_func;
       Shadow_eep_preview[i]  = (Byte)new_enab_func;   
       temp =1; 
    }  
      
    // aggiornamento del contatore delle info
    if(temp) {
       i = ADDR_EEP(cntUpdate_info); 
       temp = read_byte_eeprom(i);
       temp++;   
       eep_write_byte(i,  (Byte)temp); 
       Shadow_eep[i] = (Byte)temp;
       Shadow_eep_preview[i]  = (Byte)temp;      
    } 
 
 }  
 
 void check_control_speed_func1()
 {
    unsigned short enab_func, msk_enab_func, new_enab_func, new_msk_enab_func, i;
    Byte acc_cap, acc_caf, temp, msk, type_func, serie_unit;
   
    // -------------- bit: Enab_Fuction ----------------
    //  (LOW)  bit[7]:CAF  | bit[6]:CAP  | bit[5]:FSC    | bit[4]:ImbalanON   | bit[3]:StepLess | bit[2]:WeeklyON | bit[1]:BoostON   | bit[0]:DeFrostON. 
    //  (HIGH) bit[15]:--- | bit[14]:--- | bit[13]:---   | bit[12]:EnPreHeater| bit[11]:EnSummer| bit[10]:EnWinter| bit[9]:CtrlFilter| bit[8]:Password    
    enab_func     = read_word_eeprom(ADDR_EEP(Enab_Fuction));
    msk_enab_func = read_word_eeprom(ADDR_EEP(msk_Enab_Fuction));
    type_func     = read_byte_eeprom(ADDR_EEP(Type_func));
    serie_unit    = read_byte_eeprom(ADDR_EEP(SerialString[7]));  //es:  1565 0023 ...
         
    new_enab_func     = enab_func; 
    new_msk_enab_func = msk_enab_func;

 
    if(type_func == 0) 
    {
        //*************   
        //**  BASIC  **
        //*************   
        // Funz. BASIC, non � possibile in alcun modo abilitare le funz. CAP/CAF
        new_enab_func &= ~0x00C0; // tolgo la possibilit� di far funzione CAP e CAF
        new_msk_enab_func &= ~0x00C0; // tolgo la possibilit� di far funzione CAP e CAF
        new_msk_enab_func |= 0x0020;
    }else {  
        //*************       
        //**  EXTRA  **    
        //*************     
        acc_caf = 0;
        acc_cap = 0;  
        
        // List AccessoyHW[1]:    DPS, PCAF, PCAP, INPUT,   OUT,  ----,  ----,  MBUS  (LSB)     
        temp  = read_byte_eeprom(ADDR_EEP(AccessoyHW[3]));
        msk   = (1 << (ACC_EEP_FLW1 & 0x07));  
        
		new_msk_enab_func |= 0x0040; // enable CAP
         
    }
    
    // Se dopo tutte le varie masturbazioni mentali, risulta che nessun controllo � stato inserito, andiamo con quello di default: FSC
	// If after all the various mental masturbations, it turns out that no controls have been inserted, we go with the default one: FSC
    if((new_enab_func & 0x00C0) == 0)
       new_enab_func |= (1 << ENAB_CSF);
       
    temp =0;
    
    if(new_msk_enab_func != msk_enab_func) { 
       i = ADDR_EEP(msk_Enab_Fuction);
       eep_write_byte(i,  (Byte)new_msk_enab_func); 
       Shadow_eep[i] = (Byte)new_msk_enab_func;
       Shadow_eep_preview[i] = (Byte)new_msk_enab_func;    
       temp =1;          
    }   
    
    if(new_enab_func != enab_func) {
       i = ADDR_EEP(Enab_Fuction);
       eep_write_byte(i,  (Byte)new_enab_func); 
       Shadow_eep[i] = (Byte)new_enab_func;
       Shadow_eep_preview[i]  = (Byte)new_enab_func;   
       temp =1; 
    }  
      
    // aggiornamento del contatore delle info
    if(temp) {
       i = ADDR_EEP(cntUpdate_info); 
       temp = read_byte_eeprom(i);
       temp++;   
       eep_write_byte(i,  (Byte)temp); 
       Shadow_eep[i] = (Byte)temp;
       Shadow_eep_preview[i]  = (Byte)temp;      
    } 
 
 }  
 
 //------------------------------------------------------------------
 //  La funzione viene chiamata solo allo start-up del micro. 
 //    1. verifica il contento dell'eeprom, se vuota o corrotta,
 //       inizializza il chip eeprom. 
 //    2. Veriica i CKsum8 delle varie aree
 //    3. Carica i dati dalla eeprom alla Struttura in ram    
 //------------------------------------------------------------------
 void ctrl_eeprom()
 {
    Byte LoadVectDefault =0, cks8, flag;
    unsigned int i, addr[5];
    unsigned short size_eep, enab_func;
	
    size_eep = ADDR_EEP(check_eeprom[1]) + 1;
    
    //----- (1) carichiamo la eeprom nel buffer in ram ----------------  
    for(i=0; i <= size_eep; i++) {
      Shadow_eep[i] = eep_read_byte(i);         
      if((i%32) == 0) 
        delay_ms (2);    
      Shadow_eep_preview[i]  = Shadow_eep[i];
    }  
    
    delay_ms (10);
   
    //----- (2) verifica il contento dell'eeprom, se vuota caricala con i valori di default -----
    addr[0] =  ADDR_EEP(Type_func);  
    addr[1] =  ADDR_EEP(numMotors);    
    addr[2] =  ADDR_EEP(check_eeprom[0]); 
    addr[3] =  ADDR_EEP(version_eeprom);
    
    LoadVectDefault = 0; 
    if(Shadow_eep[addr[0]] > 1)      // Type Fuctions: 0= BASIC, 1= EXTRA
      LoadVectDefault = 1;
      
    if ((Shadow_eep[addr[1]] > 6) || (Shadow_eep[addr[1]] < 2))    // Num. Motors 
      LoadVectDefault = 2;
      
    if ((Shadow_eep[addr[2]] != CHECK_EEP) && (Shadow_eep[addr[3]] != VERSION_EEP))   // ultimi 2 byte 
      LoadVectDefault = 3;    
    
    // prima di cancellare l'eeprom, proviamo a rileggerla  
    if(LoadVectDefault)
    {
       for(i=0; i <= size_eep; i++) {    
          flag = eep_read_byte(i);   
          if(Shadow_eep[i] != flag) {
              delay_ms (2);
              Shadow_eep[i] = eep_read_byte(i); 
              Shadow_eep_preview[i]  = Shadow_eep[i];
          }else {
              if((i%16)== 0)
                delay_ms (2);            
          }                       
       }     
    }    
         
    if(LoadVectDefault) {    
      Serial_println1(" "); 
      Serial_print1("Init eeprom ("); 
      Serial_print(LoadVectDefault, DEC); 
      Serial_println1(")...."); 
      if(LoadVectDefault == 3) { 
        Serial_print1("CK_CTRL:");
        Serial_print(Shadow_eep[addr[2]], HEX);
        Serial_print1(" "); 
        Serial_println(Shadow_eep[addr[3]], HEX);
      }
      
      delay_ms (10);
      wdt_restart(WATCH_DOG);
	  
      // andiamo a riempire l'eeprom con i valori di default eeprom
      for(i=0; i <= size_eep; i++){
        eep_write_byte(i,  vect_default_eeprom[i]); 
        Shadow_eep[i] = vect_default_eeprom[i];
        Shadow_eep_preview[i] = Shadow_eep[i];
        if((i%16)==0)
          delay_ms (2); 
      }  
      
    }else {
       if(read_byte_eeprom(ADDR_EEP(Type_func)) > 1) {
           // forziamo EXTRA
           write_byte_eeprom(ADDR_EEP(Type_func) , 1);
           write_word_eeprom(ADDR_EEP(msk_Enab_Fuction), 0xFFFF);       
       }  
       
       // dalla Ver 2.06 inpostostaimo la posizione delle sonde
       if(read_byte_eeprom(ADDR_EEP(Posiz_NTC)) == 0xFF) {
           i = ADDR_EEP(Posiz_NTC);
           eep_write_byte(i, 0xE4); 
           Shadow_eep[i] = vect_default_eeprom[i];
           Shadow_eep_preview[i] = Shadow_eep[i];
       }  
       
       // dalla 2.12: gestiamo il modulo RFM, abbiamo cambiato un byte (era  progMotors = 'S'  non usato)
       if(read_byte_eeprom(ADDR_EEP(chWireless)) > 16){
           i = ADDR_EEP(chWireless);
           eep_write_byte(i, 1); 
           Shadow_eep[i] = 1;
           Shadow_eep_preview[i] = Shadow_eep[i];
       }  
       
       // dalla 2.16: impostiamo i 2 byte   (..., bit[5,4,3,2]: speed_regulator , bit[1]: 1= TempAmb=TR / TempAmb=TS, bit[0]: ***)
       if (read_byte_eeprom(ADDR_EEP(DeltaTemp_Supply)) > 200) 
       {
          // set_regulate_DXD;       
          // TempSup_hist_summer;    
          // TempSup_hist_winter;    
          for(i = ADDR_EEP(Ref_T_setting); i <= ADDR_EEP(DeltaTemp_Supply); i++) {
             eep_write_byte(i, vect_default_eeprom[i]);
             Shadow_eep[i] =  vect_default_eeprom[i];
             Shadow_eep_preview[i] = Shadow_eep[i];
          }         
       } 
      
       // dalla 2.17: i bits: ENAB_COOLING diventa ENAB_SUMMER 
       //                     ENAB_HEATING diventa ENAB_WINTER  
       // quindi non possono coesistere le 2 funzioni contemporaneamente.
       //  (HIGH) bit[15]:--- | bit[14]:--- | bit[13]:---   | bit[12]:EnPreHeater| bit[11]:EnSummer| bit[10]:EnWinter| bit[9]:CtrlFilter| bit[8]:Password  
       enab_func  = read_word_eeprom(ADDR_EEP(Enab_Fuction));  
       if((enab_func & 0x0C00) == 0x0C00) {
          // se non � presente il Cooler, attiviamo solo la funzione Winter 
          //   parte 1 list Acc.HW: (MSB)    PIR,   BPD,   AWP,   CWD,   EHD,   HWD,  PHWD,  PEHD  (LSB) 
          if(read_byte_eeprom(ADDR_EEP(AccessoyHW[0])) & (1 << ACC_EEP_CWD))
             enab_func &= ~0x0400; // togliamo la funzione WINTER: solo SUMMER
          else   
             enab_func &= ~0x0800; // togliamo la funzione SUMMER: solo WINTER
             
          write_word_eeprom(ADDR_EEP(Enab_Fuction),  enab_func);     
       }  
       
    }   
      
    // verifichiamo la versione del FW, se diversa riscriviamla in eeprom 
    addr[0] =  ADDR_EEP(SW_Vers[0]);
    addr[1] =  ADDR_EEP(SW_Vers[4]); 
    
    for(i=addr[0]; i <= addr[1]; i++){
      if(Shadow_eep[i] != vect_default_eeprom[i])
      {
         eep_write_byte(i,  vect_default_eeprom[i]); 
         Shadow_eep[i] = vect_default_eeprom[i];
         Shadow_eep_preview[i]  = Shadow_eep[i];
      }  
    }   
    


    // Andiamo a vedere la funzionalita di controllo ventole dell'unit� (CAP/CAF/CSF) e, facciamo le varie deduzioni
    check_control_speed_func();
              
    //----- (3) controlliamo i dati in ram e in eeprom coincidano -----
    delay_ms (5);
    
    flag = 0;
    for(i=0;  i < size_eep; i++) {
       if(eep_read_byte(i) !=  Shadow_eep[i]) {
         delay_ms (5);
         if(eep_read_byte(i) !=  Shadow_eep[i])
           flag++;
       } 
         
       if((i%16)==0)
          delay_ms (2);  
    } 
    // inserisci l'allarme di eeprom fault (solo qua per il momento)
    if(flag > 1) {
       Serial_println1(" "); 
       Serial_print1("Eeprom FAULT! N.Byte Error: "); 
       Serial_print(flag, DEC);
       Serial_println1("");
       InsertAlarm(ALM_EEP_FAULT); 
    }
           
 }  
 
//------------------------------------------------------------------
// Controlla i dati inseriti da inserire nella Eeprom.
// se si superano i limiti di alcuni parametri in alcuni casi 
// correggi il buffer, in altri ritorna il -1.
//------------------------------------------------------------------
int ctrl_validity_values(Byte start_addr, Byte end_addr, Byte *buff)
{
  int i, nb, ret = 0;
  unsigned short addr, val_16;  
  Byte val, val_BPD, ctrl_val;
  
    
  //--- ADDR_MODBUS: lo 0 nel mondo MODBUS  � per i comandi di broadcast  
  addr = ADDR_EEP(AddrUnit);
  if ((addr >= start_addr) && (addr <= end_addr)){
    val = buff[addr - start_addr];     
    ctrl_val  = constrain(val, 1, 63);      
    if(val != ctrl_val)
      return -1;
  }  
 
  //--- FUNZ_UNITA:  0= BASIC, 1= EXTRA   
  addr = ADDR_EEP(Type_func); 
  if ((addr >= start_addr) && (addr <= end_addr)){
    val = buff[addr - start_addr];  
    if(val > 1)
      buff[addr - start_addr] = (Byte)0x01;  
  }    
    
  
  //--- NUM_MOTORS: valori accettati: 2, 4, 6.
  addr = ADDR_EEP(numMotors);
  if ((addr >= start_addr) && (addr <= end_addr)){
    
    val = buff[addr - start_addr]; 
    
    if((val != 2) && (val != 4) && (val != 6))
      return -1; 
  }    
  
  //--- BYPASS: 0=Automatic, 1=Ext CTRL, 2=Manual CLOSE, 3=Manual OPEN 4= Automatic+Standby
  addr = ADDR_EEP(Config_Bypass);
  
  if ((addr >= start_addr) && (addr <= end_addr)){
    val_BPD = buff[addr - start_addr]; 
    
    if(val_BPD > BPD_AUTO_ON_OFF)   
       return -1;
  }
                                  
  //--- INPUT:   0= Disable, ... , 5=  0V->Bypass Open , 10V->Bypass Closed,	  
  // 1 se il bypass non � config. con Ext CTRL, e vi sono invece input config. per pilortare il bypass
  //   disabilito gli input  
  for(i=0; i < 2; i++) {
    addr = ADDR_EEP(Set_Input[i]);
    if ((addr >= start_addr) && (addr <= end_addr)){
      val = buff[addr - start_addr]; 
   
      if(((val > INP_PIR) && (val < INP_12V_ALM_FIRE_UNIT_STOP)) || (val > INP_12V_ALM_FIRE_ONLY_SUPPLY)) 
         buff[addr - start_addr] = (Byte)INP_DISABLE;  // default: Disable   
    }  
  }
  
  //--- OUTPUT  ---  
  // 0= Disable, ... , 3= Idle/Standby Mode (SDD),  128= 0= Caduta / 1=Attrazione 
  for(i=0; i < 2; i++) {
    addr = ADDR_EEP(Set_Output[i]);
    if ((addr >= start_addr) && (addr <= end_addr)){
      
      val = buff[addr - start_addr];
      if((val & 0x7F) > 6) // 6 = MAX SPEED
         return -1;        
    }  
  }  
  
  //--- SET_STEP_MOTOR: 0, 1, 2, 3=Stepless ---  
  addr = ADDR_EEP(sel_idxStepMotors);
  if ((addr >= start_addr) && (addr <= end_addr)){
    val = buff[addr - start_addr];
    
    if(val > 3)
      return -1;   
  }     

  //--- TIME BOOST: valori tra 15' a 240' ---
  addr = ADDR_EEP(Set_TimeBoost);
  if ((addr >= start_addr) && (addr <= end_addr)){
    val  = buff[addr - start_addr]; 
    
    if ((val < 15) || (val >240)) 
      return -1; 
  }
  
  //--- SETPOINT CO2 in PPM --- 
  addr = ADDR_EEP(SetPoint_CO2);
  if ((addr >= start_addr) && (addr < end_addr)){
    
    val_16  = ((buff[addr + 1 - start_addr]  << 8)&0xFF00) + buff[addr - start_addr]; 
    
    if ((val_16  <= 450) || (val_16 > 1600)) 
       return -1;
  }
  
  //--- SETPOINT RH da 20% a 100% ---   
  addr = ADDR_EEP(SetPoint_RH);
  if ((addr >= start_addr) && (addr <= end_addr)){
    val  = buff[addr - start_addr]; 
    
    if((val < 20) || (val > 99)) 
       return -1; 
  }
  
  //--- SETPOINR VOC: da 8 a 90 PPM ---
  addr = ADDR_EEP(SetPoint_VOC);
  if ((addr >= start_addr) && (addr < end_addr)){
    val_16  = ((buff[addr + 1 - start_addr]  << 8)&0xFF00) + buff[addr - start_addr]; 
    
    if ((val_16  < 8) || (val_16 > 90)) 
      return -1;
  }
  
//--- Giorni manutenzione filtro sporco: da 30 a 360gg --- // Tolto questa parte di condice perch� rompe le scatole al DPP -- Nicola dalla 2.22 in poi (andrebbe introdotta la doppia validit�)
//  addr = ADDR_EEP(gg_manut_Filter);
//  if ((addr >= start_addr) && (addr < end_addr)){
//    val_16  = ((buff[addr + 1 - start_addr]  << 8)&0xFF00) + buff[addr - start_addr]; 
    
//    if ((val_16  < 30) || (val_16 > 360)) 
//      return -1;
//  }
  
  //--- MIN. TEMP. BYPASS: 11.0�C a 30.0�C ---
  addr = ADDR_EEP(Bypass_minTempExt);
  if ((addr >= start_addr) && (addr < end_addr)){
    val_16  = ((buff[addr + 1 - start_addr]  << 8)&0xFF00) + buff[addr - start_addr]; 
    
    if ((val_16  < 120) || (val_16 > 350))
       return -1;
  }  
      
  for(i=0; i < 2; i++) {
     //--- Set-Point Temperature 1&2 value 15.0 �C a 32.0 �C. ---
     addr = ADDR_EEP(SetPointTemp[i]);
     if ((addr >= start_addr) && (addr < end_addr)){
       val_16  = ((buff[addr + 1 - start_addr]  << 8) & 0xFF00) + buff[addr - start_addr];  
   
       if ((val_16  < 150) || (val_16 > 350)) 
          return -1;
     }    
  } 
  
  return 0; // ok
}

//------------------------------------------------------------------
// Bisogna oltre che controllare i limiti anche la non sovrapposizione
// dei valori.
//------------------------------------------------------------------
Byte ctrl_lim_histTemp()
{
   signed char Ton, Toff, val;
   Byte ret =0;
    
   // ---- Limiti valori sull'isteresi temperature ON/OFF  HWD/EHD. ----
   //  si accende basso es: 21.0 , si spegne alto 24.0
   // --- CON AWP ---
   val= 0;
   Ton  = (signed char)read_byte_eeprom(ADDR_EEP(hister_AWP_Temp_Hot[1])); // Value Default  ON: -2.0�C -> 22.0�C -2.0� = 20.0�C
   Toff = (signed char)read_byte_eeprom(ADDR_EEP(hister_AWP_Temp_Hot[0])); // Value Default OFF: +1.5�C -> 22.0�C +1.5� = 23.5�C

   if(Ton < -100) {
      Ton = -100;
      val = 1;
   }
   
   if(Ton > 100) {
      Ton = 100;
      val = 1;
   }
   
   if(Toff > 100) {
      Toff = 100;
      val = 1;
   }
   
   if (Toff < (Ton + 10)) {
      Toff = (Ton + 10); 
      val = 1;
   }   
   
  
   if(val) {
     ret = 1;
     write_byte_eeprom(ADDR_EEP(hister_AWP_Temp_Hot[0]), Toff); // OFF
     write_byte_eeprom(ADDR_EEP(hister_AWP_Temp_Hot[1]),  Ton); //
   }  
 
    // ---  SENZA AWP ----
   val = 0;     
   Ton  = (signed char)read_byte_eeprom(ADDR_EEP(hister_Temp_Hot[1])); // Value default  ON: -2.0�C -> 22.0 -2.0�C = 20.0�C  
   Toff = (signed char)read_byte_eeprom(ADDR_EEP(hister_Temp_Hot[0]));  // Value default OFF: +5.0�C -> 22.0 +5.0�C = 27.0�C
  
   if(Ton < -100) {
      Ton = -100;
      val = 1;
   }
   
   if(Ton > 100) {
      Ton = 100;
      val = 1;
   }
  
   if(Toff > 100) {
      Toff = 100;
      val = 1;
   } 
  
   if (Toff < (Ton + 20)) {
      Toff = (Ton + 20); 
      val = 1;
   }  
   
  
   
   if(val) {
      ret = 1;
      write_byte_eeprom(ADDR_EEP(hister_Temp_Hot[0]), Toff); // OFF
      write_byte_eeprom(ADDR_EEP(hister_Temp_Hot[1]),  Ton); //
   }  
   // ---- Limite valori sull'isteresi temperature ON/OFF CWD ---- 
   //  si accende alto es: 26.0 , si spegne basso 21.0
   val = 0;
    
   Ton  = (signed char)read_byte_eeprom(ADDR_EEP(hister_AWP_Temp_Cold[1])); // Value default  ON: +4.0�C -> 22.0�C + 4.0�C = 26.0 �C
   Toff = (signed char)read_byte_eeprom(ADDR_EEP(hister_AWP_Temp_Cold[0])); // Value default OFF: +1.0�C -> 22.0�C + 1.0�C = 23.0 �C  
   
   if (Ton > 100) {
      Ton  = 100; 
      val = 1;
   }      
   
   if (Ton < -100) {
      Ton  = -100; 
      val = 1;
   }    
   
   if(Toff > (Ton - 10)) {
      Toff = (Ton - 10);
      val  = 1;
   }   
   
   if(Toff < -100) {
      Toff = -100;
       val  = 1;
   }  
 
   if(val) {
      ret = 1;
      write_byte_eeprom(ADDR_EEP(hister_AWP_Temp_Cold[0]), Toff); // OFF
      write_byte_eeprom(ADDR_EEP(hister_AWP_Temp_Cold[1]),  Ton); //
   }
   
   val = 0;
   Ton  = (signed char)read_byte_eeprom(ADDR_EEP(hister_Temp_Cold[1]));  // Value default  ON: +5.0�C -> 22.0�C + 4.0�C = 27.0 �C   
   Toff = (signed char)read_byte_eeprom(ADDR_EEP(hister_Temp_Cold[0]));  // Value default OFF: +0.0�C -> 22.0�C - 1.0�C = 21.0 �C  
   
   if (Ton > 100) {
      Ton  = 100; 
      val = 1;
   }   

   if (Ton < -100) {
      Ton  = -100; 
      val = 1;
   }      
   
   if(Toff > (Ton - 20)) {
      Toff = (Ton - 20);
      val  = 1;
   }   
   
   if(Toff < -100) {
      Toff = -100;
      val  = 1;
   }  
   
   if(val) {
      ret = 1;
      write_byte_eeprom(ADDR_EEP(hister_Temp_Cold[0]), Toff); // OFF
      write_byte_eeprom(ADDR_EEP(hister_Temp_Cold[1]),  Ton); //
   }
   
   return ret;
       
}  


//------------------------------------------------------------------
// Aggiorna la eeprom, dopo aver fatto i controlli dei dati
// se c'� un errore di parametri ritorna -1
//------------------------------------------------------------------
 int update_data_eeprom(unsigned short start_addr, unsigned short end_addr, Byte *buff)
 {
    Byte cnt_update_eep,  i, val;
    unsigned short a,  addr1, addr2;
    int valid_data;
   

    //--- (1) andiamo a verificare i limiti di alcuni parametri e li correggiamo ----
    valid_data = ctrl_validity_values(start_addr, end_addr, &buff[0]);    


    if(valid_data == -1)
      return -1;


    for(a=start_addr, i=0;  a <= end_addr; a++, i++) 
       write_byte_eeprom(a,  buff[i]);   

       
    //--- (2) Aggiorniamo i contatori  ---    
    // -- info ---
    // ma solo se vi sono byte modificati dentro la zona info 
    addr1 = 0;
    addr2 = ADDR_EEP(cntUpdate_info); 
        
    if(start_addr < addr2) {         
             
       cnt_update_eep = read_byte_eeprom(addr2); 
       cnt_update_eep++;
                        
       //1.  se vi � un Upgrade della Type_func, attiviamo tutte le funzioni.
       addr1 = ADDR_EEP(Type_func);
       if((Shadow_eep_preview[addr1] != Shadow_eep[addr1]) && Shadow_eep[addr1]) 
           upgrade_type_func();          
       
       
       //2. Se � cambiata una configurazione delle funzioni facciamo attenzione che rispetti i parametri EXTRA/ BASIC
       addr1 = ADDR_EEP(Enab_Fuction);
       if (Shadow_eep_preview[addr1] != Shadow_eep[addr1]) {        
           if (Shadow_eep[ADDR_EEP(Type_func)])
             i = (Shadow_eep[addr1] & ENAB_FUNC_EXTRA);
           else  { 
             i = (Shadow_eep[addr1] & ENAB_FUNC_BASIC);
             i |= ENAB_CSF;
           }  
             
           write_byte_eeprom(addr1, i); 
       } 
       
       write_byte_eeprom(addr2,  cnt_update_eep); 
       
    }  
    
   
    // --- Setting parameters ---
    // ma solo se vi sono byte modificati dentro la zona Setting par. 
    addr1 = ADDR_EEP(Set_Power_ON); 
    addr2 = ADDR_EEP(cntUpdate_SettingPar); 
       
    if ((start_addr < addr2) && (end_addr >= addr1)) 
    { 
       cnt_update_eep = read_byte_eeprom(addr2); 
       cnt_update_eep++;
       
       write_byte_eeprom(addr2,  cnt_update_eep);   
    } 

    // ---  Soglie Temperature ---
    // ma solo se vi sono byte modificati dentro la zona Soglie Temp. 
    addr1 =  ADDR_EEP(Bypass_minTempExt); 
    addr2 =  ADDR_EEP(cntUpdate_SetTemp);   
    
    if ((start_addr < addr2) && (end_addr >= addr1))
    {      
       cnt_update_eep = read_byte_eeprom(addr2); 
       cnt_update_eep++;
       
       cnt_update_eep += ctrl_lim_histTemp(); 
       
       write_byte_eeprom(addr2,  cnt_update_eep);   
    }
  
    // --- weekly program ---
    // ma solo se vi sono byte modificati dentro la zona weekly. 
    addr1 =  ADDR_EEP(cntUpdate_SetTemp) + 1;
    addr2 =  ADDR_EEP(cntUpdate_dayProg);
    
    if ((start_addr < addr2) && (end_addr >= addr1))
    {    
       cnt_update_eep = read_byte_eeprom(addr2); 
       cnt_update_eep++;
       write_byte_eeprom(addr2,  cnt_update_eep);   
    }    
          
    //--- (3) Aggiorniamo la Struttura dati in ram ---
#if 0    
    addr1 = 0;
    addr2 = ADDR_EEP(version_eeprom);  
    cnt_update_eep = read_byte_eeprom(addr2); 
    cnt_update_eep++;
    write_byte_eeprom(addr2,  cnt_update_eep);   
#endif

    return 0;
           
 }
 