#include "asf.h"
#include "stdio_serial.h"
#include "conf_board.h"
#include "conf_clock.h"
#include "smc.h"
#include "img.h"
#include <math.h>

/** Chip select number to be set */
#define ILI93XX_LCD_CS      1

struct ili93xx_opt_t g_ili93xx_display_opt;

#define PIN_PUSHBUTTON_1_MASK	PIO_PB3
#define PIN_PUSHBUTTON_1_PIO	PIOB
#define PIN_PUSHBUTTON_1_ID		ID_PIOB
#define PIN_PUSHBUTTON_1_TYPE	PIO_INPUT
#define PIN_PUSHBUTTON_1_ATTR	PIO_PULLUP | PIO_DEBOUNCE | PIO_IT_FALL_EDGE

/************************************************************************/
/* ADC                                                                     */
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


#define ADC_POT_CHANNEL 5

/** adc buffer */
static int16_t gs_s_adc_values[BUFFER_SIZE] = { 0 };

/************************************************************************/
/* GLOBAL                                                                */
/************************************************************************/
uint32_t adc_value_old;
bool isAmp = true;
uint16_t i = 0;
uint32_t amplitude = 0;
uint32_t frequencia = 0;

//! DAC channel used for test
#define DACC_CHANNEL        1 // (PB14)

//! DAC register base for test
#define DACC_BASE           DACC
//! DAC ID for test
#define DACC_ID             ID_DACC

/** Analog control value */
#define DACC_ANALOG_CONTROL (DACC_ACR_IBCTLCH0(0x02) \
| DACC_ACR_IBCTLCH1(0x02) \
| DACC_ACR_IBCTLDACCORE(0x01))

/** The analog voltage reference **/
#define VADREF    (float) 3.3
/** The maximal data value (no sign) */
#define MAX_DIGITAL ((1 << DACC_RESOLUTION) - 1)
/** The maximal (peak-peak) amplitude value */
#define MAX_AMPLITUDE (float) 5/6
/** The minimal (peak-peak) amplitude value */
#define MIN_AMPLITUDE (float) 1/6

uint32_t desenhar_seno(uint16_t i) {
	float pontos = (2*M_PI)/MAX_DIGITAL*((float) i);
	return amplitude/2*sin(pontos) + MAX_DIGITAL/2;
}

static void configure_tc(uint32_t freq)
{
	/*
	* Aqui atualizamos o clock da cpu que foi configurado em sysclk init
	*
	* O valor atual está em : 120_000_000 Hz (120Mhz)
	*/
	uint32_t ul_sysclk = sysclk_get_cpu_hz();
	
	/*
	*	Ativa o clock do periférico TC 0
	*/
	pmc_enable_periph_clk(ID_TC0);
	
	// Configura TC para operar no modo de comparação e trigger RC
	
	tc_init(TC0,0,TC_CMR_CPCTRG | TC_CMR_TCCLKS_TIMER_CLOCK5);

	// Valor para o contador de um em um segundo.
	tc_write_rc(TC0,0, (uint32_t) freq);
	//tc_write_rc(TC0,0,65536);

	NVIC_EnableIRQ((IRQn_Type) ID_TC0);
	
	tc_enable_interrupt(TC0,0,TC_IER_CPCS);
	
	tc_start(TC0, 0);
}

/**
*  Interrupt handler for TC0 interrupt.
*/
void TC0_Handler(void){
  volatile uint32_t ul_dummy, status;
  uint32_t valorDAC = 1024;
  ul_dummy = tc_get_status(TC0,0);
  UNUSED(ul_dummy);
  
  /************************************************************************/
  /* Escreve um novo valor no DAC                                         */
  /************************************************************************/
  //status = dacc_get_interrupt_status(DACC_BASE);
  //dacc_write_conversion_data(DACC_BASE, valorDAC);
  
  if (i <= MAX_DIGITAL) {
	   dacc_write_conversion_data(DACC_BASE, desenhar_seno(i++));
	   } else {
	   i = 0;
   }
}



void atualiza_freq(uint32_t val){
	ili93xx_set_foreground_color(COLOR_WHITE);
	ili93xx_draw_filled_rectangle(140,48,200,100);
	ili93xx_set_foreground_color(COLOR_BLACK);
	char buffer[10];
	snprintf(buffer, 10, "%d", val);
	ili93xx_draw_string(145, 50, (uint8_t *)buffer);
}

void atualiza_amp(uint32_t val){
	ili93xx_set_foreground_color(COLOR_WHITE);
	ili93xx_draw_filled_rectangle(125,8,200,40);
	ili93xx_set_foreground_color(COLOR_BLACK);
	char buffer[10];
	snprintf(buffer, 10, "%d", val);
	ili93xx_draw_string(130, 10, (uint8_t *)buffer);
}

/**
 * \brief ADC interrupt handler.
 * Entramos aqui quando a conversao for concluida.
 */
void ADC_Handler(void)
{
	uint32_t tempValue;
	uint32_t status ;

	status = adc_get_status(ADC);
	
	/* Checa a interrupção */
	if ((status & ADC_ISR_EOC5)) {
		tempValue = adc_get_channel_value(ADC, ADC_POT_CHANNEL);
		if ((tempValue > adc_value_old + 2) || (tempValue < adc_value_old - 2))
		{
			if (isAmp){
				amplitude = tempValue;
				atualiza_amp(amplitude);
			} else {
				frequencia = tempValue;
				tc_stop(TC0,0);
				configure_tc((frequencia*32768/2000)/4095);
				atualiza_freq(frequencia);
			}
		}
		adc_value_old = tempValue;
	}
	
}

void atualiza_variavel() {
	ili93xx_set_foreground_color(COLOR_WHITE);
	ili93xx_draw_filled_rectangle(50,100,200,170);
	ili93xx_set_foreground_color(COLOR_BLACK);
	if (isAmp) {
		ili93xx_draw_string(60, 110, (uint8_t *) "Amplitude");
	} else {
		ili93xx_draw_string(60, 110, (uint8_t *) "Frequencia");
	}
}

/**
 *  Handle Interrupcao botao 1.
 *  Decrementa o contador e atualiza o display.
 */
static void Button1_Handler(uint32_t id, uint32_t mask) {
	if (isAmp) {
		isAmp = false;
		atualiza_variavel();
	} else {
		isAmp = true;
		atualiza_variavel();
	}
}

/**
 *  Handle Interrupcao botao 1.
 *  Decrementa o contador e atualiza o display.
 */
static void Button2_Handler(uint32_t id, uint32_t mask) {
	adc_start(ADC);
}

void configure_LCD(void){
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
};

void configure_DAC() {
	 /************************************************************************/
	 /* DAC                                                                  */
	 /************************************************************************/
	 
	 /* Enable clock for DACC */
	 sysclk_enable_peripheral_clock(DACC_ID);
	 
	 /* Reset DACC registers */
	 dacc_reset(DACC_BASE);

	 /* Half word transfer mode */
	 dacc_set_transfer_mode(DACC_BASE, 0);

	 /* selects channel */
	 dacc_set_channel_selection(DACC_BASE, DACC_CHANNEL);

	 /* Enable output channel DACC_CHANNEL */
	 dacc_enable_channel(DACC_BASE, DACC_CHANNEL);

	 /* Set up analog current */
	 dacc_set_analog_control(DACC_BASE, DACC_ANALOG_CONTROL);
}

void configure_ADC(void){
	
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

/**
 *  Configure Timer Counter 0 to generate an interrupt every 1s.
 */

static void configure_Button() 
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
* \brief Application entry point for smc_lcd example.
*
* \return Unused (ANSI-C compatibility).
*/
int main(void)
{
  sysclk_init();
  board_init();

  configure_LCD();
  configure_DAC();
  configure_ADC();
  configure_tc(4095);
  configure_Button();
  ili93xx_set_foreground_color(COLOR_BLACK);
  ili93xx_draw_string(60, 110, (uint8_t *) "Amplitude");
  ili93xx_draw_string(10, 10, (uint8_t *) "Amplitude:");
  ili93xx_draw_string(10, 50, (uint8_t *) "Frequencia:");
  atualiza_freq(frequencia);
  atualiza_amp(amplitude);

  ili93xx_draw_pixmap(0,
            ILI93XX_LCD_HEIGHT-100-1,
            240-1,
            100-1,
            &image_data_maua[0]);

  while (1) {
  }
}
