/////////////////////////////////////////////////////////////////////////////////////////////
// 
// Librery: MASTER_I2C.h  1.00
//   
// Data: 24-01-14
//   
// Descrizione: 
//    Definisce le funzioni ed il Protocollo di
//    Comunicazione Tra gli slave e l'unita'(MASTER).         
// 
// Autore: Maurizio Colmone
// 
// 

#ifndef _MASTER_I2C_H
#define _MASTER_I2C_H


//------------------------------------------------------------------------
//                          __________            __________ 
//   (example)             |          |          |          |
//   SCL(1.0KHz) __________|          |__________|          |_ _ _ _
//                         :- 500 us -:- 500 us -:
//                         :- - - - 1000 us - - -:
// 
//   
//    :-----------:-----------------:--------------:
//    :  FRQ I2C  :  TIME_BIT_SCL   : 1 byte + ACK : 
//    :-----------:-----------------:--------------:
//    :   0.5 KHz :    1000 us      :   18.00 ms   :
//    :   1.0 KHz :     500 us      :     9.0 ms   : 
//    :   2.0 KHz :     250 us      :     4.5 ms   :
//    :   2.5 KHz :     200 us      :     4.0 ms   :
//    :   5.0 KHz :     100 us      :     1.8 ms   :
//    :  10.0 KHz :      50 us      :     0.9 ms   :
//------------------------------------------------------------------------
#define FREQ_I2C_500_HZ        1000
#define FREQ_I2C_1000_HZ        500
#define FREQ_I2C_2000_HZ        250
#define FREQ_I2C_2500_HZ        200
#define FREQ_I2C_5000_HZ        100
#define FREQ_I2C_10000_HZ        50

#define DELAY_1_CLOCK        (FREQ_I2C_2500_HZ)        


//-------------------------------------------------------------------------
// define acknowledge
#ifndef ACK
#define ACK 0  // SDA LOW
#endif

//-------------------------------------------------------------------------
// define not-acknowledge 
#ifndef NAK
#define NAK 1  // SDA HIGH
#endif


//----------------------- Address Accessory -------------------------------
#define  ADDR_PREHEATER		     2
#define  ADDR_HEATER 	   	    10  // HEATER (EHD, HWD)
#define  ADDR_COOLER 	   	    20  // COOLER  (CWD)
#define  ADDR_DXD 	   	        25  // DXD (Compressore Toshiba)
#define  ADDR_PROBE1_CO2        30  // Probe 1 CO2
#define  ADDR_PROBE2_CO2        31
#define  ADDR_PROBE1_RH         34  // Probe 1 RH
#define  ADDR_PROBE2_RH         35
#define  ADDR_PROBE1_VOC        38  // Probe 1 VOC
#define  ADDR_PROBE_CAP         40  // Probe Pressure for control Constant Air Pressure
#define  ADDR_PROBE_CAF         41  // Probe Pressure for control Constant Air Flow
#define  ADDR_PROBE_DSP         42  // Probe Pressure for control Filters
#define  ADDR_AWP               50  // Probe Ambient temperature
#define  ADDR_EXT1              51  // Multisensore 1
#define  ADDR_EXT2              52  // Multisensore 2
#define  ADDR_EXT3              53  // Multisensore 3
#define  ADDR_EXT4              54  // Multisensore 4
#define  ADDR_FLW1              60  // Flowmeter 1
#define  ADDR_FLW2              61  // Flowmeter 2
#define  ADDR_SSR               62  // SSR modulator
#define  ADDR_EBPD              63  // BPD external board
#define  ADDR_EBP2              64  // BPD external board (slave one)
#define  ADDR_DSC               65  // DSC Board
#define	 ADDR_DPSEN				99	// DPsensor
#define  ADDR_M24LR04E          87  // 1 0 1 0 E 1 1 R/W 
   

/*------------------------------------------------------------------- 
                   Values return Error       
  -------------------------------------------------------------------*/  
#define _ERROR_ADDR         -1  // richiesta per un altro slave o indirizzo errato
#define _ERROR_BIT          -2  // cable error, communication error  
#define _ERROR_CKSUM        -3  // Cksum error 
#define _ERROR_SEND_ACK     -4
#define _ERROR_READ_ACK     -5
#define _ERROR_NACK         -6 
#define _ERROR_NO_RESTART   -7
#define _ERROR_STOP         -8  // non sono riuscito a leggere o dare il bit di stop. 


extern int  ReadSlave(Byte addressSlave, Byte addrData, Byte *pDataRD, Byte nByte);
extern int  WriteSlave(Byte addressSlave, Byte addrData, Byte Data);
		
extern void _set_SDA(void);
extern void _set_SCL(void);
extern void _clear_SDA(void);
extern void _clear_SCL(void);
extern Byte _read_SDA(void);
extern Byte _read_SCL(void);
extern void I2C_sendStart(void); 
extern void I2C_sendStop(void);
extern void I2C_sendAck(void); 
extern void I2C_sendNak(void); 	
extern int I2C_checkAck(void);
extern void I2C_sendData(Byte value);
extern void I2C_freeBus(void);
extern Byte I2C_readData(void);

#endif
