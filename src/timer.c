#include <stdio.h>

#include "sam.h"
#include "cmd.h"
#include "console.h"
#include "clock.h"
#include "timer.h"
#include "log.h"
#include "commonmacro.h"

#define MAX_TIMERS	10

static uint32_t timer_tick_counter;
static uint32_t uptime_counter_ms;
static uint32_t uptime_counter_seconds;

//Client and Command info to register with cmd module
static struct CommandInfo cmds[] = {
	{
		.name = "status",
		.func = cmd_timerStatus,
		.help = "Get module status, usage: tmr status",
	},
	/*
	{
		.name = "test",
		.func = cmd_tmr_test,
		.help = "Run test, usage: tmr test [<op> [<arg1> [<arg2>]]] (enter no op for help)",
	},
	*/
};

static struct ClientInfo client = {
	.name = "tmr",
	.num_cmds = ARRAY_SIZE(cmds),
	.cmds = cmds,	
};


struct TimerInfo {
	uint32_t period_ms;
	uint32_t start_time;
	//uint32_t cb_user_data;
	timerState_t state;
	timerCallBackType_t callBackType;
	timerCB callBackFunction;
};

//static keyword here limits this to a single instance and assigns the value of 0
static struct TimerInfo timerList[MAX_TIMERS];

//Hardware Setup for TC0
void timer_init(){
	//enable interrupts in NVIC for TC0
	NVIC_EnableIRQ(TC0_IRQn);
	
	//enable peripheral clock for timer counter channel0 in PMC
	REG_PMC_PCER0 |= PMC_PCER0_PID23;
	
	REG_TC0_CMR0 |= TC_CMR_TCCLKS_TIMER_CLOCK3; //mainclock div 32
	//REG_TC0_IER0 |= TC_IER_COVFS; //enable couter overflow interrupt
	REG_TC0_IER0 |= TC_IER_CPCS; //enable RC compare interrupt
	REG_TC0_CMR0 |= TC_CMR_CPCTRG; //compare resets counter and clock
	REG_TC0_CCR0 |= TC_CCR_CLKEN; //enable tc clock

	//set RC compare
	REG_TC0_RC0 = 1000; // 32Mhz/32 = 1Mhz -> 1/1Mhz * 1000 = 0.001s
	//enable tc clock
	REG_TC0_CCR0 |= TC_CCR_CLKEN;
	//start timer
	REG_TC0_CCR0 |= TC_CCR_SWTRG;
	
	timer_registerCommands();
	
}

int32_t timer_registerCommands(){
	int32_t rc = 0;
	cmd_register(&client);
	return rc;
}

void timer_run(){
	static uint32_t oldTime = 0;
	
	uint32_t newTime = timer_getTicks();

	if (oldTime == newTime){
		//do nothing
	}
	else{
		oldTime = newTime;
		//process timers
		for (uint8_t i=0; i<MAX_TIMERS; i++){
			struct TimerInfo *timerInfo = &timerList[i];
			if (timerInfo->callBackType == POLL && timerInfo->state == RUNNING){
				if (newTime - timerInfo->start_time >= timerInfo->period_ms){					
					timerInfo->state = EXPIRED;
					if (timerInfo->callBackFunction != 0){
						enum TimerCallBackAction result = timerInfo->callBackFunction(i);
						if (result == RESTART){
							timerInfo->state = RUNNING;
							timerInfo->start_time += timerInfo->period_ms;
						}
					}
				}
			}
		}
	}
}

uint32_t timer_getTicks(){
	return timer_tick_counter;
}

//callable from another library that requires a timer, returns timer ID
int32_t timer_registerTimer(uint32_t period){
	for (uint8_t i=0; i < MAX_TIMERS; i++) {
		struct TimerInfo *timerInfo = &timerList[i];
		if (timerInfo->state == UNUSED) {
			timerInfo->period_ms = period;
			if (period == 0) {
				timerInfo->state = STOPPED;
			} 
			else{
				timerInfo->start_time = timer_getTicks();
				timerInfo->state = RUNNING;
			}
			timerInfo->callBackFunction = 0; //NULL
			//timerInfo->cb_user_data = 0;
			log_trace("timer: %d",i);
			return i;
		}
	}
	// Out of timers.
	log_error("No Timers Free");
	return -1;
}

//get a timer and include a callback function, returns timer ID
int32_t timer_registerTimer_cb(uint32_t period, timerCB callBackFunction, timerCallBackType_t callBackType){
	int32_t timerID;

	timerID = timer_registerTimer(period);
	if (timerID >= 0) {
		struct TimerInfo *timerInfo = &timerList[timerID];
		timerInfo->callBackType = callBackType;
		timerInfo->callBackFunction = callBackFunction;
	}
	return timerID;
}

//starts a timer with period, must use timerID that has been registered
int32_t timer_start(int32_t timerID, uint32_t period){
	int32_t rc;
	if (timerID >= 0 && timerID < MAX_TIMERS){
		struct TimerInfo *timerInfo = &timerList[timerID];
		if (timerInfo->state != UNUSED){
			timerInfo->period_ms = period;
			if (period == 0){
				timerInfo->state = STOPPED;
			}
			else{
				timerInfo->start_time = timer_getTicks();
				timerInfo->state = RUNNING;
			}
			rc = 0; //success
		}
		else{
			rc = ERR_BAD_STATE;
		}
	}
	else{
		rc = ERR_BAD_PARAMETER;
	}
	return rc;
}

int32_t timer_setPeriod(int32_t timerID, uint32_t period){
	int32_t rc;
	if (timerID >= 0 && timerID < MAX_TIMERS){
		if (period == 0){
			rc = ERR_BAD_PARAMETER;
		}
		else{
			struct TimerInfo *timerInfo = &timerList[timerID];
			timerInfo->period_ms = period;
		}
	}
	else{
		rc = ERR_BAD_PARAMETER;
	}
	
	return rc;
}

int32_t timer_release(int32_t timerID){
	if (timerID >= 0 && timerID < MAX_TIMERS) {
		struct TimerInfo *timerInfo = &timerList[timerID];
		timerInfo->state = UNUSED;		
		return 0;
	}
	return ERR_BAD_PARAMETER;
}

//returns 1 if expired
int32_t timer_isExpired(int32_t timerID){
	int32_t rc;
	if (timerID >= 0 && timerID < MAX_TIMERS){
		struct TimerInfo *timerInfo = &timerList[timerID];
		rc = timerInfo->state;		
	}
	else{
		rc = ERR_BAD_PARAMETER;
	}
 
	return rc;
}

int32_t timer_isRunning(int32_t timerID){
	int32_t rc;
	if (timerID >= 0 && timerID < MAX_TIMERS){
		struct TimerInfo *timerInfo = &timerList[timerID];
		rc = timerInfo->state = RUNNING;
	}
	else{
		rc = ERR_BAD_PARAMETER;
	}
	
	return rc;
}

int32_t timer_printInfo(int32_t timerID){
	int32_t rc;
	uint32_t timeNow = timer_getTicks();
	char buf [40];
	const char *stateStrings[] = {"Unused", "Stopped", "Running", "Expired"}; //if this is used else where then put in .h
	
	if (timerID >= 0 && timerID < MAX_TIMERS){
		struct TimerInfo *timerInfo = &timerList[timerID];
		sprintf (buf, "TimerID: %lu\r\n", timerID);
		printString(buf);
		sprintf (buf, "\t Period: %lu \r\n", timerInfo->period_ms);
		printString(buf);
		sprintf (buf, "\t State: %s\r\n", stateStrings[timerInfo->state]);
		printString(buf);
		sprintf (buf, "\t Start Time: %lu\r\n", timerInfo->start_time);
		printString(buf);
		sprintf (buf, "\t Time Now: %lu\r\n", timeNow);
		printString(buf);
	}
	else{
		rc = ERR_BAD_PARAMETER;
	}

	return rc;
}

void timer_interruptHandler(){
	//read status register to check interrupt source and clear the interrupt flag
	uint32_t status = REG_TC0_SR0;
	if ((status & TC_SR_CPCS)>=1){
		//increment counters
		timer_tick_counter++;
		if (++uptime_counter_ms == 1000){
			uptime_counter_ms = 0;
			uptime_counter_seconds++;
			/*debug
			log_trace("seconds: %d",uptime_counter_seconds);*/
		}
		//iterate through timers to check for expired and callback requires interrupt
	}
}

///////////
//COMANDS//
///////////

static const char* timer_stateString(timerState_t state)
{
	const char* rc;

	switch (state) {
		case UNUSED:
		rc = "unused";
		break;
		case STOPPED:
		rc = "stopped";
		break;
		case RUNNING:
		rc = "running";
		break;
		case EXPIRED:
		rc = "expired";
		break;
		default:
		rc = "invalid";
		break;
	}
	return rc;
}

int32_t cmd_timerStatus(int32_t argc, const char** argv){
	uint32_t now_ms = timer_getTicks();
	uint32_t sec = uptime_counter_seconds;

	static const uint32_t sec_per_day = 24 * 3600;
	static const uint32_t sec_per_hour = 3600;
	static const uint32_t sec_per_min = 60;

	printc("Up time: %lud %luh %lum %lus\n",
	sec / sec_per_day,
	(sec % sec_per_day) / sec_per_hour,
	(sec % sec_per_hour) / sec_per_min,
	(sec % sec_per_min));

	printc("Current millisecond tmr=%lu\n\n", now_ms);

	printc("ID   Period   Start time Time left  CB State\n");
	printc("-- ---------- ---------- ---------- -- ------\n");
	for (uint8_t i = 0; i < MAX_TIMERS; i++) {
		struct TimerInfo *ti = &timerList[i];
		if (ti->state == UNUSED)
		continue;
		printc("%2lu %10lu %10lu %10lu %2s %s\n", i, ti->period_ms,
		ti->start_time,
		ti->state == RUNNING ?
		ti->period_ms - (now_ms - ti->start_time) : 0,
		//ti->callBackFunction == 0 ? "N" : "Y", );
		
		ti->callBackFunction == 0 ? "N" : "Y",		
		timer_stateString(ti->state));
	}
	return 0;
}