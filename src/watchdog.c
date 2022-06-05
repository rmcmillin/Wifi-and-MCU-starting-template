#include "sam.h"
#include "watchdog.h"
#include "commonmacro.h"
#include "log.h"

struct WatchDogSettings{
	uint32_t	timeToReset;			//!< seconds until watchdog reset
	uint8_t		faultInterruptEnable;	//!< set to 1 to enable interrupt on watchdog error or underflow
	uint8_t		resetEnable;			//!< set to 1 to enable reset on error or underflow
	uint8_t		resetType;				//!< 0: all resets, 1: CPU reset
	uint8_t		debugHalt;				//!< 0: watchdog runs in debug state, 1: watchdog stops in debug state
	uint8_t		idleHalt;				//!< 0: watchdog runs in idle mode, 1: watchdog stops in idle mode
	uint8_t		timerState;				//!< 0: enable timer, 1: disable timer
}watchDogSettings;

void wdr_watchDogInit(uint8_t timeToReset, uint8_t faultInterruptEnable, uint8_t resetEnable, uint8_t resetType, uint8_t debugHalt, uint8_t idleHalt){
	//REG_WDT_MR may only be written once after a CPU reset
	//count down timer uses slow clock (32.768khz typical) div 128
	//watch dog time = 1/256 * WDV (max 0xfff) for max of 16seconds
	watchDogSettings.timeToReset = timeToReset * 256;
	watchDogSettings.faultInterruptEnable = faultInterruptEnable;
	watchDogSettings.resetEnable = resetEnable;
	watchDogSettings.resetType = resetType;
	watchDogSettings.debugHalt = debugHalt;
	watchDogSettings.idleHalt = idleHalt;
}

void wdr_watchDogEnable(){
	uint32_t registerValue = 0;
	
	registerValue |= WDT_MR_WDV(watchDogSettings.timeToReset);
	registerValue |= WDT_MR_WDD(watchDogSettings.timeToReset);
	
	if (watchDogSettings.faultInterruptEnable == WDR_INT_ENABLE){
		registerValue |= WDT_MR_WDFIEN;
		NVIC_EnableIRQ(WDT_IRQn);
	}
	
	if (watchDogSettings.resetEnable == WDR_RESET_ENABLE){
		registerValue |= WDT_MR_WDRSTEN;
	}
	
	if (watchDogSettings.resetType == WDR_RESET_TYPE_CPU){
		registerValue |= WDT_MR_WDRPROC;
	}
	
	if (watchDogSettings.idleHalt == WDR_IDLE_DISABLE){
		registerValue |= WDT_MR_WDDBGHLT;
	}
	
	if (watchDogSettings.debugHalt == WDR_DEBUG_DISABLE){
		registerValue |= WDT_MR_WDIDLEHLT;
	}
	
	REG_WDT_MR = registerValue;
}

void wdr_watchDogDisable(){
	REG_WDT_MR = WDT_MR_WDDIS;
}

void wdr_petWatchDog(){
	REG_WDT_CR = WATCH_DOG_KEY | WDT_CR_WDRSTT;
}

uint32_t wdr_getStatus(){
	return REG_WDT_SR;
}

void WDT_Handler(void) {
	uint32_t status;
	//read status register to clear interrupt
	status = wdr_getStatus();
	
	switch (status){
		case(WDT_SR_WDERR): //watchdog timer reset before WDD value
			log_trace("WatchDog Error");
			break;
		case(WDT_SR_WDUNF): //watchdog timer expired
			log_trace("WatchDog UnderFlow");
			break;
	}
}