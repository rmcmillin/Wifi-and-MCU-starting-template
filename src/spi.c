#include "sam.h"
#include "spi.h"
#include "gpio.h"
#include "log.h"

void SPI_setMode(uint8_t mode){
	/*
	Mode		CPOL	NCPHA
	Mode0		0		1
	Mode1		0		0
	Mode2		1		1
	Mode3		1		0
	*/
	if (mode == 0){
		REG_SPI_CSR &= ~SPI_CSR_CPOL;
		REG_SPI_CSR |= SPI_CSR_NCPHA;
	}
	else if (mode == 1){
		REG_SPI_CSR &= ~SPI_CSR_CPOL;
		REG_SPI_CSR &= ~SPI_CSR_NCPHA;
	}
	else if (mode == 2){
		REG_SPI_CSR |= SPI_CSR_CPOL;
		REG_SPI_CSR |= SPI_CSR_NCPHA;
	}
	else if (mode == 3){
		REG_SPI_CSR |= SPI_CSR_CPOL;
		REG_SPI_CSR &= ~SPI_CSR_NCPHA;
	}
}


void SPI_selectPeripheral(uint8_t peripheral){
	//chose peripheral
	//this only works if SPI_MR.PS = 0
	//if SPI_MR.PS = 1 then peripheral selection is done in SPI_THR.PCS
	if (peripheral == 0){
		//choose NPCS0
		REG_SPI_MR |= SPI_MR_PCS(0b1110);
	}
	else if (peripheral ==1){
		//choose NPCS1
		REG_SPI_MR |= SPI_MR_PCS(0b1101);
	}
	else if (peripheral ==2){
		//choose NPCS2
		REG_SPI_MR |= SPI_MR_PCS(0b1011);
	}
	else if (peripheral ==3){
		//choose NPCS3
		REG_SPI_MR |= SPI_MR_PCS(0b0111);
	}
}

void SPI_init(){
	//enable peripheral clock
	REG_PMC_PCER0 |= PMC_PCER0_PID21;
	//set spi master mode
	REG_SPI_MR |= SPI_MR_MSTR;
	//set fixed peripheral select(peripheral chosen in SP_MR.PCS instead of SPI_THR.PCS)
	REG_SPI_MR &= ~SPI_MR_PS;	
	REG_SPI_CR |= SPI_MR_DLYBCS(160); //delay between chip select 160/fspiclock
	REG_SPI_CSR |= SPI_CSR_DLYBS(160) | SPI_CSR_DLYBCT(160);
	REG_SPI_MR |= SPI_MR_WDRBT; //wait to transfer until RDR is empty to avoid over runs
	//set polarity and clock phase	
	SPI_setMode(0); //only mode 0 supported by winc1500
	//set clock generator (1 = peripheral clock rate), otherwise a divisor 
	//SCBR = fperipheral clock / SPCK Bit Rate
	//REG_SPI_CSR |= SPI_CSR_SCBR(20); //spck bit rate = 20M/20 = 1Mhz
	REG_SPI_CSR |= SPI_CSR_SCBR(4); //spck bit rate = 32M/4 = 8Mhz
	//chip select remains low after transfer
	REG_SPI_CSR |= SPI_CSR_CSNAAT;
	//give peripheral control of pins (Chip select pins are optional)
	//REG_PIOA_PDR |= PIO_PDR_P11; //NPCS0
	//REG_PIOA_PDR |= PIO_PDR_P31; //NPCS1
	REG_PIOA_PDR |= PIO_PDR_P12; //MISO
	REG_PIOA_PDR |= PIO_PDR_P13; //MOSI
	REG_PIOA_PDR |= PIO_PDR_P14; //sck
	
	//enable mode fault detectin
	REG_SPI_MR |= SPI_MR_MODFDIS;
	//enable interrupt on error
	REG_SPI_IER |= SPI_IER_MODF | SPI_IER_OVRES | SPI_IER_UNDES;
	NVIC_EnableIRQ(SPI_IRQn);
	
	//enable SPI
	REG_SPI_CR |= SPI_CR_SPIEN;
}

void wifi_SPI_slaveSelect(){
	//REG_PIOA_CODR |= PIO_CODR_P31; 
	gpio_clearPin(WIFI_SPI_SS_PORT, WIFI_SPI_SS_PIN_bp);
}

void wifi_SPI_slaveDeselect(){
	//wait for transmit register to be empty
	while (!(REG_SPI_SR & SPI_SR_TDRE));
	//send data to transmit register
	REG_SPI_TDR |= SPI_TDR_LASTXFER;
	//wait for received data to be ready to be read
	while (!(REG_SPI_SR & SPI_SR_RDRF));
	//REG_PIOA_SODR |= PIO_SODR_P31; //set PA30 high	
	gpio_clearPin(WIFI_SPI_SS_PORT, WIFI_SPI_SS_PIN_bp);
}

uint8_t SPI_byteExchange(uint8_t data){	
	//log_trace ("txrx");
	//wait for transmit register to be empty
	while (!(REG_SPI_SR & SPI_SR_TDRE));
	//send data to transmit register
	REG_SPI_TDR |= data;
	//wait for received data to be ready to be read
	while (!(REG_SPI_SR & SPI_SR_RDRF));
	//read received data
	return REG_SPI_RDR;
}



/*
void SPI_deselect(){
	//wait for transmit register to be empty
	while (!(REG_SPI_SR & SPI_SR_TDRE));
	//send data to transmit register
	REG_SPI_TDR |= SPI_TDR_LASTXFER;
	//wait for received data to be ready to be read
	while (!(REG_SPI_SR & SPI_SR_RDRF));	
}*/
