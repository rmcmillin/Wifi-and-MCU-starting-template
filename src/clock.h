
#define OSC_FREQ			20000000UL
#define PLLB_DIVIDER		20 //2-255 4
#define PLLB_MULTIPLIER		31 //7-62 10
//F_CPU = 20MHz/20*(29+1) = 32MHz
#define F_CPU				(OSC_FREQ/PLLB_DIVIDER)*(PLLB_MULTIPLIER+1)
//#define F_CPU	20000000UL

uint32_t clock_getFrequency ();
void clock_crystalInit();

void clock_init();
//configures MCU to use main osc and set pllb as main clock
//will required plla to output to cmos image sensor or other clock output that is divided
void systemClock_Init();
void outputClock_Init(); //configures 20Mhz clock on CLK0 pin B13

void slowClock_init();

