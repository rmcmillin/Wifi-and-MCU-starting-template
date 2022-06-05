
#include "sam.h"
#include <stdio.h>
#include <stdlib.h>
#include "uart.h"
#include "clock.h"
#include "watchdog.h"
#include "timer.h"
#include "system.h"
#include "rtc.h"
#include "console.h"
#include "cmd.h"
#include "log.h"
#include "time.h"
#include "gpio.h"
#include "spi.h"
#include "winc1500_api.h"
#include "wifi.h"


int main(void)
{
    
    SystemInit(); //Initialize the SAM system
	
	systemClock_Init(); //32Mhz Pll master clock	
	
	log_init();

	uart_init();
	log_info("Start Up");
	log_info("Clock Speed: %dHz", clock_getFrequency());
	getResetSource();//print reset source to log

	console_init();
	/*
	struct tm  ts;
	time_t now = 1646795936; //timestamp
	char buf[80];	
	
	ts = *localtime(&now);
	strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", &ts);
	log_error("%s",buf);
	*/
	
	wdr_watchDogInit(WDR_TIMEOUT_SECONDS, WDR_INT_DISABLE, WDR_RESET_ENABLE, WDR_RESET_TYPE_ALL, WDR_DEBUG_ENABLE, WDR_INT_ENABLE);			

	//enable RTC	
	rtc_init (RTC_24H_CLOCK, RTC_GREGORIAN_CALENDAR, RTC_POSITIVE_PPM_CORRECTION, RTC_CLOCK_CORRECTION_PPM, RTC_LOW_RANGE_PPM_CORRECTION);
	if (rtc_start() < 0){
		log_error("Error Starting RTC");
	}	
	
	timer_init();			
	
	gpio_init();		
	
	//start SPI
	SPI_init();
		
	//set wifi state machine initial state
	wifi_tasks = wifi_state_initialize;		

	log_trace("Entering super loop");
			
	//main loop	
    while (1) 
    {
		wdr_petWatchDog();		
		console_run();	//check for commands
		timer_run();	//poll for expired timers
				
		wifi_currentState = wifi_tasks();//wifi application state machine
				
		m2m_wifi_task(); //atwinc1500 periodic check		

    }
}


void TC0_Handler(){
	//1ms timer periodic interrupt
	timer_interruptHandler();
}

//WIFI IRQ
void PIOA_Handler(){
	//Need mask to check for pin on PIOA, but for now this is the only PIOA interrupt	
	if (gpio_getInterruptStatus(WIFI_IRQ_PORT, WIFI_IRQ_PIN_bp) == 1){		
		//handle event
		m2m_EintHandler();		
	}
}

//SPI IRQ for errors
void SPI_Handler(){
	log_warn("%spi error", REG_SPI_SR);
}
