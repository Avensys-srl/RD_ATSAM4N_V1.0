/*
 * Serial2_Class.c
 *
 * Created: 29/11/2022 05:51:20
 *  Author: Zakaria Taleb Bendiab
 */ 

#include "asf.h"

#include "Protocol_Ser2.h"
#include "string.h"
#include "data.h"

//------ status Uart2 ------------
#define UART2_FREE         0
#define RECEPTION_MESSAGE  1
#define SEND_MESSAGE       2

#define UART2_CONFIG_TX_BYTEQ_LENGTH (128+1)
#define UART2_CONFIG_RX_BYTEQ_LENGTH (128+1)

volatile uint32_t	UART2_Status_Register = 0;

static uint8_t * volatile rxTail2;
static uint8_t *rxHead2;
static uint8_t *txTail2;
static uint8_t * volatile txHead2;
static bool volatile rxOverflowed2;

static uint8_t txQueue2[UART2_CONFIG_TX_BYTEQ_LENGTH];
static uint8_t rxQueue2[UART2_CONFIG_RX_BYTEQ_LENGTH];

extern Byte status_Uart2;
extern uint32_t volatile UART2_Timeout;

extern Pdc* UART2_PDC;
extern pdc_packet_t pdc_uart2_packet;
extern uint8_t USART2_buffRX[128];

void UART2_Handler ( void )
{
	UART2_Status_Register = uart_get_status(CONF_UART2);
	uart_reset_status(CONF_UART2);
	
	*rxTail2 = CONF_UART2->UART_RHR;

	// Will the increment not result in a wrap and not result in a pure collision?
	// This is most often condition so check first
	if ( ( rxTail2    != (rxQueue2 + UART2_CONFIG_RX_BYTEQ_LENGTH-1)) &&
	((rxTail2+1) != rxHead2) )
	{
		rxTail2++;
	}
	else if ( (rxTail2 == (rxQueue2 + UART2_CONFIG_RX_BYTEQ_LENGTH-1)) &&
	(rxHead2 !=  rxQueue2) )
	{
		// Pure wrap no collision
		rxTail2 = rxQueue2;
	}
	else // must be collision
	{
		rxOverflowed2 = true;
	}
	UART2_Timeout = 0;
}

void Serial2_begin( void )
{
	txHead2 = txQueue2;
	txTail2 = txQueue2;
	rxHead2 = rxQueue2;
	rxTail2 = rxQueue2;
	
	rxOverflowed2 = false;
}

void Serial2_end( void )
{
	
}

int Serial2_available( void )
{
	uint16_t size;
	
	size = 0;
	
	if ( pdc_read_rx_counter(UART2_PDC) == 100 )
	{
		UART2_Timeout = 0;
		return 0;
	}
	else
	{
		size = ( 100 - pdc_read_rx_counter(UART2_PDC) );
		if ( UART2_Timeout >= 150 )
		{
			memcpy ( rxQueue2, USART2_buffRX, size );
			pdc_rx_init(UART2_PDC, &pdc_uart2_packet, NULL);
			UART2_Timeout = 0;
			rxHead2 = rxQueue2;
			return size;
		}
		else
		return 0;
	}
	
	/*uint8_t *snapshot_rxTail = (uint8_t*)rxTail2;
	
	if (snapshot_rxTail < rxHead2)
	{
		size = ( UART2_CONFIG_RX_BYTEQ_LENGTH - (rxHead2-snapshot_rxTail));
	}
	else
	{
		size = ( (snapshot_rxTail - rxHead2));
	}
	
	if(size > 0xFF)
	{
		return 0xFF;
	}
	
	//return size;
	if ( UART2_Timeout > 15 ) // 15 ms
		{
			return size;
		}
	else
		return 0;*/
}

int Serial2_peek( void )
{
	return 0;
}

int Serial2_read( void )
{
	uint8_t data = 0;
	
	data = *rxHead2;
	rxHead2++;
	
	/*while (rxHead2 == rxTail2 )
	{
	}
	
	data = *rxHead2;

	rxHead2++;

	if (rxHead2 == (rxQueue2 + UART2_CONFIG_RX_BYTEQ_LENGTH))
	{
		rxHead2 = rxQueue2;
	}*/
	return data;
}

void Serial2_flush( void )
{
	
}

void Serial2_write(uint8_t* b, uint16_t len)
{
	usart_serial_write_packet(PERIPH_UART , b, len);
	while ( !uart_is_tx_empty(PERIPH_UART))
	{
		
	}
	status_Uart2 = UART2_FREE;
}
