/**
 * IMT - Rafael Corsi
 * 
 * PIO - 07
 *  Configura o PIO do SAM4S (Banco A, pino 19) para operar em
 *  modo de output. Esse pino est� conectado a um LED, que em 
 *  l�gica alta apaga e l�gica baixa acende.
*/

#include <asf.h>
/* Inclui a biblioteca dfe configura��es do PMC*/
#include "Driver/pmc_maua.h"
#include "Driver/pio_maua.h"
/*
 * Prototypes
 */

/** 
 * Defini��o dos pinos
 * Pinos do uC referente aos LEDS.
 *
 * O n�mero referente ao pino (PIOAxx), refere-se ao
 * bit que deve ser configurado no registrador para alterar
 * o estado desse bit espec�fico.
 *
 * exe : O pino PIOA_19 � configurado nos registradores pelo bit
 * 19. O registrador PIO_SODR configura se os pinos ser�o n�vel alto.
 * Nesse caso o bit 19 desse registrador � referente ao pino PIOA_19
 *
 * ----------------------------------
 * | BIT 19  | BIT 18  | ... |BIT 0 |
 * ----------------------------------
 * | PIOA_19 | PIOA_18 | ... |PIOA_0|
 * ----------------------------------
 */
#define PIN_LED_BLUE 19
#define PIN_LED_GREEN 20 // definindo que o pino verde � o P20 
#define PIN_LED_RED 20
#define PIN_BUTTON 3

#define MASK_LED_BLUE (1 << PIN_LED_BLUE)


#define TEMPO 100

/** 
 * Defini��o dos ports
 * Ports referentes a cada pino
 */
#define PORT_LED_BLUE PIOA



/**
 * Main function
 * 1. configura o clock do sistema
 * 2. desabilita wathdog
 * 3. ativa o clock para o PIOA
 * 4. ativa o controle do pino ao PIO
 * 5. desabilita a prote��o contra grava��es do registradores
 * 6. ativa a o pino como modo output
 * 7. coloca o HIGH no pino
 */

int main (void)
{

	/**
	* Inicializando o clock do uP
	*/
	sysclk_init();
	
	/** 
	*  Desabilitando o WathDog do uP
	*/
	WDT->WDT_MR = WDT_MR_WDDIS;
		
	// 29.17.4 PMC Peripheral Clock Enable Register 0
	// 1: Enables the corresponding peripheral clock.
	// ID_PIOA = 11 - TAB 11-1
	_pmc_enable_clock_periferico(ID_PIOA);
	_pmc_enable_clock_periferico(ID_PIOB);
	_pmc_enable_clock_periferico(ID_PIOC);

	 //31.6.1 PIO Enable Register
	// 1: Enables the PIO to control the corresponding pin (disables peripheral control of the pin).	
	PIOA->PIO_PER = (1 << PIN_LED_BLUE ) |  (1 << PIN_LED_GREEN );
	
	PIOC->PIO_PER = (1 << PIN_LED_RED );
	PIOB->PIO_PER = (1 << PIN_BUTTON );

	// 31.6.46 PIO Write Protection Mode Register
	// 0: Disables the write protection if WPKEY corresponds to 0x50494F (PIO in ASCII).
	PIOA->PIO_WPMR = 0;
	PIOC->PIO_WPMR = 0;
	
	//Desativar o buffer de sa�da
	PIOB->PIO_ODR = (1 << PIN_BUTTON );
	
	//Ativar o pull-up
	//PIOB->PIO_PUER= (1 << PIN_BUTTON );
	_pio_pull_up(PIOB,1 << PIN_BUTTON,1);
	
	//Ativando o Deboucing
	PIOB->PIO_IFSCDR = (1 << PIN_BUTTON );
	
	
	// 31.6.4 PIO Output Enable Register
	// 1: Enables the output on the I/O line.
	//PIOA->PIO_OER |=  (1 << PIN_LED_BLUE );
	_pio_set_output(PIOA,1 << PIN_LED_BLUE,1,1);
	//PIOA->PIO_OER |=  (1 << PIN_LED_GREEN );
	_pio_set_output(PIOA,1 << PIN_LED_GREEN,1,1);
	//PIOC->PIO_OER |=  (1 << PIN_LED_RED );
	_pio_set_output(PIOC,1 << PIN_LED_RED,1,1);

	// 31.6.10 PIO Set Output Data Register
	// 1: Sets the data to be driven on the I/O line.
	
	//APAGA LED
	//PIOA->PIO_SODR = (1 << PIN_LED_BLUE );
	//PIOA->PIO_SODR = (1 << PIN_LED_GREEN );
	//PIOC->PIO_CODR = (1 << PIN_LED_RED);
	
	//ACENDE LED
	//PIOA->PIO_CODR = (1 << PIN_LED_BLUE );
	//PIOA->PIO_CODR = (1 << PIN_LED_GREEN );
	//PIOC->PIO_SODR = (1 << PIN_LED_RED);
	/**
	*	Loop infinito
	*/
		while(1){
			//Shift 3 posi��e e and 1 = transforma tudo em 0 menos o bit da posi��o 3 e joga para a posi��o 0 e compara
			if(_pio_get_output_data_status(PIOB,1 << PIN_BUTTON)){
				_pio_clear( PIOC , 1 << PIN_LED_RED);
				_pio_set(  PIOA,1 << PIN_LED_BLUE);
				_pio_set(  PIOA,1 << PIN_LED_GREEN);			
			}
			else{
				//setar em 1 o PIO_CODR ativa ele portanto PIO_ODSR fica 0 ACENDE LED VERMELHO
				_pio_clear( PIOC , 1 << PIN_LED_RED);
				//setar em 1 o PIO_SODR ativa ele portanto PIO_ODSR fica 0 APAGA LED VERDE
				//Setando 1 em PIN_LED_GREEN e 1 em PIN_LED_BLUE e fazendo um ou neles eu somo e seto os dois juntos    
				_pio_set( PIOA , 1 << PIN_LED_GREEN | 1 << PIN_LED_BLUE);
				//setar em 1 o PIO_CODR ativa ele portanto PIO_ODSR fica 0 APAGA LED AZUL
				delay_ms(TEMPO);
	
				//setar em 1 o PIO_SODR ativa ele portanto PIO_ODSR fica 1 ACENDE LED AZUL
				_pio_set(  PIOA,1 << PIN_LED_BLUE);
				//setar em 1 o PIO_SODR ativa ele portanto PIO_ODSR fica 1 APAGA LED VERMELHO
				_pio_set(  PIOC,1 << PIN_LED_RED);
				//setar em 1 o PIO_SODR ativa ele portanto PIO_ODSR fica 0 APAGA LED VERDE
				_pio_clear( PIOA , 1 << PIN_LED_GREEN);
				delay_ms(TEMPO);
	
				//setar em 1 o PIO_SODR ativa ele portanto PIO_ODSR fica 1 ACENDE LED VERDE
				_pio_set(  PIOA,1 << PIN_LED_GREEN);
				//setar em 1 o PIO_CODR ativa ele portanto PIO_ODSR fica 0 APAGA LED AZUL
				_pio_clear( PIOA , 1 << PIN_LED_BLUE);
				//setar em 1 o PIO_SODR ativa ele portanto PIO_ODSR fica 1 APAGA LED VERMELHO
				_pio_set(  PIOC,1 << PIN_LED_RED);
				delay_ms(TEMPO);
			}
	
	//delay_ms(1000);
	
	
	
	
	//PIOA->PIO_SODR = (1 << PIN_LED_BLUE );
            /*
             * Utilize a fun��o delay_ms para fazer o led piscar na frequ�ncia
             * escolhida por voc�.
             */
            //delay_ms(1000);
		
	}
}



