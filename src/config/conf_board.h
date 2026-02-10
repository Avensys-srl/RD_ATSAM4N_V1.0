/**
 * \file
 *
 * \brief User board configuration template
 *
 */
/*
 * Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
 */

#ifndef CONF_BOARD_H
#define CONF_BOARD_H

#define CONSOLE_UART                (Usart *)UART0
#define PERIPH_UART                 (Usart *)UART2
#define MAIN_I2C                    (Twi *)TWI0
#define WATCH_DOG					(Wdt *)WDT

// clock resonators
#define BOARD_FREQ_SLCK_XTAL      (32768U)
#define BOARD_FREQ_SLCK_BYPASS    (32768U)
#define BOARD_FREQ_MAINCK_XTAL    (12000000U)
#define BOARD_FREQ_MAINCK_BYPASS  (12000000U)
#define BOARD_MCK                 CHIP_FREQ_CPU_MAX
#define BOARD_OSC_STARTUP_US      15625

#define pStatusLedI2C			IOPORT_CREATE_PIN(PIOA, 11)
#define pLED5					IOPORT_CREATE_PIN(PIOA, 24)
#define pInsertEB_ModBus		IOPORT_CREATE_PIN(PIOA, 29)
#define pIPEHD					IOPORT_CREATE_PIN(PIOA, 22)
#define pEnable_TX2				IOPORT_CREATE_PIN(PIOA, 30)
#define pSwitchOFF_KTS			IOPORT_CREATE_PIN(PIOA, 13)
#define pDir					IOPORT_CREATE_PIN(PIOA, 27)
#define pBuz					IOPORT_CREATE_PIN(PIOA, 5)
#define pOut1					IOPORT_CREATE_PIN(PIOA, 23)
#define pOut2					IOPORT_CREATE_PIN(PIOB, 0)
#define pPowerMotors			IOPORT_CREATE_PIN(PIOA, 14)
#define motor_pin_4				IOPORT_CREATE_PIN(PIOB, 4)
#define motor_pin_3				IOPORT_CREATE_PIN(PIOA, 25)
#define motor_pin_2				IOPORT_CREATE_PIN(PIOA, 26)
#define motor_pin_1				IOPORT_CREATE_PIN(PIOA, 12)
#define SDA						IOPORT_CREATE_PIN(PIOA, 0)
#define SCL						IOPORT_CREATE_PIN(PIOA, 1)
#define Pin_Test				IOPORT_CREATE_PIN(PIOA, 6)

/** TWI0 pin definitions */
#define TWI0_DATA_GPIO   PIO_PA3_IDX
#define TWI0_DATA_FLAGS  IOPORT_MODE_MUX_A
#define TWI0_CLK_GPIO    PIO_PA4_IDX
#define TWI0_CLK_FLAGS   IOPORT_MODE_MUX_A

#define CONF_BOARD_TWI0

/** UART0 pins (UTXD0 and URXD0) definitions, PA9,10. */
#define PINS_UART0        (PIO_PA9A_URXD0 | PIO_PA10A_UTXD0)
#define PINS_UART0_FLAGS  (IOPORT_MODE_MUX_A)

#define PINS_UART0_PORT   IOPORT_PIOA
#define PINS_UART0_MASK   (PIO_PA9A_URXD0 | PIO_PA10A_UTXD0)
#define PINS_UART0_PIO    PIOA
#define PINS_UART0_ID     ID_PIOA
#define PINS_UART0_TYPE   PIO_PERIPH_A
#define PINS_UART0_ATTR   PIO_DEFAULT

#define CONF_BOARD_UART_CONSOLE

/** UART2 pins (UTXD2 and URXD2) definitions, PA15,16. */
#define PINS_UART2        (PIO_PA16A_URXD2 | PIO_PA15A_UTXD2)
#define PINS_UART2_FLAGS  (IOPORT_MODE_MUX_A)

#define PINS_UART2_PORT   IOPORT_PIOA
#define PINS_UART2_MASK   (PIO_PA16A_URXD2 | PIO_PA15A_UTXD2)
#define PINS_UART2_PIO    PIOA
#define PINS_UART2_ID     ID_PIOA
#define PINS_UART2_TYPE   PIO_PERIPH_A
#define PINS_UART2_ATTR   PIO_DEFAULT

#define CONF_BOARD_UART2

#endif // CONF_BOARD_H
