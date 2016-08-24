/**
 *	20-UART 
 * Prof. Rafael Corsi
 *
 *    (e.g., HyperTerminal on Microsoft Windows) with these settings:
 *   - 115200 bauds
 *   - 8 bits of data
 *   - No parity
 *   - 1 stop bit
 *   - No flow control
 */

#include "asf.h"
#include "conf_board.h"
#include "conf_clock.h"

/************************************************************************/
/* Configurações                                                        */
/************************************************************************/

#define STRING_EOL    "\r"
#define STRING_VERSAO "-- "BOARD_NAME" --\r\n" \
					  "-- Compiled: "__DATE__" "__TIME__" --"STRING_EOL

#define STRING_LED_B "LED Azul"
#define STRING_LED_G "LED Verde"
#define STRING_LED_R "LED Vermelho"



#define CONF_UART_BAUDRATE 115200
	
#define CONF_UART          CONSOLE_UART

/** 
 * LEDs
 */ 
#define PIN_LED_BLUE	19
#define PORT_LED_BLUE	PIOA
#define ID_LED_BLUE		ID_PIOA
#define MASK_LED_BLUE	(1u << PIN_LED_BLUE)


#define PIN_LED_RED	20
#define PORT_LED_RED	PIOC
#define ID_LED_RED	ID_PIOA
#define MASK_LED_RED	(1u << PIN_LED_RED)

#define PIN_LED_GREEN	20
#define PORT_LED_GREEN	PIOA
#define ID_LED_GREEN	ID_PIOA
#define MASK_LED_GREEN	(1u << PIN_LED_GREEN)

#define Freq_Init_Blink 4	//Hz

//Opções LED
int flagLiga =  0;



/************************************************************************/
/* Configura UART                                                       */
/************************************************************************/
void config_uart(void){
	
	/* configura pinos */
	gpio_configure_group(PINS_UART0_PIO, PINS_UART0, PINS_UART0_FLAGS);
	
	/* ativa clock */
	sysclk_enable_peripheral_clock(CONSOLE_UART_ID);
	
	/* Configuração UART */
	const usart_serial_options_t uart_serial_options = {
		.baudrate   = CONF_UART_BAUDRATE,
		.paritytype = UART_MR_PAR_NO,
		.stopbits   = 0
	};
	
	stdio_serial_init((Usart *)CONF_UART, &uart_serial_options);
}

/************************************************************************/
/* Display Menu                                                         */
/************************************************************************/
static void display_menu(void)
{
	puts("MENU \n\r"
		"  M : Exibe o MENU \n\r"
		 " B : ON/OFF o LED AZUL  \n\r"
		 " R : ON/OFF o LED VERMELHO \n\r "
		 " G : ON/OFF o LED VERDE \n\r "
		 " S : Status dos LED's \n\r "
		 " 1 : Pisca Azul" );
}

/************************************************************************/
/* Configura Tc                                                        */
/************************************************************************/

static void configure_tc(void)
{
	/*
	* Aqui atualizamos o clock da cpu que foi configurado em sysclk init
	*
	* O valor atual est'a em : 120_000_000 Hz (120Mhz)
	*/
	uint32_t ul_sysclk = sysclk_get_cpu_hz();
	
	/*
	*	Ativa o clock do periférico TC 0
	* 
	*/
	pmc_enable_periph_clk(ID_TC0);

	/*
	* Configura TC para operar no modo de comparação e trigger RC
	* devemos nos preocupar com o clock em que o TC irá operar !
	*
	* Cada TC possui 3 canais, escolher um para utilizar.
	*
	* Configurações de modo de operação :
	*	#define TC_CMR_ABETRG (0x1u << 10) : TIOA or TIOB External Trigger Selection 
	*	#define TC_CMR_CPCTRG (0x1u << 14) : RC Compare Trigger Enable 
	*	#define TC_CMR_WAVE   (0x1u << 15) : Waveform Mode 
	*
	* Configurações de clock :
	*	#define  TC_CMR_TCCLKS_TIMER_CLOCK1 : Clock selected: internal MCK/2 clock signal 
	*	#define  TC_CMR_TCCLKS_TIMER_CLOCK2 : Clock selected: internal MCK/8 clock signal 
	*	#define  TC_CMR_TCCLKS_TIMER_CLOCK3 : Clock selected: internal MCK/32 clock signal 
	*	#define  TC_CMR_TCCLKS_TIMER_CLOCK4 : Clock selected: internal MCK/128 clock signal
	*	#define  TC_CMR_TCCLKS_TIMER_CLOCK5 : Clock selected: internal SLCK clock signal 
	*
	*	MCK		= 120_000_000
	*	SLCK	= 32_768		(rtc)
	*
	* Uma opção para achar o valor do divisor é utilizar a funcao
	* tc_find_mck_divisor()
	*/
	tc_init(TC0,0, TC_CMR_CPCTRG | TC_CMR_TCCLKS_TIMER_CLOCK5);
	
	/*
	* Aqui devemos configurar o valor do RC que vai trigar o reinicio da contagem
	* devemos levar em conta a frequência que queremos que o TC gere as interrupções
	* e tambem a frequencia com que o TC está operando.
	*
	* Devemos configurar o RC para o mesmo canal escolhido anteriormente.
	*	
	*   ^ 
	*	|	Contador (incrementado na frequencia escolhida do clock)
	*   |
	*	|	 	Interrupcao	
	*	|------#----------- RC
	*	|	  /
	*	|   /
	*	| /
	*	|-----------------> t
	*
	*
	*/
	//tc_write_rc(TC0,0,8192);
	tc_write_rc(TC0,0,8192/2);
	/*
	* Devemos configurar o NVIC para receber interrupções do TC 
	*/
	NVIC_EnableIRQ(ID_TC0);
	
	/*
	* Opções possíveis geradoras de interrupção :
	* 
	* Essas configurações estão definidas no head : tc.h 
	*
	*	#define TC_IER_COVFS (0x1u << 0)	Counter Overflow 
	*	#define TC_IER_LOVRS (0x1u << 1)	Load Overrun 
	*	#define TC_IER_CPAS  (0x1u << 2)	RA Compare 
	*	#define TC_IER_CPBS  (0x1u << 3)	RB Compare 
	*	#define TC_IER_CPCS  (0x1u << 4)	RC Compare 
	*	#define TC_IER_LDRAS (0x1u << 5)	RA Loading 
	*	#define TC_IER_LDRBS (0x1u << 6)	RB Loading 
	*	#define TC_IER_ETRGS (0x1u << 7)	External Trigger 
	*/
	tc_enable_interrupt(TC0,0,TC_IER_CPCS);
	
	tc_start(TC0,0);
}


/************************************************************************/
/* Interrupt handler for TC0 interrupt.                           */                         
/************************************************************************/
 
void TC0_Handler(void)
{
	volatile uint32_t ul_dummy;

	/* Clear status bit to acknowledge interrupt */
	ul_dummy = tc_get_status(TC0,0);

	/* Avoid compiler warning */
	UNUSED(ul_dummy);
	
	/*Muda o estado do LED*/
	if(flagLiga == 1){
			if(pio_get_output_data_status(PORT_LED_BLUE,MASK_LED_BLUE)== 0)
				pio_set(PORT_LED_BLUE, MASK_LED_BLUE); 				//Se ligado
			else
				pio_clear(PORT_LED_BLUE, MASK_LED_BLUE);				//Se desligado
	}
	


}

/************************************************************************/
/* Main                                                                 */
/************************************************************************/
int main(void)
{
	uint8_t uc_key;

	/* Initialize the system */
	sysclk_init();
	board_init();
	
	/*Setando do clock no periférico*/
	pmc_enable_periph_clk(ID_PIOA);
	pmc_enable_periph_clk(ID_PIOC);
	
	/** Configura o timer */
	configure_tc();

	/* Configure LED 1 */
	pmc_enable_periph_clk(ID_LED_BLUE);
	pio_set_output(PORT_LED_BLUE  , MASK_LED_BLUE	,1,0,0);
	
	/* Configure LED 2 */
	pmc_enable_periph_clk(ID_LED_RED);
	pio_set_output(PORT_LED_RED  , MASK_LED_RED	,0,0,0);
	
	/* Configure LED 3 */
	pmc_enable_periph_clk(ID_LED_GREEN);
	pio_set_output(PORT_LED_GREEN  , MASK_LED_GREEN	,1,0,0);

	/* Initialize debug console */
	config_uart();
	
	/* frase de boas vindas */
	//puts(STRING_VERSAO);
	puts("+-+-+-+-+ +-+-+-+-+-+-+-+-+\n"
	"|C|O|M|P| |T|E|R|M|I|N|A|L|\n"
	"+-+-+-+-+ +-+-+-+-+-+-+-+-+\n");
		 
	/* display main menu */
	display_menu();

	while (1) {
		usart_serial_getchar((Usart *)CONSOLE_UART, &uc_key);	
		switch (uc_key) {
			case 'M':
				display_menu();
				break;
			case 'B':
				if(pio_get_output_data_status(PORT_LED_BLUE,MASK_LED_BLUE)== 0){
					//Se ligado	
					pio_set(PORT_LED_BLUE, MASK_LED_BLUE);
					break;		
				};
				if(pio_get_output_data_status(PORT_LED_BLUE,MASK_LED_BLUE)== 1){
					//Se desligado
					pio_clear(PORT_LED_BLUE, MASK_LED_BLUE);
					break;};				
			case 'R' :
				if(pio_get_output_data_status(PORT_LED_RED,MASK_LED_RED) == 1){
					//Se ligado
					pio_clear(PORT_LED_RED, MASK_LED_RED);
					break;
				};
				if(pio_get_output_data_status(PORT_LED_RED,MASK_LED_RED) == 0){
					//Se desligado
					pio_set(PORT_LED_RED, MASK_LED_RED);
					
				break;};
			case 'G' :
			if(pio_get_output_data_status(PORT_LED_GREEN,MASK_LED_GREEN)== 0){
				//Se ligado
				pio_set(PORT_LED_GREEN, MASK_LED_GREEN);
				break;
			};
			if(pio_get_output_data_status(PORT_LED_GREEN,MASK_LED_GREEN)== 1){
				//Se desligado
				pio_clear(PORT_LED_GREEN, MASK_LED_GREEN);
			break;};
			case 'S':
				if(pio_get_output_data_status(PORT_LED_BLUE,MASK_LED_BLUE)== 0){
					puts("Led Azul ON \n\r");
					}else{puts("Led Azul OFF \n\r");}
						
				if(pio_get_output_data_status(PORT_LED_RED,MASK_LED_RED)== 1){
					puts("Led Vermelho ON \n\r");
					}else{puts("Led Vermelho OFF \n\r");}
						
				if(pio_get_output_data_status(PORT_LED_GREEN,MASK_LED_GREEN)== 0){
					puts("Led Verde ON \n\r");
					}else{puts("Led Verde OFF \n\r");}
				break;
			case '1':
				if(flagLiga == 0){
					flagLiga =1 ;
					puts("Azul Piscando! \n\r");
				}else{
					flagLiga=0;
					puts("Azul não Piscando! \n\r");
				}
				break;
			default:
				printf("Opcao nao definida: %d \n\r", uc_key);
				
		}	
	}
}
