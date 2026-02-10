/*
 * Weekly_Prog.cpp
 *
 * Descrizione:
 *   L'unità puo' essere programmata per funzionare in maniera autonoma, con accensioni e
 *   spegnimenti durante la giornata, con 3 diversi tipi di portate d'aria etc...
 *   Questo file raccoglie tutte le funzioni legate a questa funzione.
 */ 

#ifdef VIEW_DEBUG_CODE_NO_KTS
	//#define VIEW_WEEKLY_DBG
#endif

#include "asf.h"

#include "eeprom_map.h"
#include "Protocol_Ser1.h"
#include "data.h"

//-----------------------------------------------------
// Definizione timer di sistema.
//-----------------------------------------------------
extern S_CLOCK_WEEK    sClockWeek;

extern SDATA  sData;

extern void TAG_DigitAccessoryOn(int nAcc_I2C);
extern void TAG_DigitAccessoryOff(int nAcc_I2C);

//---------------------------------------------------------------------
// Funzione che gira nello scheduler Sample: 1 sec.
// Aggiona l'orologio dell'unitÃ . Se vi Ã¨ un KTS si sincronizza con questo
//---------------------------------------------------------------------
int weekly_program()
{
	int range, realtime_step_30min;
	Byte set_cfg =0, SetPowerOn, numMaxRange, time_start, time_stop, temp_08, shf;
	unsigned short enab_func, msk_enab_func;
	
	// aggiorna l'orologio per il weekly.
	sClockWeek.seconds++;
	
	if(sClockWeek.seconds >= 60)
	{
		sClockWeek.seconds = 0;
		sClockWeek.minutes++;
		if(sClockWeek.minutes >= 60) {
			sClockWeek.minutes = 0;
			sClockWeek.hours++;
			if(sClockWeek.hours >= 24)
			{
				sClockWeek.hours = 0;
				sClockWeek.weekday++; // 0=lunedÃ¬, ... , 6=Domenica
				if(sClockWeek.weekday >= 7)
				sClockWeek.weekday = 0;
			}
		}
	}
	
	msk_enab_func = read_word_eeprom(ADDR_EEP(msk_Enab_Fuction));
	
	if ((msk_enab_func & (1<<ENAB_WEEKLY)) == 0)
	return 1;

	enab_func  = read_word_eeprom(ADDR_EEP(Enab_Fuction));

	// Se l'unità è impostata per funzionare, e è abilitato il weekly, verifica i range di funzionamento
	if (enab_func & (1<<ENAB_WEEKLY))
	{
		sData.status_weekly = MSK_BIT_WEEKLY_ENAB;
		realtime_step_30min = (sClockWeek.hours << 1) + (sClockWeek.minutes / 30); // gli step di programmazione sono di 30min in 30min.
		
		numMaxRange = read_byte_eeprom(ADDR_EEP(sDayProg[sClockWeek.weekday].numRange));
		
		//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' x_dbg
		#ifdef VIEW_WEEKLY_DBG
		if ((sClockWeek.seconds == 0) || (sClockWeek.seconds == 20) || (sClockWeek.seconds == 40))
		{
			Serial_println1(" ");
			Serial_print1(" DAY:");
			Serial_print(sClockWeek.weekday, DEC);
			Serial_print1(" TIME, ");
			Serial_print(sClockWeek.hours, DEC);
			Serial_print1(":");
			Serial_print(sClockWeek.minutes, DEC);
			Serial_print1(":");
			Serial_println(sClockWeek.seconds, DEC);
			Serial_print1(" Step30min: ");
			Serial_println(realtime_step_30min, DEC);
			Serial_print1(" nRange: ");
			Serial_print(numMaxRange, DEC);
		}
		#endif
		//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' x_dbg
		
		if(numMaxRange)
		{
			
			for(range=0; range < numMaxRange; range++)
			{
				// esempio: timeON=11 (Start:5h30') , timeOFF=14 (Off:7h00')
				time_start = read_byte_eeprom(ADDR_EEP(sDayProg[sClockWeek.weekday].timeON[range]));
				time_stop  = read_byte_eeprom(ADDR_EEP(sDayProg[sClockWeek.weekday].timeOFF[range]));
				//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' x_dbg
				#ifdef VIEW_WEEKLY_DBG
				if ((sClockWeek.seconds == 0) || (sClockWeek.seconds == 30))
				{
					Serial_print1(" Ton: ");
					Serial_print(time_start, DEC);
					Serial_print1(" Toff: ");
					Serial_print(time_stop, DEC);
					
				}
				#endif
				//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' x_dbg

				//------------------25/01/2016-------------------
				//  Analizzo il passo precedente all'accensione,
				//  se presente il PHWD apro la valvola in modo
				//  che vada in temperatura il coil
				//-----------------------------------------------
				if((sData.AccI2C[ACC_I2C_PREHEATER].sts & STS_ACC_CONNECT) && (sData.AccI2C[ACC_I2C_PREHEATER].sts & STS_ACC_ELECT)==0)
				{
					if (time_start == 0)
					{
						time_start = 1;
					}
					//Inizio mezz'ora prima e stacco quando chiudo
					if((realtime_step_30min  >= (time_start - 1)) && (realtime_step_30min  < time_stop))
					{
						if((sData.AccI2C[ACC_I2C_PREHEATER].sts & STS_ACC_ELECT)==0)
						{
							if((realtime_step_30min  >= (time_start - 1)) && (realtime_step_30min  < time_start))
							TAG_DigitAccessoryOn(ACC_I2C_PREHEATER);
							else
							{
								if (sData.AccI2C[ACC_I2C_PREHEATER].measure1 > 70)
								{
									TAG_DigitAccessoryOff(ACC_I2C_PREHEATER);
								}
								else
								TAG_DigitAccessoryOn(ACC_I2C_PREHEATER);
							}
							
						}
						
					}
					else
					{
						TAG_DigitAccessoryOff(ACC_I2C_PREHEATER);
					}
					
				}
				//----------------------------------------------------
				if((realtime_step_30min  >= time_start) && (realtime_step_30min  < time_stop))
				{
					// ---- Andiamo a comporre il byte: status_weekly  ----
					//          bit[7,6]: 1,0=SPEED_3 | 0,1=SPEED_2    | 0,0=SPEED_1
					//          bit[5,4]: 1,0= * * *  | 0,1=IMBAL_1_ON | 0,0=IMBAL_OFF
					//          bit[3,2]: 1,0= * * *  | 0,1=RIF_TEMP2  | 0,0=RIF_TEMP1
					//            bit[1]: WEEKLY_RUN
					//            bit[0]: WEEKLY_ENABLE
					sData.status_weekly |= MSK_BIT_WEEKLY_RUN;
					shf = (range*2);
					
					//  ConfigTemp ->  bit[7,6]:Rif. Temp. range 4 | bit[5,4]:Rif. Temp. range 3 | bit[3,2]:Rif. Temp. range 2 | bit[1,0]:Rif. Temp.range 1.
					temp_08 = read_byte_eeprom(ADDR_EEP(sDayProg[sClockWeek.weekday].ConfigTemp));
					set_cfg = (temp_08 >> shf) & 0x01; // gestiamo solo 2 soglie di temperatura
					sData.status_weekly |= (set_cfg << POS_BIT_RIF_TEMP);   // bit[3,2]: 1,1=---/ 1,0=---/ 0,1=RIF_TEMP2/ 0,0=RIF_TEMP1
					
					// ConfigImbal -> bit[7,6]:Set Imbal. range 4 | bit[5,4]:Set Imbal. range 3 | bit[3,2]:Set Imbal. range 2 | bit[1,0]:Set Imbal. range 1.
					temp_08 = read_byte_eeprom(ADDR_EEP(sDayProg[sClockWeek.weekday].ConfigImbal));
					set_cfg = (temp_08 >> shf) & 0x01;  // gestiamo solo 2 soglie di temperatura
					sData.status_weekly |= (set_cfg << POS_BIT_SET_IMBAL);  // bit[5,4]: 1,1=---/ 1,0=---/ 0,1=IMBAL_1_ON/ 0,0=IMBAL_OFF
					
					//  ConfigSpeed -> bit[7,6]:Step Speed range 4 | bit[5,4]:Step Speed range 3 | bit[3,2]:Step Speed range 2 | bit[1,0]:Step Speed range 1.
					temp_08 = read_byte_eeprom(ADDR_EEP(sDayProg[sClockWeek.weekday].ConfigSpeed));
					set_cfg = (temp_08 >> shf) & 0x03;
					sData.status_weekly |= (set_cfg << POS_STEP_SPEED);     // bit[7,6]: 1,1=---/ 1,0=SPEED_3/ 0,1=SPEED_2/ 0,0=SPEED_1
					
					break;
				}
				
			}
		}
		//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' x_dbg
		#ifdef VIEW_WEEKLY_DBG
		if ((sClockWeek.seconds == 0) || (sClockWeek.seconds == 30)) {
			Serial_println1(" ");
			Serial_print1("StsWeek:");
			if(sData.status_weekly & MSK_BIT_WEEKLY_ENAB)
			Serial_print1("Enab, ");
			if(sData.status_weekly & MSK_BIT_WEEKLY_RUN)
			Serial_print1("Run, ");
			else
			Serial_print1("StandBy,");
			
			Serial_println1(" ");
		}
		#endif
		//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' x_dbg
	}else
	sData.status_weekly = 0;
	
	
	return 1; // tieni attiva la funzione nello sheduler
	
}
