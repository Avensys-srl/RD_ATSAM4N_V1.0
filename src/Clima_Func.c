/*
 * Clima_Func.cpp
 *
 * Descrizione:
 *   Nel seguente file vengono racchiuse tutte le funzioni che gestiscono le temperature attraverso:
 *   (Bypass, Defrost, CWD, EHD, PEHD, ...)
 */ 
#include "asf.h"

#include "data.h"
#include "eeprom_map.h"
#include "alarm.h"

#ifdef VIEW_DEBUG_CODE_NO_KTS 
//#define VIEW_DEBUG_BYPASS
//#define VIEW_DEBUG_CLIMA
#endif

#define word	uint16_t

//#define _DBG_BANCO  // se definito, spengo alcuni controlli per permettere di debuggare la funzione
//#define VIEW_DEBUG_BYPASS

#define STEP_REG_DXD  5  // Step di correnzione 0-10V del compressore (5 = 0,5 volt)

// stato del Rele' e delle tensione dell'elettronica
#define POWER_SAVE     0
#define POWER_ON      'O'

//------------------ Stati BYPASS ---------------------
#define ANTIORARIO       0xFF
#define ORARIO           0
#define CLOSE_BPD        1   // bypass chiuso, il flusso dell'aria passa dallo scambiatore
#define OPEN_BPD         0   // bypass aperto 100%, il flusso dell'aria passa solo dal canale Bypass.

//---------------- Vars Global -------------------------------------------
int count_preHeaterOn = 0;
int persist_Temp_heater_off = 0;
int persist_temp_bypass_on  = 0; 
int count_active_freecooling_off=0;
int count_active_freecooling_on=0;
int standby_before;

extern SDATA  sData;

//-----------------------------------------------------
// Variabili di appoggio per EBPD e SSR
//-----------------------------------------------------

extern Byte info_SSR;
extern Byte info_EBPD; // definisco un valore iniziale fuori dal range in modo tale da obbligare la sincronizzazione sin dal primo ciclo.
extern Byte change_SSR;
extern Byte change_EBPD;
extern Byte change_EBP2;

extern Bool Tcheck;

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

extern short  Temp_modbus[4];

short Value1, Value2;
 
extern void PowerMode(Byte sts);
extern void Active_Procedure_Bypass_OpCl(Byte direct_bpd, int SecondsWait);
extern void TAG_DigitAccessoryOff(int nAcc_I2C);
extern Byte AccessoryPresent_EepList(int nAccEep);
extern void TAG_DigitAccessoryOn(int nAcc_I2C);

//------------------------------------------------------------------------
//  Gestione del dispositivo Bypass, in funzione della configurazione in
//  eeprom.
//    Config_Bypass = 0 -> Automatico in funz. delle temperature.  
//    Config_Bypass = 1 -> Viene comandato dall'Input 1  
//    Config_Bypass = 2 -> Viene comandato dall'Input 2  
//    Config_Bypass = 3 -> Manuale in Posizione CLOSE    
//    Config_Bypass = 4 -> Manuale in Posizione OPEN  
//
// NB: Bypass CLOSE, l'aria passa dallo Scambiatore
//     Bypass OPEN, l'aria passa dal canale Bypass 
//------------------------------------------------------------------------
void managementBypass(unsigned short enab_func)
{
  Byte i_rifTemp, cfg_bypass, cfg_inp, flag_ctrl_active_freecooling, run_active_freecooling, cntUpdate_SettingPar;
  int i;
  word mskBPD_Close = 0;
  short SetPointTemp, Bypass_minTemp, rif_temp_interna;
  Bool  cmd_bypass_open = false, cmd_bypass_close = false;
  
  
  // per prima cosa verifichiamo se � in test
  if( (sData.status_test & MSK_TEST_BYPASS) )
    return;

      
  cfg_bypass = read_byte_eeprom(ADDR_EEP(Config_Bypass));  

//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' x_dbg  
#ifdef VIEW_DEBUG_BYPASS
   if(cfg_bypass == BPD_AUTOMATIC)
     Serial_println1("Bypass Auto:");
   else if (cfg_bypass == BPD_EXT_CTRL)
     Serial_println1("Bypass ExtCTRL:");
   else if (cfg_bypass == BPD_AUTO_ON_OFF)
     Serial_println1("Bypass AUTO_ON_OFF:");
   else  
     Serial_println1("Bypass MAN:");
#endif       
//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' x_dbg  
  
  mskBPD_Close |= (1 << POS_BIT_BYPASS_CLOSE);
  
  switch (cfg_bypass)
  {     
    case BPD_MANUAL_CLOSE:
        cmd_bypass_close = true;
        cmd_bypass_open  = false;  
    break;
    
    case BPD_MANUAL_OPEN:
        cmd_bypass_open  = true; 
        cmd_bypass_close = false;  
    break;
    
    case BPD_AUTO_ON_OFF:
    
         if(!CkAlarm(ALM_PTFRESH_KO) && !CkAlarm(ALM_PTRET_KO))
        {     
          
           if( (count_active_freecooling_on > 0) && (count_active_freecooling_on < 60)) {
           // Mi sono attivato, rimango cos� per 5 minuti (60 cicli), a prescindere da tutto.
              if (read_byte_eeprom(ADDR_EEP(Set_Power_ON)) == 0)  //se ero spento mi accendo
              {
              write_byte_eeprom(ADDR_EEP(Set_Power_ON), 1);
              cntUpdate_SettingPar = read_byte_eeprom( ADDR_EEP( cntUpdate_SettingPar ) );
              cntUpdate_SettingPar++;
              write_byte_eeprom( ADDR_EEP( cntUpdate_SettingPar ), cntUpdate_SettingPar );
              PowerMode(POWER_ON);
              standby_before = 1;  // devo ricordarmi che ero spento
              }
              count_active_freecooling_on++;
              count_active_freecooling_off = 0;
           }else
             {              
              count_active_freecooling_on = 0;
              cmd_bypass_close = true;  //chiudo il bypass
              cmd_bypass_open  = false;  
              if (standby_before == 1 && count_active_freecooling_off == 0)  //se ero spento....
              {
                write_byte_eeprom(ADDR_EEP(Set_Power_ON), 0);  //....mi rispengo
                cntUpdate_SettingPar = read_byte_eeprom( ADDR_EEP( cntUpdate_SettingPar ) );
                cntUpdate_SettingPar++;
                write_byte_eeprom( ADDR_EEP( cntUpdate_SettingPar ), cntUpdate_SettingPar );
                PowerMode(POWER_SAVE);
                standby_before = 0;
                }
              count_active_freecooling_off++;
             }

        if( (count_active_freecooling_off > 0) && (count_active_freecooling_off <= 360)) {
             // Sono spento, attendo 30 minuti (360 cicli), prima di verificare le condizione di una nuova riaccensione.
             cmd_bypass_open = false;
             cmd_bypass_close = true;
             //count_active_freecooling_off++;       solo per velocizzare in caso di debug
             }
        if (count_active_freecooling_off > 360)
            {
            if((sData.measure_Temp[I_PROBE_RETURN] >= SetPointTemp) &&     
              (sData.measure_Temp[I_PROBE_FRESH] > Bypass_minTemp) &&                    
              (sData.measure_Temp[I_PROBE_FRESH] <= (sData.measure_Temp[I_PROBE_RETURN] + 5)))   // se ci sono le condizioni.....                                          
             {                
             if (read_byte_eeprom(ADDR_EEP(Set_Power_ON)) == 0)  //se ero spento mi accendo
              {
              write_byte_eeprom(ADDR_EEP(Set_Power_ON), 1);
              cntUpdate_SettingPar = read_byte_eeprom( ADDR_EEP( cntUpdate_SettingPar ) );
              cntUpdate_SettingPar++;
              write_byte_eeprom( ADDR_EEP( cntUpdate_SettingPar ), cntUpdate_SettingPar );
              PowerMode(POWER_ON);
              standby_before = 1;  // devo ricordarmi che ero spento
              }
              cmd_bypass_open  = true;  // apro il bypass
              cmd_bypass_close = false;
              count_active_freecooling_on++; 
              count_active_freecooling_off = 0; 
             }
           else 
             {
             count_active_freecooling_off = 1; 
             }
           }

      }
         break;
    
    
    case BPD_AUTOMATIC:      
        // se l'accessorio DXD � acceso chiudi il Bypass 
        if(DigitAccessoryOn(ACC_I2C_DXD)){
          cmd_bypass_close = true;
          cmd_bypass_open  = false;
          persist_temp_bypass_on = 0; 
        }else {      
           /* BPD_AUTOMATIC: in funzione delle temperaturare Fresh, Return */                
          i_rifTemp        = read_byte_eeprom(ADDR_EEP(idxSetPointT)) & 0x01;
          rif_temp_interna = read_word_eeprom(ADDR_EEP(SetPointTemp[i_rifTemp])); 
          Bypass_minTemp   = read_word_eeprom(ADDR_EEP(Bypass_minTempExt));
          
          if(DigitAccessoryOperating(ACC_I2C_HEATER)  && (enab_func &  (1 << ENAB_WINTER)))  {
               // andiamo a lavorare dopo lo spegnemento della resistenza.  
              SetPointTemp = rif_temp_interna + (short)read_byte_eeprom(ADDR_EEP(hister_Temp_Hot[0]));
          }else        
              SetPointTemp = rif_temp_interna;
          
      
          // La gestione del Bypass Automatic viene fatta solo se sono  funzionanti le due sonde TRet e TFres.
          if(!CkAlarm(ALM_PTFRESH_KO) && !CkAlarm(ALM_PTRET_KO))
          {    
              //============== Condizioni di Apertura del bypass ==============  
              //  1: La Temperatura all'interno � >= del SetPoint di Temp. (Attenzione se c'� l'HEATER allo spegnimento dell'HEATER)
              //  2: La Temperatura all'esterno � > della soglia minima di funz. del bypass  (default: 14.0�C)
              //  3: La Temperatura all'esterno � minore di 2.0�C della temparatura all'interno
              //  4: Non vi sono Condizionatori Accesi. 
              if((sData.measure_Temp[I_PROBE_RETURN] >= (SetPointTemp + 10)) &&     
                 (sData.measure_Temp[I_PROBE_FRESH] > Bypass_minTemp) &&                    
                 (sData.measure_Temp[I_PROBE_FRESH] <  (sData.measure_Temp[I_PROBE_RETURN]-20)) &&  
                 !DigitAccessoryOn(ACC_I2C_COOLER))                                                
              { 
                 if(persist_temp_bypass_on++ > 3) {               
                   cmd_bypass_open = true;
                   cmd_bypass_close = false;
                   persist_temp_bypass_on = 4;
                 }  
              }else
                 persist_temp_bypass_on = 0; 
                          
               //============== Condizioni di Chiusura del bypass ==============
               //  1: La Temp. Esterna >=  della Temp. Interna
               //  2: La Temp. Esterna < della Soglia min. di Funz. Bypass
               //  3: La Temperatura Interna � minore del SetPoint Temp.Interna -2.0�C (default: 20.0�C)
               //  4: Riscaldatore (EHD) � Acceso 
               //  5: il Raffreddatore (CWD)  � ACCESO.
               if((sData.measure_Temp[I_PROBE_FRESH] >= sData.measure_Temp[I_PROBE_RETURN]) ||    
                  (sData.measure_Temp[I_PROBE_FRESH] < Bypass_minTemp) ||                 
                  (sData.measure_Temp[I_PROBE_RETURN] <= (rif_temp_interna - 20)) ||  
                   DigitAccessoryOn(ACC_I2C_HEATER)  ||                                          
                   DigitAccessoryOn(ACC_I2C_COOLER))                                              
                  
               {
                 cmd_bypass_close = true;
                 cmd_bypass_open  = false;
                 persist_temp_bypass_on = 0; 
                }                          
              
          }else 
             cmd_bypass_close = true; 
          }
      
    break;
    
    case BPD_EXT_CTRL:    
        cfg_inp = read_byte_eeprom(ADDR_EEP(Set_Input[0]));  
        
        // 4 = 12V->Bypass Open ,  0V->Bypass Closed 
        // 5 =  0V->Bypass Open , 12V->Bypass Closed	
        if ((cfg_inp != INP_12V_BYPASS_OPEN) && (cfg_inp != INP_0V_BYPASS_OPEN))
            cfg_inp = read_byte_eeprom(ADDR_EEP(Set_Input[1]));  // passa a verificare l'input 2
              
        if(cfg_inp == INP_12V_BYPASS_OPEN) 
        {    
            //============== Config_4 =  12V->Bypass Open , 0V->Bypass Closed	==============     
            // 1: Se il Bypass � Aperto Ma il comando che leggiamo dall'input  � di chiusura (< 2,5Volt), attiva la procedura di chiusura
            if (((sData.status_unit & MSK_STS_BYPASS) == 0) && (sData.measure_ADC_input[i] < 250)) 
               cmd_bypass_close = true;
            // 2: Se il Bypass � Chiuso Ma il comando che leggiamo dall'input  � di apetura (> 6,0Volt), attiva la procedura di apertura    
            else if (((sData.status_unit & MSK_STS_BYPASS) == mskBPD_Close) &&  (sData.measure_ADC_input[i] > 610)) 
               cmd_bypass_open  = true;     
        }else if (cfg_inp == INP_0V_BYPASS_OPEN) 
        {
             //============== Config_5 =  0V->Bypass Open , 12V->Bypass Closed	==============
             // 1: Se il Bypass � Aperto Ma il comando che leggiamo dall'input  � di chiusura (> 6,0Volt), attiva la procedura di chiusura
            if (((sData.status_unit & MSK_STS_BYPASS) == 0) && (sData.measure_ADC_input[i] > 610)) 
               cmd_bypass_close = true;
            // 2: Se il Bypass � Chiuso Ma il comando che leggiamo dall'input  � di apetura (< 2,5Volt), attiva la procedura di apertura    
            else if (((sData.status_unit & MSK_STS_BYPASS) == mskBPD_Close) &&  (sData.measure_ADC_input[i] < 250)) 
               cmd_bypass_open  = true;         
        }  
    break;
    
  } 


//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' x_dbg  
#ifdef VIEW_DEBUG_BYPASS
   if (cfg_bypass == BPD_EXT_CTRL){
     Serial_print1("IN1: ");
     Serial_print(sData.measure_ADC_input[0], DEC);
     Serial_print1("  IN2: ");
     Serial_print(sData.measure_ADC_input[1], DEC);
  } else {
     Serial_print1("RifT: ");
     Serial_print((float)SetPointTemp/10, 1);
     Serial_print1("  BypTmin: ");
     Serial_print((float)Bypass_minTemp/10, 1);
     Serial_print1("  TF: ");
     Serial_print((float)sData.measure_Temp[I_PROBE_FRESH]/10, 1);
     Serial_print1("  TR: ");
     Serial_print((float)sData.measure_Temp[I_PROBE_RETURN]/10, 1); 
   }
   
   Serial_print1("  StsBPD: "); 
   if ((sData.status_unit & MSK_STS_BYPASS) == mskBPD_Close)
      Serial_print1("CLOSE"); 
   else if ((sData.status_unit & MSK_STS_BYPASS) == (1 << POS_BIT_BYPASS_RUN))  
      Serial_print1("Change");
   else   
      Serial_print1("OPEN"); 
   
   
   if(cmd_bypass_open)
     Serial_print1("  CmdBPD: OPEN ");
   else if(cmd_bypass_close)  
     Serial_print1("  Cmdbpd: CLOSE ");   
     
     Serial_print1("  persistenza ON: ");
     Serial_print(persist_temp_bypass_on, DEC);
     Serial_print1("  ON: ");
     Serial_print(count_active_freecooling_on, DEC);
     Serial_print1("  OFF: ");
     Serial_print(count_active_freecooling_off, DEC);
     Serial_print1("  STDBF: ");
     Serial_print(standby_before, DEC);
     
   Serial_println1(" ");  
#endif
//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' x_dbg      
 
  // attendi N secondi affinch� la velocit� dei motori diminuisca, quindi attiva il motorino BPD.
  if(sData.speed_motors_F > 200)        i = 44;     
  else if (sData.speed_motors_F > 150)  i = 37;
  else if (sData.speed_motors_F > 110)  i = 30;  
  else                                  i = 23;   
  
  
  if(cmd_bypass_open && ((sData.status_unit & MSK_STS_BYPASS) == mskBPD_Close))  
    Active_Procedure_Bypass_OpCl(OPEN_BPD, 9);  
  else if(cmd_bypass_close && ((sData.status_unit & MSK_STS_BYPASS) == !mskBPD_Close)) 
    Active_Procedure_Bypass_OpCl(CLOSE_BPD, 9);  

}  



//------------------------------------------------------------------------
// In funzione delle temperature Tfresh e Treturn, attiva gli accessori e
// pilota il bypass. la funzione gira nello scheduler (sample = 5 sec.)
//
//  Funzione Modificata dalla 2.16: introdotto Acc. DXD
//------------------------------------------------------------------------
int TemperatureRegulate()
{
  int i; 
  short TempRif, Temp_off_Heater_immediately = 0, TempAmbient = 250, soglia_Off_Heater, soglia_On_Heater, soglia_Off_Cooler, soglia_On_Cooler;
  unsigned short enab_func = 0, SetPointTemp, SetPointTemp1;
  Byte msk_bit = 0, nbyte, ref_T_setting;
  Byte i_rifTemp, stato_heater, stato_cooler, cfg_bypass;
  enab_func  = read_word_eeprom(ADDR_EEP(Enab_Fuction)); 
  cfg_bypass = read_byte_eeprom(ADDR_EEP(Config_Bypass));

//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' x_dbg 
#ifdef VIEW_DEBUG_CLIMA
  Serial_println1(" ");
  Serial_println1("T. Regulate:");
#endif
//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' x_dbg   

  //---- (1) Se l'unit� non � in funzione esci dalla funzione ----
  if((sData.status_unit & (1 << POS_BIT_UNIT_RUN)) == 0  &&  ( cfg_bypass != BPD_AUTO_ON_OFF ) && !(enab_func & (1 << ENAB_MBF))  )  {
    // Se il bypass era aperto chiudilo
    if((sData.status_unit & MSK_STS_BYPASS) == 0) 
       Active_Procedure_Bypass_OpCl(CLOSE_BPD, 2);
    
    // spengo gli accesori se attivi
    if(DigitAccessoryOn(ACC_I2C_COOLER))
       TAG_DigitAccessoryOff(ACC_I2C_COOLER);
    
    if(DigitAccessoryOn(ACC_I2C_HEATER)) 
       TAG_DigitAccessoryOff(ACC_I2C_HEATER); 
       
    if(DigitAccessoryOn(ACC_I2C_DXD)) 
       TAG_DigitAccessoryOff(ACC_I2C_DXD); 
   
    if( info_SSR > 0)
	  write_byte_eeprom(ADDR_EEP(Set_EHD_mod), 0);	
      
    return 1;  
    
  } else
  if(((sData.status_unit & (1 << POS_BIT_UNIT_RUN)) == 0)  && !(enab_func & (1 << ENAB_MBF)))
  {
     // spengo gli accessori se attivi
    if(DigitAccessoryOn(ACC_I2C_COOLER))
       TAG_DigitAccessoryOff(ACC_I2C_COOLER);
    
    if(DigitAccessoryOn(ACC_I2C_HEATER)) 
       TAG_DigitAccessoryOff(ACC_I2C_HEATER); 
       
    if(DigitAccessoryOn(ACC_I2C_DXD)) 
       TAG_DigitAccessoryOff(ACC_I2C_DXD); 
      
    if ( info_SSR > 0)
	  write_byte_eeprom(ADDR_EEP(Set_EHD_mod), 0);	  
  
  }
 
  
  //---- (2) Gestione ByPass ----
  managementBypass(enab_func);

  //---- (3)  Se � attivo almeno un accessorio di Riscaldamento o raffreddamento procedi ----
  if(!DigitAccessoryOperating(ACC_I2C_HEATER) && !DigitAccessoryOperating(ACC_I2C_COOLER) && !DigitAccessoryOperating(ACC_I2C_PREHEATER) && !AccessoryPresent_EepList(ACC_EEP_DXD) && !AccessoryPresent_EepList(ACC_EEP_SSR))
    return 1;       // se non vi � nessuno sistema per riscaldare o raffreddare, che ci sto a f� qui ??? Esco.

  //---- (4) In caso di guasto di uno dei motori spegni subito gli accessori clima a meno che non sia il MODBUS a comandare i motori   
  if ((sData.StatusMotorsR & 0xF0) || (sData.StatusMotorsF &0xF0) || CkAlarm(ALM_EB_CTRL_FAN) && !(enab_func & (1 << ENAB_MBF))) {
      if(DigitAccessoryOn(ACC_I2C_PREHEATER))  
         TAG_DigitAccessoryOff(ACC_I2C_PREHEATER);   
         
      if(DigitAccessoryOn(ACC_I2C_HEATER))  
         TAG_DigitAccessoryOff(ACC_I2C_HEATER);     
        
      if(DigitAccessoryOn(ACC_I2C_COOLER))
         TAG_DigitAccessoryOff(ACC_I2C_COOLER);    
         
      if(DigitAccessoryOn(ACC_I2C_DXD))
         TAG_DigitAccessoryOff(ACC_I2C_DXD);   
      
	  if ( info_SSR > 0)
	  write_byte_eeprom(ADDR_EEP(Set_EHD_mod), 0);	  

     return 1;    
  }    

  //------- PREHEATER  ------- 
  if((sData.status_test  & MSK_TEST_IPEHD) == 0) {
    if(DigitAccessoryOperating(ACC_I2C_PREHEATER) && (enab_func & (1 << ENAB_PREHEATING)) ) 
    {    

      if(sData.AccI2C[ACC_I2C_PREHEATER].sts & STS_ACC_ELECT)  // se l'accessorio � elettrico non mi preoccupo della formazione di ghiaccio sullo stesso
      {
        int ton_preheater = 120;// 5 sec. * 36 = 180 sec.; 
        int toff_preheater = 36;// 5 sec. * 36 = 180 sec.; 
        
        if( (sData.measure_Temp[I_PROBE_EXHAUST] < 30) && (count_preHeaterOn == 0) ) // se si forma condensa pu� ghiacciare, accendo il preheater per 3 min e tengo acceso se T_EX non � > 3�C
        {
           TAG_DigitAccessoryOn(ACC_I2C_PREHEATER); //accendo l'accessorio
           count_preHeaterOn = ton_preheater; //carico il timer (ON)
        }else if ((sData.measure_Temp[I_PROBE_EXHAUST] > 30) && (count_preHeaterOn == 0)) // se si � scaldato abbastanza spengo e inserisco una pausa di 3 min prima del prossimo controllo
        {
           TAG_DigitAccessoryOff(ACC_I2C_PREHEATER);
           count_preHeaterOn = toff_preheater; //carico il timer (OFF)
        }
         if(count_preHeaterOn > 0)  //decremento il timer fino a 0
         count_preHeaterOn--; 
        
      }
     else
        // ---------- 
        // se l'accessorio � ad acqua e la temperatura  dell'aria � bassa si congela la batteria !!!! Mantenere sempre acceso sotto i 5 gradi !!!!
        // ----------       
     {
       if(sData.AccI2C[ACC_I2C_PREHEATER].measure1 <= 50) 
       { 
          TAG_DigitAccessoryOn(ACC_I2C_PREHEATER); 
       }else if (sData.AccI2C[ACC_I2C_PREHEATER].measure1 > 70) //sopra i 7 posso spegnere
         {
            TAG_DigitAccessoryOff(ACC_I2C_PREHEATER);
         }       
     } 

     
// ---------- 
// Qui il vecchio codice:
// Non torna...se l'accessorio � ad acqua e la temperatura 
// dell'aria � bassa si congela la batteria !!!! Mantenere sempre acceso !!!!
// 
// ---------- 
//        
//           int ton_preheater = 120;// 5 sec. * 120 = 600 sec.; 
//           
//           // se la temperatura � sotto i -15�C portiamo il tempo di on a 15 min.
//           if(sData.AccI2C[ACC_I2C_PREHEATER].measure1 <= -150) 
//              ton_preheater = 180;
//              
//              
//           if(count_preHeaterOn < ton_preheater) {   
//             TAG_DigitAccessoryOn(ACC_I2C_PREHEATER);   
//           }else if((count_preHeaterOn >= ton_preheater) && (count_preHeaterOn < (ton_preheater + 240))) { // 240
//             TAG_DigitAccessoryOff(ACC_I2C_PREHEATER);  // 20  minuti OFF
//           }else if (count_preHeaterOn > (ton_preheater + 240))
//             count_preHeaterOn = 0;
//           
//           count_preHeaterOn++;
//       }else {
//           if ((sData.AccI2C[ACC_I2C_PREHEATER].measure1 > 70) || (count_preHeaterOn == 0)) { 
//             TAG_DigitAccessoryOff(ACC_I2C_PREHEATER);
//             count_preHeaterOn = 0;
//           }  
//             
//           if(count_preHeaterOn)  
//             count_preHeaterOn--; 
//           }
// 
         
       
    }else {
       //Se il preheater era Acceso spegnilo 
       if(DigitAccessoryOn(ACC_I2C_PREHEATER) == 1) {
          TAG_DigitAccessoryOff(ACC_I2C_PREHEATER);    
          count_preHeaterOn = 0;
       }     
    }  
  } 

  i_rifTemp    = read_byte_eeprom(ADDR_EEP(idxSetPointT)) & 0x01;
  SetPointTemp = read_word_eeprom(ADDR_EEP(SetPointTemp[i_rifTemp])); 
  SetPointTemp1 = read_word_eeprom(ADDR_EEP(SetPointTemp_Heater));  
  
  // <Ref_T_setting>: ..., bit[5,4,3,2]: 0..15, speed_regulator  
  //                       bit[1]: 1=TempRif=TR / 0=TempRif=TS
  //                       bit[0]: * * *
  ref_T_setting = read_byte_eeprom(ADDR_EEP(Ref_T_setting));
  if((ref_T_setting & 0x02) == 0) {
     // Riferimento su TS: preferisci AWP o accessori clima, altrimenti fallback su TSupply
     if(DigitAccessoryOperating(ACC_I2C_AWP)) {
        TempRif = sData.AccI2C[ACC_I2C_AWP].measure2;
     } else if (DigitAccessoryOperating(ACC_I2C_COOLER) && !DigitAccessoryOperating(ACC_I2C_HEATER)) {  // solo COOLER
        TempRif = sData.AccI2C[ACC_I2C_COOLER].measure1;
     } else if (DigitAccessoryOperating(ACC_I2C_HEATER) || !DigitAccessoryOperating(ACC_I2C_COOLER)) {  // solo HEATER
        TempRif = sData.AccI2C[ACC_I2C_HEATER].measure1;
     } else if (DigitAccessoryOperating(ACC_I2C_HEATER) || DigitAccessoryOperating(ACC_I2C_COOLER)) {   // entrambi
        TempRif = sData.AccI2C[ACC_I2C_HEATER].measure1;
     } else {
        TempRif = sData.measure_Temp[I_PROBE_SUPPLY];
     }
  } else {
     TempRif = sData.measure_Temp[I_PROBE_RETURN];
  }
  
  //_/\_/\_/\_/\_/\_/\_/\_/\_/\_/\_/\_/\_/\_/\_/\_/\_/\_/\_/\_/\_/\_/\_/\_/\_/\_/\_/\_/\_/\_/\_/\_/\_/\_/\
  //                        ------  ACCESSORIO  DXD -----
  //
  // Se � inserito, anche se non funzionante l'accessorio DXD, non prendere in considerazione gli altri
  // accesori clima.
  //_/\_/\_/\_/\_/\_/\_/\_/\_/\_/\_/\_/\_/\_/\_/\_/\_/\_/\_/\_/\_/\_/\_/\_/\_/\_/\_/\_/\_/\_/\_/\_/\_/\_/\ 
 
  if(AccessoryPresent_EepList(ACC_EEP_DXD)) 
  {
     int  time_regulator;     
     
     if(enab_func & (1 << ENAB_SUMMER))
       sData.RegDXD.func_compressor = 'S'; // raffredda
     else   
       sData.RegDXD.func_compressor = 'W'; // riscalda
       
     if(sData.RegDXD.func_compressor != sData.AccI2C[ACC_I2C_DXD].info) {
        /*---------------------------------------x_dbg
                 Serial_println1("Change Func.!"); 
         *---------------------------------------*/
        sData.RegDXD.processor_I2C |= CHANGE_FUNC_COMPRESS;
        return 1;
     }  
       
     if (((enab_func & (1 << ENAB_SUMMER)) == 0) && ((enab_func & (1 << ENAB_WINTER)) == 0))
     {
        if(DigitAccessoryOn(ACC_I2C_DXD)) 
           TAG_DigitAccessoryOff(ACC_I2C_DXD); 
           
       /*---------------------------------------x_dbg
        Serial_println1("Disable Acc.!"); 
        *---------------------------------------*/
 
        sData.RegDXD.processor_I2C = 0;
        sData.RegDXD.val_0_10V     = 0;
        sData.RegDXD.count         = -6; 
        return 1;
     }   
     
     
     // la funzione � richiamata ogni 5 sec., si pu� configurare il tempo di reazione del regolatore
     // partendo da un tempo minimo di 30 sec. fino ad un tempo max= 180 sec. 
     // <set_regulate_Clima>: ..., bit[5,4,3,2]: 0..15, speed_regulator  
     //                                bit[1]: 1=TempRif=TR / 0=TempRif=TS
     //                                bit[0]: * * *
     time_regulator = ((ref_T_setting >> 2) * 2); 
     time_regulator += 6; 
      
     if((ref_T_setting & 0x02) == 0)
     {
        Byte hist_T; 
               
        // isteresi da applicare alla temperatura in uscita al compressore se il controllo � sulla sonda supply 
        if(sData.RegDXD.func_compressor == 'W') {
          hist_T = read_byte_eeprom(ADDR_EEP(DeltaTemp_Supply)); // default 120 = 12.0
          SetPointTemp += hist_T; // es: 22.0 + 12.0 => 34.0 � la temperature in uscita per scadare l'ambiente e portare la temperatura interna a 22.0 
        }else {
          hist_T = read_byte_eeprom(ADDR_EEP(DeltaTemp_Supply)); // default 120 = 12.0
          
          if(SetPointTemp > hist_T)
            SetPointTemp -= hist_T; // es: 22.0 - 12.0 => 10.0 � la temperature in uscita per raffreddare l'ambiente e portare la temperatura interna a 22.0 
          else
            SetPointTemp = 0;  
        }  
     }  


//---------------------------------------x_dbg
/*
     Serial_println1(" ");
     if(sData.RegDXD.func_compressor == 'W')
       Serial_println1("Func: Winter");
     else if (sData.RegDXD.func_compressor == 'S')
       Serial_println1("Func: Summer");
     else  
       Serial_println1("Func: ? ? ?");  
      
     Serial_print1("TRif: ");
     Serial_print(TempRif, DEC); 
     Serial_print1("  SetT: ");
     Serial_print(SetPointTemp, DEC);    
     Serial_print1("  Time: ");
     Serial_println(sData.RegDXD.count, DEC); 
*/     
//---------------------------------------x_dbg

     // Se il compressore � in allarme, spegnaimoo subito
     if(CkAlarm(ALM_DXD_COMPRESS)) {
//---------------------------------------x_dbg       
//        Serial_println1(" *** Allarme *** ");
//---------------------------------------x_dbg
        if(DigitAccessoryOn(ACC_I2C_DXD) && (sData.RegDXD.count != -30)) {   // intile tenere acceso il compressore
//---------------------------------------x_dbg       
//           Serial_println1(" Spegni il compressore.");
//---------------------------------------x_dbg        
           TAG_DigitAccessoryOff(ACC_I2C_DXD); 
           sData.RegDXD.count = -30;
           sData.RegDXD.val_0_10V = 0;
           sData.RegDXD.processor_I2C |= CHANGE_VAL_0_10V;
        }   
        
        return 1; 
     }  
         
      
     if(++sData.RegDXD.count > time_regulator)
     {
       sData.RegDXD.count  = 0;
       if (sData.RegDXD.func_compressor == 'W'){
          // * * * * * * * * * * * * * * * * *
          // * * * * *  RISCALDMENTO * * * * * 
          // La sonda supply � messa dopo il compressore, se l'aria in immissione ambiente � maggiore di 19 gradi della soglia, 
          // non aumentare la potenza del compressore            
          if ((TempRif <= (SetPointTemp - 8)) && (sData.measure_Temp[I_PROBE_SUPPLY] <= (SetPointTemp + 190))) 
          {
              if(DigitAccessoryOn(ACC_I2C_DXD) == 0) 
              {  // prima accensione:      
                 TAG_DigitAccessoryOn(ACC_I2C_DXD);  
                 sData.RegDXD.val_0_10V =  60;  // si parte con il compressore al 60%
                 sData.RegDXD.count     = -24;  // Diamo +2 minuti in pi� (24 x 5sec. = 120 sec), in modo da dare il tempo al compressore di arrivare a regime
              }else
                 sData.RegDXD.val_0_10V += STEP_REG_DXD;
              
              if(sData.RegDXD.val_0_10V > 100)
                 sData.RegDXD.val_0_10V = 100; 
                
              sData.RegDXD.processor_I2C |= CHANGE_VAL_0_10V;
          }else if (TempRif > (SetPointTemp + 25)) {  
              // Troppo caldo, intile tenere acceso il compressore
              sData.RegDXD.val_0_10V = 0;
              if(DigitAccessoryOn(ACC_I2C_DXD)) 
              {   
                  TAG_DigitAccessoryOff(ACC_I2C_DXD); 
                  sData.RegDXD.count = -24;
                  sData.RegDXD.processor_I2C |= CHANGE_VAL_0_10V;
              }   
          }else if ((TempRif >= (SetPointTemp + 8)) && sData.RegDXD.val_0_10V) { 
              if(sData.RegDXD.val_0_10V > STEP_REG_DXD)
                sData.RegDXD.val_0_10V -= STEP_REG_DXD;
              else {
                sData.RegDXD.val_0_10V = 0;
                if(DigitAccessoryOn(ACC_I2C_DXD)) {   // intile tenere acceso il compressore
                   TAG_DigitAccessoryOff(ACC_I2C_DXD); 
                   sData.RegDXD.count = -24;
                }   
              }  
               
              sData.RegDXD.processor_I2C |= CHANGE_VAL_0_10V;
          }   

//---------------------------------------x_dbg
//          Serial_print1("Riscal., val_reg:  ");
//          Serial_println(sData.RegDXD.val_0_10V, DEC);
//---------------------------------------x_dbg
          
       }else if (sData.RegDXD.func_compressor == 'S') { 
          // * * * * * * * * * * * * * * * * * *
          // * * * * *  RAFFREDDAMENTO * * * * * 
          // La sonda supply � messa dopo il compressore, se l'aria in immissione ambiente � minore di 19 gradi della soglia, 
          // non aumentare la potenza del compressore  
          if ((TempRif >= (SetPointTemp + 8)) && (sData.measure_Temp[I_PROBE_SUPPLY] <= (SetPointTemp + 190))) 
          {                
            if(DigitAccessoryOn(ACC_I2C_DXD) == 0) {
              // prima accensione:   
              TAG_DigitAccessoryOn(ACC_I2C_DXD);  
              sData.RegDXD.val_0_10V =   60;  // si parte con il compressore al 60%
              sData.RegDXD.count     =  -24;  // Diamo +2 minuti in pi�, in modo da dare il tempo al compressore di arrivare a regime
            }else
              sData.RegDXD.val_0_10V += STEP_REG_DXD;
              
            if(sData.RegDXD.val_0_10V > 100)
              sData.RegDXD.val_0_10V = 100;  
              
            sData.RegDXD.processor_I2C |= CHANGE_VAL_0_10V;
            
          }else if (TempRif < (SetPointTemp - 25)) {  
              // Troppo caldo, intile tenere acceso il compressore
              sData.RegDXD.val_0_10V = 0;
              if(DigitAccessoryOn(ACC_I2C_DXD)) 
              {   
                  TAG_DigitAccessoryOff(ACC_I2C_DXD); 
                  sData.RegDXD.count = -24;
                  sData.RegDXD.processor_I2C |= CHANGE_VAL_0_10V;
              }   
          }else if ((TempRif <= (SetPointTemp - 8)) && sData.RegDXD.val_0_10V) { 
            if(sData.RegDXD.val_0_10V > STEP_REG_DXD)
              sData.RegDXD.val_0_10V -= STEP_REG_DXD;
            else {
              sData.RegDXD.val_0_10V = 0;
              if(DigitAccessoryOn(ACC_I2C_DXD)) {   // intile tenere acceso il compressore
                 TAG_DigitAccessoryOff(ACC_I2C_DXD); 
                 sData.RegDXD.count = -24;
              }   
            }  
            
            sData.RegDXD.processor_I2C |= CHANGE_VAL_0_10V;
          }
//---------------------------------------x_dbg
//          Serial_print1("Raffred., val_reg:  ");
//          Serial_println(sData.RegDXD.val_0_10V, DEC);
//---------------------------------------x_dbg
          
       }         
     }
       
     return 1;
  }else {
    sData.RegDXD.processor_I2C = 0;
    sData.RegDXD.val_0_10V    = 0;
    sData.RegDXD.count        = 0; 
  }  
  //_/\_/\_/\_/\_/\_/\_/\_/\_/\_/\_/\_/\_/\_/\_/\_/\_/\_/\_/\_/\_/\_/\_/\_/\_/\_/\_/\_/\_/\_/\_/\_/\_/\_/\   
   
  
   
  //---- (5) se � disabilitata la funzione CLIMA o non ho accessori Heater o Cooler. ---- 
  if (!DigitAccessoryOperating(ACC_I2C_HEATER) && !DigitAccessoryOperating(ACC_I2C_COOLER) && !AccessoryPresent_EepList(ACC_EEP_SSR))
  {
    // spengo gli accesori se attivi
    TAG_DigitAccessoryOff(ACC_I2C_COOLER);
    TAG_DigitAccessoryOff(ACC_I2C_HEATER);
	if ( info_SSR > 0)
	write_byte_eeprom(ADDR_EEP(Set_EHD_mod), 0);	
    return 1;
  }  
    
  /*------------------------------------------ (6) -------------------------------------------------------------------------------
   *   6.1) AWP presente (LOOP CHIUSO): prendo in considerazione le isteresi + ristrette, e la misura della temperatura quella AWP
   *   6.2) No AWP       (LOOP APERTO): prendo in considerazione le isteresi + alte e, le misure delle temp. a bordo degli Acc.  
   *
   * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   *                                EXAMPLE CLIMA
   *    C  ^
   *  ..   |   
   *  30   +
   *  28   + - - - - - - - - - - - - - - - x * - - - - - - COLDER_ON  = (SetPointTemp + hister_on)
   *  26   + - - - - - - - - - - - - x * * : - * - - - - - HEATER_OFF = (SetPointTemp + hister_off)
   *  24   +                     * * :     :     *
   *  22   + * - - - - - - - - * - - : - - : - - - x * - - SetPointTemp 
   *  20   +   * *         * *       :     :       :       & COLDER_OFF  = (SetPointTemp - hister_off) = SetPointTemp
   *  18   + - - - x * - * - - - - - : - - : - - - : - - - HEATER_ON  = (SetPointTemp - hister_on)
   *  16   +       :   *             :     :       :
   *  ..   |       :                 :     :       :
   *       +-------:-----------------:-----:-------:--------->
   *           EHD_ON           EHD_OFF CWD_ON   CWD_OFF     tempo
   *
   *  NB: Prima di attivare il COOLER verifica se il bypass puo darmi una mano (risparmio energetico).
   *
   * -------------------------------------------------------------------------------------------------------------------------------*/  
  
   Value1 = read_byte_eeprom(ADDR_EEP(hister_Temp_Hot[0]));
   Value2 = read_byte_eeprom(ADDR_EEP(hister_Temp_Hot[1]));
   
   soglia_Off_Heater = SetPointTemp1 + (signed char)read_byte_eeprom(ADDR_EEP(hister_Temp_Hot[0]));
   soglia_On_Heater  = SetPointTemp1 + (signed char)read_byte_eeprom(ADDR_EEP(hister_Temp_Hot[1]));
   soglia_Off_Cooler = SetPointTemp + (signed char)read_byte_eeprom(ADDR_EEP(hister_Temp_Cold[0]));
   soglia_On_Cooler  = SetPointTemp + (signed char)read_byte_eeprom(ADDR_EEP(hister_Temp_Cold[1])); 
 
 //------------------------------------ x_dbg    
/* 
    Serial_println1(" ");
    Serial_print1("T.rif: ");
    Serial_print(((float)TempRif)/10, 1);
*/    
//------------------------------------ x_dbg          
       
    
   // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
   //                                 WINTER   
   // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
   if((enab_func & (1 << ENAB_WINTER)) && !(enab_func & (1 << ENAB_MBF))) 
   {
      TAG_DigitAccessoryOff(ACC_I2C_COOLER); 
      
      if(DigitAccessoryOperating(ACC_I2C_HEATER) || AccessoryPresent_EepList(ACC_EEP_SSR) )
      {      
         if (TempRif < soglia_On_Heater) 
		 {
            TAG_DigitAccessoryOn(ACC_I2C_HEATER);
			write_byte_eeprom(ADDR_EEP(Set_EHD_mod), 100);
		 }
         else if (TempRif >= soglia_Off_Heater) 
         {
            // Diamo qualche secondo in pi� prima di spegnere
            if(++persist_Temp_heater_off > 1) { 
               TAG_DigitAccessoryOff(ACC_I2C_HEATER); 
			   write_byte_eeprom(ADDR_EEP(Set_EHD_mod), 0);
               persist_Temp_heater_off = 0; 
            }    
         }else
			 { 
				persist_Temp_heater_off = 0;
				if ( !DigitAccessoryOn(ACC_I2C_HEATER))
					sData.AccI2C[ACC_I2C_HEATER].status_comand &= ~ CMD_ON;
			 }
         
      }else {
         //Se HEATER era Acceso spegnilo 
         if (DigitAccessoryOn(ACC_I2C_HEATER) == 1 || (info_SSR > 0) )
		 {
            TAG_DigitAccessoryOff(ACC_I2C_HEATER); 
			
			if ( info_SSR > 0)
	        write_byte_eeprom(ADDR_EEP(Set_EHD_mod), 0);
		 }
        
         persist_Temp_heater_off = 0; 
      } 
//------------------------------------ x_dbg    
/*
      Serial_print1("  Sgl.Winter, ON:");
      Serial_print(((float)soglia_On_Heater)/10, 1); 
      Serial_print1("  OFF:");
      Serial_println(((float)soglia_Off_Heater)/10, 1); 
      
      if(DigitAccessoryOperating(ACC_I2C_HEATER)) {
        if(DigitAccessoryOn(ACC_I2C_HEATER)) 
          Serial_println1("  Heater ON");
        else  
          Serial_println1("  Heater OFF");
      }else  
         Serial_println1("  Heater Not Op.");
*/         
//------------------------------------ x_dbg            
      
    }else if((enab_func & (1 << ENAB_SUMMER)) && !(enab_func & (1 << ENAB_MBF))) {   
	  // spegni gli accessori di riscaldamento
      TAG_DigitAccessoryOff(ACC_I2C_HEATER);
       if (info_SSR > 0)
		  write_byte_eeprom(ADDR_EEP(Set_EHD_mod), 0); 
	  // 
      // o o o o o o o o o o o o o o o o o o o o o o o o o o o o o o o o o o o o o 
      //                                SUMMER   
      // o o o o o o o o o o o o o o o o o o o o o o o o o o o o o o o o o o o o o 
      if(DigitAccessoryOperating(ACC_I2C_COOLER))
      {         
        // Se la temperatura esterna � minore di quella interna di 3.0 Gradi lavoriamo prima con il Bypass.
        if((TempRif >= soglia_On_Cooler) && (sData.measure_Temp[I_PROBE_FRESH] > (sData.measure_Temp[I_PROBE_RETURN] - 40))) 
          TAG_DigitAccessoryOn(ACC_I2C_COOLER);
        else if ((TempRif < soglia_Off_Cooler) || (sData.measure_Temp[I_PROBE_FRESH] < (sData.measure_Temp[I_PROBE_RETURN] - 50))) 
          TAG_DigitAccessoryOff(ACC_I2C_COOLER);
      }else  {
        //Se la funzione � disable, spegnilo 
        if(DigitAccessoryOn(ACC_I2C_COOLER) == 1)
          TAG_DigitAccessoryOff(ACC_I2C_COOLER); 
      }
	}else if(!(enab_func & (1 << ENAB_WINTER)) && !(enab_func & (1 << ENAB_SUMMER)) && !(enab_func & (1 << ENAB_MBF)))
	  {
		    // spengo gli accesori se attivi
        TAG_DigitAccessoryOff(ACC_I2C_COOLER);
        TAG_DigitAccessoryOff(ACC_I2C_HEATER);
	    if ( info_SSR > 0)
    	write_byte_eeprom(ADDR_EEP(Set_EHD_mod), 0);	  
	  }
//------------------------------------ x_dbg    
/*
      Serial_print1("  Sgl.Summer, ON:");
      Serial_print(((float)soglia_On_Cooler)/10, 1); 
      Serial_print1("  OFF:");
      Serial_print(((float)soglia_Off_Cooler)/10, 1); 
      
     if(DigitAccessoryOperating(ACC_I2C_COOLER)) {
        if(DigitAccessoryOn(ACC_I2C_COOLER)) 
          Serial_println1("  Cooler ON");
        else  
          Serial_println1("  Cooler OFF");
      }else  
         Serial_println1("  Cooler Not Op.");
*/         
//------------------------------------ x_dbg          

   return 1;
   
}

int ThermoCheck()
{

   int tolT  = 300; // 3 gradi prima di segnalare il fuori scala
   int tolRH =  10; // 3 gradi prima di segnalare il fuori scala
  
   if(Tcheck == 0)  //inizializzo i valori da controllare
   {
   S2_T  = sData.AccI2C[ACC_I2C_EXT2].measure1+385; // S2 // T  SUPPLY
   S2_RH = sData.AccI2C[ACC_I2C_EXT2].measure2; // S2 // RH SUPPLY

   T3 = sData.AccI2C[ACC_I2C_SSR].measure3+90;  // T3 // TEMP. AFTER EHD
   
   T4 = Temp_modbus[I_PROBE_EXHAUST]+70; // T4 // TEMP. EXHAUST 
   T5 = Temp_modbus[I_PROBE_SUPPLY]+50;  // T5 // TEMP. AFTER DX COIL
   Tcheck = 1;
   }

   // tengo aggiornati questi valori ogni volta che gira la funzione
   S1_T  = sData.AccI2C[ACC_I2C_EXT1].measure1+295; // S1 // T  FRESH
   S1_RH = sData.AccI2C[ACC_I2C_EXT1].measure2; // S1 // RH FRESH
   S4_T  = sData.AccI2C[ACC_I2C_EXT4].measure1+285; // S4 // T  RETURN
   S4_RH = sData.AccI2C[ACC_I2C_EXT4].measure2; // S4 // RH RETURN 

   T1 = sData.AccI2C[ACC_I2C_SSR].measure1+60;  // T1 // TEMP. REF. INLET
   T2 = sData.AccI2C[ACC_I2C_SSR].measure2+60;  // T2 // TEMP. REF. OUTLET 
  
  

   // sola correzione

   S2_T  = sData.AccI2C[ACC_I2C_EXT2].measure1+385; // S2 // T  SUPPLY
   S2_RH = sData.AccI2C[ACC_I2C_EXT2].measure2; // S2 // RH SUPPLY

   T3 = sData.AccI2C[ACC_I2C_SSR].measure3+90;  // T3 // TEMP. AFTER EHD
   
   T4 = Temp_modbus[I_PROBE_EXHAUST]+70; // T4 // TEMP. EXHAUST 
   T5 = Temp_modbus[I_PROBE_SUPPLY]+50;  // T5 // TEMP. AFTER DX COIL
   
   
//   // inizio il controllo di coerenza
//   if ( S1_T < S4_T) // FRESCO FUORI vs CALDO DENTRO
//   {
//       
//      if ( (S2_T < (S1_T-tolT)) || (S2_T >= S1_T))  // S2 T non pu� essere pi� freddo di S1_T o fuori tolleranza 
//      {
//          S2_T  = sData.AccI2C[ACC_I2C_EXT2].measure1+290; 
//          
//          if ( (S2_RH < S1_RH)  || (S2_RH > (S1_RH + tolRH) ) )   //S2 RH non pu� essere maggiore o fuori tolleranza       
//          {
//           S2_RH = sData.AccI2C[ACC_I2C_EXT2].measure2; 
//          }
//          else //correzione coerente per la sola umidita'
//          {
//           S2_RH = S1_RH;
//          }        
//       }
//      else // correzione coerente di temperatura e umidita'
//      {
//          S2_T  = S1_T+5;
//          S2_RH = S1_RH;
//      }
//        
//    
//    if( (T5 < (S2_T-tolT)) || (T5 >= S2_T) )  // T5 non pu� essere pi� freddo di S2_T o fuori tolleranza
//      {
//          T5 = Temp_modbus[I_PROBE_SUPPLY]+50;  // T5 // TEMP. AFTER DX COIL
//      }
//    else
//    {
//       T5 = S2_T+5;
//    }
//      
//      
//    if( (T3 < (T5-tolT)) || (T3 >= T5) )  // T3 non pu� essere pi� freddo di T5 o fuori tolleranza
//      {
//         T3 = sData.AccI2C[ACC_I2C_SSR].measure3+80;  // T3 // TEMP. AFTER EHD
//      }
//    else
//    {
//       T3 = T5+5;
//    }
//     
//   
//   if( (T4 > (S4_T+tolT)) || (T4 <= S4_T) )  // T4 non pu� essere pi� caldo di S4_T o fuori tolleranza
//      {
//      T4 = Temp_modbus[I_PROBE_EXHAUST]; // T4 // TEMP. EXHAUST  
//      }
//    else
//    {
//      T4 = S4_T-5;
//    }
//
//   if (info_EBPD == 0) // se il bypass � chiuso
//   {
//     if ((S2_T > S4_T)  ) // S2_T non pu� essere superiore a S4_T
//     {
//      S2_T = S4_T-10;
//     }    
//     if ((T4 > S2_T)  )  // T4 non pu� essere superiore a S2_T
//     {
//       T4 = S2_T-10;
//     }
//   }
//   
//
//
//   }
//   else // FRESCO DENTRO vs CALDO FUORI
//   {
//      if ( (S2_T > (S1_T+tolT)) || (S2_T <= S1_T))  // S2 T non pu� essere pi� caldo di S1_T o fuori tolleranza 
//      {
//          S2_T  = sData.AccI2C[ACC_I2C_EXT2].measure1+290; 
//          
//          if ( (S2_RH > S1_RH)  || (S2_RH < (S1_RH - tolRH) ) )   //S2 RH non pu� essere minore o fuori tolleranza       
//          {
//           S2_RH = sData.AccI2C[ACC_I2C_EXT2].measure2; 
//          }
//          else //correzione coerente per la sola umidita'
//          {
//           S2_RH = S1_RH;
//          }        
//       }
//      else // correzione coerente di temperatura e umidita'
//      {
//          S2_T  = S1_T-5;
//          S2_RH = S1_RH;
//      }
//        
//    
//    if( (T5 > (S2_T+tolT)) || (T5 <= S2_T) )  // T5 non pu� essere pi� caldo di S2_T o fuori tolleranza
//      {
//          T5 = Temp_modbus[I_PROBE_SUPPLY]+50;  // T5 // TEMP. AFTER DX COIL
//      }
//    else
//    {
//       T5 = S2_T-5;
//    }
//      
//      
//    if( (T3 > (T5+tolT)) || (T3 <= T5) )  // T3 non pu� essere pi� caldo di T5 o fuori tolleranza
//      {
//         T3 = sData.AccI2C[ACC_I2C_SSR].measure3+80;  // T3 // TEMP. AFTER EHD
//      }
//    else
//    {
//       T3 = T5-5;
//    }
//     
//   
//   if( (T4 < (S4_T-tolT)) || (T4 >= S4_T) )  // T4 non pu� essere pi� freddo di S4_T o fuori tolleranza
//      {
//      T4 = Temp_modbus[I_PROBE_EXHAUST]; // T4 // TEMP. EXHAUST  
//      }
//    else
//    {
//      T4 = S4_T+5;
//    }
//   }
//      
//   if (info_EBPD == 0) // se il bypass � chiuso
//   {
//     if ((S2_T < S4_T)  ) // S2_T non pu� essere minore a S4_T
//     {
//      S2_T = S4_T+10;
//     }    
//     if ((T4 < S2_T)  )  // T4 non pu� essere minore a S2_T
//     {
//       T4 = S2_T+10;
//     }
//   }    
// 
   return 1;  
}
