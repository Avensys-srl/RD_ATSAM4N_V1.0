/*
 * ADC.cpp
 *
 */ 

#include "asf.h"
#include "common/build_config.h"

#include <math.h>
#include "data.h"
#include "eeprom_map.h"
#include "alarm.h"

#define byte		Byte

// stato del Rele' e delle tensione dell'elettronica
#define POWER_SAVE     0
#define POWER_ON      'O'

 // scegliere multipli di 4: 4, 8, 12, ....
 #define NUM_SAMPLE_NTC    80  // ho una conversione ogni 4,0 sec.
 #define NUM_SAMPLE_QAIR   64  //
 #define NUM_SAMPLE_INP    16  //
 #define NUM_SAMPLE_I_KTS  24  //
 
 //-----------------------------------------------------
 unsigned long  preview_50_Mills = 0;

 //-----------------------------------------------------
 // pAnProbe_Temp1=0, pAnProbe_Temp2=1,  pAnProbe_Temp3=2, pAnProbe_Temp4=3,
 // pAnSensQual_Ari1=4, pAnSensQual_Ari2=5,  pAnAssorb_ByPass=6,
 // pAnInp1=7, pAnInp2=8, pAnAssorb_KTS=10.
 unsigned long  Acc_Adc[11] = {0,0,0,0,0,0,0,0,0,0,0};
 int nSample_Adc[11]        = {0,0,0,0,0,0,0,0,0,0,0};
	 
 extern SDATA  sData;
 extern short  Temp_modbus[4];
 
 //---------- indici sonde di temperatura -------------
 extern Byte i_TFresh;
 extern Byte i_TReturn;
 extern Byte i_TSupply;
 extern Byte i_TExaust;
 
 //-----------  stato rele' unita ---------------------
 extern Byte PowerMotors;
 
 extern float BPD_Current;
 
 extern struct {
	 uint16_t CH0;
	 uint16_t CH1;
	 float CH0_Value;
	 float CH1_Value;
	 bool State;
	 bool Start_Conversion;
 } ADS7142_IC9;
 
 extern unsigned long millis( void );
 extern unsigned analogRead ( byte Channel_Num );
 extern void Ctrl_FunzionamentoBPD();
 float read_probeTemp(unsigned int temp);
 extern void ADS7142_IC9_Start_Conversion ( void );
 extern void ADS7142_IC9_Read ( void );
 
 //-----------------------------------------------------
 // Func:
 //   read_analogInput
 //
 // Descrizione:
 //   Va a leggere i vari canali analogici, in base
 //   alle configurazioni in eep, e alle funzioni attive.
 //
 //-----------------------------------------------------
 void read_analogInput(void)
 {
	 
	 unsigned long timerMills;
	 unsigned int average_values;
	 float  temp;
	 int i;
	 Byte nbit, nbyte, numNTC, val_inp;
	 
	 timerMills = millis();
	 
	 
	 // se sta girando il motorino BPD verifica gli assorbimenti
	 if(sData.status_unit & (1 << POS_BIT_BYPASS_RUN))
		Ctrl_FunzionamentoBPD();
	 else
		BPD_Current = 0;	

	 
	 // ============================ SAMPLE: 50 ms ============================
	 if (((timerMills -  preview_50_Mills) >= 50) || (preview_50_Mills > timerMills))
	 {
		 preview_50_Mills = timerMills;
		 
		 wdt_restart(WATCH_DOG);
		 #ifdef WORK_WDT
		 //ClearWDT();  // teniamo vivo il WacthDog (timeout 0.7 sec.)
		 #endif
		 // se vi è un dato anomalo, mettiamo solo 2 sonde come limite
		 numNTC = read_byte_eeprom(ADDR_EEP(numNTC));
		 
		 if(numNTC > (I_PROBE_EXHAUST + 1))
		 numNTC = 4;
		 
		 //--------------  Probes Temperature -------------------------
		 //                          Standard       |    ex. Custum
		 //                        -----------------+-----------------
		 // pAnProbe_Temp1 = 0 |-> PROBE_FRESH.     |   PROBE_RETURN. (1)
		 // pAnProbe_Temp2 = 1 |-> PROBE_RETURN.    |   PROBE_SUPPLY  (2)
		 // pAnProbe_Temp3 = 2 |-> PROBE_SUPPLY.    |   PROBE_FRESH   (0)
		 // pAnProbe_Temp4 = 3 |-> PROBE_EXHAUST    |   PROBE_EXHAUST (3)
		 Acc_Adc[I_PROBE_FRESH] += analogRead(i_TFresh);
		 nSample_Adc[I_PROBE_FRESH]++;


		 Acc_Adc[I_PROBE_RETURN] += analogRead(i_TReturn);
		 nSample_Adc[I_PROBE_RETURN]++;
		 
		 if(numNTC > 2) {
			 Acc_Adc[I_PROBE_SUPPLY] += analogRead(i_TSupply);
			 nSample_Adc[I_PROBE_SUPPLY]++;
		 }else
		 nSample_Adc[I_PROBE_SUPPLY] = 0;
		 
		 if(numNTC > 3) {
			 Acc_Adc[I_PROBE_EXHAUST] += analogRead(i_TExaust);
			 nSample_Adc[I_PROBE_EXHAUST]++;
		 }else
		 nSample_Adc[I_PROBE_EXHAUST] = 0;
		 

		 for(i=0; i <= I_PROBE_EXHAUST; i++)
		 {
			 if(i < numNTC) {
				 if(nSample_Adc[i] >= NUM_SAMPLE_NTC) {
					 
					 average_values = Acc_Adc[i] / nSample_Adc[i];
					 
					 if(average_values > 1000) { // > 100.0°C 9750
						 InsertDelayAlarm(ALM_PTFRESH_KO + i);
						 }else {
						 RemoveAlarm(ALM_PTFRESH_KO + i);
						 temp = read_probeTemp(average_values) * 100;
						 
						 if(i == I_PROBE_RETURN) {
							 // se è inserito l'accessorio AWP, si sostituisce all T return.
							 if(!DigitAccessoryOperating(ACC_I2C_AWP))
							 {
								 sData.measure_Temp[i] = (int)(temp/10);
								 Temp_modbus[i] = (int)temp;
							 }
						 }else
						 {
							 sData.measure_Temp[i] = (int)(temp/10);
							 Temp_modbus[i] = (int)temp;
						 }
						 
					 }
					 
					 // Teniamo in memoria il 25% di valori per la successiva conversione
					 nSample_Adc[i] >>= 2;
					 Acc_Adc[i] >>= 2;

				 }
				 }else {
				 nSample_Adc[i] = 0;
				 Acc_Adc[i]     = 0;
				 RemoveAlarm(ALM_PTFRESH_KO + i);
			 }
		 }


		 //---- Assorbimento KTS ----
		 Acc_Adc[pAnAssorb_KTS] += analogRead(pAnAssorb_KTS);
		 nSample_Adc[pAnAssorb_KTS]++;
		 
		 if(nSample_Adc[pAnAssorb_KTS] >= NUM_SAMPLE_I_KTS) {
			 average_values = Acc_Adc[pAnAssorb_KTS] / nSample_Adc[pAnAssorb_KTS];
			 sData.measure_I_KTS = (average_values * 8) / 33;
			 Acc_Adc[pAnAssorb_KTS] >>= 2;
			 nSample_Adc[pAnAssorb_KTS] >>= 2;
		 }
		 
		 
		 //---- Input 1 & Input 2 ----
		 for(i=0; i < 2; i++) {
			 Acc_Adc[pAnInp1 + i] += analogRead(pAnInp1 + i);
			 nSample_Adc[pAnInp1 + i]++;
			 
			 if(nSample_Adc[pAnInp1 + i] >= NUM_SAMPLE_INP) {
				 average_values = Acc_Adc[pAnInp1 + i]/  nSample_Adc[pAnInp1 + i];
				 
				 if(PowerMotors == POWER_SAVE)
				 sData.measure_ADC_input[i] = (average_values * 5) / 3; // fondo scala 6,0V  = 1024
				 else
				 sData.measure_ADC_input[i] = average_values; // fondo scala 10,2V = 1024
				 
				 nSample_Adc[pAnInp1 + i] = 0;
				 Acc_Adc[pAnInp1 + i]     = 0;
			 }
		 }
		 
		 // Read AC/DC 12v and 5v
		 if ( ADS7142_IC9.State )
		 {
			 ADS7142_IC9_Read ();
			 //ADS7142_IC9_Start_Conversion ( );
		 }
	 }
	 
 }

 //-----------------------------------------------------
 // Type NTC Thermistors :
 // B-Constant           :  3950
 // Resistance NTC @25 C :  10K
 //-----------------------------------------------------
 float read_probeTemp(unsigned int temp) {
	 float K;
	 float B=3950.0;
	 float tmp=0.0;
	 float T=0.0;
	 int val = 0;
	 
	 const float res = 10000.0;      // Resistance value
	 
	 float Vread = 0;
	 
	 K=res*exp(-B/298.15);
	 
	 val =1023-temp;
	 Vread=3.3*(float)val/1023.0;
	 tmp=(res*3.3)/Vread-res;

	 T=(B/log(tmp/K))-273.15;
	 
	 return T;
 }
