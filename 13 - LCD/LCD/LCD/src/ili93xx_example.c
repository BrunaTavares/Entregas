/**
 * \file
 *
 * \brief lcd controller ili93xx example.
 *
 * Copyright (c) 2014-2015 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel microcontroller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 */

/**
 * \mainpage lcd controller ili93xx Example
 *
 * \section Purpose
 *
 * This example demonstrates how to configure lcd controller ili9325 or
 * ili9341 to control the LCD on the board.
 *
 * \section Requirements
 *
 * This package can be used with SAM4E-EK evaluation kits.
 *
 * \section Description
 *
 * This example first configure ili93xx for access the LCD controller,
 * then initialize the LCD, finally draw some text, image, basic shapes (line,
 * rectangle, circle) on LCD.
 *
 * \section Usage
 *
 * -# Build the program and download it inside the evaluation board.
 * -# Some text, image and basic shapes should be displayed on the LCD.
 *
 */
/*
 * Support and FAQ: visit <a href="http://www.atmel.com/design-support/">Atmel Support</a>
 */

#include "asf.h"
#include "stdio_serial.h"
#include "conf_board.h"
#include "conf_clock.h"
#include "smc.h"

/** Chip select number to be set */
#define ILI93XX_LCD_CS      1

struct ili93xx_opt_t g_ili93xx_display_opt;

//uint8_t cnt[] = {0x30,0x31,0x32,0x00};

/**
 *  Configure UART console.
 */
static void configure_console(void)
{
	const usart_serial_options_t uart_serial_options = {
		.baudrate = CONF_UART_BAUDRATE,
		.paritytype = CONF_UART_PARITY
	};

	/** Configure console UART. */
	sysclk_enable_peripheral_clock(CONSOLE_UART_ID);
	stdio_serial_init(CONF_UART, &uart_serial_options);
}

/**
	Vari�veis para o contador do display e tempo.
*/
static uint32_t counter = 0;
static uint32_t tempo = 0;

#define PIN_PUSHBUTTON_1_MASK	PIO_PB3
#define PIN_PUSHBUTTON_1_PIO	PIOB
#define PIN_PUSHBUTTON_1_ID		ID_PIOB
#define PIN_PUSHBUTTON_1_TYPE	PIO_INPUT
#define PIN_PUSHBUTTON_1_ATTR	PIO_PULLUP | PIO_DEBOUNCE | PIO_IT_RISE_EDGE

#define PIN_PUSHBUTTON_2_MASK	PIO_PC12
#define PIN_PUSHBUTTON_2_PIO	PIOC
#define PIN_PUSHBUTTON_2_ID		ID_PIOC
#define PIN_PUSHBUTTON_2_TYPE	PIO_INPUT
#define PIN_PUSHBUTTON_2_ATTR	PIO_PULLUP | PIO_DEBOUNCE | PIO_IT_FALL_EDGE

/** IRQ priority for PIO (The lower the value, the greater the priority) */
#define IRQ_PRIOR_PIO    0

/**
	Desenha um ret�ngulo em cima do valor anterior do contador e sobrescreve o novo valor.
*/
void atualiza_contador(){
	//Limpa a tela com um ret�ngulo branco
	ili93xx_set_foreground_color(COLOR_WHITE);
	ili93xx_draw_filled_rectangle(125,100,200,170);
	
	ili93xx_set_foreground_color(COLOR_BLACK);
	char buffer[10];
	snprintf(buffer, 10, "%d", counter);
	ili93xx_draw_string(130, 110, (uint8_t *)buffer);
}

/**
	Desenha um ret�ngulo em cima do valor anterior do contador do tempo e sobrescreve o novo valor.
	Separa o contador em minutos e segundos tamb�m.
*/
void atualiza_tempo(){
	//limpa a tela com branco 
	ili93xx_set_foreground_color(COLOR_WHITE);
	ili93xx_draw_filled_rectangle(130,280,ILI93XX_LCD_WIDTH, ILI93XX_LCD_HEIGHT);
	
	
	int seg = tempo % 60;
	int min = tempo / 60;
	
	char buffer[10];
	snprintf(buffer, 10, "%02d:%02d", min, seg);
	ili93xx_set_foreground_color(COLOR_BLACK);
	ili93xx_draw_string(140, 300, (uint8_t *)buffer);
}

/**
 *  Handle Interrupcao Bt1 -> Bt2 
 *  Soma 1 a vari�vel global do contador
 *	Atualiza display
 */
static void Button1_Handler(uint32_t id, uint32_t mask)
{
	counter++;
	atualiza_contador();
}

/**
 *  Handle Interrupcao Bt2 -> Bt3 
 *  Subtrai 1 do valor da variavel global do contador
 *	Atualiza o display
 */
static void Button2_Handler(uint32_t id, uint32_t mask)
{
	counter--;
	atualiza_contador();
}

/**
 *  Interrupt handler for TC0 interrupt. 
 *  Incrementa o contador do tempo e atualiza o display.
 */
void TC0_Handler(void)
{
	volatile uint32_t ul_dummy;

	/* Clear status bit to acknowledge interrupt */
	ul_dummy = tc_get_status(TC0,0);
	

	/* Avoid compiler warning */
	UNUSED(ul_dummy);
	
	/* Adiciona 1 para o tempo*/
	tempo++;
	atualiza_tempo();
}

/**
 *  \brief Configure the Pushbuttons
 *
 *  Configure the PIO as inputs and generate corresponding interrupt when
 *  pressed or released.
 *
 *	Configura os dois bot�es
 */
static void configure_buttons(void)
{
	pmc_enable_periph_clk(PIN_PUSHBUTTON_1_ID);
	pmc_enable_periph_clk(PIN_PUSHBUTTON_2_ID);
	
	pio_set_input(PIN_PUSHBUTTON_1_PIO, PIN_PUSHBUTTON_1_MASK, PIO_PULLUP | PIO_DEBOUNCE);
	pio_set_input(PIN_PUSHBUTTON_2_PIO, PIN_PUSHBUTTON_2_MASK, PIO_PULLUP | PIO_DEBOUNCE);

	pio_set_debounce_filter(PIN_PUSHBUTTON_1_PIO, PIN_PUSHBUTTON_1_MASK, 10);
	pio_set_debounce_filter(PIN_PUSHBUTTON_2_PIO, PIN_PUSHBUTTON_2_MASK, 10);
	
	pio_handler_set(PIN_PUSHBUTTON_1_PIO, PIN_PUSHBUTTON_1_ID, PIN_PUSHBUTTON_1_MASK, PIO_IT_FALL_EDGE | PIO_PULLUP, Button1_Handler);
	pio_handler_set(PIN_PUSHBUTTON_2_PIO, PIN_PUSHBUTTON_2_ID, PIN_PUSHBUTTON_2_MASK, PIO_IT_FALL_EDGE | PIO_PULLUP, Button2_Handler);
	
	pio_enable_interrupt(PIN_PUSHBUTTON_1_PIO, PIN_PUSHBUTTON_1_MASK);
	pio_enable_interrupt(PIN_PUSHBUTTON_2_PIO, PIN_PUSHBUTTON_2_MASK);

	NVIC_SetPriority((IRQn_Type) PIN_PUSHBUTTON_1_ID, 0);
	NVIC_SetPriority((IRQn_Type) PIN_PUSHBUTTON_2_ID, 0);
	
	NVIC_EnableIRQ((IRQn_Type) PIN_PUSHBUTTON_1_ID);
	NVIC_EnableIRQ((IRQn_Type) PIN_PUSHBUTTON_2_ID);
}

/**
 *  Configure Timer Counter 0 to generate an interrupt every 1s.
 */
static void configure_tc(void)
{
	/*
	* Aqui atualizamos o clock da cpu que foi configurado em sysclk init
	*
	* O valor atual est� em : 120_000_000 Hz (120Mhz)
	*/
	uint32_t ul_sysclk = sysclk_get_cpu_hz();
	
	/*
	*	Ativa o clock do perif�rico TC 0
	*/
	pmc_enable_periph_clk(ID_TC0);
	
	// Configura TC para operar no modo de compara��o e trigger RC
	
	tc_init(TC0,0,TC_CMR_CPCTRG | TC_CMR_TCCLKS_TIMER_CLOCK5);

	// Valor para o contador de um em um segundo.
	tc_write_rc(TC0,0,32768);

	NVIC_EnableIRQ((IRQn_Type) ID_TC0);
	
	tc_enable_interrupt(TC0,0,TC_IER_CPCS);
	
	tc_start(TC0, 0);
}


/**
 * \brief Application entry point for smc_lcd example.
 *
 * \return Unused (ANSI-C compatibility).
 */
int main(void)
{
	sysclk_init();
	board_init();
	
	/** Configura os bot�es, o TC e desabilita o Watchdog Timer. */
	configure_buttons();
	WDT->WDT_MR = WDT_MR_WDDIS;
	configure_tc();

	/** Initialize debug console */
	configure_console();

	/** Enable peripheral clock */
	pmc_enable_periph_clk(ID_SMC);

	/** Configure SMC interface for Lcd */
	smc_set_setup_timing(SMC, ILI93XX_LCD_CS, SMC_SETUP_NWE_SETUP(2)
			| SMC_SETUP_NCS_WR_SETUP(2)
			| SMC_SETUP_NRD_SETUP(2)
			| SMC_SETUP_NCS_RD_SETUP(2));
	smc_set_pulse_timing(SMC, ILI93XX_LCD_CS, SMC_PULSE_NWE_PULSE(4)
			| SMC_PULSE_NCS_WR_PULSE(4)
			| SMC_PULSE_NRD_PULSE(10)
			| SMC_PULSE_NCS_RD_PULSE(10));
	smc_set_cycle_timing(SMC, ILI93XX_LCD_CS, SMC_CYCLE_NWE_CYCLE(10)
			| SMC_CYCLE_NRD_CYCLE(22));
#if ((!defined(SAM4S)) && (!defined(SAM4E)))
	smc_set_mode(SMC, ILI93XX_LCD_CS, SMC_MODE_READ_MODE
			| SMC_MODE_WRITE_MODE
			| SMC_MODE_DBW_8_BIT);
#else
	smc_set_mode(SMC, ILI93XX_LCD_CS, SMC_MODE_READ_MODE
			| SMC_MODE_WRITE_MODE);
#endif
	/** Initialize display parameter */
	g_ili93xx_display_opt.ul_width = ILI93XX_LCD_WIDTH;
	g_ili93xx_display_opt.ul_height = ILI93XX_LCD_HEIGHT;
	g_ili93xx_display_opt.foreground_color = COLOR_BLACK;
	g_ili93xx_display_opt.background_color = COLOR_WHITE;

	/** Switch off backlight */
	aat31xx_disable_backlight();

	/** Initialize LCD */
	ili93xx_init(&g_ili93xx_display_opt);

	/** Set backlight level */
	aat31xx_set_backlight(AAT31XX_AVG_BACKLIGHT_LEVEL);

	ili93xx_set_foreground_color(COLOR_WHITE);
	ili93xx_draw_filled_rectangle(0, 0, ILI93XX_LCD_WIDTH,
			ILI93XX_LCD_HEIGHT);
			
	/** Turn on LCD */
	ili93xx_display_on();
	ili93xx_set_cursor_position(0, 0);

	/** Escreve os nomes no display. */
	ili93xx_set_foreground_color(COLOR_BLACK);
	ili93xx_draw_string(10, 20, (uint8_t *)"Bruna Tavares");
	ili93xx_draw_string(10, 40, (uint8_t *)"Bruno Campos");
	ili93xx_draw_string(10, 60, (uint8_t *)"Keneth Yamada");
	
	/** Desenha linha */
	ili93xx_set_foreground_color(COLOR_BLUEVIOLET);
	ili93xx_draw_line(0, 90, 240, 90);
	
	/** Escreve contador e tempo no display. */
	ili93xx_set_foreground_color(COLOR_BLACK);
	ili93xx_draw_string(10, 110, (uint8_t *)"Contador");
	ili93xx_draw_string(10, 300, (uint8_t *)"Tempo ");

	/** Escreve valores iniciais do contador. */
	ili93xx_draw_string(130, 110, (uint8_t *)"000");
	ili93xx_draw_string(140, 300, (uint8_t *)"00:00");	

	while (1) {
		// Coloca o microcontrolador em modo Sleep aguardando interrup��es.
		pmc_sleep(SAM_PM_SMODE_SLEEP_WFI);
	}
}

