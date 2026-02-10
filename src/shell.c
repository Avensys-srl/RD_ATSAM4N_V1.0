/*
 * shell.cpp
 *
 */ 

#include "asf.h"

#include "Protocol_Ser1.h"
#include "eeprom_map.h"
#include "data.h"
#include "alarm.h"

#define word	uint16_t

// stato del Rele' e delle tensione dell'elettronica
#define POWER_SAVE     0
#define POWER_ON      'O'

//--------------------------------
#define NEW_LINE      'n'
#define NO_NEW_LINE    0

// ------------- Funzioni Abilitate in base alla config. ---------------
//  bit[7]:CAF] | bit[6]:CAP  | bit[5]:CSF | bit[4]:ImbalanON | bit[3]:StepLess | bit[2]:WeeklyENAB | bit[1]:BoostON | bit[0]:DeFrostENAB.
#define ENAB_FUNC_EXTRA    0xFF  //      1            1             1              1                  1                   1                   1               1
#define ENAB_FUNC_BASIC    0x29  //      0            0             1              0                  1                   0                   0

extern void eep_write_byte(unsigned int addr, Byte data_to_send);
extern Byte eep_read_byte(unsigned int addr);
extern unsigned short calc_size_unit();
extern void test_bypass(void);
extern void test_IO(void);
extern void testSleepMode(void);
extern void testMotors(void);
extern void testUart1(void);
extern void testI2C();

//----------  [menu principale]  --------------------
const char str_Title_MainMenu[]  = {"= = = = = =[ MAIN MENU ]= = = = = ="};
const char str_0_Row_MainMenu[] = {" [0]  Exit menu, and Starting Prog."};
const char str_1_Row_MainMenu[] = {" [1]  Write eeprom default"};
const char str_2_Row_MainMenu[] = {" [2]  Read Config. Data Unit"};
const char str_3_Row_MainMenu[] = {" [3]  Write Serial Number"};
const char str_4_Row_MainMenu[] = {" [4]  Set Data Motors & Bypass"};
const char str_5_Row_MainMenu[] = {" [5]  Setting NTC"};
const char str_6_Row_MainMenu[] = {" [6]  enter [MANUAL TESTING MENU]"};
const char str_7_Row_MainMenu[] = {" [7]  Start AutoDiagnostics"};
const char str_8_Row_MainMenu[] = {" [8]  View Weekly Program"};
const char str_9_Row_MainMenu[] = {" [9]  Insert String Config. Unit"};

//--------  [1]  Write eeprom default  ---------------
const char str_Title_1_Menu[]     = {"---[ 1  Write eeprom Default ]---"};
const char str_confirm_1_Menu[]   = {"Do you want to proceed with the initializ. the eeprom (y/n)?"};
const char str_error_char_1_Menu[]= {"please, just enter the character  'y' (Yes) or 'n' (No)"};

//--------  [2]  Read Configuration Data  ------------
const char str_Title_2_Menu[]     = {"---[ 2  Read Config. Data Unit ]---"};
const char str_addrUnit_2_Menu[]  = {"  2.01-Address Unit.: "};
const char str_Control_2_Menu[]   = {"  2.02-Type Unit....: "};
const char str_VerHW_2_Menu[]     = {"  2.03-Version HW...: "};
const char str_VerSW_2_Menu[]     = {"  2.04-Version SW...: "};
const char str_SerNumb_2_Menu[]   = {"  2.05-Serial Number: "};
const char str_NumMotors_2_Menu[] = {"  2.06-Number Motors: "};
const char str_NumPulse_2_Menu[]  = {"  2.07-Number pulse : "};
const char str_EnabCAF_2_Menu[]   = {"  2.08-Enab.Func.CAF: "};
const char str_EnabCAP_2_Menu[]   = {"  2.09-Enab.Func.CAP: "};
const char str_CapMotors_2_Menu[] = {"  2.10-Capac. Motors: "};
const char str_NumNTC_2_Menu[]    = {"  2.11-Number NTC...: "};
const char str_PosNTC_2_Menu[]    = {"  2.12-Position NTC : "};
const char str_Bypass_2_Menu[]    = {"  2.13-Bypass Close : "};
const char str_Accessory_2_Menu[] = {"  2.14-PreConf.Acces: "};


//---------  [3]  Write Serial Number  ---------------
const char str_Title_3_Menu[]      = {"---[ 3  Write Serial Number ]---"};
const char str_actualy_SN_3_Menu[] = {" Actualy SN is: "};
const char str_enter_SN_3_Menu[]   = {" Enter 18 numb. with or without spaces to compose the new SN."};
const char str_enter_SN_DESC_1[]   = {"Descript SN:[4:SalesOrd] [4:Size] [3:Cfg] [4:Data] [3:NProg]"};   
const char str_enter_SN_DESC_2[]   = {"[3:Cfg] [1]: 0=OSC, 1=SSC, 2=EOS, 3=EOS"}; 
const char str_enter_SN_DESC_3[]   = {"        [2]: 0=none, 1=A, 2=B, .."}; 
const char str_enter_SN_DESC_4[]   = {"        [3]: 0=none, 1=1, 2=2, .."}; 
const char str_new_SN_3_Menu[]     = {" The New SN is: "};
const char str_error1_3_Menu[]     = {" Error, The SN is not correct!"};
const char str_error2_3_Menu[]     = {" Error, the number of digits is less than 18!"};
const char str_error3_3_Menu[]     = {" Error, the number of digits is greater than 18!"};

//---------  [4]  Set Data Motors & Bypass -------------------
const char str_Title_4_Menu[]     = {"---[ 4  Set Data Motors & Bypass ]---"};
const char str_NumMotors_4_Menu[] = {" 4.1-Set Number Motors (2, 4, 6): "};
const char str_NumPulse_4_Menu[]  = {" 4.2-Set Number pulses/revolution (01,..,10): "};
const char str_CapMotors_4_Menu[] = {" 4.5-Set Capacity Motors(40%,..,100%): "};
const char str_Bypass_4_Menu[]    = {" 4.6-Set Bypass Close(0=Orario,1=Antiora): "};
const char str_Enab_CAP_4_Menu[]  = {" 4.7-Enable function CAP (y or n): "};
const char str_Enab_CAF_4_Menu[]  = {" 4.8-Enable function CAF (y or n): "};
const char str_error1_4_Menu[]    = {" Error, the number of pulses/revolution varies from 1 to 10!"};
const char str_error4_4_Menu[]    = {" Error, the value of capacity is not correct!"};
const char str_error5_4_Menu[]    = {" Error, the value of capac. is not correct (max=100, min=40)!"};

//----------  [5]  Setting NTC  -----------------
const char str_Title_5_Menu[]       = {"---[ 5  Setting NTC ]---"};
const char str_enter_NTC_5_Menu[]   = {" Enter the new value of NTC (2, 3, 4), actualy is "};
const char str_new_NTC_5_Menu[]     = {" Digit the new number: "};
const char str_error1_5_Menu[]      = {" Error, the entered data is not a num. comprised between 2 & 4"};
const char str_Info_cfg_NTC_5_Menu[] = {" Set the Position NTC: (TFresh).(TReturn).(TSupply).(TExuast)"};
const char str_ex_1_cfg_NTC_5_Menu[] = {"   Example 1= 0.1.2.3 -> TFre=0, TRet=1, TSup=2, TExa=3"};
const char str_ex_2_cfg_NTC_5_Menu[] = {"   Example 2= 1.2.0.3 -> TFre=1, TRet=2, TSup=0, TExa=3"};
const char str_set_cfg_NTC_5_Menu[]  = {" Digit new Position (TFr.TRe.TSu.TEx): "};

//----------- [6]  Menu manual Testing  ---------------
const char str_Title_6_Menu[] = {"= = = =[ MANUAL TESTING MENU ]= = = ="};
const char str_row0_6_Menu[]  = {" [0]  Return [MAIN MENU]"};
const char str_row1_6_Menu[]  = {" [1]  Test Bypass"};
const char str_row2_6_Menu[]  = {" [2]  Test IO"};
const char str_row3_6_Menu[]  = {" [3]  Test SleepMode"};
const char str_row4_6_Menu[]  = {" [4]  Test Motors"};
const char str_row5_6_Menu[]  = {" [5]  Test Uart 1"};
const char str_row6_6_Menu[]  = {" [6]  Test I2C Bus"};

//----------- [6]  view weeky program  ---------------
const char str_Title_8_Menu[] = {"---[ 8  View Weekly Program ]---"};

//----------- [9]  Config Unit  ---------------
const char str_Error_9_Menu[] = {"Error, device configuration failed!"};  

//-------------------- global vars ----------------------
Byte buff_RS232_rx[64];

extern SDATA  sData;
extern Byte Shadow_eep[256];
extern Byte Shadow_eep_preview[256];

//---------------------------------------------------
// sprompt Memu Principale
//---------------------------------------------------
void Stampa_Prompt()
{
  Serial_println1(" ");  
  Serial_print1("> Select the number (0,..,9): ");     
}  
//----------------------------------------------------
// prompt Menu manual Testing
//----------------------------------------------------
void printPrompMenu6()
{
   Serial_println1(" "); 
   Serial_print1("> Select Test (0,..,6): ");   
}  


//------------------------------------------------------
// Attendi la risposta dal terminale
//------------------------------------------------------
int wait_response_by_terminal()
{
  Byte nb = 0, nb_preview;
  Byte exit_loop = 0;
  int i;
   
  // scarichiamo prima il buffer
   nb = Serial_available(); 
   for(i=0; i< nb; i++) 
     buff_RS232_rx[0] = Serial_read();
   
   nb_preview = 0;
   
   while(!exit_loop)
   { 
	 wdt_restart(WATCH_DOG);
	   
     delay_ms (75);
        
     nb = Serial_available();    
     
     if(nb) {
       if(/*nb_preview == */nb) {
           for(i=0; i< nb; i++) {
               buff_RS232_rx[i] = Serial_read();
               if(i >= 62) {
                 nb=62;
                 break;
               }
           }
           exit_loop = 1;
       }else  
           nb_preview = nb;      
    } 
  }

  return nb;  
}

//----------------------------------------------------------
// stampa una stringa utilizzando la memoria in flash
//----------------------------------------------------------
int PrintFlashString(const char *str_f, char newline)
{
   char str[64];
   int len =0;
   
   do {
     str[len] = *str_f++;
     if(++len >= 62)
       break;
   }while(*str_f);
   
   str[len] = 0;  
   
   if(newline == NEW_LINE)
     Serial_println1(str);
   else  
     Serial_print1(str);
   
}  


//---------------------------------------------------
// Stampa il menu con le opzioni
//---------------------------------------------------
void Stampa_menu_config()
{
     Serial_println1(" "); 
     Serial_println1(" ");
     PrintFlashString(str_Title_MainMenu,  NEW_LINE);
     Serial_println1(" ");
     delay_ms (10);
     PrintFlashString(str_0_Row_MainMenu, NEW_LINE);
     PrintFlashString(str_1_Row_MainMenu, NEW_LINE);
     PrintFlashString(str_2_Row_MainMenu, NEW_LINE);
     PrintFlashString(str_3_Row_MainMenu, NEW_LINE);
     PrintFlashString(str_4_Row_MainMenu, NEW_LINE);
     delay_ms (20);
     PrintFlashString(str_5_Row_MainMenu, NEW_LINE);
     PrintFlashString(str_6_Row_MainMenu, NEW_LINE);
     PrintFlashString(str_7_Row_MainMenu, NEW_LINE);
     PrintFlashString(str_8_Row_MainMenu, NEW_LINE);
     PrintFlashString(str_9_Row_MainMenu, NEW_LINE);
     
}  


//---------------------------------------------------
// (1) Write eeprom default
//---------------------------------------------------
void WriteEepromDefault()
{ 
   Byte nb;
   unsigned int a, addr_end;
    
   PrintFlashString(str_Title_1_Menu,  NEW_LINE);
   Serial_println1(" "); 
   
   PrintFlashString(str_confirm_1_Menu, NEW_LINE);

   while(1)
   {    
     nb = wait_response_by_terminal();
      
     if(nb >= 1) {
       if ((buff_RS232_rx[0] == 'Y') || (buff_RS232_rx[0] == 'y'))
         break; // sci dal loop
       else if ((buff_RS232_rx[0] == 'N') || (buff_RS232_rx[0] == 'n'))
         return;
       else  
         PrintFlashString(str_error_char_1_Menu, NEW_LINE);
     }else
       PrintFlashString(str_error_char_1_Menu, NEW_LINE);       
   }     

   
   Serial_println1(" ");
   Serial_println1("Init eeprom");        
   
   addr_end = ADDR_EEP(check_eeprom[1]);  
   for(a=0; a <= addr_end; a++) {
      eep_write_byte(a,  vect_default_eeprom[a]); 
      Shadow_eep[a] = vect_default_eeprom[a];
      
      if((a&0x0F) == 0x0F) {
        Serial_print1(".");
        delay_ms (50);
      }  
   }   
   Serial_println1("  Done");
} 

//---------------------------------------------------
//
//---------------------------------------------------
void PrintSN()
{
  unsigned short addr, a;
  char buff[24];
  
  addr = ADDR_EEP(SerialString[0]);
  
  for(a=0; a <22; ) {
    if ((a==4) || (a==9) || (a==13) || (a==18)) 
      buff[a] = ' ';  // mettiamo gli spazi per comprendere meglio il significato del serial number
    else
      buff[a] = eep_read_byte(addr++);
    a++;
  }  
  buff[a] = 0; // chiudiamo la stringa
  Serial_println1(buff); // es:  1565 0023 015 1351 002  
}  

//---------------------------------------------------
//
//---------------------------------------------------
void PrintPositionNTC(Byte val)
{
    Byte n;
    
    Serial_print1("TFr=");
    n = val & 0x03;
    Serial_print(n, DEC);
  
    Serial_print1(" TRe=");
    n = (val >> 2) & 0x03;
    Serial_print(n, DEC);
  
    Serial_print1(" TSu=");
    n = (val >> 4) & 0x03;
    Serial_print(n, DEC);
    
    Serial_print1(" TEx=");
    n = (val >> 6) & 0x03;
    Serial_println(n, DEC);  
}  

//---------------------------------------------------
// --- Read Config. Data Units ---
//   Address Unit: 1
//   Functio.Unit: EXTRA
//   Version HW  : 4.0
//   Version SW  : 2.01 
//   SerialNumber: 1565 0023 015 1351 002
//   Numb. Motors: 2
//   Numb. Pulse : 2
//   Enab Fun.CAF: Y/N
//   Enab Fun.CAP: Y/N
//   Capac.Motors: 100%
//   Number NTC  : 4
//   Position NTC: TFr=0 TRe=1 TSu=2 Tex=3
//---------------------------------------------------
void read_cfg_data()
{
  unsigned short addr, a;
  Byte val;
  char buff[24];
  
  PrintFlashString(str_Title_2_Menu,  NEW_LINE);

  // Address Unit: 1
  PrintFlashString(str_addrUnit_2_Menu, NO_NEW_LINE);
  a = (word)eep_read_byte(ADDR_EEP(AddrUnit));
  Serial_println(a, DEC);

  // Control Unit: BASIC/EXTRA
  PrintFlashString(str_Control_2_Menu, NO_NEW_LINE);
  val = eep_read_byte(ADDR_EEP(Type_func));
  
  if(val)
    Serial_println1("EXTRA"); 
  else  
    Serial_println1("BASIC"); 
    
  // Version HW  : 4.0
  PrintFlashString(str_VerHW_2_Menu, NO_NEW_LINE);
  addr = ADDR_EEP(HW_Vers[0]);
  for(a=0; a < 4; a++)
    buff[a] = eep_read_byte(addr+a);
  buff[a] = 0; // chiudiamo la stringa
  Serial_println1(buff);  
  
  delay_ms (50);
  
  // Version SW  : 2.01
  PrintFlashString(str_VerSW_2_Menu, NO_NEW_LINE);
  addr = ADDR_EEP(SW_Vers[0]);
  for(a=0; a < 5; a++)
    buff[a] = eep_read_byte(addr+a);
    
  buff[a] = 0; // chiudiamo la stringa
  Serial_println1(buff);  
    
  // SerialNumber: 1565 0023 015 1351 002
  PrintFlashString(str_SerNumb_2_Menu, NO_NEW_LINE);
  PrintSN();

  // Numb. Motors: 2, 4, 6
  PrintFlashString(str_NumMotors_2_Menu, NO_NEW_LINE);
  addr = ADDR_EEP(numMotors);
  val = eep_read_byte(addr);
  Serial_println(val, DEC);
  
  // Num.TH round: 
  PrintFlashString(str_NumPulse_2_Menu, NO_NEW_LINE);
  addr = ADDR_EEP(numPulseMotors);
  val = eep_read_byte(addr);
  Serial_println(val, DEC);
  
  delay_ms (50);
  
  //
  PrintFlashString(str_EnabCAF_2_Menu, NO_NEW_LINE);
  addr = ADDR_EEP(msk_Enab_Fuction);
  val = eep_read_byte(addr);
  if (val & (1 << ENAB_CAF))
    Serial_println1("Yes");
  else 
    Serial_println1("No");  

  PrintFlashString(str_EnabCAP_2_Menu, NO_NEW_LINE);
  if (val & (1 << ENAB_CAP))
    Serial_println1("Yes");
  else 
    Serial_println1("No");  
  
  // Capac.Motors: 100%,..,40% 
  PrintFlashString(str_CapMotors_2_Menu, NO_NEW_LINE);
  addr = ADDR_EEP(depotMotors);
  val = eep_read_byte(addr);
  Serial_print(val, DEC);
  Serial_println1("% ");
  
  // Number NTC  : 2, 3, 4
  PrintFlashString(str_NumNTC_2_Menu, NO_NEW_LINE);
  addr = ADDR_EEP(numNTC);
  val = eep_read_byte(addr);
  Serial_println(val, DEC);
  
  // Position NTC
  PrintFlashString(str_PosNTC_2_Menu, NO_NEW_LINE);
  addr = ADDR_EEP(Posiz_NTC);
  val = eep_read_byte(addr);
  PrintPositionNTC(val);
  
  delay_ms (50);
  
  // Set Bypass_Close
  PrintFlashString(str_Bypass_2_Menu, NO_NEW_LINE);
  addr = ADDR_EEP(RotazioneBypass);
  val = eep_read_byte(addr);
  if(val)
    Serial_println1("1=AntiOrario (default)");
  else
    Serial_println1("0=Orario");
     
  // List AccessoyHW[0]:   ----,   BPD,   AWP,   CWD,   EHD,   HWD,  PHWD,  IPEHD (LSB) 
  // List AccessoyHW[2]:  P2CO2, P1CO2, -----,  P2RH,  P1RH, -----, P1VOC, -----, (LSB)    
//  List AccessoyHW[0]:  parte 1 list Acc.HW: (MSB)    PIR,   BPD,   AWP,   CWD,   EHD,   HWD,  PHWD,  PEHD  (LSB) 
//  List AccessoyHW[1]:  parte 2 list Acc.HW: (MSB)    DPS,   CAF,   CAP,   INP,   OUT,  DDPV2,   RFM,  MBUS  (LSB) 
//  List AccessoyHW[2]:  parte 3 list Acc.HW: (MSB)  P2CO2, P1CO2, -----,  P2RH,  P1RH,  ----, P1VOC, -----  (LSB) 
//  List AccessoyHW[3]:  parte 4 list Acc.HW: (MSB)  -----, -----, -----, -----, -----, -----, -----, -----  (LSB)


  PrintFlashString(str_Accessory_2_Menu, NO_NEW_LINE);
  
  addr = ADDR_EEP(AccessoyHW[0]);
  buff[0] = eep_read_byte(addr);
  buff[1] = eep_read_byte(addr+1);
  buff[2] = eep_read_byte(addr+2);
  
  if(((buff[0] & 0x01) == 0) && ((buff[1] & 0x08) == 0) && ((buff[2] & 0xDA) == 0))
    Serial_print1("- - - ");
  else {  
    if (buff[0] & 0x01) Serial_print1("PreHeater  ");
    if (buff[1] & 0x04) Serial_print1("DPP_V2  ");
    if (buff[2] & 0x02) Serial_print1("VOC_Sensor  ");
    if (buff[2] & 0x18) Serial_print1("RH_Sensor  ");
    if (buff[2] & 0xC0) Serial_print1("CO2_Sensor  ");
  }

  Serial_println1(" ");
  delay_ms (800);
}


//------------------------------------------------------
// Inserisci il nuovo serial number
//
// ---[ 3  Write Serial Number ]---
// Enter 18 numbers with or without a space to compose the new SN.
//
//  Actualy SN: 1565 0023 015 1351 002
//  New SN....: 
//------------------------------------------------------
void Set_SerialNumb()
{
   Byte i, nb, numSN;
   char newVal[22], wr_eep = 0;
   unsigned short addr;
   
   PrintFlashString(str_Title_3_Menu, NEW_LINE);
   Serial_println1(" ");  
   PrintFlashString(str_actualy_SN_3_Menu, NO_NEW_LINE);
   PrintSN(); 
   PrintFlashString(str_enter_SN_DESC_1, NEW_LINE);
   PrintFlashString(str_enter_SN_DESC_2, NEW_LINE);
   delay_ms (10);
   PrintFlashString(str_enter_SN_DESC_3, NEW_LINE);
   PrintFlashString(str_enter_SN_DESC_4, NEW_LINE);

   Serial_println1(" ");
   PrintFlashString(str_enter_SN_3_Menu, NEW_LINE);
   
   nb = wait_response_by_terminal();  
   
   if(nb == 1)  
     PrintSN();     
   else { 
      for(i=0, numSN=0; i< nb; i++)  
      {
          if ((buff_RS232_rx[i] == ' ') || (buff_RS232_rx[i] == '-') || (buff_RS232_rx[i] == '.') || (buff_RS232_rx[i] == '\n')) 
             continue;
          else if ((buff_RS232_rx[i] < '0') || (buff_RS232_rx[i] > '9')) {
             numSN = 0;
             break;
          }else {
             newVal[numSN] = buff_RS232_rx[i];
             if(++numSN >= 20)
               numSN = 20;
          }            
      }  
      
      if (numSN == 0)  
        PrintFlashString(str_error1_3_Menu, NEW_LINE); 
      else if(numSN < 18)       
        PrintFlashString(str_error2_3_Menu, NEW_LINE); 
      else if (numSN > 18)   
        PrintFlashString(str_error3_3_Menu, NEW_LINE); 
      else {
        newVal[18] = 0; 
        wr_eep  = 1;
      }  
   }       
          
   // aggiorniamo al eeprom e la shadow.
   if(wr_eep) {       
       addr = ADDR_EEP(SerialString[0]);
       
       for(i=0; i <18; i++) {
         eep_write_byte(addr,  newVal[i]); 
         Shadow_eep[addr] = newVal[i];
         addr++;
       } 
       Serial_println1(" ");
       PrintFlashString(str_new_SN_3_Menu, NO_NEW_LINE);
       PrintSN(); 
       
       sData.size_unit = calc_size_unit();
   }  
}  


//------------------------------------------------------
// Inserisci i dati relaiti ai motori
//
//  ---[ 4  Set Data Motors ]---
//   Set Number Motors(2, 4, 6):
//   Set Number TH round: 
//   Set Type Motors (F=Forward, B=Backward):
//   Set Program Motors (A, S, P):
//   Set Capacity Motors(40%,..,100%):
//
//------------------------------------------------------
void Set_dataMotors()
{
  Byte nb, buff_data_motors[12], val, wr_eep = 0;
  unsigned short addr1, addr2, i, val_16;
  
  PrintFlashString(str_Title_4_Menu, NEW_LINE);
  Serial_println1(" ");

  /* Leggiamo prima la configurazione in eeprom:
    [0]  numMotors       : 1 byte
    [1]  numPulseMotors  : 1 byte
    [2]   * none *       : 1 byte 
    [3]   * none *       : 1 byte
    [4]  depotMotors     : 1 byte
    [5]   * none *       : 1 byte
    [6]   * none *       : 1 byte            
    [7]  RotazioneBypass : 1 byte 
         Enab. CAP       : 1 bit
         Enab. CAF       : 1 bit
    
   */
  addr1 = ADDR_EEP(numMotors);     
  addr2 = ADDR_EEP(RotazioneBypass);   
  for(i=addr1; i <= addr2; i++)
      buff_data_motors[i-addr1] = eep_read_byte(i);
  
  //--- 1 Set numbers Motors ---
  PrintFlashString(str_NumMotors_4_Menu,  NO_NEW_LINE);
  nb = wait_response_by_terminal();
  if ((nb>=1) && (buff_RS232_rx[0] != 0x0A))  {
    if((buff_RS232_rx[0] != '2') && (buff_RS232_rx[0] != '4') && (buff_RS232_rx[0] != '6')) {
      Serial_print(buff_RS232_rx[0], BYTE);
      Serial_println1(" , Error the units can have 2, 4 or 6 motors!");
      return;
    }
    
    buff_data_motors[0] = buff_RS232_rx[0] - '0';  
    wr_eep = 1;  
  }
  Serial_println(buff_data_motors[0], DEC);  
  
  //--- 2  numPulseMotors ---
  PrintFlashString(str_NumPulse_4_Menu,   NO_NEW_LINE);
  nb = wait_response_by_terminal();
  if ((nb>=1) && (buff_RS232_rx[0] != 0x0A))  
  {
     // il numero può avere anche 2 cifre
    i = (nb -1)& 0x03; 
   
    if((buff_RS232_rx[0] < '0') || (buff_RS232_rx[0] > '9'))
      i = 0; // errore
   
    if(nb > 2) {
      if((buff_RS232_rx[1] < '0') || (buff_RS232_rx[1] > '9'))
        i = 0; // errore
    }  
    
    if(i) {      
       val = buff_RS232_rx[0] - '0';
       
       if(i>1) { 
         val *= 10;
         val += buff_RS232_rx[1] - '0';
       }    
       
       if((val < 1) || (val > 16)) { // for test we put max pulse number to 16
         Serial_print(val, DEC);
         PrintFlashString(str_error1_4_Menu,  NEW_LINE);  // ...Error, the number of pulses/revolution varies from 1 to 10!
         return;
       }  
 
       wr_eep = 1;
             
    }else {
       PrintFlashString(str_error1_4_Menu,  NEW_LINE);
       return;
    }      
    
    buff_data_motors[1] = val;
  }   
  Serial_println(buff_data_motors[1], DEC); // se solo il carattere '\n' riporta il valore in eeprom 
  

  //--- 3  depotMotors ---
  PrintFlashString(str_CapMotors_4_Menu,  NO_NEW_LINE);
  nb = wait_response_by_terminal();
  
  if((nb>=1) && (buff_RS232_rx[0] != 0x0A))    
  {
     for(i=0; i <nb; i++) {
       if((buff_RS232_rx[i] < '0') && (buff_RS232_rx[i] > '9') || (i>3)) {
          PrintFlashString(str_error4_4_Menu,  NEW_LINE);  // ...Error, the value of capacity is not correct!
          return;         
       }         
     }  
     
     if(nb == 3) { // 2 cifre 
        val = (buff_RS232_rx[0] -'0') *10;
        val += (buff_RS232_rx[1] -'0');       
        if(val < 40) {
          Serial_println1(" Error, the minimum value is 40!"); 
          return; 
        }  
       
        buff_data_motors[4] = val;
        wr_eep = 1;
     }else if(nb == 4) { // 3 cifre
        val_16 = (buff_RS232_rx[0] -'0') * 100;
        val_16 += (buff_RS232_rx[1] -'0') * 10;
        val_16 += (buff_RS232_rx[2] -'0');
       
        if(val_16 > 100) {
          val_16 = 100;
          Serial_println1(" Warning, the maximum value is 100!"); 
        }
       
        buff_data_motors[4] = (Byte)val_16;
        wr_eep = 1;
     }else {
        Serial_println1("  Error, the value is not correct (max=100, min=40)!");         
        return; 
     }  
  }
  
  Serial_print(buff_data_motors[4], DEC); // se solo il carattere '\n' riporta il valore in eeprom 
  Serial_println1("%"); 
  
  //--- 4  Set_Bypass  ---
  PrintFlashString(str_Bypass_4_Menu,    NO_NEW_LINE);
  nb = wait_response_by_terminal();

  if ((nb>=1) && (buff_RS232_rx[0] != 0x0A))  
  {
      if((buff_RS232_rx[0] != '0') && (buff_RS232_rx[0] != '1'))
      {
         Serial_println1("  Error the data set is not Correct!");
         return;
      }  
      
      if (buff_RS232_rx[0] == '0') 
        buff_data_motors[7] = 0;     // Orario
      else   
        buff_data_motors[7] = 0xFF;  // AntiOrario
      
      wr_eep = 1;  
  } 
  
  if(buff_data_motors[7])
    Serial_println1("Antiorario"); // se solo il carattere '\n' riporta il valore in eeprom 
  else 
    Serial_println1("Orario"); // se solo il carattere '\n' riporta il valore in eeprom  
  
  if(eep_read_byte(ADDR_EEP(Type_func)))  // solo se l'unità è extra
  { 
      //--- 5  Enable CAP  --- 
      PrintFlashString(str_Enab_CAP_4_Menu,    NO_NEW_LINE);
      nb = wait_response_by_terminal();
      
      buff_data_motors[8] = eep_read_byte(ADDR_EEP(msk_Enab_Fuction));   //  2 byte:  (LOW) bit[7]:CAF  | bit[6]:CAP  | bit[5]:CSF 
      buff_data_motors[9] = buff_data_motors[8];
    
      if ((nb>=1) && (buff_RS232_rx[0] != 0x0A))  
      {
          if((buff_RS232_rx[0] == 'y') || (buff_RS232_rx[0] == 'Y'))
              buff_data_motors[8] |= (1 << ENAB_CAP);
          else if ((buff_RS232_rx[0] == 'n') || (buff_RS232_rx[0] == 'N'))
              buff_data_motors[8] &= ~(1 << ENAB_CAP);
          else {
             Serial_println1("  Error the data set is not Correct!");
             return; 
          }    
      } 
      
      if(buff_data_motors[8] & (1 << ENAB_CAP))
        Serial_println1("Enable"); // se solo il carattere '\n' riporta il valore in eeprom 
      else 
        Serial_println1("Disable"); // se solo il carattere '\n' riporta il valore in eeprom  
        
      //--- 6  Enable CAF  --- 
      PrintFlashString(str_Enab_CAF_4_Menu,    NO_NEW_LINE);
      nb = wait_response_by_terminal();
      
      if ((nb>=1) && (buff_RS232_rx[0] != 0x0A))  
      {
          if((buff_RS232_rx[0] == 'y') || (buff_RS232_rx[0] == 'Y'))
              buff_data_motors[8] |= (1 << ENAB_CAF);
          else if ((buff_RS232_rx[0] == 'n') || (buff_RS232_rx[0] == 'N'))
              buff_data_motors[8] &= ~(1 << ENAB_CAF);
          else {
             Serial_println1("  Error the data set is not Correct!");
             return; 
          }    
      } 
      
      if(buff_data_motors[8] & (1 << ENAB_CAF))
        Serial_println1("Enable"); // se solo il carattere '\n' riporta il valore in eeprom 
      else 
        Serial_println1("Disable"); // se solo il carattere '\n' riporta il valore in eeprom 
      
      if(buff_data_motors[8] != buff_data_motors[9])
        eep_write_byte(ADDR_EEP(msk_Enab_Fuction),  buff_data_motors[8]);
  } 
    
  // salva la configurazione
  if(wr_eep) {
    Serial_println1(" ");
    Serial_print1("Saving new values "); 
    addr1 = ADDR_EEP(numMotors);     
    addr2 = ADDR_EEP(depotMotors);  
    for(i=addr1; i <= addr2; i++) {
        eep_write_byte(i,  buff_data_motors[i-addr1]);
        Shadow_eep[i] = buff_data_motors[i-addr1];
        delay_ms (150);
        Serial_print1("."); 
    }    
    
    i=ADDR_EEP(RotazioneBypass);
    eep_write_byte(i,  buff_data_motors[7]);
    Shadow_eep[i] = buff_data_motors[7];
    
    Serial_println1("  Done"); 
  }  
}  

//------------------------------------------------------
// Inserisci i dati relaiti alle sonde
//
//  ---[ 5  Setting NTC ]---
//   Enter the number NTC active (2, 3, 4),  Actualy is 4   
//   Digit the new number: 3 
//
//   Set the Position NTC: TFresh | TReturn | TSupply | TExuast  
//    Example 1= 0123 -> TFresh=0, TReturn=1, TSupply=2, TExuast=3"
//    Example 2= 1203 -> TFresh=1, TReturn=2, TSupply=0, TExuast=3
//
//   Digit new Position NTC (TFresh,...,TExaust):
//------------------------------------------------------
void SettingNTC()
{
  Byte nb, val, i, n;
  unsigned short addr;
  
  PrintFlashString(str_Title_5_Menu, NEW_LINE);
  Serial_println1(" ");  
    
  PrintFlashString(str_enter_NTC_5_Menu, NO_NEW_LINE);
  addr = ADDR_EEP(numNTC);  
  val = eep_read_byte(addr);
  Serial_println(val,  DEC); 
  
  PrintFlashString(str_new_NTC_5_Menu, NO_NEW_LINE);
  
  nb = wait_response_by_terminal(); 
  
  if(nb > 1) 
  {  
    for(i=0, n=0; i <nb; i++) {
      if(buff_RS232_rx[i] == ' ') 
        continue;
     
        
      if((buff_RS232_rx[i] < '0') && (buff_RS232_rx[i] > '9'))  {
         Serial_println1(" Error, the data is not number!");
         return;
      }else {
        val = buff_RS232_rx[i];
        break;
      }      
    }  
    
    val -= '0';
    
    if ((val < 2) || (val > 4)) { 
       PrintFlashString(str_error1_5_Menu, NEW_LINE); // // Error, the entered data is not a number comprised between 2 and 4"
       return; 
    }
  }
  Serial_println(val,  DEC);
  
  eep_write_byte(addr,  val);
  Shadow_eep[addr] = val;
  
  Serial_println1(" ");
  PrintFlashString(str_Info_cfg_NTC_5_Menu, NEW_LINE);
  PrintFlashString(str_ex_1_cfg_NTC_5_Menu, NEW_LINE);
  PrintFlashString(str_ex_2_cfg_NTC_5_Menu, NEW_LINE);
  Serial_println1(" "); 
  PrintFlashString(str_set_cfg_NTC_5_Menu, NO_NEW_LINE);
  
  nb   = wait_response_by_terminal(); 
  
  addr = ADDR_EEP(Posiz_NTC);  
  val   = eep_read_byte(addr);
  
  if(nb > 1) 
  {  
      for(i=0, n=0, val=0; i <nb; i++) 
      {
          if((buff_RS232_rx[i] == ' ') || (buff_RS232_rx[i] == '.') || (buff_RS232_rx[i] == ','))
            continue;
                 
          if((buff_RS232_rx[i] < '0') && (buff_RS232_rx[i] > '3'))  {
             Serial_println1("  Error, the position of NTC is not valid (min= 0, max=3)!");
             return;
          }else {
             
             val |= (buff_RS232_rx[i] - '0') << (n *2);
             n++;
          }      
          
          if(n >= 4)
            break; 
      }  
      
      if(n < 4) {
         Serial_println1(" Error, configuration is incomplete!");
         return;
      }  
  }
  
  PrintPositionNTC(val);   
  
  eep_write_byte(addr,  val);
  Shadow_eep[addr] = val;
}  

//------------------------------------------------------
// Leggi la config. del weekly
//
//------------------------------------------------------
void readWeeklyProg()
{
  int range, t;
  Byte numMaxRange,  time_start, time_stop, day, temp_08, shf, StepSpeed;
  short enab_func;
  
  PrintFlashString(str_Title_8_Menu, NEW_LINE);
  Serial_println1(" ");  
  
  enab_func   = read_word_eeprom(ADDR_EEP(Enab_Fuction));  
  
  if (enab_func & (1<<ENAB_WEEKLY)) 
     Serial_println1(" Weekly Enable:");
  else 
     Serial_println1("  Weekly NOT Enable:");   
  
  for(day= 0; day <7; day++) {
    Serial_println1(" ");
    Serial_print1("  Day ");
    Serial_println(day, DEC);
    
    numMaxRange = read_byte_eeprom(ADDR_EEP(sDayProg[day].numRange));
    
    for(range=0; range <  numMaxRange; range++) {
       time_start = read_byte_eeprom(ADDR_EEP(sDayProg[day].timeON[range]));
       time_stop  = read_byte_eeprom(ADDR_EEP(sDayProg[day].timeOFF[range]));
       Serial_print1("Range(");
       Serial_print((range+1), DEC);
       
       Serial_print1(") Ton: ");
       t = time_start / 2;       
       Serial_print(t, DEC);
       if(time_start & 0x01)
         Serial_print1(":30 ");
       else 
         Serial_print1(":00 "); 
       
       Serial_print1(" Toff: ");
       
       t = time_stop / 2;       
       Serial_print(t, DEC);
       if(time_stop & 0x01)
         Serial_print1(":30 ");
       else 
         Serial_print1(":00 "); 
       
       Serial_print1(" Speed: ");
       //  ConfigSpeed -> bit[7,6]:Step Speed range 4 | bit[5,4]:Step Speed range 3 | bit[3,2]:Step Speed range 2 | bit[1,0]:Step Speed range 1. 
       temp_08 = read_byte_eeprom(ADDR_EEP(sDayProg[day].ConfigSpeed));
       shf = (range*2);
       StepSpeed = (temp_08 >> shf) & 0x03;  
       Serial_println(StepSpeed, DEC);   

    }  
  }            
}  

//-----------------------------------------------------
//  ---[ 6  Menu manual Testing ]---
//   [0] Return Main Menu 
//   [1] Test Bypass 
//   [2] Test OUTPUT 
//   [3] Test INPUT 
//   [4] Test Motors 
//   [5] Test Uart 1 
//
//  Select Test(0,..,5):
//-----------------------------------------------------
void PrintMenuManTesting()
{  
  PrintFlashString(str_Title_6_Menu,  NEW_LINE);
  Serial_println1(" ");
  PrintFlashString(str_row0_6_Menu,   NEW_LINE);    
  PrintFlashString(str_row1_6_Menu,   NEW_LINE);     
  PrintFlashString(str_row2_6_Menu,   NEW_LINE);
  PrintFlashString(str_row3_6_Menu,   NEW_LINE);
  PrintFlashString(str_row4_6_Menu,   NEW_LINE);
  PrintFlashString(str_row5_6_Menu,   NEW_LINE);
  PrintFlashString(str_row6_6_Menu,   NEW_LINE);
}  


//-----------------------------------------------------
//
//-----------------------------------------------------
void manualTesting()
{
   Byte nb, i;
   int ret_val;
    
   Serial_println1(" ");
   PrintMenuManTesting();
   printPrompMenu6();
   
   do {     
     nb = wait_response_by_terminal(); 
   
     if((nb==1) && (buff_RS232_rx[0] == '\n')) {
       Serial_println1(" "); 
       Serial_println1(" ");
       PrintMenuManTesting();
       printPrompMenu6();
     }else {
       // stampiamo il carattere
       for(i=0; i < nb; i++) {
          Serial_print(buff_RS232_rx[i], BYTE);
          Serial_print1(" ");
       }  
            
       switch (buff_RS232_rx[0]){
          case '0':  
             Stampa_menu_config();
          return;
          
          case '1':  //   [1] Test Bypass       
             test_bypass();  
          break;
          
          case '2':  //   [2] Test Input/Output
             test_IO();  
          break;
          
          case '3':  //   [3] Test SleepMode
             testSleepMode();
          break;
          
          case '4':  //   [4] Test Motors 
             testMotors();
          break;
          
          case '5':  //   [5] Test Uart 1 
             testUart1();
          break;
          
          case '6':
             testI2C();
          break;
          
                  
          default:               
             Serial_print1(" Error, the number is not correct!");
             Serial_println1(" ");
             PrintMenuManTesting();
          break;
                       
       }
       
       printPrompMenu6();   
     }  
              
      
     delay_ms (50);  
   
   }while(1);
  
}  

/* --------------------------------------------------------------- *
    crc_16 calcolo del crc_16
    Parametri di ingresso:
    buffer: stringa di caratteri di cui calcolare il CRC-16
    length: numero di bytes della stringa
    Questa funzione ritorna il valore di CRC-16
 * --------------------------------------------------------------- */
unsigned short CRC_16 (unsigned char *buffer, unsigned int length)
{
    unsigned short i, j, temp_bit, temp_int, crc;
    crc = 0xFFFF;

     for ( i = 0; i < length; i++ ) {
        temp_int = (unsigned char) *buffer++;
        crc ^= temp_int;
        for ( j = 0; j < 8; j++ ) {
            temp_bit = crc & 0x0001;
            crc >>= 1;
            if ( temp_bit != 0 )
                crc ^= 0xA001;
        }
    }
    return (crc);
}


//================================================================================
// Da una stringa di 55 byte, configuro l'unità (vedi protocollo 
// Configurazione Unità 1.0).
//
//-----------+--------+---------------------------------------
// I_BUFF_RX  NumBYTE       DESCRIPTION
//-----------+--------+---------------------------------------
//     0        2	start message
//     2	1	Version Protocol
//     3	1	Type Func. Basic = 0, Extra=1
//     4       18	Serial Number
//    22	1	 * none*
//    23	1	Chiusura Capitolo
//    24	1	Numbers Motors
//    25	2	Numbers Pulse for Round
//    27	1	Enab. Table CAF
//    28	1	Enable Table CAP
//    29	3	Depotenziamento
//    32	1	Rotazione Bypass (0= Orario, 1=Anti..)
//    33	1	Inseriamo abilitazione del DPPv2 ( =='0' assente ; != '0'  presente)
//    34        2       * none *
//    36	1	Chiusura Capitolo
//    37	1	Numero di NTC
//    38	1	Posizione Input Tfresh   (0 default)
//    39	1	Posizione Input Treturn  (1 default)
//    40	1	Posizione Input Tsupply  (2 default)
//    41	1	Posizione Input Texaust  (3 default)
//    42        1       Sensori RH/CO2/VOC interni.
//    43	1	 * none*
//    44	1	Chiusura Capitolo
//    45	8	* none *
//    53	2	CRC 16bit  (53:LO , 54:HI)
//
// Example string: sm001234003501131450010;202YY0500000;4012310;00000000;;
//                 sm011870002502614380010;204YY1001000;4012310;00000000;;
//                 sm011906009302614410010;202NN0660000;4012300;00000000;;
//                 sm116994003801418140010;204YY0800100;4012300;00000000;;
//
//================================================================================
void writeConfigUnit()
{
   int i, nb;
   unsigned short crc16_send = 0xFFFF, crc16_calc = 0xFFFF, addr; 
   Byte val;
  
   nb = wait_response_by_terminal();
      
   //-- -- -- -- -- -- -- -- -- -- -- -- -- --
   //  Controlliamo la validità del messaggio.
   //-- -- -- -- -- -- -- -- -- -- -- -- -- --
   // 1^ Controllo:  star messagge   
   if ((buff_RS232_rx[0] != 's') || (buff_RS232_rx[1] != 'm')) {     
     Serial_println1("Error: Start Message");//PrintFlashString(str_Error_9_Menu,   NEW_LINE);    
     return;
   }  
   
   // 2^ Controllo: i 3 Byte: 23^, 36^, 44^ devono essere il carattere ';'
   if ((buff_RS232_rx[23] != ';') || (buff_RS232_rx[36] != ';') || (buff_RS232_rx[44] != ';')) { 
     Serial_println1("Error: Carattere Separatore ';'"); //PrintFlashString(str_Error_9_Menu,   NEW_LINE);    
     return;
   }  
   
   // 3^ Controllo: CRC16
   if ((buff_RS232_rx[53] != ';') && (buff_RS232_rx[54] != ';')) 
   {
       crc16_send = (buff_RS232_rx[54] << 8) & 0xFF00;
       crc16_send |= buff_RS232_rx[53];
       
       crc16_calc = CRC_16 (&buff_RS232_rx[0], 53);
       
       if(crc16_send != crc16_calc)
       {
         Serial_println1("Error: CRC16");    
         return;
       }  
   }

   
   //-- -- -- -- -- -- -- -- -- -- -- -- --
   // Iniziamo a configurare l'unità
   //-- -- -- -- -- -- -- -- -- -- -- -- --
   //--- Typer func. (1=EXTRA / 0=BASIC)---
   val = (buff_RS232_rx[3] - '0') & 0x01;
   
   write_byte_eeprom(ADDR_EEP(Type_func), val);
   if (val) {
     val = ENAB_FUNC_EXTRA;
     //--- Enab. Funz. CAF & CAP ---
     if ((buff_RS232_rx[27] == 'N') || (buff_RS232_rx[27] == 'n'))  
       val &= ~(1 << ENAB_CAF);
   
     if ((buff_RS232_rx[28] == 'N') || (buff_RS232_rx[28] == 'n'))  
       val &= ~(1 << ENAB_CAP);
     
     write_byte_eeprom(ADDR_EEP(msk_Enab_Fuction), val);
     
   }else 
     write_byte_eeprom(ADDR_EEP(msk_Enab_Fuction),  ENAB_FUNC_BASIC);
     
   
   check_control_speed_func();  // verifichiamo se è corretto l'impostazione CAP/CAF in base al type_func.  
          
   //--- Serial number (18 caratteri) ---
   addr = ADDR_EEP(SerialString[0]);       
   for(i=0; i <18; i++)
     write_byte_eeprom((addr+i), buff_RS232_rx[4+i]);
     
   //--- Numero dei motori ---
   addr = ADDR_EEP(numMotors);
   val  = (buff_RS232_rx[24] -'0');
   
   if ((val != 2) && (val != 4) && (val != 6))
   {
     PrintFlashString(str_Error_9_Menu,   NEW_LINE);    
     return;
   }     
   write_byte_eeprom(addr, val);

   //--- Numero di pulsazioni per giro dei motori ---
   addr = ADDR_EEP(numPulseMotors); 
   val  = (buff_RS232_rx[25] - '0')*10 + (buff_RS232_rx[26] - '0');
   write_byte_eeprom(addr, val);
  
   
   //--- Depotenziamento ---
   addr = ADDR_EEP(depotMotors); 
   val  = ((buff_RS232_rx[29] - '0')*100) + ((buff_RS232_rx[30] - '0') * 10) + (buff_RS232_rx[31] - '0');
   if(val > 100)
     val = 100;
   else if ( val < 40)
     val = 40;  
   write_byte_eeprom(addr, val);  
 
   //--- Rotazione Bypass ---
   addr = ADDR_EEP(RotazioneBypass); 
   if(buff_RS232_rx[32] == '0')
      write_byte_eeprom(addr, 0);        // Orario
   else 
      write_byte_eeprom(addr, 0xFF);     // AntiOrario
      
   //--- DPP v2 ---
   addr = ADDR_EEP(AccessoyHW[1]);
   if(buff_RS232_rx[33] == '0')
     {
      val =  read_byte_eeprom(addr);
      val &= ~(1<<2);
      write_byte_eeprom(addr, val);      // DPP NON PRESENTE
      }
   else 
     {
      val =  read_byte_eeprom(addr);
      val |= (1<<2);
      write_byte_eeprom(addr, val);      // DPP PRESENTE
      }

   
   //--- Numero NTC ---
   addr = ADDR_EEP(numNTC);
   val  = buff_RS232_rx[37] - '0';
   if(val > 4)
     val = 4;
   else if(val < 2)
     val = 2;      
   write_byte_eeprom(addr, val);  
  
   //--- Posizione NTC ---
   // bit 7,6 = Exuast, bit 5,4 = Supply, bit 3,2 = Return, bit 1,0 = Fresh
   addr = ADDR_EEP(Posiz_NTC); 
   val = 0;
   // TFresh bit: 1,0
   val |= (buff_RS232_rx[38] - '0') & 0x03;
   // TReturn bit: 3,2
   val |= (((buff_RS232_rx[39] - '0') & 0x03) << 2);
   // TSupply bit: 5,4
   val |= (((buff_RS232_rx[40] - '0') & 0x03) << 4);
   // TExaust bit: 7,6
   val |= (((buff_RS232_rx[41] - '0') & 0x03) << 6);
   write_byte_eeprom(addr, val); 
   
   //--- Config. Sonde Analogiche RH/CO2/VOC ---
   // List AccessoyHW[2]:  P2CO2, P1CO2,  EBPD,  P2RH,  P1RH,   SSR, P1VOC, -----, (LSB) 
   addr = ADDR_EEP(AccessoyHW[2]);
   val =  read_byte_eeprom(addr); 
   val &= 0b00100101;
   
   switch(buff_RS232_rx[42])  
   {
       // Sensore P1RH Abilitato 
       case '1': val |= 0b00001000; break;       
       // Sensore P1CO2 Abilitato      
       case '2': val |= 0b01000000; break;
       // Sensore P1VOC Abilitato       
       case '3': val |= 0b00000010; break;
       // Sensori P1RH & P1CO2  Abilitati      
       case '4': val |= 0b01001000; break; 
       // Sensori P1RH & P1VOC  Abilitati      
       case '5': val |= 0b00001010; break;
   }
   write_byte_eeprom(addr, val);    
   
   
   // Verifica se vi è inserito anche il IPEHD
   // List AccessoyHW[0]:    DXD,   BPD,   AWP,   CWD,   EHD,   HWD,  PHWD,  PEHD  (LSB) 
   addr = ADDR_EEP(AccessoyHW[0]);
   val =  read_byte_eeprom(addr); 
   val &= 0xFE;
   
   if(buff_RS232_rx[43] == '1')  
     val |= 0x01;

   write_byte_eeprom(addr, val);
   
   // Accessory da rilevare se connessi. Azzero il byte degli accessori
   // List AccessoyHW[3]:  -----, -----,  FLW2,  FLW1,  EXT4,  EXT3,  EXT2,  EXT1, (LSB)
   addr = ADDR_EEP(AccessoyHW[3]);  
   write_byte_eeprom(addr, 0x00);   
   
   //---- aggiorniamo l'eeprom ----
   for(i=0; i <= 90; i++) {
      if(Shadow_eep_preview[i] != Shadow_eep[i]) {
         eep_write_byte(i,  Shadow_eep[i]); 
         Shadow_eep_preview[i]  = Shadow_eep[i];
      }         
   }  
   
   Serial_println1("Write Config Unit is Successfull!");
   Serial_println1(" ");
    
   delay_ms (250);
    
   read_cfg_data();
   
}  


/****************************************************************************************
  Name: shell_config_menu
 
  Description: La funzione serve per configurare il modulo RD ed interrogarlo  
 
 ****************************************************************************************/
void shell_config_menu(Byte dec_sec)
{
   int i, exit_menu = 0; 
   Byte press_key = 0;
   Byte buff[64], sn;
   int val = 0, nbyte;
   Byte val_sn[8], i_sn=0, cifre = 1;
   Byte ins_error = 0;

   //digitalWrite(pSwitchOFF_KTS, LOW);   // KTS OFF
   ioport_set_pin_level(pSwitchOFF_KTS, IOPORT_PIN_LEVEL_LOW);
   
   // scarichiamo prima il buffer da schifezze
   press_key = Serial_available(); 
   if(press_key){
      // scarichiamo il buffer       
     for(i=0; i< press_key; i++) 
        buff[0] = Serial_read();   
   }  
   
   Serial_println1(" "); 
   delay_ms (10);
   Serial_print1("RD02 FW:");  // "RD02 FW: 2.09"
   Serial_print(VERSION_FW[0], BYTE);
   Serial_print(VERSION_FW[1], BYTE); 
   Serial_print(VERSION_FW[2], BYTE); 
   Serial_print(VERSION_FW[3], BYTE); 
   Serial_println(VERSION_FW[4], BYTE);    
   Serial_print1("Press [KEY] and [SEND] to show Config Menu ");  
   
   for(i=0; i < dec_sec; i++)
   { 
     delay_ms (100);
     
     press_key = Serial_available();     
     
     if(i == 5)
       //digitalWrite(pBuz,  LOW);
	   ioport_set_pin_level(pBuz, IOPORT_PIN_LEVEL_LOW);
     
     if(press_key) { 
       Serial_println1(" OK");
       break;
     }else {
        if(i&0x01)
           Serial_print1(". ");
     } 
   }  
   
   //digitalWrite(pBuz,LOW);
   ioport_set_pin_level(pBuz, IOPORT_PIN_LEVEL_LOW);
  
   if(press_key)
   {
     // E' stato premuto un tasto, andiamo a stampare il menu  
     Stampa_menu_config();      
     Stampa_Prompt();    
     
     delay_ms (100);
     
     // scarichiamo il buffer       
     for(i=0; i< press_key; i++) {
        buff[i] = Serial_read();  
        buff[i] = 0;
     }   
     
	 wdt_restart(WATCH_DOG);
	         
     while (!exit_menu)
     {
          
         delay_ms (50);
         
		 wdt_restart(WATCH_DOG);
		 
         press_key = Serial_available();  
         
         if(press_key) {
           
           delay_ms (50);
           // scarichiamo il buffer
           //press_key = Serial_available();  
           
           for(i=0; i< press_key; i++) { 
              buff[i] = Serial_read();  
              Serial_print(buff[i], BYTE); 
           }
           
           Serial_println1(" ");           
           
          
           switch(buff[0])
           {                 
             /**  0  Exit menu, and Starting Prog. **/
             case '0':  
                Serial_println1(" ");
                Serial_println1("Starting program");  
                exit_menu = 1; 
             return;
           
             /**  1  Write eeprom default  **/
             case '1': 
                WriteEepromDefault();
             break;

             /**  2  Read Configuration Data **/
             case '2':
                read_cfg_data();           
             break;
             
             /**  3  Enter Serial Number **/
             case '3':
                Set_SerialNumb();              
             break;
             
             /** 4  Set Data Motors  **/
             case '4':
                Set_dataMotors();
             break;
                          
             /** 5  Set Number NTC (2-4) **/
             case '5':
                SettingNTC();
             break;  
  
             /** 6  Menu manual Testing **/
             case '6':
                manualTesting(); 
             break;        
             
             /** 7 Start AutoDiagnostics **/
             case '7':

             break;
             
             case '8':
                readWeeklyProg();
             break;
             
              case '9':
                writeConfigUnit();
             break;
             
             case '\n':
                Stampa_menu_config(); 
             break;

             default:
                Serial_println1("Sorry, the function Select is not present");  
             break;
         }  
         
          // ripuliamo il buffer
          for(i=0; i< press_key; i++) 
             buff[i] = 0;  
          
          if(!exit_menu) 
            Stampa_Prompt();  

       }
                  
       press_key = 0;
     }
   }
   
   Serial_println1("Starting program");   
     
}

/***********
 *  E O F
 ***********/