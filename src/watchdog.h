
#define WATCH_DOG_KEY	0xA5000000

#define WDR_TIMEOUT_SECONDS	8

#define WDR_INT_ENABLE	1
#define WDR_INT_DISABLE	0

#define WDR_RESET_ENABLE	1
#define WDR_RESET_DISABLE	0

#define WDR_RESET_TYPE_ALL	0
#define WDR_RESET_TYPE_CPU	1

#define WDR_TIMER_ENABLE	0
#define WDR_TIMER_DISABLE	1

#define WDR_IDLE_ENABLE		0
#define WDR_IDLE_DISABLE	1

#define WDR_DEBUG_ENABLE	0
#define WDR_DEBUG_DISABLE	1

void wdr_watchDogInit(uint8_t timeToReset, uint8_t faultInterruptEnable, uint8_t resetEnable, uint8_t resetType, uint8_t debugHalt, uint8_t idleHalt);
void wdr_watchDogEnable();
void wdr_watchDogDisable();
void wdr_petWatchDog();
uint32_t wdr_getStatus();