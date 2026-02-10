/*
 * Seriale_2.cpp
 *
 * Descrizione:
 *   La seconda seriale, viene fatta utilizzando un interrupt esterno
 *   per la ricezione.
 *   la velocitÃ  Ã¨ 38400 Bps.
 *
 *   pinRX2  D7
 *   pinTX2  D6
 */ 


#include "asf.h"

#include "Protocol_Ser2.h"

//---- STATI RICEZIONE SERIALI --------
#define  RUN_DOWNLOAD              1
#define  BUFFER_RX_EMPY            0
#define  ERROR_HEAD_MESSAGE       -1
#define  ERROR_LEN_MESSAGE        -2
#define  ERROR_CKSUM_16           -3

//------- in base al BR calcola il periodo del bit -----------------
#define INT_RX_BYTE        1
#define INT_TX_BYTE        4

Byte _FLAG_INT_UART2_SW  = 0;
//------ status Uart2 ------------
#define UART2_FREE         0
#define RECEPTION_MESSAGE  1
#define SEND_MESSAGE       2 

#define lowByte(i)    ( (char) i )
#define highByte(i)    ( (char) ( ((int) i) >> 8) )

#define word uint16_t

#define DIM_MAX_BUFF_SER1         128  // Attenzione il driver della uart1 HA DIMENSIONE massima di 128 Byte
#define DIM_MAX_BUFF_SER2         128

#define RX_BUFFER_SIZE 128

Byte status_Uart2 = UART2_FREE;

unsigned int _bitPeriod  = 260;
Byte offset = 0;
int lenTxU2 = 0;
int lenRXU2 = 0;
int iBuffU2 = 0;
Byte lenRXU2_old = 0xFF;

static Byte rx_buf2_tail = 0;

extern Byte buff_ser2[DIM_MAX_BUFF_SER2];   // buffer di appoggio

extern word calc_cksum_16(Byte * buffer, int lenght);

//-----------------------------------------------------------------------------
//  Abilita L'interrupt Esterno di ricezione  dello start bit
//-----------------------------------------------------------------------------   
void EnableInt2RX()
{
    
    /*CloseTimer5();
    // Clear the interrupt flag
    INTClearFlag(INT_T5);
    
    // inizializza l'interrupt INT2
    ConfigINT2(EXT_INT_ENABLE | FALLING_EDGE_INT | EXT_INT_PRI_4);
    
    // abilita il pin per L'interrupt external 2
    EnableINT2;
    
    offset = 0;
    lenRXU2_old = 0xFF;
        
     //Clear the interrupt flag
    INTClearFlag(INT_INT2);  */
}  

/*=================================================================
 * Inizializza la seconda seriale. 
 *  Pin RX2 = D7 -> INT2
 *  Pin TX2 = D6 -> RD2
 *
 *=================================================================*/
 void init_Seriale_2(long baudRate)
 {   
    // configura il pin TX2 come output
    /*pinMode(pTX2,OUTPUT);   
    digitalWrite(pTX2,    HIGH);  
   
    EnableInt2RX();
    
    //---  Inizializze timer TMR5 : Interrupt : 100ms ---
    // Configure Timer 5 using PBCLK as input, 1:256 prescaler
    // BUAD: 19200: interrupt every 52,0833us :->  (80MHZ  * 52,0833us) / 8 = 520,83  
    // BUAD: 38400: interrupt every 26,0417us :->  (80MHZ  * 26,0417us) / 8 = 260,42    
    _bitPeriod =  (unsigned int)((long)(80000000L / (baudRate * 8))) -1;*/
         
}

//--------------------------------------
// Libera la seriale 2, e riparti da zero
//--------------------------------------
void free_serial_2()
{
    iBuffU2 = 0;
    status_Uart2 = UART2_FREE;
    lenRXU2_old  = 0xFF; 
    EnableInt2RX();
}  


//-----------------------------------------------------------------------------
//  Fai patire il timer per cadenzare i bit  
//-----------------------------------------------------------------------------   
void start_Timer_bit(unsigned int period)
{
   /* OpenTimer5((T5_SOURCE_INT | T5_IDLE_CON | T5_PS_1_8),  period); 

    // Set up the timer interrupt with a priority of 2
    INTEnable(INT_T5, INT_ENABLED);
    INTSetVectorPriority(INT_TIMER_5_VECTOR, INT_PRIORITY_LEVEL_2);
    INTSetVectorSubPriority(INT_TIMER_5_VECTOR, INT_SUB_PRIORITY_LEVEL_2);
    
    // Start Timer1
    T5CONbits.TON   = 1;  */     
}  


//-----------------------------------------------------------------------------
//    
//----------------------------------------------------------------------------- 
/*void __ISR(_TIMER_5_VECTOR, ipl2) Timer5Handler(void)
{
    byte persist_bit = 0; 
    
    // Clear the interrupt flag
    INTClearFlag(INT_T5);
    
//    digitalWrite(pStatusLedI2C, HIGH);  // led ACCESO 

    if(_FLAG_INT_UART2_SW == INT_RX_BYTE)
    {
        // offset of the bit in the byte: from 0 (LSB) to 7 (MSB)        
        if(offset <= 7)
        {
          persist_bit = 0; 
          // effettuiamo 12 campionamento tra 1/4 e 3/4 del bit
          for(short sample = 0; sample < 10; sample++) {
             if(digitalRead(pRX2))
               persist_bit++;
            
             asm("nop"); 
             asm("nop"); 
             asm("nop"); 
             asm("nop"); 
          }
                                                  
          if(persist_bit >= 8) 
             buff_ser2[lenRXU2] |= (1 << offset); // 8 su 10 alto, allora è 1
          else  
             buff_ser2[lenRXU2] &= ~(1 << offset);             
        } 
        
        offset++;               
        if(offset > 8) {
            offset = 0;                      
            EnableInt2RX(); // riabilita l'interrupt
            _FLAG_INT_UART2_SW = 0;
            status_Uart2 = RECEPTION_MESSAGE;
            lenRXU2++;
            if(lenRXU2 >= 127)
              lenRXU2 = 0;     
        }           
        
    }else if(_FLAG_INT_UART2_SW == INT_TX_BYTE){
        // trasmissione : 8 bit dati + 1 bit stop
        if(offset == 0)
          digitalWrite(pTX2, LOW);               // bit Start 
        else if(offset <= 8)  { 
          if(buff_ser2[iBuffU2] & (1 << (offset-1))) // bit 0, ... ,7 : DATA
             digitalWrite(pTX2, HIGH);
          else  
             digitalWrite(pTX2, LOW);
        }else 
           digitalWrite(pTX2, HIGH);             // bit 9: STOP 
           
        offset++;  
           
        if(offset >= 10) {
           offset = 0;
           iBuffU2++;
           if(iBuffU2 >= lenTxU2) {
               EnableInt2RX(); // riabilita l'interrupt per la ricezione
               _FLAG_INT_UART2_SW = 0;
               status_Uart2 = UART2_FREE;
               digitalWrite(pTX2,    HIGH);  
           }           
        }  
    }else {
       EnableInt2RX(); // riabilita l'interrupt 
       status_Uart2 = UART2_FREE;
       digitalWrite(pTX2,    HIGH);         
    }   
    
//    digitalWrite(pStatusLedI2C, LOW);  // led spento 
  
}*/


/*===========================================================*
          interrupt handler: INT2
  
  L'interrupt si attiva allo start_bit del byte        

  one byte of serial data (LSB first)
    .....\ +    +    +    +    +    +    +    +    +   + ... sample  
   	 \---/\---/\---/\---/\---/\---/\---/\---/\---/---\
   	 start  0    1    2    3    4    5    6    7  stop
  
 
*============================================================*/
/*void __ISR(_EXTERNAL_2_VECTOR, ipl4) IntRx2Handler(void)
{
    int cnt; 
    
    asm ("ei");
    
    // Chiudi l'interrupt e gestisci solo il pin
    CloseINT2();    
   
    // Imposta il pin RD9 (D7) come input per la lettura del Bit
    pinMode(pRX2,INPUT);
    
    _FLAG_INT_UART2_SW = INT_RX_BYTE;
    
    // andiamo a posizionarci circa 1/5 del bit per l'acquisizione dell'input.
    for(cnt=0; cnt <= (_bitPeriod >> 4); cnt++) 
      asm("nop"); 
    
    offset = 0;     
    start_Timer_bit(_bitPeriod - 1);

}  */

/*===========================================================*
   Invia un messaggio sul canale della seriale 2     
 *===========================================================*/
void Send_Message_Ser2(Byte *pBuff, int len)
{
  int i;
  word cksum16;  

  for(i=0; i <(len - 1); i++) 
    buff_ser2[i] = *pBuff++;

  // secondo Protocol_Ser2, completiamo il buffer prima di inviare
  buff_ser2[IHM2_START_MSG_0] = PROTO_START_MSG_0;    
  buff_ser2[IHM2_START_MSG_1] = PROTO_START_MSG_1;    
  
  i = buff_ser2[IHM2_POS_CRC_LO];
  
  // calcoliamo il Cksum_16
  cksum16 = calc_cksum_16(&buff_ser2[0],  i);
  
  buff_ser2[i]   = lowByte(cksum16);    
  buff_ser2[i+1] = highByte(cksum16); 
  
  lenTxU2 = i + 2;

  // -- Inviamo il messaggio --
  _FLAG_INT_UART2_SW = INT_TX_BYTE;
  status_Uart2       = SEND_MESSAGE;
  
  //CloseINT2();   // disabilitiamo la ricezione
  offset  = 0;    
  iBuffU2 = 0;
   
  start_Timer_bit(_bitPeriod);
  
  Serial2_write(buff_ser2, lenTxU2); 
}

/*===========================================================*
   Func:
    Read_Message_Ser2

   Description:    
     Se ritorniamo un valore >0 alora vi Ã¨ un messaggio buono
     da analizzare.
     codici di ritorno:
		 1 = RUN_DOWNLOAD
		 0 = BUFFER_RX_EMPY                      
        -1 = ERROR_HEAD_MESSAGE               
        -2 = ERROR_LEN_MESSAGE          
        -3 = ERROR_CKSUM_16         
 *===========================================================*/
int Read_Message_Ser2(void)
{  
  unsigned short Cksum16, Cksum16_calc;
  int  Byte_aval = 0;
  int  i, len;
  int iCks;
  unsigned short cks16_buff, cks16_calc; 
  
  Byte_aval = Serial2_available();
  
  // se abbiamo ricevuto dei carattere, attendiamo la fine del messaggio prima di andare a scaricarlo.
  if(Byte_aval) {
	  if(Byte_aval != rx_buf2_tail) {
		  // finchè il numero di caratteri aumenta aspetta ad andare a scaricare
		  // il messaggio.
		  rx_buf2_tail = Byte_aval;
		  return RUN_DOWNLOAD; // aquisizione del messaggio in corso, aspettiamo di che finisca.
	  }
	  }else {
	  if (rx_buf2_tail == 0)
	  return BUFFER_RX_EMPY; // BUFFER EMPY
  }
  
  //--------------------------------------------------------------------
  // Abbiamo ricevuto un messaggio, andiamo a verificare se è corretto
  //---------------------------------------------------------------------
  len = (rx_buf2_tail % RX_BUFFER_SIZE);
  rx_buf2_tail = 0;

  // scarichiamo il messaggio
  for(i=0; i < len; i++) {
	  buff_ser2[i] = Serial2_read();
  }
  
  //---------- (1) Controllo: Testata del messaggio ----------
  if ((buff_ser2[IHM2_START_MSG_0] != PROTO_START_MSG_0) || (buff_ser2[IHM2_START_MSG_1] != PROTO_START_MSG_1)) {
	  return ERROR_HEAD_MESSAGE;
  }
  
  //---------- (2) Controllo: il messaggio è per me? ----------
  if ((buff_ser2[IHM2_ADDR_RECEIVER] != ADDR_RD_UNIT)) {
	  return BUFFER_RX_EMPY;
  }
  
  //---------- (3) Controllo: lunghezza del messaggio ----------
  iCks = buff_ser2[IHM2_POS_CRC_LO];
  if (iCks != (len - 2)) {
	  return ERROR_LEN_MESSAGE;
  }
  
  //---------- (4) Controllo: verifico il Cksum ----------
	cks16_calc = (unsigned int)calc_cksum_16(&buff_ser2[0], iCks);
	cks16_calc &= 0x0FFFF;
	cks16_buff = (unsigned int)(buff_ser2[iCks + 1] << 8) & 0xFF00;
	cks16_buff |= buff_ser2[iCks];
	  
	if(cks16_calc != cks16_buff) {
		return ERROR_CKSUM_16;
		}else {
		return len;
		  
	//---------- (5) Messaggio Ok, Copia il buffer temporaneo nel buffer 2 ----------
	for(i=0; i <len; i++)
	buff_ser2[i] = buff_ser2[i];
	}
	  
}
