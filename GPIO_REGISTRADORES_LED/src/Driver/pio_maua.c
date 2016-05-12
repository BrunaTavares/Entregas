#include "pio_maua.h"

void _pio_clear( Pio *p_pio,const uint32_t ul_mask){
	p_pio->PIO_CODR = ul_mask;
}

void _pio_set(  Pio *p_pio,const uint32_t ul_mask){
	p_pio->PIO_SODR = ul_mask;

}

void _pio_pull_up(Pio *p_pio,const uint32_t ul_mask,const uint32_t ul_pull_up_enable){
	
	// Se ul_pull_up_enable for 1, ativa o pull-up
	if(ul_pull_up_enable == 1){
		p_pio->PIO_PUER = ul_mask;
	} 
	
	// Senão, desabilita o pull-up
	else{
		p_pio->PIO_PUDR = ul_mask;
	}
}

void _pio_pull_down(Pio *p_pio,const uint32_t ul_mask,const uint32_t ul_pull_down_enable){
	
	// Se ul_pull_up_enable for 1, ativa o pull-up
	if(ul_pull_down_enable == 1){
		p_pio->PIO_PPDER = ul_mask;
	}
	
	// Senão, desabilita o pull-up
	else{
		p_pio->PIO_PPDDR = ul_mask;
	}
}

void _pio_set_output(Pio *p_pio,const uint32_t ul_mask,const uint32_t ul_default_level,const uint32_t ul_pull_up_enable){
			
	_pio_pull_up(p_pio,ul_mask,ul_pull_up_enable);
		
	if(ul_default_level == 1){	
		_pio_set(  p_pio,ul_mask);
	}
	else {
		_pio_clear( p_pio , ul_mask);
	}
	
	//Setar como saída, se PIO_OER for 1
	p_pio->PIO_OER =  (ul_mask);
}

//_pio_set_input(PIOB, 1 << 3, PIO_PULLUP | PIO_DEBOUNCE);
void _pio_set_input(Pio *p_pio,const uint32_t ul_mask,const uint32_t ul_attribute){
	
	if(ul_attribute & PIO_PULLUP)
		_pio_pull_down(p_pio, ul_mask, 1);
	else
		_pio_pull_down(p_pio, ul_mask, 0);
	
	if(ul_attribute & PIO_DEGLITCH)
		p_pio->PIO_IFER = ul_mask;
	else
		p_pio->PIO_IFDR = ul_mask;
		
	if(ul_attribute & PIO_DEBOUNCE)
		p_pio->PIO_IFSCDR = ul_mask;
	else 
		p_pio->PIO_IFSCER = ul_mask;
	p_pio->PIO_ODR =  (ul_mask);
	
}


uint32_t _pio_get_output_data_status(const Pio *p_pio,const uint32_t ul_mask){
	
	//Leitura do PIO_OSDR, que fica em 1 1
	//Ao colocar & 10000 quero ver se na posição de 1 tem zero ou não 1 && 1 = 1 e 1 && 0 = 0
	if(p_pio->PIO_PDSR & ul_mask){
		return 1; 
	}
	else{
		return 0; 
	}
}


