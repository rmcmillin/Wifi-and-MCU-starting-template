#define UART_BUFFER_SIZE	80

void uart_init();
uint8_t uart_getCharacter(uint8_t *c);
void transmitByte(uint8_t data);
int8_t uart_isTxIdle();

//interrupt
void UART0_Handler( void);
