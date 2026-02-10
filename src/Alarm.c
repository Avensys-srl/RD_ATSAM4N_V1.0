/*
 * Alarm.cpp
 *
 */ 

#include "asf.h"

#include "alarm.h"
#include "data.h"


#ifdef VIEW_DEBUG_CODE_NO_KTS //''''''''''''''''''''''''''''''''''''''''''''''''' x_dbg
#define VIEW_INSERT_ALARM
#endif

#define MAX_PERSIST  8

Byte persist_alarm[TOT_ALARMS];

//-----------  Strutture Dati RAM / EEPROM -----------
extern SDATA  sData;

//----------------------------------------------------
// Inizializza gli allarmi
//----------------------------------------------------
void InitAlarm(void) {
	int i;
	
	for(i=0; i < 13; i++)
	sData.Events[i] = 0;
	
	for(i=0; i < TOT_ALARMS; i++)
	persist_alarm[i] = 0;
}

//----------------------------------------------------
// Ritorna lo stato dell'allarme, se attivo TRUE (1)
//----------------------------------------------------
Bool CkAlarm(int nalm)
{
	Byte ie, sh;
	
	ie = (nalm >> 3);
	sh = nalm & 0x07;
	
	if(sData.Events[ie] & (1 <<sh))
	return true;
	
	return false;
}

//----------------------------------------------------
// Inserisce l'allarme nell'array Events
//----------------------------------------------------
void InsertAlarm(int nalm)
{
	Byte ie, sh, msk = 0;
	
	ie = (nalm >> 3);
	sh = nalm & 0x07;
	msk |= (1 << sh);
	
	sData.Events[ie] |= msk;
}

//----------------------------------------------------
// Inserisce l'allarme nell'array Events
//----------------------------------------------------
void InsertDelayAlarm(int nalm)
{
	Byte ie, sh, msk = 0;
	
	ie = (nalm >> 3);
	sh = nalm & 0x07;
	msk |= (1 << sh);
	
	if(persist_alarm[nalm] <= MAX_PERSIST)
	persist_alarm[nalm]++;
	else
	sData.Events[ie] |= msk;
}


//----------------------------------------------------
// rimuove l'allarme dall'array Events
//----------------------------------------------------
void RemoveAlarm(int nalm)
{
	Byte ie, sh;
	
	ie = (nalm >> 3);
	sh = nalm & 0x07;
	
	if(sData.Events[ie] & (1 << sh)) {
		sData.Events[ie] &= ~(1 << sh);
	}
	
	persist_alarm[nalm] = 0;
}
