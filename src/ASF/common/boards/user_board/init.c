/**
 * \file
 *
 * \brief User board initialization template
 *
 */
/*
 * Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
 */

#include <asf.h>
#include <board.h>
#include <conf_board.h>

/**
 * \brief Set peripheral mode for IOPORT pins.
 * It will configure port mode and disable pin mode (but enable peripheral).
 * \param port IOPORT port to configure
 * \param masks IOPORT pin masks to configure
 * \param mode Mode masks to configure for the specified pin (\ref ioport_modes)
 */
#define ioport_set_port_peripheral_mode(port, masks, mode) \
	do {\
		ioport_set_port_mode(port, masks, mode);\
		ioport_disable_port(port, masks);\
	} while (0)

/**
 * \brief Set peripheral mode for one single IOPORT pin.
 * It will configure port mode and disable pin mode (but enable peripheral).
 * \param pin IOPORT pin to configure
 * \param mode Mode masks to configure for the specified pin (\ref ioport_modes)
 */
#define ioport_set_pin_peripheral_mode(pin, mode) \
	do {\
		ioport_set_pin_mode(pin, mode);\
		ioport_disable_pin(pin);\
	} while (0)

uint32_t test1;
uint32_t test2;
	
void board_init(void)
{
	/* This function is meant to contain board-specific initialization code
	 * for, e.g., the I/O pins. The initialization can rely on application-
	 * specific board configuration, found in conf_board.h.
	 */
	
	#ifndef CONF_BOARD_KEEP_WATCHDOG_AT_INIT
	/* Disable the watchdog */
	//WDT->WDT_MR = WDT_MR_WDDIS;
	//wdt_disable( WATCH_DOG );
	#endif
	
	//WDT->WDT_MR = WDT_MR_WDDIS; // Disable the watchdog
	//wdt_disable( WATCH_DOG );
	
	/* Initialize IOPORT */
	ioport_init();
	
	REG_CCFG_SYSIO |= CCFG_SYSIO_SYSIO4;
	
	ioport_set_pin_dir(pStatusLedI2C, IOPORT_DIR_OUTPUT);
	ioport_set_pin_dir(pLED5, IOPORT_DIR_OUTPUT);
	ioport_set_pin_level(pStatusLedI2C, IOPORT_PIN_LEVEL_HIGH);
	ioport_set_pin_level(pLED5, IOPORT_PIN_LEVEL_HIGH);
	
	ioport_set_pin_dir(pInsertEB_ModBus, IOPORT_DIR_INPUT);
	
	ioport_set_pin_dir(pIPEHD, IOPORT_DIR_OUTPUT);
	ioport_set_pin_level(pIPEHD, IOPORT_PIN_LEVEL_LOW);
	
	ioport_set_pin_dir(pEnable_TX2, IOPORT_DIR_OUTPUT);
	ioport_set_pin_level(pEnable_TX2, IOPORT_PIN_LEVEL_LOW);
	
	ioport_set_pin_dir(pSwitchOFF_KTS, IOPORT_DIR_OUTPUT);
	ioport_set_pin_level(pSwitchOFF_KTS, IOPORT_PIN_LEVEL_LOW);
	
	ioport_set_pin_dir(pDir, IOPORT_DIR_OUTPUT);
	
	ioport_set_pin_dir(pBuz, IOPORT_DIR_OUTPUT);
	ioport_set_pin_level(pBuz, IOPORT_PIN_LEVEL_LOW);
	
	ioport_set_pin_dir(pOut1, IOPORT_DIR_OUTPUT);
	ioport_set_pin_level(pOut1, IOPORT_PIN_LEVEL_LOW);
	
	ioport_set_pin_dir(pOut2, IOPORT_DIR_OUTPUT);
	ioport_set_pin_level(pOut2, IOPORT_PIN_LEVEL_LOW);
	
	ioport_set_pin_dir(pPowerMotors, IOPORT_DIR_OUTPUT);
	
	ioport_set_pin_dir(motor_pin_4, IOPORT_DIR_OUTPUT);
	ioport_set_pin_level(motor_pin_4, IOPORT_PIN_LEVEL_LOW);
	
	ioport_set_pin_dir(motor_pin_3, IOPORT_DIR_OUTPUT);
	ioport_set_pin_level(motor_pin_3, IOPORT_PIN_LEVEL_LOW);
	
	ioport_set_pin_dir(motor_pin_2, IOPORT_DIR_OUTPUT);
	ioport_set_pin_level(motor_pin_2, IOPORT_PIN_LEVEL_LOW);
	
	ioport_set_pin_dir(motor_pin_1, IOPORT_DIR_OUTPUT);
	ioport_set_pin_level(motor_pin_1, IOPORT_PIN_LEVEL_LOW);
	
	ioport_set_pin_dir(SDA, IOPORT_DIR_OUTPUT);
	ioport_set_pin_level(SDA, IOPORT_PIN_LEVEL_HIGH);
	
	ioport_set_pin_dir(SCL, IOPORT_DIR_OUTPUT);
	ioport_set_pin_level(SCL, IOPORT_PIN_LEVEL_HIGH);
	
	ioport_set_pin_dir(Pin_Test, IOPORT_DIR_OUTPUT);
	ioport_set_pin_level(Pin_Test, IOPORT_PIN_LEVEL_HIGH);
	
	// bypass step motors
	
	ioport_set_pin_dir(TWI0_CLK_GPIO, IOPORT_DIR_OUTPUT);
	ioport_set_pin_level(TWI0_CLK_GPIO, IOPORT_PIN_LEVEL_LOW);
	delay_ms (1);
	ioport_set_pin_level(TWI0_CLK_GPIO, IOPORT_PIN_LEVEL_HIGH);
	delay_ms (1);
	ioport_set_pin_level(TWI0_CLK_GPIO, IOPORT_PIN_LEVEL_LOW);
	delay_ms (1);
	ioport_set_pin_level(TWI0_CLK_GPIO, IOPORT_PIN_LEVEL_HIGH);
	delay_ms (1);
	ioport_set_pin_level(TWI0_CLK_GPIO, IOPORT_PIN_LEVEL_LOW);
	delay_ms (1);
	ioport_set_pin_level(TWI0_CLK_GPIO, IOPORT_PIN_LEVEL_HIGH);
	delay_ms (1);
	ioport_set_pin_level(TWI0_CLK_GPIO, IOPORT_PIN_LEVEL_LOW);
	delay_ms (1);
	ioport_set_pin_level(TWI0_CLK_GPIO, IOPORT_PIN_LEVEL_HIGH);
	delay_ms (1);
	ioport_set_pin_level(TWI0_CLK_GPIO, IOPORT_PIN_LEVEL_LOW);
	delay_ms (1);
	ioport_set_pin_level(TWI0_CLK_GPIO, IOPORT_PIN_LEVEL_HIGH);
	delay_ms (1);
	ioport_set_pin_level(TWI0_CLK_GPIO, IOPORT_PIN_LEVEL_LOW);
	delay_ms (1);
	ioport_set_pin_level(TWI0_CLK_GPIO, IOPORT_PIN_LEVEL_HIGH);
	delay_ms (1);
	ioport_set_pin_level(TWI0_CLK_GPIO, IOPORT_PIN_LEVEL_LOW);
	delay_ms (1);
	ioport_set_pin_level(TWI0_CLK_GPIO, IOPORT_PIN_LEVEL_HIGH);
	delay_ms (1);
	ioport_set_pin_level(TWI0_CLK_GPIO, IOPORT_PIN_LEVEL_LOW);
	delay_ms (1);
	ioport_set_pin_level(TWI0_CLK_GPIO, IOPORT_PIN_LEVEL_HIGH);
	delay_ms (1);
	ioport_set_pin_level(TWI0_CLK_GPIO, IOPORT_PIN_LEVEL_LOW);
	delay_ms (1);
	ioport_set_pin_level(TWI0_CLK_GPIO, IOPORT_PIN_LEVEL_HIGH);
	delay_ms (1);
	
	#if defined(CONF_BOARD_TWI0)
	ioport_set_pin_peripheral_mode(TWI0_DATA_GPIO, TWI0_DATA_FLAGS);
	ioport_set_pin_peripheral_mode(TWI0_CLK_GPIO, TWI0_CLK_FLAGS);
	#endif
	
	#if defined (CONF_BOARD_UART_CONSOLE)
	/* Configure UART pins */
	ioport_set_port_peripheral_mode(PINS_UART0_PORT, PINS_UART0,
	PINS_UART0_FLAGS);
	#endif
	
	#if defined (CONF_BOARD_UART2)
	/* Configure UART pins */
	ioport_set_port_peripheral_mode(PINS_UART2_PORT, PINS_UART2,
	PINS_UART2_FLAGS);
	#endif
	
	#if defined (WATCH_DOG)
	/* Configure Watchdog */
	test1 = wdt_get_timeout_value(5000000, 32768);
	wdt_init(WATCH_DOG , WDT_MR_WDRSTEN | WDT_MR_WDRPROC, test1, test1 ); // 2s watchdog
	//test2 = wdt_get_us_timeout_period(WATCH_DOG, 32768);
	//NVIC_EnableIRQ(WDT_IRQn);
	#endif
	
} 
