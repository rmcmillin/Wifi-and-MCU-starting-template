//MCU PIN DEFINITIONS
#define WIFI_ENABLE_PORT	PIOA
#define WIFI_ENABLE_PIN_bp	3
#define WIFI_RESET_PORT		PIOA
#define WIFI_RESET_PIN_bp	5
#define WIFI_SPI_SS_PORT	PIOA
#define WIFI_SPI_SS_PIN_bp	11
#define WIFI_IRQ_PORT		PIOA
#define WIFI_IRQ_PIN_bp		1
#define WIFI_WAKE_PORT		PIOA
#define WIFI_WAKE_PIN_bp	2

#define TESTPORT	PIOA
#define TESTPIN		27

typedef enum Direction{
	INPUT,
	OUTPUT
}direction_t;

typedef enum PeripheralFunction{
	A,
	B,
	C,
}peripheralFunction_t;

typedef enum InterruptType{
	INT_INPUT_CHANGE,
	INT_RISING_EDGE,
	INT_FALLING_EDGE,
	INT_LOW_LEVEL,
	INT_HIGH_LEVEL,		
}interruptType_t;

void gpio_init();

void gpio_peripheralEnableOnPin(Pio *port, uint32_t pin, peripheralFunction_t peripheralFunction);

void gpio_pinSetDirection(Pio *port, uint32_t pin, direction_t direction);
void gpio_setPin(Pio *port, uint32_t pin);
void gpio_clearPin(Pio *port, uint32_t pin);
uint32_t gpio_getPinState (Pio *port, uint32_t pin);
uint32_t gpio_getPortState (Pio *port);
void gpio_glitchDebounceEnable (Pio *port, uint32_t pin);
void gpio_glitchDebounceDisable (Pio *port, uint32_t pin);
void gpio_glitchFilterEnable (Pio *port, uint32_t pin);
void gpio_glitchFilterDisable (Pio *port, uint32_t pin);
void gpio_pullUpEnable (Pio *port, uint32_t pin);
void gpio_pullUpDisable (Pio *port, uint32_t pin);
void gpio_pullDownEnable (Pio *port, uint32_t pin);
void gpio_pullDownDisable (Pio *port, uint32_t pin);

uint32_t gpio_getInterruptStatus (Pio *port, uint32_t pin);
void gpio_interruptEnable(Pio *port, uint32_t pin, interruptType_t interruptType);
void gpio_interruptDisable(Pio *port, uint32_t pin);

