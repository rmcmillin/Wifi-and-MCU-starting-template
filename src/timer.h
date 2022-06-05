
void timer_init(); //setup hardware
int32_t timer_registerCommands(); //register commands with cmd.c, success return 0
void timer_run(); //superloop - check for expired timers, run callbacks

typedef enum TimerState{
	UNUSED = 0,
	STOPPED,
	RUNNING,
	EXPIRED,
}timerState_t;

typedef enum TimerCallBackType {
	POLL,
	INTERRUPT
}timerCallBackType_t;

enum TimerCallBackAction {
	NONE,
	RESTART,
};

typedef enum TimerCallBackAction (*timerCB)(int32_t timerID);

//functions to be used by other libraries
uint32_t timer_getTicks(); //returns millisecond timer value timer_tick_counter
int32_t timer_registerTimer(uint32_t period); //callable from another library that requires a timer, returns timer ID (no callback)
int32_t timer_registerTimer_cb(uint32_t period, timerCB callBackFunction, timerCallBackType_t callBackType); //get a timer and include a callback function
int32_t timer_start(int32_t timerID, uint32_t period);
int32_t timer_setPeriod(int32_t timerID, uint32_t period);
int32_t timer_release(int32_t timerID);
int32_t timer_isExpired(int32_t timerID);
int32_t timer_isRunning(int32_t timerID);
int32_t timer_printInfo(int32_t timerID); 

//Commands for cmd.c module

//interrupt handler
void timer_interruptHandler();

///////////
//COMANDS//
///////////

int32_t cmd_timerStatus(int32_t argc, const char** argv);