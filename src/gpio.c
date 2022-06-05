#include "sam.h"
#include "gpio.h"
#include "log.h"

void gpio_init(){
	//set pin directions
	gpio_pinSetDirection(WIFI_SPI_SS_PORT, WIFI_SPI_SS_PIN_bp, OUTPUT);
	gpio_pinSetDirection(WIFI_ENABLE_PORT, WIFI_ENABLE_PIN_bp, OUTPUT);
	gpio_pinSetDirection(WIFI_RESET_PORT, WIFI_RESET_PIN_bp, OUTPUT);
	
	gpio_clearPin(WIFI_ENABLE_PORT, WIFI_ENABLE_PIN_bp);
	gpio_clearPin(WIFI_RESET_PORT, WIFI_RESET_PIN_bp);
	
	REG_PMC_PCER0 |= PMC_PCER0_PID11; //enable PIOA clock
	//REG_PMC_PCER0 |= PMC_PCER0_PID12; //enable PIOB clock
	//enable interrupts
	NVIC_EnableIRQ(PIOA_IRQn);
	//NVIC_EnableIRQ(PIOB_IRQn);
}

//Peripheral pin multiplexing
void gpio_peripheralEnableOnPin(Pio *port, uint32_t pin, peripheralFunction_t peripheralFunction){
	switch (peripheralFunction){
		case A:{
			port->PIO_ABCDSR [0] &= ~(1 << pin);
			port->PIO_ABCDSR [1] &= ~(1 << pin);
			break;
		}
		case B:{
			port->PIO_ABCDSR [0] |= 1 << pin;
			port->PIO_ABCDSR [1] &= ~(1 << pin);
			break;
		}
		case C:{
			port->PIO_ABCDSR [0] &= ~(1 << pin);
			port->PIO_ABCDSR [1] |= 1 << pin;
			break;
		}
		default:{
			log_error("Bad parameter: peripheralFunction");
		}
	}
}

//Set pin as input or output
void gpio_pinSetDirection(Pio *port, uint32_t pin, direction_t direction){
	switch (direction){
		case (INPUT):{
			port->PIO_ODR |= 1 << pin;
			break;
		}
		case (OUTPUT):{
			port->PIO_OER |= 1 << pin;
			break;
		}
		default:{
			log_error("Bad parameter: direction");
		}
	}
}

//Sets pin level HIGH
void gpio_setPin(Pio *port, uint32_t pin){
	port->PIO_SODR |= 1 << pin;
}

//Sets pin level LOW
void gpio_clearPin(Pio *port, uint32_t pin){
	port->PIO_CODR |= 1 << pin;
}

//Returns pin level- 1:HIGH, 0:LOW
uint32_t gpio_getPinState (Pio *port, uint32_t pin){
	return (port->PIO_PDSR & (1 << pin)) >> pin;
}

//Returns all pin levels on port- 1:HIGH, 0:LOW
uint32_t gpio_getPortState (Pio *port){
	return port->PIO_PDSR;
}

//Enable debounce on pin (disables glitch filter)
void gpio_glitchDebounceEnable (Pio *port, uint32_t pin){
	port->PIO_IFSCER |= 1 << pin;
}

//disable debounce on pin	
void gpio_glitchDebounceDisable (Pio *port, uint32_t pin){
	port->PIO_IFSCDR |= 1 << pin;
}

//Enable glitch filter on pin (disables debounce)
void gpio_glitchFilterEnable (Pio *port, uint32_t pin){
	port->PIO_IFER |= 1 << pin;		
}

//Disables glitch filter on pin
void gpio_glitchFilterDisable (Pio *port, uint32_t pin){
	port->PIO_IFDR |= 1 << pin;
}

//Enable pull up on pin
void gpio_pullUpEnable (Pio *port, uint32_t pin){
	port->PIO_PUER |= 1 << pin;
}

//Disabled pull up on pin
void gpio_pullUpDisable (Pio *port, uint32_t pin){
	port->PIO_PUDR |= 1 << pin;
}

//Enable pull down on pin
void gpio_pullDownEnable (Pio *port, uint32_t pin){
	port->PIO_PPDER |= 1 << pin;
}

//Disable pull down on pin
void gpio_pullDownDisable (Pio *port, uint32_t pin){
	port->PIO_PPDDR |= 1 << pin;
}


uint32_t gpio_getInterruptStatus (Pio *port, uint32_t pin){
	return (port->PIO_ISR & (1 << pin)) >> pin;
}

//Enable interrupt on pin
void gpio_interruptEnable(Pio *port, uint32_t pin, interruptType_t interruptType){	
	
	//gpio_getInterruptStatus(port, pin);//clear any existing interrupt
	//log_info("%d %d",interruptType, INT_FALLING_EDGE);
	port->PIO_IER |= 1 << pin;//enable interrupt on pin
	if (interruptType != INT_INPUT_CHANGE){
		port->PIO_AIMER |= 1 << pin;
	}
	switch (interruptType){						
		case INT_INPUT_CHANGE:{
			//nothing extra to do
			break;
		}
		case INT_RISING_EDGE:{
			port->PIO_ESR |= 1 << pin;
			port->PIO_REHLSR |= 1 << pin;
			break;
		}
		case INT_FALLING_EDGE:{			
			port->PIO_ESR |= 1 << pin;
			port->PIO_FELLSR |= 1 << pin;
			break;
		}
		case INT_LOW_LEVEL:{
			port->PIO_LSR |= 1 << pin;
			port->PIO_FELLSR |= 1 << pin;
			break;
		}
		case INT_HIGH_LEVEL:{
			port->PIO_LSR |= 1 << pin;
			port->PIO_REHLSR |= 1 << pin;
			break;
		}
		default:{
			log_error("Bad parameter: interrupt type");
		}
	}
			
}

//Disable interrupt on pin
void gpio_interruptDisable(Pio *port, uint32_t pin){
	port->PIO_IDR |= 1 << pin;
}