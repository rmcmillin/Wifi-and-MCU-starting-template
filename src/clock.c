#include "sam.h"
#include "clock.h"

uint32_t clock_getFrequency (){
	return F_CPU;
}

void clock_crystalInit(){
	REG_CKGR_MOR |= CKGR_MOR_KEY_PASSWD | CKGR_MOR_MOSCXTEN; //enable external crystal
	while (!(REG_PMC_SR & PMC_SR_MOSCXTS)); //wait for crystal to become ready
	REG_CKGR_MOR |= CKGR_MOR_KEY_PASSWD | CKGR_MOR_MOSCSEL; //select crystal
	REG_PMC_MCKR |= PMC_MCKR_CSS_MAIN_CLK; //master clock source selection - choose main clock
	while (!(REG_PMC_SR & PMC_SR_MCKRDY)); //wait until main clock ready
	REG_PMC_MCKR |= PMC_MCKR_PRES_CLK_1; //select processer prescaler (0 - no divisor)
	while (!(REG_PMC_SR & PMC_SR_MCKRDY)); //wait until main clock ready
}

void clock_init(){
	//check order pg 522 in datasheet
	
	//Select external crystal as main oscillator
	REG_CKGR_MOR |= CKGR_MOR_KEY_PASSWD | CKGR_MOR_MOSCXTEN; //enable external crystal
	while (!(REG_PMC_SR & PMC_SR_MOSCXTS)); //wait for crystal to become ready
	REG_CKGR_MOR |= CKGR_MOR_KEY_PASSWD | CKGR_MOR_MOSCSEL; //select crystal
	
	//setup PLL B and enable as main clock
	//2-255
	REG_CKGR_PLLBR |= CKGR_PLLBR_DIVB(PLLB_DIVIDER); //20/4 = 5mhz
	//7-62
	REG_CKGR_PLLBR |= CKGR_PLLBR_MULB(PLLB_MULTIPLIER); //(9+1)*5 = 50mhz
	REG_PMC_MCKR |= PMC_MCKR_CSS_PLLB_CLK; //choose pllb as master clock
	while (!(REG_PMC_SR & PMC_SR_MCKRDY)); //wait until main clock ready
	REG_PMC_MCKR |= PMC_MCKR_PRES_CLK_1; //select processer prescaler (0 - no divisor)
	while (!(REG_PMC_SR & PMC_SR_MCKRDY)); //wait until main clock ready
}

void systemClock_Init(){
	//set moscxten in CKGR_MOR
	//enable external crystal and set startup time to be 64*8 slow clock cycles
	REG_CKGR_MOR |= CKGR_MOR_KEY_PASSWD | CKGR_MOR_MOSCXTEN | CKGR_MOR_MOSCXTST(64);
	//define startup time - MOSCXTST field in CKGR_MOR (before or after enable?)
	//wait for crystal stabilization - wait for MOSCXTS field in PMC_SR to be set
	while (!(REG_PMC_SR & PMC_SR_MOSCXTS));
	
	//switch mainclock to oscillator setting MOSCSEL in CKGR_MOR
	REG_CKGR_MOR |= CKGR_MOR_KEY_PASSWD | CKGR_MOR_MOSCSEL;
	//wait for selection to finish - wait for MOSCSELS to be set in PMC_SR
	while (!(REG_PMC_SR & PMC_SR_MOSCSELS));
	
	//check main clock frequency
	//Read CKGR_MCFR until the MAINFRDY field is set
	//while (!(REG_CKGR_MCFR & CKGR_MCFR_MAINFRDY));
	//read the MAINF field in CKGR_MCFR
	//frequency = REG_CKGR_MCFR & CKGR_MCFR_MAINF_Msk;
	//number is number of main clock cylces in 16 slow clock cycles
	// if zero then change to internal osc
	
	//everything to configure pll is in CKGR_PLLBR.
	//All fields in CKGR_PLLxR can be programmed in a single write operation
	//set pllB divider 0 - 127
	//set pllB multiplier 0 - 62
	//The PLLxCOUNT field specifies the number of slow clock cycles before the LOCKx bit is set in the PMC_SR
	//after CKGR_PLLxR has been written.
	REG_PMC_WPMR = PMC_WPMR_WPKEY(0x504D43);
	REG_CKGR_PLLBR |= CKGR_PLLBR_DIVB(PLLB_DIVIDER) | CKGR_PLLBR_MULB(PLLB_MULTIPLIER) | CKGR_PLLBR_PLLBCOUNT(32);
	//Once CKGR_PLLxR has been written, the user must wait for the LOCKx bit to be set in the PMC_SR. This
	//can be done either by polling LOCKx in PMC_SR
	while (!(REG_PMC_SR & PMC_SR_LOCKB));
	//wait for lockx bit to be set before setting as main clock
	
	//select masterclock and prescaler in PMC_MCKR
	
	//PMC_MCKR must not be programmed in a single write operation. The programming sequence for PMC_MCKR is as follows:
	/*
	If a new value for CSS field corresponds to PLL clock,
	? Program the PRES field in PMC_MCKR.
	? Wait for the MCKRDY bit to be set in PMC_SR.
	? Program the CSS field in PMC_MCKR.
	? Wait for the MCKRDY bit to be set in PMC_SR.
	*/
	REG_PMC_WPMR = PMC_WPMR_WPKEY(0x504D43);
	REG_PMC_MCKR |= PMC_MCKR_PRES_CLK_1;
	while (!(REG_PMC_SR & PMC_SR_MCKRDY));
	REG_PMC_WPMR = PMC_WPMR_WPKEY(0x504D43) ;
	REG_PMC_MCKR |= PMC_MCKR_CSS_PLLB_CLK;
	while (!(REG_PMC_SR & PMC_SR_MCKRDY));

	
}

void outputClock_Init(){
	//set pinb13 to peripheral b
	REG_PIOB_ABCDSR |= 1 << 13;
	//REG_PIOB_OER |= PIO_OER_P13;
	REG_PIOB_PDR |= PIO_PER_P13;
	//PMC_PCKx
	REG_PMC_WPMR = PMC_WPMR_WPKEY(0x504D43);
	REG_PMC_PCK |= PMC_PCK_CSS_MAIN_CLK;// |  PMC_PCK_PRES_CLK_2; //PCK0 uses main clock (fast osc 20MHz)
	
	REG_PMC_SCER |= PMC_SCER_PCK0; //enable PCK0 on pin B13 (do we need to set as output?)
	//wait till ready PCKRDYx in PMC_SR
	while (!(REG_PMC_SR & PMC_SR_PCKRDY0));
}

void slowClock_init(){
	//slow clock setup
	//switch from RC osc to external crystal, disable RC osc, disable PIO on xtal pins
	REG_SUPC_CR |= SUPC_CR_XTALSEL | SUPC_CR_KEY_PASSWD;
	//read status register.oscsel bit and wait for slow clock to be selected
	while (!(REG_SUPC_SR & SUPC_SR_OSCSEL));
	
}