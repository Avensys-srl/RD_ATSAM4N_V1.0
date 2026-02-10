//////////////////////////////////////////////////////////////////////////////////////////////// 
//
//    FILE: MASTER_I2C.cpp
//  AUTHOR: Maurizio Colmone.
// VERSION: 1.00
// PURPOSE: 
//
// HISTORY: 
// 0.1.00 - 2014-01-21 initial version
//
//

#include "asf.h"

#include "MASTER_I2C.h"


////////////////////////////////////////////////////////////////////////////////////////////////
//
//   PUBLIC
//
////////////////////////////////////////////////////////////////////////////////////////////////


//------------------------------------------------------------------
//
//------------------------------------------------------------------
void I2C_freeBus(void)
{
  _set_SCL();
  _set_SDA();
}

//--------------------------------------------------------------------------------------------
// Function: I2C_WriteSlave
//
// Description: 
//     2- MASTER -> SLAVE : LETTURA (R/W=1).
// 
//  1) First part: Request Read data
//	
//  --------+-+---------------+-+---------------+-+---------------+-+---------------+-+.. 
//          |S| ADDR_SLAV+ R/W|A|   ADDR_DATA   |A|  NUM_BYTE     |A|    CKSUM_1    |A| 
//  --------+-+---------------+-+---------------+-+---------------+-+---------------+-+... 
//   SDA   1 0 b b b b b b b 1 0 b b b b b b b b 0 b b b b b b b b 0 b b b b b b b b 0 ...
//  --------+-+---------------+-+---------------+-+---------------+-+---------------+-+... 
//   SCL   --|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_
//	 
//	
//	2) Second part: Read Data from Slave
//	
//  . . ---+---------------+-+---------------+-+. . .+---------------+-+---------------+-+----+
//  ReSTART| 0 + ADDR_SLAV |A|     DATA 0    |A|     | DATA +NUM_BYTE|A|    CKSUM_2    |N|STOP 
//  . .----+---------------+-+---------------+-+. . .+---------------+-+---------------+-+----+
//   SDA   0 0 b b b b b b b 0 b b b b b b b b 0. . . b b b b b b b b 0 b b b b b b b b 1 1         
//  . . ---+---------------+-+---------------+-+. . .+---------------+-+---------------+-+----+
//   SCL   |_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|. . ._|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|---
//	    
//-----------------------------------------------------------------------------------------------
int ReadSlave(Byte addressSlave, Byte addrData, Byte *pDataRD, Byte nByte)
{    
  
	Byte addressByte;
	Byte i;
	Byte temp, checksum, checksum_calc;

  
	addressByte = (addressSlave << 1) + 1;
 
	I2C_sendStart();
  
	//-------- 1^ Byte:  indirizzo Slave + richiesta di lettura -----//   
	I2C_sendData(addressByte); 

	if(I2C_checkAck()!= ACK) {
		I2C_sendStop(); 
		return _ERROR_READ_ACK;
	}
       
	//-------- 2^ Byte:  Address DATA  -----// 
	I2C_sendData(addrData); 
  
	if(I2C_checkAck()!= ACK) {
		I2C_sendStop(); 
		
		return _ERROR_READ_ACK;
	}
     
   
	//-------- 3^ Byte:  Numero Byte da leggere  -----// 
	I2C_sendData(nByte); 
  
	if(I2C_checkAck()!= ACK) {
		I2C_sendStop();  
		return _ERROR_READ_ACK; 
	}

  
	//-------- 4^ Byte:  Cksum  -----// 
	checksum = ~((addressByte + addrData + nByte) % 256);
  
	I2C_sendData(checksum); 
  
	if(I2C_checkAck()!= ACK) {
		I2C_sendStop();  
		return -11; //_ERROR_CKSUM; 
	}
	
	delay_us (DELAY_1_CLOCK); 
      
	//-------- RESTART Conditions  -----// 
	I2C_sendStart();
  
	//-------- 5^ Byte:  indirizzo Slave + richiesta di lettura -----// 
	i = (addressByte & 0xFF);
	
	I2C_sendData(i); 
 
	if(I2C_checkAck()!= ACK)
	{  
		I2C_sendStop(); 
		return _ERROR_READ_ACK;  
	}   
  
	//-------- dal 6^ BYTE al (6+nByte)^ BYTE  ----------------//
	// Calcoliamo il 2 Cksum_8, che è il complemento a 1 della somma dei byte ricevuti + addressByte
	temp = addressByte;
	for(i=0; i < nByte; i++) {  
		pDataRD[i] = I2C_readData();
		temp += pDataRD[i];
		checksum_calc = ~temp;
		I2C_sendAck(); 
	}
  
	//------------ leggiamo l'ultimo byte il cksum ----------------//
	checksum = I2C_readData();
  
	if(checksum != checksum_calc) { 
		I2C_sendNak(); // errore Cksum : send acknowledge 
		I2C_sendStop(); 
		return _ERROR_CKSUM;
	}else
		I2C_sendAck(); // OK: send NOT-acknowledge
    
	I2C_sendStop(); 
    
	return 1; // tutto OK. 
}


//---------------------------------------------------------------------------------------------
// Function: I2C_WriteSlave
//
// Description: 
//      Comando di Scrittura di 1 byte.      
//   	MASTER -> SLAVE : SCRITTURA (R/W = 0)
//      	
//	 
//  ------+-+-------------+-+-+---------------+-+---------------+-+---------------+-+---+ 
//        |S| ADDR_SLAV   |W|A| ADDR_DATA_WR  |A|    DATA_WR    |A|    CKSUM      |A|STOP 
//  ------+-+-------------+-+-+---------------+-+---------------+-+---------------+-+---+ 
//   SDA   0 b b b b b b b 0 0 b b b b b b b b 0 b b b b b b b b 0 b b b b b b b b 0  /
//  ------+-+-------------+-+-+---------------+-+---------------+-+---------------+-+---+ 
//   SCL --|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|--  
//        
//---------------------------------------------------------------------------------------------           
int WriteSlave(Byte addressSlave, Byte addrData, Byte Data)
{    
  
	Byte addressByte;
	Byte checksum;

	addressByte = (addressSlave << 1);
	addressByte &= 0xFE;

	I2C_freeBus();  
	I2C_sendStart();
  
	//-------- 1^ Byte:  indirizzo Slave + richiesta di lettura -----//   
	I2C_sendData(addressByte); 

 
	if(I2C_checkAck() != ACK) {
	    I2C_sendStop(); 
		return _ERROR_READ_ACK;
    }
	
	//-------- 2^ Byte:  addrData Write -----// 
	I2C_sendData(addrData); 
  
	if(I2C_checkAck() != ACK) {
		I2C_sendStop();  
		return _ERROR_READ_ACK;//_ERROR_READ_ACK; 
    }
   
	//-------- 3^ Byte:  byte data Write -----// 
	I2C_sendData(Data); 
  
	if(I2C_checkAck() != ACK) {
	    I2C_sendStop();  
		return _ERROR_READ_ACK;
	}
  
	//-------- 5^ Byte:  Cksum  -----// 
	// Calcoliamo il Cksum_8, che è il complemento a 1 della somma dei byte spediti + 1
	checksum = ~((addressByte + addrData + Data) % 256);
  
	I2C_sendData(checksum); 
  
	if(I2C_checkAck() != ACK) {
		I2C_sendStop();  
		return _ERROR_CKSUM; 
	}
	 

	I2C_sendStop(); 
  
  
    return 1;  // Tutto OK.
}    



////////////////////////////////////////////////////////////////////////////////////////////////
//
//   PRIVATE
//
////////////////////////////////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------
void _set_SDA(void)
{
  //pinMode(_pinSDA,INPUT);
  ioport_set_pin_dir(SDA, IOPORT_DIR_INPUT);
}

//------------------------------------------------------------------
//
//------------------------------------------------------------------
void _set_SCL(void)
{
  //pinMode(_pinSCL,INPUT);
  ioport_set_pin_dir(SCL, IOPORT_DIR_INPUT);
}

//------------------------------------------------------------------
//
//------------------------------------------------------------------
void _clear_SDA(void)
{
  //pinMode(_pinSDA,OUTPUT);  // SDA
  //digitalWrite(_pinSDA,LOW);
  ioport_set_pin_dir(SDA, IOPORT_DIR_OUTPUT);
  ioport_set_pin_level(SDA, IOPORT_PIN_LEVEL_LOW);
}

//------------------------------------------------------------------
//
//------------------------------------------------------------------
void _clear_SCL(void)
{
  //pinMode(_pinSCL,OUTPUT);  // SCL
  //digitalWrite(_pinSCL,LOW);
  ioport_set_pin_dir(SCL, IOPORT_DIR_OUTPUT);
  ioport_set_pin_level(SCL, IOPORT_PIN_LEVEL_LOW);
} 

//------------------------------------------------------------------
//
//------------------------------------------------------------------
Byte _read_SDA(void)
{
  //pinMode(_pinSDA,INPUT);  // SDA
  //return digitalRead(_pinSDA);
  ioport_set_pin_dir(SDA, IOPORT_DIR_INPUT);
  return ( ioport_get_pin_level(SDA));
}

//------------------------------------------------------------------
//
//------------------------------------------------------------------
Byte _read_SCL(void)
{
  //pinMode(_pinSCL,INPUT);  // SCL
  //return digitalRead(_pinSCL);
  ioport_set_pin_dir(SCL, IOPORT_DIR_INPUT);
  return ( ioport_get_pin_level(SCL));
}


//--------------------------------------------------------------
//  Invia il comando di start sul bus.
//--------------------------------------------------------------
void I2C_sendStart(void) 
{
  _set_SDA();
  _set_SCL();

  delay_us (DELAY_1_CLOCK); 
  _clear_SDA();
  delay_us(DELAY_1_CLOCK); 
}

//--------------------------------------------------------------
void I2C_sendStop(void) 
{  
  _clear_SCL();
  _clear_SDA();
  delay_us (DELAY_1_CLOCK);
  _set_SCL();
  delay_us (DELAY_1_CLOCK);
  _set_SDA();
  delay_us (DELAY_1_CLOCK);
 
}

//------------------------------------------------------------------
// Invio il bit di ACKNOLEGE, se non riesco restituisco 0
// altrimenti 1.
//          __
//   SCL __|  |
//       _      
//   SDA  |___|
//------------------------------------------------------------------
void I2C_sendAck(void) 
{ 
  _clear_SCL();
  delay_us (DELAY_1_CLOCK>>1); 
  _clear_SDA();
  delay_us (DELAY_1_CLOCK>>1); 
  _set_SCL();
  delay_us (DELAY_1_CLOCK);
  _clear_SCL();
  _set_SDA();
}

/*-------------------------------------------------------------------------*/
void I2C_sendNak(void) // send NOT-acknowledge
{ 
  _clear_SCL();
  delay_us (DELAY_1_CLOCK>>1);
  _set_SDA();
  delay_us (DELAY_1_CLOCK>>1); 
  _set_SCL();
  delay_us (DELAY_1_CLOCK);
  _clear_SCL();
  _set_SDA();
}

//------------------------------------------------------------------
//
//------------------------------------------------------------------
int I2C_checkAck(void)
{
  int time_wait = (DELAY_1_CLOCK >> 3);
  Byte input = 0;
  
  _clear_SCL();
  delay_us (DELAY_1_CLOCK);
  _set_SCL();
  
  input = 0;
  delay_us (time_wait);  //+1/8
  input = _read_SDA();
  delay_us (time_wait);  //+1/8
  input += _read_SDA();
  delay_us (time_wait);  //+1/8
  input += _read_SDA();
  delay_us (time_wait);  //+1/8
  input += _read_SDA();
  delay_us (time_wait);  //+1/8
  input += _read_SDA();
  delay_us (time_wait);  //+1/8
  input += _read_SDA();
  delay_us (time_wait);  //+1/8
  input += _read_SDA();  
  delay_us (time_wait);  //+1/8
  _clear_SCL();
   
  if(input >= 4)
    return NAK;
  else
    return ACK;
}


//--------------------------------------------------------------------
// Invia 1 byte sul canale I2C, Essendo Master genero il segnale di clock
// ovvero SCL
//
//             __    __    __    __    __    __    __    __
// SCL(bit) __|7 |__|6 |__|5 |__|4 |__|3 |__|2 |__|1 |__|0 |
//            
//
//--------------------------------------------------------------------
void I2C_sendData(Byte value) 
{
  Byte i;
  Byte maske = 0x80;
  for (i=8;i>0;i--)
  { 
    _clear_SCL();
    delay_us (DELAY_1_CLOCK >>1);
    // a metà del clock basso cambio il valore dell' SDA, in modo che lo slave 
    // ha tutto il tempo per leggere il valore del bit corretto.
    if ((value & maske) != 0)
    {
      _set_SDA();
    }
    else
    {
      _clear_SDA();
    }
    delay_us (DELAY_1_CLOCK >>1);
    _set_SCL();
    maske >>= 1;
    delay_us (DELAY_1_CLOCK);
    _clear_SCL();
  }
  _set_SDA();
}


//--------------------------------------------------------------------
// leggo 1 Byte dal canale I2C,  Essendo Master genero il segnale di 
// clock.
//--------------------------------------------------------------------
Byte I2C_readData(void) // read Byte from E2 Interface
{
  int time_wait = (DELAY_1_CLOCK >> 3);
  Byte read_data;
  Byte data_in = 0x00;
  Byte maske = 0x80;
  
  for (maske=0x80;maske>0;maske >>=1)
  { 	
    _clear_SCL();
    delay_us (DELAY_1_CLOCK);
    _set_SCL();
	
	// dopo 1/8 del bit di clock inizio a leggere
	delay_us (time_wait);  // 1/8
	read_data = _read_SDA();

	// 2/8 del bit_clock	
	delay_us (time_wait);  
    read_data += _read_SDA();

    // 3/8 del bit_clock	
    delay_us (time_wait);  
    read_data += _read_SDA();
	
    // 4/8 del bit_clock	
    delay_us (time_wait);  
    read_data += _read_SDA();	
		
	// 5/8 del bit_clock		
	delay_us (time_wait);  
	read_data += _read_SDA();
		
	// 6/8 del bit_clock		
	delay_us (time_wait);  
	read_data += _read_SDA();	

	// 7/8 del bit_clock	
    delay_us (time_wait);
	read_data += _read_SDA();
	
	// 8/8 del bit_clock	
    delay_us (time_wait);

	if (read_data >= 5)
    {
      data_in |= maske;
    }
	
    _clear_SCL();
  }
  
  return data_in;
}


/************
 * E O F
 ************/