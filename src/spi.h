#include "sam.h"
//#include "basic_uart.h"

//pin and mode defines for all slaves
#define SPI_NHDLCD_SS	31 //PIO_CODR_P31 or PIO_SODR_P31 //pin slave is on
#define SPI_NHDLCD_MODE	3 //spi mode

#define DUMMYBYTE 0x00

void SPI_setMode(uint8_t mode);

void SPI_selectPeripheral(uint8_t peripheral);
/*
built in pins for spi slaves, but can use any gpio active low using slave select and slavedeselect
*/

void SPI_init();

void wifi_SPI_slaveSelect();
/*
slave is pin
*/

void wifi_SPI_slaveDeselect();
/*
wait for transfer to be comlete
signal last transfer
slave pin goes high to deselect
*/

uint8_t SPI_byteExchange(uint8_t data);
/*
send a byte, receive a byte
*/

