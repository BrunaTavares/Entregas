#include "asf.h"
#include "stdio_serial.h"
#include "conf_board.h"
#include "conf_clock.h"
#include "smc.h"
#include <math.h>

/** Chip select number to be set */
#define ILI93XX_LCD_CS      1

struct ili93xx_opt_t g_ili93xx_display_opt;

#define PIN_PUSHBUTTON_1_MASK	PIO_PB3
#define PIN_PUSHBUTTON_1_PIO	PIOB
#define PIN_PUSHBUTTON_1_ID		ID_PIOB
#define PIN_PUSHBUTTON_1_TYPE	PIO_INPUT
#define PIN_PUSHBUTTON_1_ATTR	PIO_PULLUP | PIO_DEBOUNCE | PIO_IT_FALL_EDGE

#define PIN_PUSHBUTTON_2_MASK	PIO_PC12
#define PIN_PUSHBUTTON_2_PIO	PIOC
#define PIN_PUSHBUTTON_2_ID		ID_PIOC
#define PIN_PUSHBUTTON_2_TYPE	PIO_INPUT
#define PIN_PUSHBUTTON_2_ATTR	PIO_PULLUP | PIO_DEBOUNCE | PIO_IT_FALL_EDGE

/************************************************************************/
/* ADC                                                           */
/************************************************************************/

/** Size of the receive buffer and transmit buffer. */
#define BUFFER_SIZE     (100)
/** Reference voltage for ADC,in mv. */
#define VOLT_REF        (3300)
/* Tracking Time*/
#define TRACKING_TIME    1
/* Transfer Period */
#define TRANSFER_PERIOD  1
/* Startup Time*/
#define STARTUP_TIME ADC_STARTUP_TIME_4

/** The maximal digital value */
#define MAX_DIGITAL     (4095)

/** adc buffer */
static int16_t gs_s_adc_values[BUFFER_SIZE] = { 0 };

#define ADC_POT_CHANNEL 5

/************************************************************************/
/* GLOBAL                                                                */
/************************************************************************/
int adc_value_old;

/************************************************************************/
/* LCD                                                                  */
/************************************************************************/
/** Chip select number to be set */
#define ILI93XX_LCD_CS      1

struct ili93xx_opt_t g_ili93xx_display_opt;

void atualiza_lcd(double valor){
	//Limpa circulo
	ili93xx_set_foreground_color(COLOR_WHITE);
	ili93xx_draw_filled_rectangle(35, 240, 200, 75);
	//ili93xx_set_foreground_color(COLOR_WHITE);
	//ili93xx_draw_filled_rectangle(0,90,ILI93XX_LCD_WIDTH,200);
	
	//Desenha Circulo
	ili93xx_set_foreground_color(COLOR_BLACK);
	ili93xx_draw_circle(120, 160, 80);
	
	//ili93xx_set_foreground_color(COLOR_BLACK);
	//ili93xx_draw_circle(ILI93XX_LCD_WIDTH/2, ILI93XX_LCD_HEIGHT/2, 40);
	double angulo = valor * M_PI/4095;
	double x, y;
	x = ILI93XX_LCD_WIDTH/2 - cos(angulo)*80;
	y = ILI93XX_LCD_HEIGHT/2 - sin(angulo)*80;
	ili93xx_draw_line(ILI93XX_LCD_WIDTH/2, ILI93XX_LCD_HEIGHT/2, x, y);
	char buffer[10];
	snprintf(buffer, 10, "%4.0f Ohms", valor);
	ili93xx_draw_string(130, 110, (uint8_t *)buffer);
}

/************************************************************************/
/* PROTOTYPES                                                           */
/************************************************************************/

void configure_LCD();
void configure_ADC();


/************************************************************************/
/* INTERRUPTION HANDLER                                                 */
/************************************************************************/
/**
 * \brief Timmer handler (100ms) starts a new conversion.
 */
void TC0_Handler(void)
{
	volatile uint32_t ul_dummy;

	/* Clear status bit to acknowledge interrupt */
	ul_dummy = tc_get_status(TC0,0);
	

	/* Avoid compiler warning */
	UNUSED(ul_dummy);
	
	adc_start(ADC);
	
}
static void push_button_handle(uint32_t id, uint32_t mask)
{
	adc_start(ADC);
	
}


/**
* \brief ADC interrupt handler.
*/

void ADC_Handler(void)
{
	uint32_t resistencia;
	uint32_t status ;

	status = adc_get_status(ADC);
	
	/* Checa se a interrup��o � devido ao canal 5 */
	if ((status & ADC_ISR_EOC5)) {
		resistencia = adc_get_channel_value(ADC, ADC_POT_CHANNEL);
		if ((resistencia > adc_value_old + 2) || (resistencia < adc_value_old - 2))
		{
			atualiza_lcd(resistencia);
		}
		adc_value_old = resistencia;
	}
	
	
}


/************************************************************************/
/* CONFIGs                                                              */
/************************************************************************/

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
	//tc_write_rc(TC0,0,32768);
	tc_write_rc(TC0,0,65536);

	NVIC_EnableIRQ((IRQn_Type) ID_TC0);
	
	tc_enable_interrupt(TC0,0,TC_IER_CPCS);
	
	tc_start(TC0, 0);
}

void configure_lcd()
{
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
}

void configure_botao(void)
{
	pmc_enable_periph_clk(PIN_PUSHBUTTON_1_ID);
	pio_set_input(PIN_PUSHBUTTON_1_PIO, PIN_PUSHBUTTON_1_MASK, PIN_PUSHBUTTON_1_ATTR);
	pio_set_debounce_filter(PIN_PUSHBUTTON_1_PIO, PIN_PUSHBUTTON_1_MASK, 10);
	pio_handler_set(PIN_PUSHBUTTON_1_PIO, PIN_PUSHBUTTON_1_ID,PIN_PUSHBUTTON_1_MASK, PIN_PUSHBUTTON_1_ATTR ,push_button_handle);
	pio_enable_interrupt(PIN_PUSHBUTTON_1_PIO, PIN_PUSHBUTTON_1_MASK);
	NVIC_SetPriority((IRQn_Type) PIN_PUSHBUTTON_1_ID, 0);
	NVIC_EnableIRQ((IRQn_Type) PIN_PUSHBUTTON_1_ID);
}


void configure_adc(void)
{
	/* Enable peripheral clock. */
	pmc_enable_periph_clk(ID_ADC);
	/* Initialize ADC. */
	/*
	* Formula: ADCClock = MCK / ( (PRESCAL+1) * 2 )
	* For example, MCK = 64MHZ, PRESCAL = 4, then:
	* ADCClock = 64 / ((4+1) * 2) = 6.4MHz;
	*/
	/* Formula:
	*     Startup  Time = startup value / ADCClock
	*     Startup time = 64 / 6.4MHz = 10 us
	*/
	adc_init(ADC, sysclk_get_cpu_hz(), 6400000, STARTUP_TIME);
	/* Formula:
	*     Transfer Time = (TRANSFER * 2 + 3) / ADCClock
	*     Tracking Time = (TRACKTIM + 1) / ADCClock
	*     Settling Time = settling value / ADCClock
	*
	*     Transfer Time = (1 * 2 + 3) / 6.4MHz = 781 ns
	*     Tracking Time = (1 + 1) / 6.4MHz = 312 ns
	*     Settling Time = 3 / 6.4MHz = 469 ns
	*/
	adc_configure_timing(ADC, TRACKING_TIME	, ADC_SETTLING_TIME_3, TRANSFER_PERIOD);

	adc_configure_trigger(ADC, ADC_TRIG_SW, 0);

	//adc_check(ADC, sysclk_get_cpu_hz());

	/* Enable channel for potentiometer. */
	adc_enable_channel(ADC, ADC_POT_CHANNEL);

	/* Enable the temperature sensor. */
	adc_enable_ts(ADC);

	/* Enable ADC interrupt. */
	NVIC_EnableIRQ(ADC_IRQn);

	/* Start conversion. */
	adc_start(ADC);

	//adc_read_buffer(ADC, gs_s_adc_values, BUFFER_SIZE);

	//adc_get_channel_value(ADC, ADC_POT_CHANNEL);

	/* Enable PDC channel interrupt. */
	adc_enable_interrupt(ADC, ADC_ISR_EOC5);
}






/************************************************************************/
/* MAIN                                                                 */
/************************************************************************/

int main(void)
{
	sysclk_init();
	board_init();

	configure_lcd();
	configure_tc();
	//configure_botao();
	configure_adc();
	
	
	
//Desenha o circulo
//ili93xx_set_foreground_color(COLOR_BLACK);
//ili93xx_draw_circle(120, 160, 80);

/** Draw text, image and basic shapes on the LCD */
ili93xx_set_foreground_color(COLOR_BLACK);
ili93xx_draw_string(10, 20, (uint8_t *)"14 - ADC");

atualiza_lcd(0.8);
//Vertical Inferior
//ili93xx_draw_line(120, 160, 120, 240);

//Vertical superior
//ili93xx_set_foreground_color(COLOR_BLACK);
//ili93xx_draw_string(110, 60, (uint8_t *)"0.5");
//ili93xx_draw_line(120, 160, 120, 80);

//Horizontal Direita
//ili93xx_set_foreground_color(COLOR_BLACK);
//ili93xx_draw_string(220, 160, (uint8_t *)"1");
//ili93xx_draw_line(120, 160, 200, 160);


//horizontal Esquerda
//ili93xx_set_foreground_color(COLOR_BLACK);
//ili93xx_draw_string(20, 160, (uint8_t *)"0");
//ili93xx_draw_line(120, 160, 40, 160);


	
	while (1) {
	}
}
