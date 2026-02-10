/*
 * Serial1_Class.cpp
 *
 * Created: 26/11/2022 02:33:43
 *  Author: Zakaria Taleb bendiab
 */ 

#include "asf.h"

#include "Protocol_Ser1.h"
#include "string.h"
#include "data.h"

#define UART0_CONFIG_TX_BYTEQ_LENGTH (128+1)
#define UART0_CONFIG_RX_BYTEQ_LENGTH (128+1)

volatile uint32_t	UART0_Status_Register = 0;
extern volatile uint32_t UART0_Timeout;

static uint8_t * volatile rxTail;
static uint8_t *rxHead;
static uint8_t *txTail;
static uint8_t * volatile txHead;
static bool volatile rxOverflowed;

static uint8_t txQueue[UART0_CONFIG_TX_BYTEQ_LENGTH];
static uint8_t rxQueue[UART0_CONFIG_RX_BYTEQ_LENGTH];

extern Pdc* UART0_PDC;
extern pdc_packet_t pdc_uart0_packet;
extern uint8_t USART0_buffRX[128];

void UART0_Handler ( void )
{
	UART0_Status_Register = uart_get_status(CONF_UART0);
	uart_reset_status(CONF_UART0);
	
	*rxTail = CONF_UART0->UART_RHR;

	// Will the increment not result in a wrap and not result in a pure collision?
	// This is most often condition so check first
	if ( ( rxTail    != (rxQueue + UART0_CONFIG_RX_BYTEQ_LENGTH-1)) &&
	((rxTail+1) != rxHead) )
	{
		rxTail++;
	}
	else if ( (rxTail == (rxQueue + UART0_CONFIG_RX_BYTEQ_LENGTH-1)) &&
	(rxHead !=  rxQueue) )
	{
		// Pure wrap no collision
		rxTail = rxQueue;
	}
	else // must be collision
	{
		rxOverflowed = true;
	}
	UART0_Timeout = 0;
}

void Serial_begin( void )
{
	txHead = txQueue;
	txTail = txQueue;
	rxHead = rxQueue;
	rxTail = rxQueue;
	
	rxOverflowed = false;
}

void Serial_end( void )
{
	
}

int Serial_available( void )
{
	uint16_t size;
	
	size = 0;
	
	if ( pdc_read_rx_counter(UART0_PDC) == 120 )
		{
			UART0_Timeout = 0;
			return 0;
		}
	else
		{
			size = ( 120 - pdc_read_rx_counter(UART0_PDC) );
			if ( UART0_Timeout >= 150 )
			{
				memcpy ( rxQueue, USART0_buffRX, size );
				pdc_rx_init(UART0_PDC, &pdc_uart0_packet, NULL);
				UART0_Timeout = 0;
				rxHead = rxQueue;
				return size;
			}
			else
				return 0;
		}	
		
	/*uint8_t *snapshot_rxTail = (uint8_t*)rxTail;
	
	if (snapshot_rxTail < rxHead)
	{
		size = ( UART0_CONFIG_RX_BYTEQ_LENGTH - (rxHead-snapshot_rxTail));
	}
	else
	{
		size = ( (snapshot_rxTail - rxHead));
	}
	
	if(size > 0xFF)
	{
		return 0xFF;
	}
	
	//return size;
	if ( UART0_Timeout > 15 ) // 15 ms
		{
			return size;
		}
	else
		return 0;*/
}

int Serial_peek( void )
{
	return 0;
}

int Serial_read( void )
{
	uint8_t data = 0;
	
	data = *rxHead;
	rxHead++;
	
    /*while (rxHead == rxTail )
    {
    }
    
    data = *rxHead;

    rxHead++;

    if (rxHead == (rxQueue + UART0_CONFIG_RX_BYTEQ_LENGTH))
    {
        rxHead = rxQueue;
    }*/
    return data;
}

void Serial_flush( void )
{
	
}

void Serial_write(uint8_t* b, uint16_t len)
{
	usart_serial_write_packet(CONSOLE_UART , b, len);
	while ( !uart_is_tx_empty(CONSOLE_UART))
	{
		
	}
}

void Serial_print1(const char* buff)
{
	usart_serial_write_packet(CONSOLE_UART , buff, strlen (buff));
	while ( !uart_is_tx_empty(CONSOLE_UART))
	{
		
	}
}

void Serial_print(Byte b, int base)
{
	char Buffer[50];
	
	memset ( Buffer, 0, sizeof(Buffer));
	if ( base == BYTE )
	{
		 sprintf(Buffer ,"%c", b);
		 usart_serial_write_packet(CONSOLE_UART , Buffer, strlen (Buffer));
		 while ( !uart_is_tx_empty(CONSOLE_UART))
		 {
			 
		 }
		 return;
	}
	
	if ( base == DEC )
	{
		sprintf(Buffer ,"%d", b);
		usart_serial_write_packet(CONSOLE_UART , Buffer, strlen (Buffer));
		while ( !uart_is_tx_empty(CONSOLE_UART))
		{
			
		}
		return;
	}
	
	if ( base == HEX )
	{
		sprintf(Buffer ,"%X", b);
		usart_serial_write_packet(CONSOLE_UART , Buffer, strlen (Buffer));
		while ( !uart_is_tx_empty(CONSOLE_UART))
		{
			
		}
		return;
	}
	
	if ( base == 1 )
	{
		sprintf(Buffer ,"%.1f", b);
		usart_serial_write_packet(CONSOLE_UART , Buffer, strlen (Buffer));
		while ( !uart_is_tx_empty(CONSOLE_UART))
		{
			
		}
		return;
	}
	
}

void Serial_println1(const char* buff)
{
	usart_serial_write_packet(CONSOLE_UART ,  buff, strlen (buff));
	while ( !uart_is_tx_empty(CONSOLE_UART))
	{
		
	}
	usart_serial_write_packet(CONSOLE_UART ,  "\r\n", 2);
	while ( !uart_is_tx_empty(CONSOLE_UART))
	{
		
	}
}

void Serial_println(Byte b, int base)
{
	char Buffer[50];
	
	memset ( Buffer, 0, sizeof(Buffer));
	if ( base == BYTE )
	{
		sprintf(Buffer ,"%c", b);
		usart_serial_write_packet(CONSOLE_UART , Buffer, strlen (Buffer));
		while ( !uart_is_tx_empty(CONSOLE_UART))
		{
			
		}
		usart_serial_write_packet(CONSOLE_UART ,  "\r\n", 2);
		while ( !uart_is_tx_empty(CONSOLE_UART))
		{
			
		}
		return;
	}
	
	if ( base == DEC )
	{
		sprintf(Buffer ,"%d", b);
		usart_serial_write_packet(CONSOLE_UART , Buffer, strlen (Buffer));
		while ( !uart_is_tx_empty(CONSOLE_UART))
		{
			
		}
		usart_serial_write_packet(CONSOLE_UART ,  "\r\n", 2);
		while ( !uart_is_tx_empty(CONSOLE_UART))
		{
			
		}
		return;
	}
	
	if ( base == HEX )
	{
		sprintf(Buffer ,"%X", b);
		usart_serial_write_packet(CONSOLE_UART , Buffer, strlen (Buffer));
		while ( !uart_is_tx_empty(CONSOLE_UART))
		{
			
		}
		usart_serial_write_packet(CONSOLE_UART ,  "\r\n", 2);
		while ( !uart_is_tx_empty(CONSOLE_UART))
		{
			
		}
		return;
	}
	
	if ( base == 1 )
	{
		sprintf(Buffer ,"%.1f", b);
		usart_serial_write_packet(CONSOLE_UART , Buffer, strlen (Buffer));
		while ( !uart_is_tx_empty(CONSOLE_UART))
		{
			
		}
		usart_serial_write_packet(CONSOLE_UART ,  "\r\n", 2);
		while ( !uart_is_tx_empty(CONSOLE_UART))
		{
			
		}
		return;
	}
}
