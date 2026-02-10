/*
 * Scheduler.h
 *
 * Author: Maurizio Colmone
 *
 * Created on 29 luglio 2013, 8.14
 */ 


#ifndef SCHEDULER_H_
#define SCHEDULER_H_

	#define MAX_PROCESS_RUN  16   // numero massimo di processi in corso

	typedef int (*CallBack_FUNC)();

	//--- Definizione struttura dello sheduler
	typedef struct {
		unsigned int  enable;
		unsigned int  countdown_0s1;
		unsigned int  value_timer_0s1;
		CallBack_FUNC call_function;  // puntatore alla funzione
	}S_SHEDULER;

	extern S_SHEDULER  sSheduler[MAX_PROCESS_RUN]; // massimo 8 processi


	//---- proto  ---
	extern void init_scheduler();
	extern void process_scheduler();
	extern int  insert_function_scheduler(CallBack_FUNC name_func, unsigned int sample);
	extern void Scheduler_Timer_Callback ( void );

#endif /* SCHEDULER_H_ */