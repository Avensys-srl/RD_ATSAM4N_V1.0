/*
 * Filter_Func.cpp
 *
 * Descrizione:
 *   Nel seguente file vengono racchiuse tutte le funzioni legate alla gestione del timer con c
 *   cadenza secondo, esempio manutezione filtri etc.
 */ 

#include "asf.h"

#include "eeprom_map.h"
#include "alarm.h"
#include "data.h"

#define word	uint16_t

#define MINSPEED            64
#define TEMPO               30  //(numero cicli 1 circa ogni secondo)

//--------  Vars hour funz.  --------
static Byte read_hours_eep = 0;
unsigned int  countSecondFunz = 0;
extern unsigned long countHoursFunz;

extern SDATA  sData;

extern const int  pAnSensQual_Ari1;
extern const int  pAnSensQual_Ari2;

uint32_t P_Hour = 0;
uint32_t P_Minute = 0;
uint32_t P_Second = 0;

extern int update_data_eeprom(unsigned short start_addr, unsigned short end_addr, Byte *buff);
void  check_clogged_filter();
void  verifica_clogged_filter();
void  calibrazione_clogged_filter();
extern unsigned analogRead ( Byte Channel_Num );

//---------------------------------------------------------------------
// Funzione che gira nello scheduler Sample: 1,0 sec.
// Conteggia le ore di funzionamento, e verifica la scadenza della pulizia
// filtri
//---------------------------------------------------------------------
int operatingHoursCounter()
{
	unsigned short enab_func =0;

	// per precauzione andiamo in timeout sul test.
	if(sData.timeout_test){
		sData.timeout_test--;
	}
	else
	sData.status_test = 0;

	// alla partenza andiamo a leggere il dato memorizzato.
	if(!read_hours_eep) {
		read_hours_eep = 1;
		countHoursFunz = read_Dword_eeprom(ADDR_EEP(hour_runnig));
	}
	// ----------- status_unit ---------
	//  bit[3,2,1,0] :  HOW IT'S WORKING?
	// --------------+------------------
	//      0,1,0,1  :  RUN BOOST
	//      0,1,0,0  :  RUN IMBALANCE
	//      0,0,1,1  :  RUN POST_VENT
	//      0,0,1,0  :  RUN DeFROST
	//      0,0,0,1  :  RUN NORMAL
	//      0,0,0,0  :  STANDBY = NO RUN
	if(sData.status_unit & 0x000F) {
		countSecondFunz++;
		if(countSecondFunz >= 3600) {
			countSecondFunz = 0;
			countHoursFunz++;
			if((countHoursFunz % 8) == 0){
				word addr_start, addr_end;
				Byte buff[8];

				// hour_runnig            4 byte:  ogni 10 ore memorizza in eeprom l'avanzamento del timer.
				// time_lastCloggedFilers 4 byte:  tempo in ore dell'ultima pulizia filtri.

				buff[0] = (countHoursFunz & 0x000FF); // byte Low
				buff[1] = (countHoursFunz >> 8);
				buff[2] = (countHoursFunz >> 16);
				buff[3] = (countHoursFunz >> 24);     // byte high

				addr_start = ADDR_EEP(hour_runnig);
				addr_end   = addr_start + 3;

				enab_func = read_word_eeprom(ADDR_EEP(Enab_Fuction));

				if ((enab_func & (1 << ENAB_CTRL_FILTER)) == 0)
				{
					// se la funzione non è abilitata, sposta il timer dell'ultima pulizia filtri
					// in modo che quando viene abilitata parte il conteggio.
					buff[4] = buff[0];    // byte Low
					buff[5] = buff[1];
					buff[6] = buff[2];
					buff[7] = buff[3];    // byte high

					addr_end   = addr_start + 7;
				}

				update_data_eeprom(addr_start, addr_end, &buff[0]);  // 4 / 8 byte
			}
		}
	}

	// controlla la pulizia filtri.
	check_clogged_filter();
	verifica_clogged_filter();
	calibrazione_clogged_filter();
	
	rtc_get_time(RTC, &P_Hour, &P_Minute, &P_Second);
		 
	return 1; // 1 per rimanere nello scheduler
}


//---------------------------------------------------------------------
//  Controlla lo stato dei filtri, la funzione gira nella
//  operatingHoursCounter() che sta nello sheduler.
//---------------------------------------------------------------------
void  check_clogged_filter()
{
	unsigned long int last_timeCloggedFilters,     // tempo in ore (mem. in eeprom) dove che è stata fatta l'ultima pulizia.

	setting_timeCloggedFilters,  // impostazione in eeprom del tempo (gg) da far trascorrere prima di segnalare filtri sporchi.
	timeLimit_To_clogged_filers; // tempo limite superato il quale viene segnalata la pulizia filtri
	unsigned short enab_func =0;
	word DPP_data;                        // 2 byte nel caso di DPP attivo così suddivisi:
	Byte DPP_FilterLevel;                 // 1 byte: Soglia percentuale di cambio filtro per DPP
	Byte DPP_Threshold;	                // 1 byte: bit[0]:   Richiesta: 0 = Nessuna richiesta, 1 = Richiesta lettura
	//         bit[1]:   Tipo Lettura Richiesta: 0 = calibrazione, 1 = verifica
	//         bit[2-6]: Ultima lettura livello filtro (0-20) (in step di 5%)
	Byte calibrazione;                    //  Valore percentuale di calibrazione ( 0 = nessuna calibrazione effettuata)
	Byte cntUpdate_SettingPar;

	enab_func = read_word_eeprom(ADDR_EEP(Enab_Fuction));

	// Se la funzione di controllo è disabilitata, togliamo l'allarme e usciamo
	if((enab_func & (1 << ENAB_CTRL_FILTER))==0) {
		RemoveAlarm(ALM_FILTER);
		return;
	}


	last_timeCloggedFilters    = read_Dword_eeprom(ADDR_EEP(time_lastCloggedFilters));
	calibrazione               = read_byte_eeprom(ADDR_EEP(DPP_Calibrationvalue));

	// Se sono attivo come DPS (timer) faccio una semplice verifica temporale

	if((enab_func & (1 << ENAB_CTRL_FILTER))!=0 & (enab_func & (1 << ENAB_DPP))==0 ) {

		setting_timeCloggedFilters = (long)read_word_eeprom(ADDR_EEP(gg_manut_Filter)) * 24;
		timeLimit_To_clogged_filers = setting_timeCloggedFilters + last_timeCloggedFilters;

		if(countHoursFunz > timeLimit_To_clogged_filers)
		InsertAlarm(ALM_FILTER);
		else
		RemoveAlarm(ALM_FILTER);

	}

	// Se sono attivo come DPP (pressostato) faccio una semplice verifica 1 volta ogni 200 ore
	if((enab_func & (1 << ENAB_CTRL_FILTER))!=0 & (enab_func & (1 << ENAB_DPP))!=0 ) {

		if (last_timeCloggedFilters == 0)
		RemoveAlarm(ALM_FILTER);

		// ogni 200 ore di funzionamento (circa 8 giorni) faccio una verifica di filtro sporco
		// dopo che sono trascorsi i primi 8 giorni
		if  (countHoursFunz >= 200)
		{
			timeLimit_To_clogged_filers = (countHoursFunz - last_timeCloggedFilters) % 200;
		}
		else //non faccio nessuna verifica
		timeLimit_To_clogged_filers =1;
		

		if(timeLimit_To_clogged_filers == 0 & CkAlarm(ALM_FILTER)==0 & calibrazione != 0 )   //se sono passate 200 ore e non ho l'allarme filtri attivo e ho fatto la calibrazione
		{
			DPP_data = (word)read_word_eeprom(ADDR_EEP(gg_manut_Filter));
			DPP_FilterLevel = DPP_data >>   8; //Ricavo il byte alto
			DPP_Threshold   = DPP_data & 0xFF;  //Ricavo il byte basso

			DPP_FilterLevel = DPP_FilterLevel | 0x03; // Attivo la richiesta di verifica
			DPP_data = DPP_Threshold | (DPP_FilterLevel << 8); // ricompongo i byte e scrivo
			write_word_eeprom(ADDR_EEP(gg_manut_Filter), DPP_data); // Mando la richiesta di verifica (scrivo in Eeprom il valore)
			cntUpdate_SettingPar = read_byte_eeprom( ADDR_EEP( cntUpdate_SettingPar ) );
			cntUpdate_SettingPar++;
			write_byte_eeprom( ADDR_EEP( cntUpdate_SettingPar ), cntUpdate_SettingPar );
			return;
		}

		if(timeLimit_To_clogged_filers == 0 & CkAlarm(ALM_FILTER)==0 & calibrazione == 0 )   //se sono passate 200 ore e non ho l'allarme filtri attivo e NON ho fatto la calibrazione --> passo in DPS 90gg
		{
			enab_func &= ~(1 << ENAB_DPP);
			write_word_eeprom(ADDR_EEP(Enab_Fuction),enab_func);
			write_word_eeprom(ADDR_EEP(gg_manut_Filter), 90);
			write_Dword_eeprom(ADDR_EEP(time_lastCloggedFilters), countHoursFunz);
			cntUpdate_SettingPar = read_byte_eeprom( ADDR_EEP( cntUpdate_SettingPar ) );
			cntUpdate_SettingPar++;
			write_byte_eeprom( ADDR_EEP( cntUpdate_SettingPar ), cntUpdate_SettingPar );
			return;
		}


	}

}


//---------------------------------------------------------------------
//  Verifica lo sporcamento dei filtri, la funzione gira nella
//  operatingHoursCounter() che sta nello sheduler.
//---------------------------------------------------------------------
int cnt_tempo=0;
int cnt_step=0;

void  verifica_clogged_filter()
{
	unsigned short enab_func =0;
	word DPP_data;
	Byte DPP_FilterLevel;
	Byte DPP_Threshold;
	Byte calibrazione;
	enab_func = read_word_eeprom(ADDR_EEP(Enab_Fuction));
	DPP_data = (word)read_word_eeprom(ADDR_EEP(gg_manut_Filter));
	DPP_FilterLevel = DPP_data >>   8;  //Ricavo il byte alto
	DPP_Threshold   = DPP_data & 0xFF;  //Ricavo il byte basso
	int dpp_status;
	float sporcamento;
	Byte cntUpdate_SettingPar;
	Byte cntUpdate_info;
	unsigned long int last_timeCloggedFilters;

	//  Serial_println1(" <<< VERIFICA prima IF >>>");
	if((enab_func & (1 << ENAB_CTRL_FILTER))!=0 & (enab_func & (1 << ENAB_DPP))!=0  & (DPP_FilterLevel & 0x03)==3 ) {

		//  Controlliamo il livello di sporcamento
		//  Siamo in verifica, prendiamo il controllo dei motori
		
		//  Leggiamo gli stati
		calibrazione  = read_byte_eeprom(ADDR_EEP(DPP_Calibrationvalue));

		sData.status_unit |= (1 << POS_BIT_DPP_CHECK); // prendiamo il controllo motori

		//  Serial_println1(" <<< VERIFICA dopo IF >>>");
		if (cnt_step==0)
		{
			sData.speed_motors_R = calibrazione;
			sData.speed_motors_F = calibrazione;
			cnt_step =1;
		}
		
		//  Passo oltre ogni 9 cicli (circa 10 sec.) in modo da dare tempo hai motori di adattarsi
		if(++cnt_tempo <= TEMPO)
		return;

		dpp_status = analogRead(pAnSensQual_Ari1);  // leggo il pressostato

		if(dpp_status > 10 &  cnt_step == 1 & sData.speed_motors_R !=  MINSPEED  )  // è scattato e sono alla calibrazione precedente in partenza, decremento fintanto che smette di scattare
		{
			sData.speed_motors_R -= 13 ;
			sData.speed_motors_F -= 13 ;
			//      Serial_println1(" <<< VERIFICA sto DIMINUENDO >>>");
			cnt_tempo=(int)TEMPO/2;
			cnt_step =1;
			return;
		}
		else if(dpp_status > 10 &  cnt_step == 1 & sData.speed_motors_R ==  MINSPEED  )  // è scattato e sono al minimo non riesco a controllare sotto questa soglia
		{
			// Calcolo il fattore di sporcamento
			sporcamento = (int) ((1.0f  - (float) MINSPEED / (float) calibrazione) * 100.0f);
			if (sporcamento > DPP_Threshold)
			{
				DPP_FilterLevel = ((int)(sporcamento/5) << 2);             // Scrivo l'ultima verifica (20 step da 5%) e spengo la richiesta
				sData.status_unit &= ~(1 << POS_BIT_DPP_CHECK);            // libero il controllo motori
				InsertAlarm(ALM_FILTER);                                   // inserisco l'allarme filtro sporco
				last_timeCloggedFilters=1;
				write_Dword_eeprom(ADDR_EEP(time_lastCloggedFilters), last_timeCloggedFilters);
				cntUpdate_info = read_byte_eeprom( ADDR_EEP( cntUpdate_info ) );
				cntUpdate_info++;
				write_byte_eeprom( ADDR_EEP( cntUpdate_info ), cntUpdate_info );
				DPP_data = DPP_Threshold | (DPP_FilterLevel << 8);         // ricompongo i byte
				write_word_eeprom(ADDR_EEP(gg_manut_Filter), DPP_data);    // scrivo in Eeprom il valore
				cntUpdate_SettingPar = read_byte_eeprom( ADDR_EEP( cntUpdate_SettingPar ) );
				cntUpdate_SettingPar++;
				write_byte_eeprom( ADDR_EEP( cntUpdate_SettingPar ), cntUpdate_SettingPar );
				//     Serial_println1(" <<< VERIFICA MINIMO: SONO SPORCO >>>");
				cnt_tempo=0;                                               // azzero il contatore
				cnt_step=0;                                                // azzero il contatore
				return;
			}
			else
			{
				// Non ho visibilità sotto questa soglia, non sono ancora sporco a sufficienza, passo al dps
				write_byte_eeprom(ADDR_EEP(DPP_Calibrationvalue), 0);    // scrivo la calibrazione = 0 (passo a DPS)
				sData.status_unit &= ~(1 << POS_BIT_DPP_CHECK);            // libero il controllo motori
				DPP_FilterLevel |= 0x00;                                 // spengo la richiesta
				DPP_data = DPP_Threshold | (DPP_FilterLevel << 8);         // ricompongo i byte
				write_word_eeprom(ADDR_EEP(gg_manut_Filter), DPP_data);    // scrivo in Eeprom il valore
				cntUpdate_SettingPar = read_byte_eeprom( ADDR_EEP( cntUpdate_SettingPar ) );
				cntUpdate_SettingPar++;
				write_byte_eeprom( ADDR_EEP( cntUpdate_SettingPar ), cntUpdate_SettingPar );
				//     Serial_println1(" <<< VERIFICA MINIMO: PASSO A DPS >>>");
				cnt_tempo=0;                                             // azzero il contatore
				cnt_step= 0;                                             // azzero il contatore
				return;
			}
		}
		else if(dpp_status < 10 &  cnt_step == 1 & sData.speed_motors_R >=  MINSPEED  )  // NON è scattato a una nuova soglia
		{
			// Calcolo il fattore di sporcamento
			sporcamento = (int) ((1.0f  - (float) sData.speed_motors_R / (float) calibrazione) * 100.0f);
			if (sporcamento > DPP_Threshold)
			{
				DPP_FilterLevel = ((int)(sporcamento/5) << 2);             // Scrivo l'ultima verifica (20 step da 5%) e spengo la richiesta
				sData.status_unit &= ~(1 << POS_BIT_DPP_CHECK);            // libero il controllo motori
				InsertAlarm(ALM_FILTER);                                   // inserisco l'allarme filtro sporco
				last_timeCloggedFilters=1;
				write_Dword_eeprom(ADDR_EEP(time_lastCloggedFilters), last_timeCloggedFilters);
				cntUpdate_info = read_byte_eeprom( ADDR_EEP( cntUpdate_info ) );
				cntUpdate_info++;
				write_byte_eeprom( ADDR_EEP( cntUpdate_info ), cntUpdate_info );
				DPP_data = DPP_Threshold | (DPP_FilterLevel << 8);         // ricompongo i byte
				write_word_eeprom(ADDR_EEP(gg_manut_Filter), DPP_data);    // scrivo in Eeprom il valore
				cntUpdate_SettingPar = read_byte_eeprom( ADDR_EEP( cntUpdate_SettingPar ) );
				cntUpdate_SettingPar++;
				write_byte_eeprom( ADDR_EEP( cntUpdate_SettingPar ), cntUpdate_SettingPar );
				//     Serial_println1(" <<< VERIFICA > MINIMO: SONO SPORCO >>>");
				cnt_tempo=0;                                               // azzero il contatore
				cnt_step=0;                                                // azzero il contatore
				return;
			}
			else
			{
				// I filtri non sono ancora sporchi a sufficienza, memorizzo il valore rilevato e aspetto la prossima verifica
				DPP_FilterLevel = ((int)(sporcamento/5) << 2);             // Scrivo l'ultima verifica (20 step da 5%) e spengo la richiesta
				sData.status_unit &= ~(1 << POS_BIT_DPP_CHECK);            // libero il controllo motori
				DPP_data = DPP_Threshold | (DPP_FilterLevel << 8);         // ricompongo i byte
				write_word_eeprom(ADDR_EEP(gg_manut_Filter), DPP_data);    // scrivo in Eeprom il valore
				cntUpdate_SettingPar = read_byte_eeprom( ADDR_EEP( cntUpdate_SettingPar ) );
				cntUpdate_SettingPar++;
				write_byte_eeprom( ADDR_EEP( cntUpdate_SettingPar ), cntUpdate_SettingPar );
				//    Serial_println1(" <<< VERIFICA > MINIMO: SONO ANCORA PULITO >>>");
				cnt_tempo=0;                                               // azzero il contatore
				cnt_step=0;                                                // azzero il contatore
				return;
			}

		}
		
	}
	else
	// non stato chiesta una verifica, usciamo
	return;



}


//---------------------------------------------------------------------
//  Calibrazione dei filtri, la funzione gira nella
//  operatingHoursCounter() che sta nello sheduler.
//---------------------------------------------------------------------

void  calibrazione_clogged_filter()
{
	unsigned short enab_func =0;
	word DPP_data;
	Byte DPP_FilterLevel;
	Byte DPP_Threshold;
	int dpp_status;
	Byte cntUpdate_SettingPar;
	
	enab_func = read_word_eeprom(ADDR_EEP(Enab_Fuction));
	DPP_data = (word)read_word_eeprom(ADDR_EEP(gg_manut_Filter));
	DPP_FilterLevel = DPP_data >>   8;  //Ricavo il byte alto
	DPP_Threshold   = DPP_data & 0xFF;  //Ricavo il byte basso

	// Serial_println1(" <<< CALIBRAZIONE prima IF >>>");
	if((enab_func & (1 << ENAB_CTRL_FILTER))!=0 & (enab_func & (1 << ENAB_DPP))!=0  & (DPP_FilterLevel & 0x01)==1 & (DPP_FilterLevel & 0x02)==0  ) {

		//  Facciamo la taratura del sistema
		//  Siamo in Calibrazione
		sData.status_unit |= (1 << POS_BIT_DPP_CHECK); // prendiamo il controllo motori
		
		if (cnt_step==0)
		{
			sData.speed_motors_R = 255;
			sData.speed_motors_F = 255;
			cnt_step =1;
		}
		//   Serial_println1(" <<< CALIBRAZIONE DOPO IF >>>");
		if(++cnt_tempo <= TEMPO)
		return;

		dpp_status = analogRead(pAnSensQual_Ari1);  // leggo il pressostato
		

		if(dpp_status < 10 & sData.speed_motors_R == 255)          // sono al massimo e non scatta: il punto di sporcamento > della curva massima = calibrazione a 255
		{
			write_byte_eeprom(ADDR_EEP(DPP_Calibrationvalue), 255);  // scrivo la calibrazione
			sData.status_unit &= ~(1 << POS_BIT_DPP_CHECK);          // libero il controllo motori
			DPP_FilterLevel &= ~0x03;                                // spengo la richiesta
			DPP_data = DPP_Threshold | (DPP_FilterLevel << 8);         // ricompongo i byte
			write_word_eeprom(ADDR_EEP(gg_manut_Filter), DPP_data);    // scrivo in Eeprom il valore
			cntUpdate_SettingPar = read_byte_eeprom( ADDR_EEP( cntUpdate_SettingPar ) );
			cntUpdate_SettingPar++;
			write_byte_eeprom( ADDR_EEP( cntUpdate_SettingPar ), cntUpdate_SettingPar );
			cnt_tempo=0;                                             // azzero il contatore
			cnt_step=0;                                              // azzero il contatore
			//     Serial_println1(" <<< CALIBRAZIONE 255 >>>");
			return;
		}
		else if (dpp_status > 10 & cnt_step==1)                      // sono al massimo e scatta: vediamo cosa succede al minimo
		{
			sData.speed_motors_R =  MINSPEED;
			sData.speed_motors_F =  MINSPEED;
			//     Serial_println1(" <<< VADO AL MINIMO >>>");
			cnt_tempo=0;
			cnt_step =2;
			return;
		}
		else if (cnt_step==2 & dpp_status > 10 & sData.speed_motors_R ==  MINSPEED) // sono al minimo e scatta: non abbiamo visibilità per lo scatto del DPP, torniamo al DPS (calibrazione =0)
		{
			write_byte_eeprom(ADDR_EEP(DPP_Calibrationvalue), 0);    // scrivo la calibrazione = 0 (passo a DPS)
			sData.status_unit &= ~(1 << POS_BIT_DPP_CHECK);          // libero il controllo motori
			DPP_FilterLevel &= ~0x03;                                // spengo la richiesta
			DPP_data = DPP_Threshold | (DPP_FilterLevel << 8);         // ricompongo i byte
			write_word_eeprom(ADDR_EEP(gg_manut_Filter), DPP_data);    // scrivo in Eeprom il valore
			cntUpdate_SettingPar = read_byte_eeprom( ADDR_EEP( cntUpdate_SettingPar ) );
			cntUpdate_SettingPar++;
			write_byte_eeprom( ADDR_EEP( cntUpdate_SettingPar ), cntUpdate_SettingPar );
			cnt_tempo=0;                                             // azzero il contatore
			cnt_step= 0;                                             // azzero il contatore
			//    Serial_println1(" <<< CALIBRAZIONE 0 >>>");
			return;
		}
		else if (cnt_step==2 & dpp_status < 10 )                   // sono al minimo (inizialmente) e non scatta: incrementiamo fino a trovare il valore per cui scatta
		{
			sData.speed_motors_R += 13 ;
			sData.speed_motors_F += 13 ;
			//    Serial_println1(" <<< INCREMENTO >>>");
			cnt_tempo=TEMPO/2;
			cnt_step =2;
			return;
		}
		else if (cnt_step==2 & dpp_status > 10 & sData.speed_motors_R > MINSPEED)  // Ho trovato la velocità per cui scatta: la memorizzo nella calibrazione
		{
			write_byte_eeprom(ADDR_EEP(DPP_Calibrationvalue), sData.speed_motors_R);    // scrivo la calibrazione = valore di regolazione motori
			sData.status_unit &= ~(1 << POS_BIT_DPP_CHECK);          // libero il controllo motori
			DPP_FilterLevel &= ~0x03;                                // spengo la richiesta
			DPP_data = DPP_Threshold | (DPP_FilterLevel << 8);         // ricompongo i byte
			write_word_eeprom(ADDR_EEP(gg_manut_Filter), DPP_data);    // scrivo in Eeprom il valore
			cntUpdate_SettingPar = read_byte_eeprom( ADDR_EEP( cntUpdate_SettingPar ) );
			cntUpdate_SettingPar++;
			write_byte_eeprom( ADDR_EEP( cntUpdate_SettingPar ), cntUpdate_SettingPar );
			//   Serial_println1(" <<< CALIBRAZIONE > MINSPEED >>>");
			cnt_tempo=0;                                             // azzero il contatore
			cnt_step= 0;                                             // azzero il contatore
			return;
		}

	}
	else
	// non stato chiesta una calibrazione, usciamo
	return;
}
