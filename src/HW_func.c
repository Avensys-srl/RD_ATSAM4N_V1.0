/*
 * HW_func.cpp
 *
 *  Descrizione: Nel seguente file vengono inserite tutte le funzioni legate ad operazioni
 *               HW pilotate dal modulo RD2. Esempio Pilota Buzzer o Bypass...
 */ 

#include "asf.h"
#include "common/build_config.h"

#include "data.h"
#include "Scheduler.h"
#include "eeprom_map.h"
#include "alarm.h"
#include "Protocol_Ser1.h"

// Period needed for timer 1 to trigger an interrupt every 5ms
#define PERIOD_T4                     1563    // Interrupt timer 4 : (256 *PERIOD_T4) /80MHz =  5,002ms

#define CUNSUMER_MIN_KTS                15    // 15
#define TIMER_KTS_KO                    40    // Numero di cicli: 60 secondi di attesa prima di riavviare il KTS (1.5 sec./ciclo)
#define FULLTIME                       240    // Numero di cicli: 10 min di attesa prima di invertire il comando SUMMER/WINTER (2.5 sec./ciclo) = 240 cicli

//--------Soglie di Fine corsa Bypass --------
#define MIN_STEP_CTRL_CURRENT_BPD     500    // 500 Numero di step minimo, prima di controllare l'assorbimento del motore bypass 
#define MAX_STEP_FINE_CORSA           10000   // 1500 Numero massimo di step del motore, circa 140 DG dopo di che fermiamo il motore.
#define SOGLIA_MAX_ASSORB_BYPASS_FC    845   // 845 = 230mA Assorb. massimo motorino Bypass quando incontra un FermoCorsa con tensione 11,8V. ( 1023 -> 275ma) 1690
#define SOGLIA_MIN_ASSORB_BYPASS_FC    123    // 123 => 33mA Assorb. minimo  motorino Bypass quando incontra un FermoCorsa con tensione 11,8V.
// queste 2 soglie generano l'allarme di bypass guasto 
#define SOGLIA_MIN_ASSORB_BYPASS_KO     95     //  95 => 26mA
#define SOGLIA_MAX_ASSORB_BYPASS_KO    900     // 900 = 245mA errore assorbimento motorino Bypass alto

//------------------ Stati BYPASS ---------------------
#define ANTIORARIO       0xFF
#define ORARIO           0
#define CLOSE_BPD        1   // bypass chiuso, il flusso dell'aria passa dallo scambiatore
#define OPEN_BPD         0   // bypass aperto 100%, il flusso dell'aria passa solo dal canale Bypass.

// stato del Rele' e delle tensione dell'elettronica
#define POWER_SAVE     0
#define POWER_ON      'O'

//------------------ GLOBAL VARS -------------------------------------
//---- Vars Buzzer ---- 
Byte numBeep      = 0;
Byte StatusBuzzer = 0;
//---- Vars Bypass ----
Byte RotaBypass = ANTIORARIO;
Byte PositionBypass = OPEN_BPD; 
unsigned int  numStepBypass    = 0;
short count_win = FULLTIME;
short count_sum = FULLTIME;
Bool output_on = 0;
short soft_start=6;

volatile uint32_t	TC0_Status_Register = 0;

Bool Buzzer_Off = false;

//-----------  stato rele' unita ---------------------
extern Byte PowerMotors;

extern SDATA  sData;

extern int  cnt_sec_no_link_KTS;
extern Byte connect_KTS;

extern Byte info_EBPD; // definisco un valore iniziale fuori dal range in modo tale da obbligare la sincronizzazione sin dal primo ciclo.

float BPD_Current = 0;

extern Byte AccessoryPresent_EepList(int nAccEep);
extern unsigned analogRead ( Byte Channel_Num );
extern float calcDepot();
extern Byte eep_read_byte(unsigned int addr);
void eep_write_byte(unsigned int addr, Byte data_to_send);

//-----------------------------------------------------
// Func: PowerMode
//
// Stato input:
//  sts = POWER_SAVE 
//  sts = POWER_ON
//-----------------------------------------------------
void PowerMode(Byte sts)
{
   if(sts == POWER_SAVE) {
     PowerMotors = POWER_SAVE;
     //digitalWrite(pPowerMotors, HIGH); // Diseccita il rele'
	 ioport_set_pin_level(pPowerMotors, IOPORT_PIN_LEVEL_HIGH);    
   if(read_byte_eeprom(ADDR_EEP(SerialString[7])) == '6') // Sono una unit� della serie 6 il rotore dello scambiatore va pilotato 
	  {
      ioport_set_pin_level(pIPEHD, IOPORT_PIN_LEVEL_LOW);  
    }     
   }else {

     PowerMotors = POWER_ON;
     //digitalWrite(pPowerMotors, LOW); // eccita il rele' 
	 ioport_set_pin_level(pPowerMotors, IOPORT_PIN_LEVEL_LOW);
   if(read_byte_eeprom(ADDR_EEP(SerialString[7])) == '6') // Sono una unit� della serie 6 il rotore dello scambiatore va pilotato 
	  {
        ioport_set_pin_level(pIPEHD, IOPORT_PIN_LEVEL_HIGH);   
    }
   }  
}


//#ifdef __cplusplus
//extern "C" {
//#endif

//--------------------------------------------------------------------
// Function:  startMotorBPD
//
// Description:
//  Attiva il timer 0 per lo step motors
//--------------------------------------------------------------------
int startMotorBPD()
{
	uint32_t ul_div = 0;
	uint32_t ul_tc_clks = 0;
	uint32_t ul_sysclk = sysclk_get_cpu_hz();
	
   //---  Inizializze timer TC0 : Interrupt : 5ms ---
   // Period matches: 5ms
   
   pmc_enable_periph_clk(ID_TC0);
   
   //tc_find_mck_divisor( 200, ul_sysclk , &ul_div, &ul_tc_clks, ul_sysclk);
   
   tc_init(TC0,0,TC_CMR_CPCTRG | TC_CMR_TCCLKS_TIMER_CLOCK4);
   
   tc_write_rc(TC0,0,3906);
   //TC0->TC_CHANNEL[0].TC_RC = ( ul_sysclk / ul_div ) / 1;
   
   NVIC_EnableIRQ((IRQn_Type) ID_TC0);
   
   tc_enable_interrupt(TC0,0,TC_IER_CPCS);
   
   tc_start(TC0, 0); // Start TC0 
   
   return 0; 
}  


void TC0_Handler ( void )
{
	 int step_motor; 
	 
	TC0_Status_Register = tc_get_status(TC0, 0);
	//ioport_toggle_pin_level(Pin_Test);

    numStepBypass++;
    step_motor = (numStepBypass % 8);

   if (numStepBypass >= MAX_STEP_FINE_CORSA) 
   {                                       
       //CloseTimer4();
	   tc_stop(TC0, 0); // Start TC0 
	   tc_disable_interrupt(TC0,0,TC_IER_CPCS);
       //digitalWrite(motor_pin_1, LOW);
       //digitalWrite(motor_pin_2, LOW);
       //digitalWrite(motor_pin_3, LOW);
       //digitalWrite(motor_pin_4, LOW);
	   ioport_set_pin_level(motor_pin_1, IOPORT_PIN_LEVEL_LOW);
	   ioport_set_pin_level(motor_pin_2, IOPORT_PIN_LEVEL_LOW);
	   ioport_set_pin_level(motor_pin_3, IOPORT_PIN_LEVEL_LOW);
	   ioport_set_pin_level(motor_pin_4, IOPORT_PIN_LEVEL_LOW);        
       return;   
    }              
    
    
    if(RotaBypass == ANTIORARIO)  
    {
       // ROTAZIONE ANTIORARIA Motorino Bypass
       switch (step_motor) {
          case 0:
            //digitalWrite(motor_pin_1, HIGH);
            //digitalWrite(motor_pin_2, LOW);
            //digitalWrite(motor_pin_3, LOW);
            //digitalWrite(motor_pin_4, LOW);
			ioport_set_pin_level(motor_pin_1, IOPORT_PIN_LEVEL_HIGH);
			ioport_set_pin_level(motor_pin_2, IOPORT_PIN_LEVEL_LOW);
			ioport_set_pin_level(motor_pin_3, IOPORT_PIN_LEVEL_LOW);
			ioport_set_pin_level(motor_pin_4, IOPORT_PIN_LEVEL_LOW);       
          break;
          
          case 1:
            //digitalWrite(motor_pin_1, HIGH);
            //digitalWrite(motor_pin_2, HIGH);
            //digitalWrite(motor_pin_3, LOW);
            //digitalWrite(motor_pin_4, LOW);
			ioport_set_pin_level(motor_pin_1, IOPORT_PIN_LEVEL_HIGH);
			ioport_set_pin_level(motor_pin_2, IOPORT_PIN_LEVEL_HIGH);
			ioport_set_pin_level(motor_pin_3, IOPORT_PIN_LEVEL_LOW);
			ioport_set_pin_level(motor_pin_4, IOPORT_PIN_LEVEL_LOW);
          break;
          
          case 2:
            //digitalWrite(motor_pin_1, LOW);
            //digitalWrite(motor_pin_2, HIGH);
            //digitalWrite(motor_pin_3, LOW);
            //digitalWrite(motor_pin_4, LOW);
			ioport_set_pin_level(motor_pin_1, IOPORT_PIN_LEVEL_LOW);
			ioport_set_pin_level(motor_pin_2, IOPORT_PIN_LEVEL_HIGH);
			ioport_set_pin_level(motor_pin_3, IOPORT_PIN_LEVEL_LOW);
			ioport_set_pin_level(motor_pin_4, IOPORT_PIN_LEVEL_LOW);
          break; 
          
          case 3:
            //digitalWrite(motor_pin_1, LOW);
            //digitalWrite(motor_pin_2, HIGH);
            //digitalWrite(motor_pin_3, HIGH);
            //digitalWrite(motor_pin_4, LOW);
			ioport_set_pin_level(motor_pin_1, IOPORT_PIN_LEVEL_LOW);
			ioport_set_pin_level(motor_pin_2, IOPORT_PIN_LEVEL_HIGH);
			ioport_set_pin_level(motor_pin_3, IOPORT_PIN_LEVEL_HIGH);
			ioport_set_pin_level(motor_pin_4, IOPORT_PIN_LEVEL_LOW);
          break;     
         
          case 4:
            //digitalWrite(motor_pin_1, LOW);
            //digitalWrite(motor_pin_2, LOW);
            //digitalWrite(motor_pin_3, HIGH);
            //digitalWrite(motor_pin_4, LOW);
			ioport_set_pin_level(motor_pin_1, IOPORT_PIN_LEVEL_LOW);
			ioport_set_pin_level(motor_pin_2, IOPORT_PIN_LEVEL_LOW);
			ioport_set_pin_level(motor_pin_3, IOPORT_PIN_LEVEL_HIGH);
			ioport_set_pin_level(motor_pin_4, IOPORT_PIN_LEVEL_LOW);
          break;
         
          case 5:
            //digitalWrite(motor_pin_1, LOW);
            //digitalWrite(motor_pin_2, LOW);
            //digitalWrite(motor_pin_3, HIGH);
            //digitalWrite(motor_pin_4, HIGH);
			ioport_set_pin_level(motor_pin_1, IOPORT_PIN_LEVEL_LOW);
			ioport_set_pin_level(motor_pin_2, IOPORT_PIN_LEVEL_LOW);
			ioport_set_pin_level(motor_pin_3, IOPORT_PIN_LEVEL_HIGH);
			ioport_set_pin_level(motor_pin_4, IOPORT_PIN_LEVEL_HIGH);
          break;
         
          case 6:
            //digitalWrite(motor_pin_1, LOW);
            //digitalWrite(motor_pin_2, LOW);
            //digitalWrite(motor_pin_3, LOW);
            //digitalWrite(motor_pin_4, HIGH);
			ioport_set_pin_level(motor_pin_1, IOPORT_PIN_LEVEL_LOW);
			ioport_set_pin_level(motor_pin_2, IOPORT_PIN_LEVEL_LOW);
			ioport_set_pin_level(motor_pin_3, IOPORT_PIN_LEVEL_LOW);
			ioport_set_pin_level(motor_pin_4, IOPORT_PIN_LEVEL_HIGH);
          break; 
          
          case 7:
            //digitalWrite(motor_pin_1, HIGH);
            //digitalWrite(motor_pin_2, LOW);
            //digitalWrite(motor_pin_3, LOW);
            //digitalWrite(motor_pin_4, HIGH);
			ioport_set_pin_level(motor_pin_1, IOPORT_PIN_LEVEL_HIGH);
			ioport_set_pin_level(motor_pin_2, IOPORT_PIN_LEVEL_LOW);
			ioport_set_pin_level(motor_pin_3, IOPORT_PIN_LEVEL_LOW);
			ioport_set_pin_level(motor_pin_4, IOPORT_PIN_LEVEL_HIGH);
          break;         
       }      
    }else {
       // ROTAZIONE ORARIA motorino Bypass
       switch (step_motor) {
          case 0:
            //digitalWrite(motor_pin_1, HIGH);
            //digitalWrite(motor_pin_2, LOW);
            //digitalWrite(motor_pin_3, LOW);
            //digitalWrite(motor_pin_4, HIGH);
			ioport_set_pin_level(motor_pin_1, IOPORT_PIN_LEVEL_HIGH);
			ioport_set_pin_level(motor_pin_2, IOPORT_PIN_LEVEL_LOW);
			ioport_set_pin_level(motor_pin_3, IOPORT_PIN_LEVEL_LOW);
			ioport_set_pin_level(motor_pin_4, IOPORT_PIN_LEVEL_HIGH);
          break;
          
          case 1:
            //digitalWrite(motor_pin_1, LOW);
            //digitalWrite(motor_pin_2, LOW);
            //digitalWrite(motor_pin_3, LOW);
            //digitalWrite(motor_pin_4, HIGH);
			ioport_set_pin_level(motor_pin_1, IOPORT_PIN_LEVEL_LOW);
			ioport_set_pin_level(motor_pin_2, IOPORT_PIN_LEVEL_LOW);
			ioport_set_pin_level(motor_pin_3, IOPORT_PIN_LEVEL_LOW);
			ioport_set_pin_level(motor_pin_4, IOPORT_PIN_LEVEL_HIGH);
          break;
          
          case 2:
            //digitalWrite(motor_pin_1, LOW);
            //digitalWrite(motor_pin_2, LOW);
            //digitalWrite(motor_pin_3, HIGH);
            //digitalWrite(motor_pin_4, HIGH);
			ioport_set_pin_level(motor_pin_1, IOPORT_PIN_LEVEL_LOW);
			ioport_set_pin_level(motor_pin_2, IOPORT_PIN_LEVEL_LOW);
			ioport_set_pin_level(motor_pin_3, IOPORT_PIN_LEVEL_HIGH);
			ioport_set_pin_level(motor_pin_4, IOPORT_PIN_LEVEL_HIGH);
          break; 
          
          case 3:
            //digitalWrite(motor_pin_1, LOW);
            //digitalWrite(motor_pin_2, LOW);
            //digitalWrite(motor_pin_3, HIGH);
            //digitalWrite(motor_pin_4, LOW);
			ioport_set_pin_level(motor_pin_1, IOPORT_PIN_LEVEL_LOW);
			ioport_set_pin_level(motor_pin_2, IOPORT_PIN_LEVEL_LOW);
			ioport_set_pin_level(motor_pin_3, IOPORT_PIN_LEVEL_HIGH);
			ioport_set_pin_level(motor_pin_4, IOPORT_PIN_LEVEL_LOW);
          break;     
         
          case 4:
            //digitalWrite(motor_pin_1, LOW);
            //digitalWrite(motor_pin_2, HIGH);
            //digitalWrite(motor_pin_3, HIGH);
            //digitalWrite(motor_pin_4, LOW);
			ioport_set_pin_level(motor_pin_1, IOPORT_PIN_LEVEL_LOW);
			ioport_set_pin_level(motor_pin_2, IOPORT_PIN_LEVEL_HIGH);
			ioport_set_pin_level(motor_pin_3, IOPORT_PIN_LEVEL_HIGH);
			ioport_set_pin_level(motor_pin_4, IOPORT_PIN_LEVEL_LOW);
          break;
         
          case 5:
            //digitalWrite(motor_pin_1, LOW);
            //digitalWrite(motor_pin_2, HIGH);
            //digitalWrite(motor_pin_3, LOW);
            //digitalWrite(motor_pin_4, LOW);
			ioport_set_pin_level(motor_pin_1, IOPORT_PIN_LEVEL_LOW);
			ioport_set_pin_level(motor_pin_2, IOPORT_PIN_LEVEL_HIGH);
			ioport_set_pin_level(motor_pin_3, IOPORT_PIN_LEVEL_LOW);
			ioport_set_pin_level(motor_pin_4, IOPORT_PIN_LEVEL_LOW);
          break;
         
          case 6:
            //digitalWrite(motor_pin_1, HIGH);
            //digitalWrite(motor_pin_2, HIGH);
            //digitalWrite(motor_pin_3, LOW);
            //digitalWrite(motor_pin_4, LOW);
			ioport_set_pin_level(motor_pin_1, IOPORT_PIN_LEVEL_HIGH);
			ioport_set_pin_level(motor_pin_2, IOPORT_PIN_LEVEL_HIGH);
			ioport_set_pin_level(motor_pin_3, IOPORT_PIN_LEVEL_LOW);
			ioport_set_pin_level(motor_pin_4, IOPORT_PIN_LEVEL_LOW);
          break; 
          
          case 7:
            //digitalWrite(motor_pin_1, HIGH);
            //digitalWrite(motor_pin_2, LOW);
            //digitalWrite(motor_pin_3, LOW);
            //digitalWrite(motor_pin_4, LOW);
			ioport_set_pin_level(motor_pin_1, IOPORT_PIN_LEVEL_HIGH);
			ioport_set_pin_level(motor_pin_2, IOPORT_PIN_LEVEL_LOW);
			ioport_set_pin_level(motor_pin_3, IOPORT_PIN_LEVEL_LOW);
			ioport_set_pin_level(motor_pin_4, IOPORT_PIN_LEVEL_LOW);  
          break;                 
       }            
      
    }      
}

//---------------------------------------------------------------------
// Function:  BuzzerFunc
//
// Description:
//   Funzione che gira nello scheduler per N volte.
//---------------------------------------------------------------------
int BuzzerFunc () 
{
  if(StatusBuzzer) {
    //digitalWrite(pBuz, LOW);
	ioport_set_pin_level(pBuz, IOPORT_PIN_LEVEL_LOW);
    if(numBeep)
      numBeep--;
    StatusBuzzer = 0;  
  }else {
    if(numBeep) {
      //digitalWrite(pBuz,HIGH);
	  ioport_set_pin_level(pBuz, IOPORT_PIN_LEVEL_HIGH);
      StatusBuzzer = 1; 
    }  
  }  
  
  if ( numBeep == 0 )
		Buzzer_Off = false;
		
  return numBeep; 
}


//---------------------------------------------------------------------
// Function:  ActiveBuzzer
//
// Description:
//   Per evitare di generare ritardi con i delay(), il buzzer si appoggia 
//   al timer 5 dello scheduler.
//   Vengono pssati 2 parametri:
//     1: numBeepBeep -> numero di ripetizioni dei Beep ON e OFF
//     2: freq_100ms  -> Frequenza del segnale ON e OFF, step di 100ms. 
//                   
//---------------------------------------------------------------------
void ActiveBuzzer(Byte numBeepBeep, unsigned int freq_100ms)
{
    if((sData.status_test & MSK_TEST_ENABLE) && (sData.status_test & MSK_TEST_BUZZER) == 0)
      return;
      
    // Tieni attivo il cicalino per N*0,1 sec., poi disattivalo
    //digitalWrite(pBuz,HIGH);
	ioport_set_pin_level(pBuz, IOPORT_PIN_LEVEL_HIGH);       
    StatusBuzzer = 1;
    numBeep      = numBeepBeep;
    insert_function_scheduler(BuzzerFunc,  freq_100ms); 
    
    if (sData.status_test & MSK_TEST_ENABLE) 
      sData.status_test &= ~MSK_TEST_BUZZER;
}  


//---------------------------------------------------------------------
// Function:  Active_Procedure_Bypass_OpCl
//
// Description:
//   Attiva la procedura di chiusura o apertura Bypass. gli step sono 
//   cosi suddivisi:
//     1. inizializza il Timer4 e lo stato di BypassRUN 
//     2. Attendi 5 secondi senza effetuare nessuna operazione
//     3. Inizia a pilotare il motore Bypass (CLOSE/OPEN)
//     4. Dopo 7 secondi dall'inizializzazione controlla l'assorbimento
//        per verificare il fine corsa.
//     5. Raggiunto il fine corsa Cambia stato del BYPASS.
//---------------------------------------------------------------------
void Active_Procedure_Bypass_OpCl(Byte OpenClose, int SecondsWait)
{
   
    numStepBypass  = 0;
    
    PositionBypass = OpenClose;
	unsigned short enab_func;
	enab_func = read_word_eeprom(ADDR_EEP(Enab_Fuction)); 
    
    if(read_byte_eeprom(ADDR_EEP(RotazioneBypass)) == ORARIO) 
    {
        if(OpenClose == CLOSE_BPD) 
           RotaBypass = ORARIO;
        else  
           RotaBypass = ANTIORARIO;                      
    }else {
        if(OpenClose == CLOSE_BPD) 
           RotaBypass = ANTIORARIO;
        else  
           RotaBypass = ORARIO; 

    }  
    
    if(sData.status_unit & (1 << POS_BIT_BYPASS_RUN))
      return;

    // segnaliamo che il bypass sta iniziando il cambiamento di stato (OPEN->CLOSE 0 CLOSE->OPEN).
    // quindi il regolatore porter�  la velocit�  al minimo.
    sData.status_unit &= ~MSK_STS_BYPASS; //pulisco lo stato
    sData.status_unit |= (1 << POS_BIT_BYPASS_RUN); //imposto la rotazione


	// Se c'� presente l'accessorio bypass (vite senza fine) inviamo il comando di cambio di stato.
	
	if( AccessoryPresent_EepList(ACC_EEP_EBPD) && !(enab_func & (1 << ENAB_MBF)) )
	{
		
		if(RotaBypass == ANTIORARIO)
		{	
			if (info_EBPD != 0)
				write_byte_eeprom(ADDR_EEP(Set_BPD_mod), 0);			
		}
		else if(RotaBypass == ORARIO)
		{
			if (info_EBPD != 100)
				write_byte_eeprom(ADDR_EEP(Set_BPD_mod), 100);	
			
		}
		
		sData.status_unit &= ~MSK_STS_BYPASS; //pulisco lo stato
		if(OpenClose == CLOSE_BPD)   
		sData.status_unit |= (1 << POS_BIT_BYPASS_CLOSE);   //imposto il nuovo stato OPEN = 0 o CLOSE = 1
		
	}
	else 
	if(read_byte_eeprom(ADDR_EEP(SerialString[7])) == '6') // Sono una unit� della serie 6 il bypass � eseguito come stop/avvio rotore dello scambiatore Je suis un appareil de la s�rie 6 ; la d�rivation est r�alis�e par un rotor d'�changeur marche/arr�t		
	{
			if(RotaBypass == ANTIORARIO)
			{
				ioport_set_pin_level(pIPEHD, IOPORT_PIN_LEVEL_HIGH); // uso il pin della IPEHD per dare/togliere alimentazione all'inverter
				ioport_set_pin_level(motor_pin_1, IOPORT_PIN_LEVEL_HIGH);// uso il pin della BPD per dare/togliere consenso all'inverter
			}
			else if(RotaBypass == ORARIO)
			{
				ioport_set_pin_level(pIPEHD, IOPORT_PIN_LEVEL_LOW); // uso il pin della IPEHD per dare/togliere alimentazione all'inverter
				ioport_set_pin_level(motor_pin_1, IOPORT_PIN_LEVEL_LOW); // uso il pin della BPD per dare/togliere consenso all'inverter
			}		
			sData.status_unit &= ~MSK_STS_BYPASS; //pulisco lo stato
			if(OpenClose == CLOSE_BPD)
			sData.status_unit |= (1 << POS_BIT_BYPASS_CLOSE);   //imposto il nuovo stato OPEN = 0 o CLOSE = 1
		}
		else	
			// Aspettiamo N secondi prima di attivare il motore Bypass, in modo tale da far rallentare la pressione dell'aria 
			// e quindi evitare assorbimenti anomali che possono creare problemi.
			insert_function_scheduler(startMotorBPD, ((SecondsWait * 10) + 1)); //scheduliamo funzione di rotazione e tempo di attesa.
		
}  


//--------------------------------------------------------------------
// Function: Ctrl_FunzionamentoBPD
//
// Description:
//  Durante la chiusura o apertura, controlla l'assorbimento del motore
//  del Bypass.
//--------------------------------------------------------------------
void Ctrl_FunzionamentoBPD()
{  
  // verifica l'assorbimento del bypass solo dopo 40 dg di movimento
  if (numStepBypass >= MIN_STEP_CTRL_CURRENT_BPD)
  {
     int valueADC_Bypass;
     short absorption_bypass;
     Bool StopMotor = false;
     
     
     if (numStepBypass >= MAX_STEP_FINE_CORSA) 
     {  
       StopMotor = true;         
       RemoveAlarm(ALM_BYPASS_KO);          
     }else {                   
       valueADC_Bypass = analogRead(pAnAssorb_ByPass);
              
       if(RotaBypass == ANTIORARIO) {  // rotation antoclickwize
         
         // ROTAZIONE ANTIORARIA , Controlliamo la soglia massima
         // 570-580 senza fermo
         // 595-615 Con Sforzo del fermo
         if ((valueADC_Bypass > SOGLIA_MAX_ASSORB_BYPASS_KO) || (valueADC_Bypass < (SOGLIA_MIN_ASSORB_BYPASS_KO >> 1)))
         {
             //StopMotor = true; /////////////////////////////////////////////
             // segnaliamo l'allarme Bypass
#ifndef _DISABLE_ALARM_BYPASS_         
             if(read_byte_eeprom(ADDR_EEP(Config_Bypass)) != BPD_MANUAL_CLOSE)  
               InsertAlarm(ALM_BYPASS_KO);
#endif             
                       
         }else 
			 if (valueADC_Bypass > SOGLIA_MAX_ASSORB_BYPASS_FC)
				 {           
					 //StopMotor = true;  ////////////////////////////////////////////////
             
		#ifndef _DISABLE_ALARM_BYPASS_  
					 if(read_byte_eeprom(ADDR_EEP(Config_Bypass)) != BPD_MANUAL_CLOSE){  
					   if(numStepBypass < (MIN_STEP_CTRL_CURRENT_BPD + 100))
						 InsertAlarm(ALM_BYPASS_KO);
					   else 
						 RemoveAlarm(ALM_BYPASS_KO);
					 }else  
						 RemoveAlarm(ALM_BYPASS_KO);
		#endif               
               
				 } 
       }else { // rotation clockwize
         
         // ROTAZIONE ORARIA, controlliamo la soglia minima  
         // 225-240 senza fermo
         // 200-215 Con Sforzo del fermo
         if(valueADC_Bypass < SOGLIA_MIN_ASSORB_BYPASS_KO) {
            //StopMotor = true;  ///////////////////////////////////////////////////////
            // segnaliamo l'allarme Bypass
#ifndef _DISABLE_ALARM_BYPASS_        
            if(read_byte_eeprom(ADDR_EEP(Config_Bypass)) != BPD_MANUAL_CLOSE) 
              InsertAlarm(ALM_BYPASS_KO);
#endif            
         } else 
		 if (valueADC_Bypass < SOGLIA_MIN_ASSORB_BYPASS_FC) 
			 {
				//StopMotor = true;  ///////////////////////////////////////////////////////////
            
				// se non riecso a superare i 50 dg, c'� comunque un errore... 
#ifndef _DISABLE_ALARM_BYPASS_    
				if(read_byte_eeprom(ADDR_EEP(Config_Bypass)) != BPD_MANUAL_CLOSE){  
				  if(numStepBypass < (MIN_STEP_CTRL_CURRENT_BPD + 100))
					InsertAlarm(ALM_BYPASS_KO);
				  else
					RemoveAlarm(ALM_BYPASS_KO); 
				}else  
					RemoveAlarm(ALM_BYPASS_KO); 
#endif              
			 }  
       }
       
	   BPD_Current = (( 3300 / 1023 ) * valueADC_Bypass); // value of voltage in mv
	   BPD_Current = BPD_Current / 12; // value of current in ma
	   
       //absorption_bypass = (valueADC_Bypass * 3)/11;
	   absorption_bypass = (ushort)BPD_Current;
      
       if(absorption_bypass <= sData.absorption_bypass_min)
          sData.absorption_bypass_min = absorption_bypass;
      
       if(absorption_bypass > sData.absorption_bypass_max) 
          sData.absorption_bypass_max = absorption_bypass;
     }      
          
     if (StopMotor) 
     {
//----------------------------------------       
//         Serial_println1(" ");
//        Serial_print1("Nstep= ");    
//         Serial_println(numStepBypass, DEC);
//----------------------------------------             
         numStepBypass = 0;
         //CloseTimer4(); 
		  tc_stop(TC0, 0); // Start TC0
		  tc_disable_interrupt(TC0,0,TC_IER_CPCS);
         //digitalWrite(motor_pin_1, LOW);
         //digitalWrite(motor_pin_2, LOW);
         //digitalWrite(motor_pin_3, LOW);
         //digitalWrite(motor_pin_4, LOW); 
		 ioport_set_pin_level(motor_pin_1, IOPORT_PIN_LEVEL_LOW);
		 ioport_set_pin_level(motor_pin_2, IOPORT_PIN_LEVEL_LOW);
		 ioport_set_pin_level(motor_pin_3, IOPORT_PIN_LEVEL_LOW);
		 ioport_set_pin_level(motor_pin_4, IOPORT_PIN_LEVEL_LOW);   
              
         sData.status_unit &= ~MSK_STS_BYPASS;    
      
         if(PositionBypass == CLOSE_BPD) 
            sData.status_unit |= (1 << POS_BIT_BYPASS_CLOSE);            
      }     
      
   }else {
      sData.absorption_bypass_min = 255;        
      sData.absorption_bypass_max = 0;   
   }    

}  



//---------------------------------------------------------------------
// Function: Test_Connect_KTS
//
// Description:
//   Funzione che gira nello scheduler (Sample: 1,5 sec.) 
//   Verifica l'assorbimento del KTS connesso alla MB. Se supera  0,18 A
//   oppure se per 30 secondi non comunica com l'unit�, Resetta la sua 
//   alimentazione.
//---------------------------------------------------------------------
static Byte Procedure_reset_KTS_ON = 0; 

int Test_Connect_KTS()
{
  
    if(sData.status_test  & MSK_TEST_ENABLE) {      
        Procedure_reset_KTS_ON = 0; 
        //digitalWrite(pSwitchOFF_KTS, HIGH);    // KTS: 12V ON  
		ioport_set_pin_level(pSwitchOFF_KTS, IOPORT_PIN_LEVEL_HIGH);     // KTS: 12V ON  
        cnt_sec_no_link_KTS = 0;   
        RemoveAlarm(ALM_KTS_FAULT);    
        return 1;       
    }  
       
    if ((sData.measure_I_KTS >= CUNSUMER_MIN_KTS) || Procedure_reset_KTS_ON)   // > 35mA
    {
       // KTS: Inserito nel connettore RJ45
       cnt_sec_no_link_KTS++;
        
       // 1: assorbimento anomalo > di 200mA  
       // 2: Manca la comunicazione da almeno 30 secondi   
       if ((cnt_sec_no_link_KTS == TIMER_KTS_KO) || (sData.measure_I_KTS > 200))  { // 200
           Procedure_reset_KTS_ON = 1;
           //digitalWrite(pSwitchOFF_KTS, LOW);     // KTS: 12V OFF
		   ioport_set_pin_level(pSwitchOFF_KTS, IOPORT_PIN_LEVEL_LOW);
           ActiveBuzzer(1, 15);     
           InsertAlarm(ALM_KTS_FAULT);
           connect_KTS = 0;
       }else if (cnt_sec_no_link_KTS >= TIMER_KTS_KO+3) {
           Procedure_reset_KTS_ON = 0; 
           //digitalWrite(pSwitchOFF_KTS, HIGH);    // KTS: 12V ON   
		   ioport_set_pin_level(pSwitchOFF_KTS, IOPORT_PIN_LEVEL_HIGH);    
           cnt_sec_no_link_KTS = 0;            
       } 
       
    }else  {
       // KTS: NON Inserito nel connettore RJ45. 
       cnt_sec_no_link_KTS = 0;  
       connect_KTS         = 0;
       RemoveAlarm(ALM_KTS_FAULT);       
    }  
    
#ifdef WORK_WDT    
   //ClearWDT();
#endif    
	wdt_restart(WATCH_DOG);
    
    return 1; // tiene sempre viva la funzione nello sheduler   
}  

//---------------------------------------------------------------------
// Function: Gestione_Relays
//
// Description:
//   La funzione gira sotto scheduler: 2,5 sample. 
//   In base alle impostazione in eeprom vado a pilotare i 2 Rele
//---------------------------------------------------------------------
int Gestione_Relays()
{
   int i, j;
   Byte cfg_relay, rele_disseccitato_condiz_on;
   Byte condizione_on =0;
   int pRelay[2] = {pOut1, pOut2};
   unsigned short SetPointTemp, enab_func;
   Byte i_rifTemp;
   unsigned short SetExtTemp;
   int MaxSpeed;
   
   if(sData.status_test & MSK_TEST_OUTPUT)
     return 1;

   for(i=0; i <2; i++) 
   {
      // sEep.Set_Output[i]:  0= Disable                 (il rele rimane non alimentato)                   
      //                      1= Bypass Status Open      (il rele passa in attrazione se: Bypass Open)	
      //                      2= Common Fault Status     (il rele passa in attrazione se: Unit� in allarme)		
      //                      3= Unit is Run (ex: SDD)   (il rele passa in attrazione se: Unita � in funz.)	
      //                      4= Pilota la valvola       (il rele passa in attrazione se: AWP � minore del Setpoint di temperatura)
      //                      5= Estate / Inverno        (il rele passa in attrazione se: Fresh � minore della temperatura Bypass_minTempExt)
	  //                      6= Max Speed			     (il rele passa in attrazione se: L'unit� raggiunge il 100% di ventilazione)
      //                    128= Disable                 (il rele rimane in attrazione)
      //                    129= Bypass Status Open      (il rele si Diseccita se: Bypass Open)
      //                    130= Common Fault Status     (il rele si Diseccita se: Unit� in allarme)
      //                    131= Unit is Run (ex: SDD)   (il rele si Diseccita se: Unita � in funz.)    
      //                    132= Pilota la valvola       (il rele si Diseccita se: AWP � minore del Setpoint di temperatura)
      //                    133= Estate / Inverno        (il rele si Diseccita se: Fresh � minore della temperatura Bypass_minTempExt) 
	  //                    134= Max Speed			     (il rele si Diseccita se: L'unit� raggiunge il 100% di ventilazione)	  
      cfg_relay = read_byte_eeprom(ADDR_EEP(Set_Output[i])); 
      
      //   0x80 = il rele passa da Attrazione a Riposo se soddisfa la condizione 
      //   0x00 = il rele passa da Riposo in Attrazione se soddisfa la condizione 
      rele_disseccitato_condiz_on = cfg_relay & 0x80; 
      condizione_on =0;
      
      switch (cfg_relay &0x07){
        case 1:   
          //  1= Bypass Status 	
          if((sData.status_unit & MSK_STS_BYPASS) == 0) 
              condizione_on = 1;  // Bypass Open 
        break;
      
        case 2:   
          //  2= Common Fault Status      
          //     ci appogiamo allo stato dei leds
          //     LED_ALARM, bit:[3,2]=(11=none, 10=ON, 01=Toggle, 00=OFF)          
          if(sData.status_Leds_ctrlFan & 0x08) 
            condizione_on = 1; // condizione soddisfatta 
              
        break;
      
        case 3:   
          //  3= Unit is Run (ex: SDD)  
          //     bit[5,...,0]:  b010000= RUN BOOST, b001000= RUN IMBALANCE, b000100=RUN POST_VENT, b000010=RUN DeFROST, b00001=RUN, b000000=STANDBY   
          if (sData.status_unit & MSK_UNIT_OPERATING) 
            condizione_on = 1;  // condizione soddisfatta     
        break;             
        
        case 4:   
          //  4= Pilota la valvola  
            i_rifTemp    = read_byte_eeprom(ADDR_EEP(idxSetPointT)) & 0x01;
            SetPointTemp = read_word_eeprom(ADDR_EEP(SetPointTemp[i_rifTemp])); 
            enab_func = read_word_eeprom(ADDR_EEP(Enab_Fuction));
            
          if(enab_func & (1 << ENAB_SUMMER))
           {
            if (sData.measure_AWP_max >  SetPointTemp) 
                condizione_on = 1;  // condizione soddisfatta -- raffreddo    
           } 
           else
           {
            if (sData.measure_AWP_max <  SetPointTemp) 
                condizione_on = 1;  // condizione soddisfatta -- scaldo
           }
        break; 

        case 5:   
          //  5= Estate/Inverno  

            SetExtTemp = read_word_eeprom(ADDR_EEP(Bypass_minTempExt)); 
            if ((sData.status_unit & MSK_UNIT_OPERATING) && (soft_start == 0))
            {
            if (sData.measure_Temp[I_PROBE_FRESH] >  SetExtTemp)
            {
                  count_win = FULLTIME;
               if( count_sum > 0)
                   {
                   count_sum--;
                   condizione_on = 0;  // condizione di persistenza non soddisfatta -- sono in winter
                   output_on = 0;
                   }
               if( count_sum == 0)
               {
                   condizione_on = 1;  // condizione soddisfatta -- sono in summer
                   output_on = 1;
               }
            }
            else
            {
                  count_sum = FULLTIME;
               if( count_win > 0)
                   {
                   count_win--;
                   condizione_on = 1;  // condizione di persistenza non soddisfatta -- sono in summer
                   output_on = 1;
                   }
               if( count_win == 0)
               {
                   condizione_on = 0;  // condizione soddisfatta -- sono in winter
                   output_on = 0;
               }
            }
            }
            else
            {
                  condizione_on = 0;  // se la macchina � spenta non devo pilotare niente
                  output_on = 0;
                  if(soft_start <= 0)
                  soft_start = 0;
                  else soft_start--;// per i primi 15 secondi dall'accensione iniziale (no stand-by) non faccio niente per lasciare andare in temperatura le sonde
            }
            
        break; 
		
		case 6:   
          //  6= Max Speed  
			
			MaxSpeed = (int)((float)255 * calcDepot());
			
		  //controllo se sono al massimo
			if (sData.speed_motors_R >= MaxSpeed || sData.speed_motors_F >= MaxSpeed )
				condizione_on = 1;
				else
				condizione_on = 0;	
            
        break;
       
      }

      //---- pilotiamo i rele' ----
      if(rele_disseccitato_condiz_on) {
         if(condizione_on) {     
            // il rel� si dissecita     
            //digitalWrite(pRelay[i],   LOW);
			if ( i == 0 )
				ioport_set_pin_level(pOut1, IOPORT_PIN_LEVEL_LOW);
			if ( i == 1 )
				ioport_set_pin_level(pOut2, IOPORT_PIN_LEVEL_LOW);	
            sData.StatusOutput &= ~(1 << i);
            
         }else{
            // il rel� si eccita, perch� non viene soddisfatta la condizione
            //digitalWrite(pRelay[i],   HIGH);
			if ( i == 0 )
				ioport_set_pin_level(pOut1, IOPORT_PIN_LEVEL_HIGH);
			if ( i == 1 )
				ioport_set_pin_level(pOut2, IOPORT_PIN_LEVEL_HIGH);
            sData.StatusOutput |= (1 << i);
            
         }  
      }else {
         if(condizione_on) { 
            // il rel� si eccita, perch� viene soddisfatta la condizione
            //digitalWrite(pRelay[i],   HIGH);
			if ( i == 0 )
				ioport_set_pin_level(pOut1, IOPORT_PIN_LEVEL_HIGH);
			if ( i == 1 )
				ioport_set_pin_level(pOut2, IOPORT_PIN_LEVEL_HIGH);
            sData.StatusOutput |= (1 << i);
            
         }else {
            // il rel� si dissecita
            //digitalWrite(pRelay[i],   LOW);
			if ( i == 0 )
				ioport_set_pin_level(pOut1, IOPORT_PIN_LEVEL_LOW);
			if ( i == 1 )
				ioport_set_pin_level(pOut2, IOPORT_PIN_LEVEL_LOW); 
            sData.StatusOutput &= ~(1 << i);
                       
         }  
      }  
   }

   return 1;   
}  

//---------------------------------------------------------------------
// Function: Check_SerialNumber
//
// Description:
//   Controlla la validita' del serial number, se � default,
//   Imposta il depotenziamento in base alla lettura del canale AN9
//   secondo una certa tabella.
//---------------------------------------------------------------------
void Check_SerialNumber()
{
   int val_16 = 0, i;
   Byte ord_vent = 0, data_prod =0, depot = 100;
           
  // SN[18] byte:  Ascii:  "OrVe SIZE CFG DATA PRG",
  // 1. se Ordine i vendita = '0','0','0','0'  
  for(i=0, ord_vent = 0; i < 4; i++) {     
    if(read_byte_eeprom(ADDR_EEP(SerialString[i])) == '0')
      ord_vent++;
  } 
  // 2. se la data � = '0','0','0','0'  
  for(i=11, data_prod=0; i < 15; i++) {     
    if(read_byte_eeprom(ADDR_EEP(SerialString[i])) == '0')
      data_prod++;
  } 
  
  // se 1 && 2  si verificano allora il serial number � default
  // quindi andiamo a prendere il valore del livello HW
  if((data_prod >= 4) && (ord_vent >= 4)) {   
     i = 0;
     while(1) {
         //digitalWrite(pBuz,  LOW);
         //digitalWrite(pStatusLedI2C, LOW);
         //digitalWrite(pLED5, LOW);
		 ioport_set_pin_level(pBuz, IOPORT_PIN_LEVEL_LOW);
		 ioport_set_pin_level(pStatusLedI2C, IOPORT_PIN_LEVEL_LOW);
		 ioport_set_pin_level(pLED5, IOPORT_PIN_LEVEL_LOW);
         delay_ms (1000);
		 wdt_restart(WATCH_DOG);
         if(i++ & 0x01)
           Serial_println1("Error Serial Number!!");
         //digitalWrite(pBuz,  HIGH);
         //digitalWrite(pStatusLedI2C, HIGH);
         //digitalWrite(pLED5, HIGH);
		 ioport_set_pin_level(pBuz, IOPORT_PIN_LEVEL_HIGH);
		 ioport_set_pin_level(pStatusLedI2C, IOPORT_PIN_LEVEL_HIGH);
		 ioport_set_pin_level(pLED5, IOPORT_PIN_LEVEL_HIGH);
         delay_ms (250);
     }  
    
  }
    
}

