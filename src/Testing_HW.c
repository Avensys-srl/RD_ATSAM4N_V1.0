/*
 * Testing_HW.cpp
 *
 *  Author: M.Colmone
 */ 

#include "asf.h"
#include "common/build_config.h"

#include "Protocol_Ser1.h"
#include "data.h"
#include "eeprom_map.h"
#include "alarm.h"

//------------ macro ------------------------------
#define N_TOOGLE_BIT_I2C  10

//--------------------------------
#define NEW_LINE      'n'
#define NO_NEW_LINE    0

//------------------ Stati BYPASS ---------------------
#define ANTIORARIO       0xFF
#define ORARIO           0
#define CLOSE_BPD        1   // bypass chiuso, il flusso dell'aria passa dallo scambiatore
#define OPEN_BPD         0   // bypass aperto 100%, il flusso dell'aria passa solo dal canale Bypass.

#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#define bitWrite(value, bit, bitvalue) ((bitvalue) ? bitSet(value, bit) : bitClear(value, bit))

//-------------------------------------------------
//  DEFINIZIOEN STRINGHE PER FUNZ. TEST
//-------------------------------------------------
//----------- [6.1]  Test Bypass ----------------------
const char str_1_test_BDP[] = {"  ---< Test Bypass >---"};
const char str_2_test_BDP[] = {" Bypass  OPEN "};
const char str_3_test_BDP[] = {" Bypass CLOSE "};

//----------- [6.2]  Test IO ----------------------
const char str_1_test_IO[] = {"  ---< Test Input/Output >---"};
const char str_2_test_IO[] = {"Connect the outputs of the X4 connector "};
const char str_3_test_IO[] = {"(EB: AELEBDRJ02) as follows:"};
const char str_4_test_IO[] = {" [10] + [8] + [7] = (+12V)"};
const char str_5_test_IO[] = {" [11] + [6]       = (IN_1)"};
const char str_6_test_IO[] = {"  [9] + [5]       = (IN_2)"};
const char str_7_test_IO[] = {"press enter when you ready!"};
const char str_8_test_IO[] = {"1/2-Relays OFF: "};
const char str_9_test_IO[] = {"2/2-Relays  ON: "};

//------------ [6.3] Test SleepMode ---------------
const char str_1_test_SM[] = {"  ---< Test Seep Mode >---"};
const char str_2_test_SM[] = {"1/2-IDLE  MODE, Relay Power  ON, Read status Motors"};
const char str_3_test_SM[] = {"2/2-SLEEP MODE, Relay Power OFF, Read status Motors"};

//------------ [6.4] Test Motors ---------------
const char str_1_test_motors[] = {"  ---< Test Motors >---"};
const char str_2_test_motors[] = {"Number of Motors configured: "};
const char str_3_test_motors[] = {"1/3: Beginning at the minimum fans speed,"};
const char str_4_test_motors[] = {"2/3: Change the fans speed at the medium speed,"};
const char str_5_test_motors[] = {"3/3: Change the fans speed at the max speed,"};

//----------- [6.5]  Test Uart 1 ------------------
const char str_1_test_uart1[] = {"  ---< Test UART1 >---"};
const char str_HW_Testing[]   = {"Remember to HW connect RJ45 output, before to star Test"};
const char str_msg_uart1[]    = {"String test to check the correct operation of the UART1"};

//----------- [6.6]  Test I2C Bus ------------------
const char str_1_test_I2C[] = {"  ---< Test I2C >---"};
const char str_2_test_I2C[] = {"TEST AT  1 KHZ:"};
const char str_3_test_I2C[] = {"TEST AT 50 KHZ:"};

extern Byte RotaBypass;
extern Byte PositionBypass;
extern unsigned int  numStepBypass;
extern SDATA  sData;

//-----------------------------------------------------
// Variabili di appoggio per TEST IPEHD
//-----------------------------------------------------

extern bool flag_ipehd;
extern int counter_ipehd;

extern int PrintFlashString(const char *str_f, char newline);
extern int startMotorBPD();
extern void Ctrl_FunzionamentoBPD();
extern unsigned analogRead ( Byte Channel_Num );
extern void polling_CTRL_FAN(void);
extern int intercepts_messages_serial_2();
extern Byte eep_read_byte(unsigned int addr);
extern int wait_response_by_terminal();

//-----------------------------------------------------
//    Stampa risultato test
//-----------------------------------------------------
void print_result_test(Byte esito)
{
	Serial_print1("Result Test ");
	delay_ms (150);
	Serial_print1(".");
	delay_ms(150);
	Serial_print1(".");
	delay_ms(150);
	
	if(esito == 'F')
	Serial_println1(".FAILED!");
	else
	Serial_println1(".PASSED");
	
	delay_ms(750);
	
	wdt_restart(WATCH_DOG);
}

//-----------------------------------------------------
//        [1]  Test Bypass
// Viene richiamata dalla shell, si compone in 2 fasi:
//   1: Apre il bypass e verifica l'assorbimento
//   2: Chiude il bypass e verifica l'assorbimento
//-----------------------------------------------------
void test_bypass()
{
	int esito =0;
	unsigned short valueBypass, val_16;
	float fval;
	Byte nTest = 1;
	
	//-------------------------------
	//---- step 1: open Bypass   ----
	//-------------------------------
	Serial_println1(" ");
	PrintFlashString(str_1_test_BDP,  NEW_LINE);
	
	for(nTest = 1; nTest < 6; nTest++)
	{
		PrintFlashString(str_2_test_BDP,  NO_NEW_LINE);
		Serial_print(nTest, DEC);
		Serial_print1(" ");
		
		PositionBypass = OPEN_BPD;
		numStepBypass  = 0;
		
		if(read_byte_eeprom(ADDR_EEP(RotazioneBypass)) == ORARIO)
		RotaBypass = ANTIORARIO;
		else
		RotaBypass = ORARIO;
		
		// segnaliamo che il bypass sta iniziando il cambiamento di stato (OPEN->CLOSE 0 CLOSE->OPEN).
		// quindi il regolatore portera' la velocita' al minimo.
		sData.status_unit &= MSK_STS_BYPASS;
		sData.status_unit |= (1 << POS_BIT_BYPASS_RUN);
		startMotorBPD();
		
		do {
			Ctrl_FunzionamentoBPD();
			
			if((numStepBypass % 300)==0) {
				Serial_print1(".");
				delay_ms (10);
			}else
			delay_ms (5);
			
			wdt_restart(WATCH_DOG);
			#ifdef WORK_WDT
			//ClearWDT();  // teniamo vivo il WacthDog
			#endif
			
		}while(sData.status_unit & (1 << POS_BIT_BYPASS_RUN));
		
		Serial_print1(" Imin= ");
		Serial_print(sData.absorption_bypass_min, DEC);
		Serial_print1("mA,  Imax= ");
		Serial_print(sData.absorption_bypass_max, DEC);
		Serial_print1("mA");
		
		wdt_restart(WATCH_DOG);
		
		if(sData.absorption_bypass_min < 30) {
			Serial_print1(" ,ERROR!");
			esito = 'F';
			//digitalWrite(pBuz,  HIGH);
			ioport_set_pin_level(pBuz, IOPORT_PIN_LEVEL_HIGH);
			delay_ms (400);
			//digitalWrite(pBuz,  LOW);
			ioport_set_pin_level(pBuz, IOPORT_PIN_LEVEL_LOW);
			delay_ms (400);
			//digitalWrite(pBuz,  HIGH);
			ioport_set_pin_level(pBuz, IOPORT_PIN_LEVEL_HIGH);
			delay_ms (400);
			//digitalWrite(pBuz,  LOW);
			ioport_set_pin_level(pBuz, IOPORT_PIN_LEVEL_LOW);
			break; // esci dalla loop
		}
		
		wdt_restart(WATCH_DOG);
		delay_ms (2500);
		
		//-------------------------------
		//---- step 2: Close Bypass  ----
		//-------------------------------
		Serial_println1(" ");
		PrintFlashString(str_3_test_BDP,  NO_NEW_LINE);
		Serial_print(nTest, DEC);
		Serial_print1(" ");
		
		PositionBypass = CLOSE_BPD;
		numStepBypass  = 0;
		
		if(read_byte_eeprom(ADDR_EEP(RotazioneBypass)) == ORARIO)
		RotaBypass = ORARIO;
		else
		RotaBypass = ANTIORARIO;
		
		// segnaliamo che il bypass sta iniziando il cambiamento di stato (OPEN->CLOSE 0 CLOSE->OPEN).
		// quindi il regolatore portera' la velocita' al minimo.
		sData.status_unit &= MSK_STS_BYPASS;
		sData.status_unit |= (1 << POS_BIT_BYPASS_RUN);
		startMotorBPD();
		
		valueBypass = 0;
		do {
			Ctrl_FunzionamentoBPD();
			
			if((numStepBypass % 300)==0) {
				Serial_print1(".");
				delay_ms (10);
			}else
			delay_ms (2);
			wdt_restart(WATCH_DOG);
			
		}while(sData.status_unit & (1 << POS_BIT_BYPASS_RUN));
		
		Serial_print1(" Imin= ");
		Serial_print(sData.absorption_bypass_min, DEC);
		Serial_print1("mA,  Imax= ");
		Serial_print(sData.absorption_bypass_max, DEC);
		Serial_print1("mA");
		
		// absorption_bypass = (805 * 3)/11 = 220mA;
		if(sData.absorption_bypass_max >= 220) {
			Serial_print1(" ,ERROR!");
			esito = 'F';
			//digitalWrite(pBuz,  HIGH);
			ioport_set_pin_level(pBuz, IOPORT_PIN_LEVEL_HIGH);
			delay_ms (400);
			//digitalWrite(pBuz,  LOW);
			ioport_set_pin_level(pBuz, IOPORT_PIN_LEVEL_LOW);
			delay_ms (400);
			//digitalWrite(pBuz,  HIGH);
			ioport_set_pin_level(pBuz, IOPORT_PIN_LEVEL_HIGH);
			delay_ms (400);
			//digitalWrite(pBuz,  LOW);
			ioport_set_pin_level(pBuz, IOPORT_PIN_LEVEL_LOW);
			break;
		}
		
		wdt_restart(WATCH_DOG);
		Serial_println1(" ");
		delay_ms (3000);
		wdt_restart(WATCH_DOG);
	}
	
	
	delay_ms (100);
	print_result_test(esito);
}


//-----------------------------------------------------
//               [2] test IO
//   -- Test Input/Output --
//   Connect the outputs of the X4 connector
//   (EB: AELEBDRJ02) as follows:
//   [10] + [8] + [7] : (+12V)
//   [11] + [6]       : (IN_1)
//    [9] + [5]       : (IN_2)
//
//  press enter when ready.
//
//  1/2-Relay off: INP_1= 0.0V   INP_2= 0.0V
//  2/2-Relay on:  INP_1=11.5V   INP_2=11.4V
//  Result Test  ...OK/FAILED!
//-----------------------------------------------------
void test_IO()
{
	Byte nb, esito = 'O';
	float fval[2];
	
	Serial_println1(" ");
	PrintFlashString(str_1_test_IO,  NEW_LINE);
	PrintFlashString(str_2_test_IO,  NEW_LINE);
	PrintFlashString(str_3_test_IO,  NEW_LINE);
	PrintFlashString(str_4_test_IO,  NEW_LINE);
	PrintFlashString(str_5_test_IO,  NEW_LINE);
	PrintFlashString(str_6_test_IO,  NEW_LINE);
	delay_ms (100);
	Serial_println1(" ");
	PrintFlashString(str_7_test_IO,  NEW_LINE);  // press enter when ready.
	
	nb = wait_response_by_terminal();
	
	
	//digitalWrite(pOut1,  LOW);
	ioport_set_pin_level(pOut1, IOPORT_PIN_LEVEL_LOW);
	//digitalWrite(pOut2,  LOW);
	ioport_set_pin_level(pOut2, IOPORT_PIN_LEVEL_LOW);
	delay_ms (50);
	Serial_println1(" ");
	PrintFlashString(str_8_test_IO,  NO_NEW_LINE); // 1/2-Relay off:
	delay_ms (700);
	fval[0] = analogRead(pAnInp1);
	fval[1] = analogRead(pAnInp2);
	fval[0] /= 100;
	fval[1] /= 100;
	
	Serial_print1(" INP1= ");
	Serial_print(fval[0], 1);
	Serial_print1("V INP2= ");
	Serial_print(fval[1], 1);
	Serial_print1("V");
	
	wdt_restart(WATCH_DOG);
	
	if((fval[0] > 1.0) || (fval[1] > 1.0)) {
		Serial_print1("  ,ERROR!");
		esito = 'F';
	}
	
	Serial_println1(" ");
	delay_ms (700);
	//digitalWrite(pOut1,   HIGH);
	ioport_set_pin_level(pOut1, IOPORT_PIN_LEVEL_HIGH);
	//digitalWrite(pOut2,   HIGH);
	ioport_set_pin_level(pOut2, IOPORT_PIN_LEVEL_HIGH);
	delay_ms (50);
	PrintFlashString(str_9_test_IO,  NO_NEW_LINE); // 1/2-Relay off:
	
	delay_ms (2300);
	fval[0] = analogRead(pAnInp1);
	fval[1] = analogRead(pAnInp2);
	fval[0] /= 100;
	fval[1] /= 100;
	
	Serial_print1(" INP1= ");
	Serial_print(fval[0], 1);
	Serial_print1("V INP2= ");
	Serial_print(fval[1], 1);
	Serial_print1("V");
	
	if((fval[0] < 9.5) || (fval[1] < 9.5)) {
		Serial_print1("  ,ERROR!");
		esito = 'F';
	}
	
	wdt_restart(WATCH_DOG);
	
	Serial_println1(" ");
	delay_ms (500);
	//digitalWrite(pOut1,   LOW);
	ioport_set_pin_level(pOut1, IOPORT_PIN_LEVEL_LOW);
	//digitalWrite(pOut2,   LOW);
	ioport_set_pin_level(pOut2, IOPORT_PIN_LEVEL_LOW);
	
	print_result_test(esito);
	
}

//-----------------------------------------------------
//         [3]  Test Seep Mode
// 1/3-IDLE MODE,  Relay Power ON, Read status Motors
// 2/3-SLEEP MODE, Relay Power OFF, Read status Motors
// 3/3-Relay Power ON
//-----------------------------------------------------
void testSleepMode()
{
	int ret_val, i, esito =0;
	
	Serial_println1(" ");
	PrintFlashString(str_1_test_SM,  NEW_LINE);
	delay_ms (500);
	//digitalWrite(pPowerMotors, HIGH); // Diseccita il rele' e metti 'elettronica in SLEEP NO SW
	ioport_set_pin_level(pPowerMotors, IOPORT_PIN_LEVEL_HIGH);
	delay_ms (100);
	
	PrintFlashString(str_2_test_SM,  NO_NEW_LINE);  // IDLE MODE
	delay_ms (1500);
	
	wdt_restart(WATCH_DOG);
	
	sData.speed_motors_R = 0;
	sData.speed_motors_F = 0;
	//  [1,0] LED BYPASS    (00 = OFF, 01=Toggle, 10= ON, 11= none)
	//  [3,2] LED ALARM     (00 = OFF, 01=Toggle, 10= ON, 11= none)
	//  [5,4] LED MOTORS R  (00 = OFF, 01=Toggle, 10= ON, 11= none)
	//  [7,6] LED MOTORS F  (00 = OFF, 01=Toggle, 10= ON, 11= none)
	sData.status_Leds_ctrlFan = 0;
	
	// Attendiamo la risposta CTRL_FAN
	for(i=0; i < 5; i++) {
		// inviamo il comando alla CTRL_FAN
		polling_CTRL_FAN();
		delay_ms (50);
		ret_val = intercepts_messages_serial_2();
		delay_ms (20);
		ret_val = intercepts_messages_serial_2();
		if(ret_val)
		break;
		delay_ms (300);
		
		wdt_restart(WATCH_DOG);
	}
	
	if(ret_val == 0)
	{
		Serial_println1(" ");
		Serial_println1("Error, Link KO CTRL_FAN!");
		delay_ms (800);
		return;
	}
	
	
	// Status Motors R & F,
	// bit: 7.none | 6.AlarmM3 | 5.AlarmM2 | 4.AlarmM1 | 3.none | 2.PowerON_M3 | 1.PowerON_M2 | 0.PowerON_M1
	if(((sData.speed_motors_R & 0x07) == 0) && ((sData.speed_motors_R & 0x07) == 0)) {
		Serial_print1("  ,ERROR NO Power!");
		esito = 'F';
	}
	
	Serial_println1(" ");
	delay_ms (400);
	//digitalWrite(pPowerMotors,           HIGH);  // ci si parta a 6.3V con il rele in off
	ioport_set_pin_level(pPowerMotors, IOPORT_PIN_LEVEL_HIGH);
	delay_ms (100);
	PrintFlashString(str_3_test_SM,  NO_NEW_LINE);  // SLEEP MODE
	delay_ms (3000);
	
	wdt_restart(WATCH_DOG);
	
	// Attendiamo la risposta CTRL_FAN
	for(i=0; i < 4; i++) {
		// inviamo il comando alla CTRL_FAN
		polling_CTRL_FAN();
		delay_ms (60);
		ret_val = intercepts_messages_serial_2();
		delay_ms (20);
		ret_val = intercepts_messages_serial_2();
		if(ret_val)
		break;
		delay_ms (300);
	}
	
	wdt_restart(WATCH_DOG);
	
	// Status Motors R & F:  bit.. 7.none | 6.AlarmM3 | 5.AlarmM2 | 4.AlarmM1 | 3.none | 2.PowerON_M3 | 1.PowerON_M2 | 0.PowerON_M1
	if(((sData.speed_motors_R & 0x07) != 0) || ((sData.speed_motors_R & 0x07) != 0)){
		Serial_print1("  ,ERROR The Relay Not Open!");
		esito = 'F';
	}
	
	Serial_println1(" ");
	delay_ms (100);
	//digitalWrite(pPowerMotors, LOW); // Diseccita il rele' e metti 'elettronica in SLEEP NO SW
	ioport_set_pin_level(pPowerMotors, IOPORT_PIN_LEVEL_LOW);
	delay_ms (100);
	
	print_result_test(esito);
}


//--------------------------------------------------------------
//           [4]  test Motors
//
//--------------------------------------------------------------
void testMotors(void)
{
	Byte esito = 0, sec_wait, nMotorsCfg, ckMotorsOn, rpmFail[6];
	int i, step_test =0, ret_val, ret_val_1, sglVF, sglVR;
	
	Serial_println1(" ");
	PrintFlashString(str_1_test_motors,   NEW_LINE);
	
	nMotorsCfg = eep_read_byte(ADDR_EEP(numMotors));
	PrintFlashString(str_2_test_motors,   NO_NEW_LINE);
	Serial_println(nMotorsCfg, DEC);
	
	for(i=0; i<6; i++)
	rpmFail[i] = 0;
	
	// 1. Do il comando di accensione motori e rele power ON
	for(step_test =0;  step_test < 3; step_test++)
	{
		Serial_println1(" ");

		if(step_test == 0) {
			PrintFlashString(str_3_test_motors,   NO_NEW_LINE);
			sData.speed_motors_R = 52;
			sData.speed_motors_F = 52;
			sec_wait = 20;
			}else if (step_test == 1) {
			PrintFlashString(str_4_test_motors,   NO_NEW_LINE);
			sData.speed_motors_R = 127;
			sData.speed_motors_F = 127;
			sec_wait = 15;
			}else if (step_test == 2) {
			PrintFlashString(str_5_test_motors,   NO_NEW_LINE);
			sData.speed_motors_R = 255;
			sData.speed_motors_F = 255;
			sec_wait = 15;
		}
		
		Serial_print1(" waiting .");
		
		for(i=0; i< sec_wait; i++) {
			// inviamo il comando alla CTRL_FAN per 10 volte in modo da valutare la CTRL_FAN
			delay_ms (930);
			//  [1,0] LED BYPASS    (00=OFF, 01=Toggle, 10=ON, 11=none)
			//  [3,2] LED ALARM     (00=OFF, 01=Toggle, 10=ON, 11=none)
			//  [5,4] LED MOTORS R  (00=OFF, 01=Toggle, 10=ON, 11=none)
			//  [7,6] LED MOTORS F  (00=OFF, 01=Toggle, 10=ON, 11=none)
			sData.status_Leds_ctrlFan = 0xA0;
			polling_CTRL_FAN();
			// attendiamo la risposta  della CTRL_FAN
			delay_ms (60);
			ret_val = intercepts_messages_serial_2();
			delay_ms (20);
			ret_val = intercepts_messages_serial_2();
			Serial_print1(".");
			
			wdt_restart(WATCH_DOG);
		}
		
		
		if(ret_val == 0)
		{
			polling_CTRL_FAN();
			delay_ms (60);
			ret_val_1 = intercepts_messages_serial_2();
			delay_ms (20);
			ret_val_1 = intercepts_messages_serial_2();
			
			if(ret_val_1) {
				Serial_println1(" ");
				Serial_println1("Error, Link KO CTRL_FAN!");
				sData.status_Leds_ctrlFan = 0x00;
				sData.speed_motors_R = 0;
				sData.speed_motors_F = 0;
				polling_CTRL_FAN();
				delay_ms (500);
				return;
			}
		}
		
		//--- stampa gli stati e le misure ---
		ckMotorsOn = 0;
		sglVR = sData.speed_motors_R * 4;
		sglVF = sData.speed_motors_F * 4;
		
		//------- Return -------------
		Serial_println1(" ");
		Serial_print1(" FANS R, Cmd_10V: ");
		// tensione letta sul comando dei motori ramo RETURN (0.00V a 11.00V) il valore e' moltipl. per 100, quindi 1100 = 11.00V
		Serial_print((float)sData.feedbk_v_speed_motors_R/100, 2);
		Serial_print1("V ");
		// StatusMotors:   bit: 7.none | 6.AlarmM3 | 5.AlarmM2 | 4.AlarmM1 | 3.none | 2.PowerON_M3 | 1.PowerON_M2 | 0.PowerON_M1
		Serial_print1(" R1:");
		if(sData.StatusMotorsR &0x01) {
			Serial_print(sData.rpm_motors_R[0], DEC);
			Serial_print1(" rpm");
			ckMotorsOn++;
			
			if(sData.rpm_motors_R[0] < sglVR) {
				rpmFail[0] = 1;
				esito = 'F';
			}
		}else
		Serial_print1("OFF");
		
		Serial_print1("  R2:");
		if(sData.StatusMotorsR &0x02) {
			Serial_print(sData.rpm_motors_R[1], DEC);
			Serial_print1(" rpm");
			ckMotorsOn++;
			
			if(sData.rpm_motors_R[1] < sglVR) {
				rpmFail[1] = 1;
				esito = 'F';
			}
		}else
		Serial_print1("OFF");
		
		Serial_print1("  R3:");
		if(sData.StatusMotorsR &0x04) {
			Serial_print(sData.rpm_motors_R[2], DEC);
			Serial_print1(" rpm");
			ckMotorsOn++;
			
			if(sData.rpm_motors_R[2] < sglVR) {
				rpmFail[2] = 1;
				esito = 'F';
			}
		}else
		Serial_print1("OFF");
		
		
		
		//------- Fresh -------------
		Serial_println1(" ");
		Serial_print1(" FANS F, Cmd_10V: ");
		// Tensione letta sul comando dei motori ramo Fresh (0.00V a 11.00V) il valore e' moltipl. per 100, quindi 1100 = 11.00V
		Serial_print((float)sData.feedbk_v_speed_motors_F/100, 2);
		Serial_print1("V ");
		// StatusMotors:   bit: 7.none | 6.AlarmM3 | 5.AlarmM2 | 4.AlarmM1 | 3.none | 2.PowerON_M3 | 1.PowerON_M2 | 0.PowerON_M1
		Serial_print1(" F1:");
		if(sData.StatusMotorsF &0x01) {
			Serial_print(sData.rpm_motors_F[0], DEC);
			Serial_print1(" rpm");
			ckMotorsOn++;
			
			if(sData.rpm_motors_F[0] < sglVF) {
				rpmFail[3] = 1;
				esito = 'F';
			}
		}else
		Serial_print1("OFF");
		
		Serial_print1("  F2:");
		if(sData.StatusMotorsF &0x02) {
			Serial_print(sData.rpm_motors_F[1], DEC);
			Serial_print1(" rpm");
			ckMotorsOn++;
			
			if(sData.rpm_motors_F[1] < sglVF) {
				rpmFail[4] = 1;
				esito = 'F';
			}
		}else
		Serial_print1("OFF");
		
		Serial_print1("  F3:");
		if(sData.StatusMotorsF &0x04) {
			Serial_print(sData.rpm_motors_F[2], DEC);
			Serial_print1(" rpm");
			ckMotorsOn++;
			
			if(sData.rpm_motors_F[2] < sglVF) {
				rpmFail[5] = 1;
				esito = 'F';
			}
		}else
		Serial_print1("OFF");
		
		Serial_println1(" ");
		Serial_print1(" Link CTRL_FAN: ");
		Serial_print(sData.level_link_CTRLFAN, DEC);
		Serial_println1("/10");
		
		if(ckMotorsOn != nMotorsCfg)
		esito = 'F';
		
		if(sData.feedbk_v_speed_motors_F  <  ((sglVF * 9) /10))
		esito = 'F';
		
		if(sData.feedbk_v_speed_motors_R  <  ((sglVR * 9) /10))
		esito = 'F';
		
	}
	
	// Spegniamo le ventole...
	sData.status_Leds_ctrlFan = 0x00;
	sData.speed_motors_R = 0;
	sData.speed_motors_F = 0;
	polling_CTRL_FAN();
	delay_ms (200);
	polling_CTRL_FAN();
	
	delay_ms (200);
	
	// 9. stampo esito
	Serial_println1(" ");
	print_result_test(esito);
	
	wdt_restart(WATCH_DOG);
}

//-----------------------------------------------------
//         [5]  test Uart1
// Il test, si può fase solo con un piccolo
// HW che mette in cc RX con TX solo dopo aver
//-----------------------------------------------------
void testUart1(void)
{
	Byte nb, i, flag, esito=0;
	char buff_test[60];
	float fval_16 = 0;
	
	//digitalWrite(pSwitchOFF_KTS, LOW);   // KTS OFF
	ioport_set_pin_level(pSwitchOFF_KTS, IOPORT_PIN_LEVEL_LOW);
	Serial_println1(" ");
	PrintFlashString(str_1_test_uart1,   NEW_LINE);
	PrintFlashString(str_HW_Testing,   NEW_LINE);
	Serial_println1(" ");
	
	delay_ms (2000);
	
	wdt_restart(WATCH_DOG);
	
	// scarichiamo prima il buffer
	nb = Serial_available();
	if(nb) {
		for(i=0; i< nb; i++)
		buff_test[0] = Serial_read();
	}
	Serial_print1("TX: ");
	delay_ms (50);
	
	//digitalWrite(pSwitchOFF_KTS, HIGH);   // KTS ON
	ioport_set_pin_level(pSwitchOFF_KTS, IOPORT_PIN_LEVEL_HIGH);
	delay_ms (100);
	fval_16 += analogRead(pAnAssorb_KTS);
	delay_ms (100);
	fval_16 += analogRead(pAnAssorb_KTS);
	delay_ms (100);
	
	PrintFlashString(str_msg_uart1,   NO_NEW_LINE); //spariamo la stringa
	
	delay_ms (100);
	fval_16 += analogRead(pAnAssorb_KTS);
	delay_ms (100);
	fval_16 += analogRead(pAnAssorb_KTS);
	delay_ms (100);

	// puliamo e scarichiamo prima il buffer
	for(i=0; i <50; i++)
	buff_test[i] = 0;
	
	nb = Serial_available();
	if(nb) {
		for(i=0; i<nb; i++)
		buff_test[i] = Serial_read();
	}

	//digitalWrite(pSwitchOFF_KTS, LOW);   // KTS OFF
	ioport_set_pin_level(pSwitchOFF_KTS, IOPORT_PIN_LEVEL_LOW);
	delay_ms (400);
	
	for(i=0, flag =0; i <nb; i++)
	{
		if(buff_test[i] != str_msg_uart1[i])
		flag++;
	}
	buff_test[i] = 0;
	
	Serial_println1(" ");
	Serial_print1("RX: ");
	delay_ms (250);
	Serial_println1(buff_test);
	
	delay_ms (1000);
	
	wdt_restart(WATCH_DOG);
	
	Serial_println1(" ");
	Serial_print1("Len RX Message: ");
	Serial_println(nb, DEC);
	Serial_print1("Absorption current HW on RJ45: ");
	Serial_print((fval_16/16), 1);
	Serial_println1(" mA");
	
	if(flag || (nb < 3))
	esito = 'F';
	
	print_result_test(esito);
	
}


//-----------------------------------------------------
//
//-----------------------------------------------------
void print_bit_I2C(Byte *sda, Byte *scl, Byte versus)
{
	int i;
	
	delay_ms (50);
	if(versus == 0) {
		Serial_println1(" ");
		Serial_print1("SDA_OUT:  ");
		
		for(i=0; i <N_TOOGLE_BIT_I2C; i++) {
			if(*sda++)
			Serial_print1("1 ");
			else
			Serial_print1("0 ");
			
		}
		
		delay_ms (50);
		Serial_println1(" ");
		Serial_print1("SCL_IN :  ");
		
		for(i=0; i <N_TOOGLE_BIT_I2C; i++) {
			if(*scl++)
			Serial_print1("1 ");
			else
			Serial_print1("0 ");
		}
		}else {
		Serial_println1(" ");
		Serial_print1("SCL_OUT:  ");
		
		for(i=0; i <N_TOOGLE_BIT_I2C; i++) {
			if(*scl++)
			Serial_print1("1 ");
			else
			Serial_print1("0 ");
			
		}
		
		delay_ms (50);
		Serial_println1(" ");
		Serial_print1("SDA_IN :  ");
		
		for(i=0; i <N_TOOGLE_BIT_I2C; i++) {
			if(*sda++)
			Serial_print1("1 ");
			else
			Serial_print1("0 ");
		}
	}
}

//-----------------------------------------------------
//         [6]  test I2C bus
//  Andiamo a verificare il Loop tra SDA e SCL con
//  2 campioni di frequenza, 1 KHz e 50 KHz.
//  Per far ciò come il Test Uart1 dobbiamo utilizzare
//  un piccolo HW che cc i 2 PIN.
//-----------------------------------------------------
void testI2C()
{
	Byte status_pSCL[N_TOOGLE_BIT_I2C];
	Byte status_pSDA[N_TOOGLE_BIT_I2C];
	Byte i, n, j, esito = 0;
	int pTestSDA = 10, pTestSCL= 11;
	
	Serial_println1(" ");
	PrintFlashString(str_1_test_I2C,   NEW_LINE);
	PrintFlashString(str_HW_Testing,   NEW_LINE);
	delay_ms (1300);
	
	wdt_restart(WATCH_DOG);
	
	esito = 0;
	
	for(n=0; n <2; n++){
		Serial_println1(" ");
		if(n==0)
		PrintFlashString(str_2_test_I2C,   NO_NEW_LINE);
		else {
			Serial_println1(" ");
			PrintFlashString(str_3_test_I2C,   NO_NEW_LINE);
		}
		
		delay_ms (400);
		
		wdt_restart(WATCH_DOG);
		
		for(i=0; i < N_TOOGLE_BIT_I2C; i++) {
			status_pSCL[i] = 0;
			status_pSDA[i] = 0;
		}
		
		// 1. Step:
		//         SDA Cambia stato per 10 volte
		//         SCL verifica il cambiamento.
		//digitalWrite(pStatusLedI2C,  LOW);  // led spento
		ioport_set_pin_level(pStatusLedI2C, IOPORT_PIN_LEVEL_LOW);
		//pinMode(pTestSDA,         OUTPUT);
		//digitalWrite(pTestSDA,     HIGH);
		ioport_set_pin_dir(SDA, IOPORT_DIR_OUTPUT);
		ioport_set_pin_level(SDA, IOPORT_PIN_LEVEL_HIGH);
		//pinMode(pTestSCL,          INPUT);
		ioport_set_pin_dir(SCL, IOPORT_DIR_INPUT);
		delay_ms (20);
		
		for(i=0; i <N_TOOGLE_BIT_I2C; i++) {
			//digitalWrite(pStatusLedI2C,  HIGH);
			ioport_set_pin_level(pStatusLedI2C, IOPORT_PIN_LEVEL_HIGH);
			if(i&0x01) {
				//digitalWrite(pTestSDA,     HIGH);
				ioport_set_pin_level(SDA, IOPORT_PIN_LEVEL_HIGH);
				status_pSDA[i] = 1;
				}else {
				//digitalWrite(pTestSDA,     LOW);
				ioport_set_pin_level(SDA, IOPORT_PIN_LEVEL_LOW);
				status_pSDA[i] = 0;
			}
			
			asm ("nop");
			asm ("nop");
			//status_pSCL[i]  = digitalRead(pTestSCL);
			status_pSCL[i]  = ioport_get_pin_level(SCL);
			
			if(n==0)
			delay_ms (1);
			else {
				for(j=0; j<80; j++) {
					asm ("nop");
					asm ("nop");
				}
				
			}
		}
		
		for(i=0; i <N_TOOGLE_BIT_I2C; i++) {
			if(status_pSDA[i] != status_pSCL[i])
			esito = 'F';
		}
		
		if(n==0)
		print_bit_I2C(&status_pSDA[0], &status_pSCL[0], 0);
		
		//digitalWrite(pStatusLedI2C,  LOW);
		ioport_set_pin_level(pStatusLedI2C, IOPORT_PIN_LEVEL_LOW);
		// 2. Step
		//        SCL Cambia stato per 10 volte
		//        SDA Verifica lo satato
		//pinMode(pTestSDA,        INPUT);
		ioport_set_pin_dir(SDA, IOPORT_DIR_INPUT);
		//pinMode(pTestSCL,          OUTPUT);
		//digitalWrite(pTestSCL,     HIGH);
		ioport_set_pin_dir(SCL, IOPORT_DIR_OUTPUT);
		ioport_set_pin_level(SCL, IOPORT_PIN_LEVEL_HIGH);
		delay_ms (20);
		
		for(i=0; i <N_TOOGLE_BIT_I2C; i++) {
			//digitalWrite(pStatusLedI2C,  HIGH);
			ioport_set_pin_level(pStatusLedI2C, IOPORT_PIN_LEVEL_HIGH);
			
			if(i&0x01) {
				//digitalWrite(pTestSCL,     HIGH);
				ioport_set_pin_level(SCL, IOPORT_PIN_LEVEL_HIGH);
				status_pSCL[i] = 1;
				}else {
				//digitalWrite(pTestSCL,     LOW);
				ioport_set_pin_level(SCL, IOPORT_PIN_LEVEL_LOW);
				status_pSCL[i] = 0;
			}
			
			asm ("nop");
			asm ("nop");
			
			//status_pSDA[i] = digitalRead(pTestSDA);
			status_pSDA[i] = ioport_get_pin_level(SDA);
			
			if(n==0)
			delay_ms (1);
			else {
				for(j=0; j<80; j++) {
					asm ("nop");
					asm ("nop");
				}
			}
		}
		
		for(i=0; i <N_TOOGLE_BIT_I2C; i++) {
			if(status_pSDA[i] != status_pSCL[i])
			esito = 'F';
		}
		delay_ms (100);
	}
	
	print_bit_I2C(&status_pSDA[0], &status_pSCL[0], 1);
	
	delay_ms (100);
	//pinMode(pTestSDA,         OUTPUT);
	ioport_set_pin_dir(SDA, IOPORT_DIR_OUTPUT);
	//digitalWrite(pStatusLedI2C,  LOW);
	ioport_set_pin_level(pStatusLedI2C, IOPORT_PIN_LEVEL_LOW);
	
	Serial_println1(" ");
	print_result_test(esito);
}

int IPEHD_test()
{
	Byte test;
	Byte cntUpdate_SetTemp;
	test = read_byte_eeprom(ADDR_EEP(Ref_T_setting));
	
	
	if( (bitRead(test,6)==1) && (flag_ipehd == 0) && (counter_ipehd == 0))
	{
		//digitalWrite(pIPEHD, HIGH);
		ioport_set_pin_level(pIPEHD, IOPORT_PIN_LEVEL_HIGH);
		flag_ipehd = 1;
		counter_ipehd = 10;
		return 1;
	}else
	if ((bitRead(test,6)==1) && (flag_ipehd == 1) && (counter_ipehd > 1))
	{
		counter_ipehd--;
		return 1;
		
	}else
	if ((bitRead(test,6)==1) && (flag_ipehd == 1) && (counter_ipehd == 1))
	{
		bitClear(test,6);
		write_byte_eeprom(ADDR_EEP(Ref_T_setting), test);
		cntUpdate_SetTemp = read_byte_eeprom( ADDR_EEP( cntUpdate_SetTemp ) );
		cntUpdate_SetTemp++;
		write_byte_eeprom( ADDR_EEP( cntUpdate_SetTemp ), cntUpdate_SetTemp );
		//digitalWrite(pIPEHD, LOW);
		ioport_set_pin_level(pIPEHD, IOPORT_PIN_LEVEL_LOW);
		counter_ipehd =  0;
		flag_ipehd = 0;
		return 1;
	}
	
	return 1;
	
}
