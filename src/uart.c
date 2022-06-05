#include "string.h"

#include "sam.h"
#include "uart.h"
#include "clock.h"

struct UartState{
	//uint8_t txBuffer[UART_BUFFER_SIZE];
	uint8_t rxBuffer[UART_BUFFER_SIZE];
	uint8_t rxBufferGetIndex; //oldest unread data
	uint8_t rxBufferPutIndex; //new characters inserted
};

//make array if more than one uart instance is used
static struct UartState uartState;

void uart_init(){
	//configure PIO controller A  - disable means enable peripheral on pins
	REG_PIOA_PDR |= PIO_PDR_P9; //disable PIOA control of PA9 and enable peripheral on pin
	REG_PIOA_PDR |= PIO_PDR_P10; //disable PIOA control of PA9 and enable peripheral on pin
	REG_PIOA_ABCDSR &=  ~(PIO_ABCDSR_P9);
	REG_PIOA_ABCDSR &=  ~(PIO_ABCDSR_P10);
	
	//configure PMC UART Clock
	REG_PMC_PCER0 |= PMC_PCER0_PID8; //enable UART0 clock
	
	//configure buad rate
	REG_UART0_BRGR |= F_CPU/(16*115200);
	
	//parity
	REG_UART0_MR |= UART_MR_PAR_NO;
	
	//mode
	//normal mode default
	
	//enable transmit/receive
	REG_UART0_CR |= UART_CR_TXEN;
	REG_UART0_CR |= UART_CR_RXEN;
	
	//enable interrupt on receive
	REG_UART0_IER |= UART_IER_RXRDY;
	
	//initialize state data
	memset(&uartState, 0, sizeof(uartState));

	NVIC_EnableIRQ(UART0_IRQn);
}

uint8_t uart_getCharacter(uint8_t *c){
	if (uartState.rxBufferGetIndex == uartState.rxBufferPutIndex){
		return 0;
	}
	uartState.rxBufferGetIndex ++;
	if (uartState.rxBufferGetIndex == UART_BUFFER_SIZE){
		uartState.rxBufferGetIndex = 0;
	}
	*c = uartState.rxBuffer[uartState.rxBufferGetIndex];
	return 1;
}

void transmitByte(uint8_t data){
	//wait for ready
	while (!(REG_UART0_SR&UART_SR_TXRDY));
	while (!(REG_UART0_SR&UART_SR_TXEMPTY));
	REG_UART0_THR |= data;
}

void UART0_Handler( void) {
	uint32_t status = REG_UART0_SR;
	if ((status & UART_SR_RXRDY)){
		//increment index
		uartState.rxBufferPutIndex++;
		if (uartState.rxBufferPutIndex == UART_BUFFER_SIZE){
			uartState.rxBufferPutIndex = 0;
		}
		uartState.rxBuffer[uartState.rxBufferPutIndex] = REG_UART0_RHR;
		//transmitByte(readByte);
	}
}

//returns 1 if idle
int8_t uart_isTxIdle(){
	int8_t rc = 0;

	if (uartState.rxBufferGetIndex != uartState.rxBufferPutIndex){
		rc = 1;
	}

	return rc;
}