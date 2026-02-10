/*
 * Seriale_1.cpp
 *
 * Author/Date: Maurizio Colmone dic.2013
 *
 * Description: Il file gestisce il traffico sulla seriale 1, tramite
 *              il protocollo ser1.
 */ 

#include "asf.h"

#include "Protocol_Ser1.h"
#include <stdint.h>

//---- STATI RICEZIONE SERIALI --------
#define  RUN_DOWNLOAD              1
#define  BUFFER_RX_EMPY            0
#define  ERROR_HEAD_MESSAGE       -1
#define  ERROR_LEN_MESSAGE        -2
#define  ERROR_CKSUM_16           -3

#define word uint16_t
#define lowByte(i)    ( (char) i )
#define highByte(i)    ( (char) ( ((int) i) >> 8) )
#define RX_BUFFER_SIZE 128

#define DIM_MAX_BUFF_SER1         128  // Attenzione il driver della uart1 HA DIMENSIONE massima di 128 Byte
#define DIM_MAX_BUFF_SER2         128

//--------------- Var Global -----------------------------

static Byte rx_buf_tail = 0;
extern Byte buff_ser1[DIM_MAX_BUFF_SER1];     // buffer di appoggio
unsigned short zaki1;

//---------------------------------------------------------
// Funzione che Calcola il Cksum su 16 bit e ritorna il 
// complemento a 1 della somma.
//---------------------------------------------------------
word calc_cksum_16(Byte * buffer, int lenght)
{
  
  word cksum16_temp = 0;
  
  for(int i=0; i < lenght; i++)
    cksum16_temp += buffer[i];
    
  return ~cksum16_temp;
}


//---------------------------------------------------------
// Manda un messaggio sulla seriale 1. 
//---------------------------------------------------------
void Write_Message_Ser1(Byte *buff)
{
   unsigned short Cksum16, len, i;
   Byte i_pos_crc_lo;
   
   // testa del messaggio
   buff[IHM1_START_MSG_0] = PROTO_START_MSG_0;
   buff[IHM1_START_MSG_1] = PROTO_START_MSG_1;
   
   // lunghezza del messaggio
   i_pos_crc_lo = buff[IHM1_POS_CRC_LO]; 
   len = i_pos_crc_lo + 2; 
   len &= 0x7F; // max 127 byte. 
   
   // calcola il cksum
   Cksum16 = calc_cksum_16(buff,  i_pos_crc_lo);
   
   buff[i_pos_crc_lo ]   = lowByte(Cksum16);
   buff[i_pos_crc_lo+1]  = highByte(Cksum16);

   Serial_write(buff, len);     
}


/*---------------------------------------------------------------                                                                
                                                                 
  Name:  Read_Message_Ser1                                          
   
  Description:
    Analizza se vi sono messaggi in ricezione.
    Tempo di ricezione di 1 byte è circa 1,05ms.
        
                                                                      
  Return:
    Value statusRX |            Description
   ----------------+-----------------------------------------
         > 4           Ricezione OK , Lunghezza del Messaggio
          1            RUN_DOWNLOAD
          0            BUFFER EMPY nessun messaggio.
         -1            ERROR_HEAD_MESSAGE
         -2            ERROR_LEN_MESSAGE
         -2            ERROR_CKSUM_16
         
 --------------------------------------------------------------- */          
int Read_Message_Ser1()
{  
   unsigned short Cksum16, Cksum16_calc;
   int  Byte_aval = 0;
   int  i, len;
  

#ifdef _MAPPING_FUNC  //------------------------------x_dbg
   return 0;
#endif   //-------------------------------------------

  
  
   Byte_aval = Serial_available();
   
   // se abbiamo ricevuto dei carattere, attendiamo la fine del messaggio prima di andare a scaricarlo.
   if(Byte_aval) {      
       if(Byte_aval != rx_buf_tail) {
         // finchè il numero di caratteri aumenta aspetta ad andare a scaricare
         // il messaggio.
         rx_buf_tail = Byte_aval;
         return RUN_DOWNLOAD; // aquisizione del messaggio in corso, aspettiamo di che finisca.
       }            
   }else { 
     if (rx_buf_tail == 0)
       return BUFFER_RX_EMPY; // BUFFER EMPY     
   }  
   
   //--------------------------------------------------------------------
   // Abbiamo ricevuto un messaggio, andiamo a verificare se è corretto
   //---------------------------------------------------------------------   
   len = (rx_buf_tail % RX_BUFFER_SIZE);
   rx_buf_tail = 0;

   // scarichiamo il messaggio
   for(i=0; i < len; i++) {
     buff_ser1[i] = Serial_read();     
   }  
   
   // 1. Verifichiamo L'intestazione del messaggio.
   if((buff_ser1[IHM1_START_MSG_0] != PROTO_START_MSG_0) || (buff_ser1[IHM1_START_MSG_1] != PROTO_START_MSG_1))
     return  ERROR_HEAD_MESSAGE;
     
   // 2. Verifichaimo la lunghezza del messaggio.
   if(((buff_ser1[IHM1_POS_CRC_LO] + 2) != len) || ( len <= IHM1_START_DATA))
     return  ERROR_LEN_MESSAGE;
   
   // 3. Verifichiamo il cksum_16 del messaggio
   i = buff_ser1[IHM1_POS_CRC_LO];
   Cksum16 = (unsigned short)(((unsigned short)buff_ser1[i+1] << 8) | buff_ser1[i]) ;
   Cksum16 &= 0x0FFFF;
   Cksum16_calc = calc_cksum_16(buff_ser1, i);
               
   if (Cksum16 != Cksum16_calc)  
     return  ERROR_CKSUM_16;


//---------------------------------------------------
/*
if((buff_ser1[IHM1_TYPE_COMAND] == COMAND_WRITE_EEPROM) && (buff_ser1[IRQW_ADDR_BYTE_START_EEP] == 46)) {
  Serial_println1(" ");
  Serial_println1("- - - - - - - - - - - - - - - - - ");
  Serial_println1("Wr.ADDR:046");
  
  for(i=0;  i < buff_ser1[IRQW_ADDR_NUM_BYTE_EEP]; i++)  {
     Serial_print1(buff_ser1[IRQW_START_DATA_EEPROM + i], HEX); 
     Serial_print1(" "); 
  }   
  
  Serial_println1(" "); 
  Serial_println1("- - - - - - - - - - - - - - - - - ");  
}  
*/
//---------------------------------------------------
                   
    
    return len;
 
 }
 