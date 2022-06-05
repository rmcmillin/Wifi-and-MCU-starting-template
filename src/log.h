#define LOG_TOGGLE_CHAR '\x0c'

//Functions to be completed by user
void uartSendByte(uint8_t byte);//function that transmits a single byte onto the serial bus
uint32_t getTimestamp ();		//returns a timestamp from RTC or 1 second counter

//Edit these values
#define LOG_USE_COLOR			//comment this line if colour not desired
#define LOG_TIME				//comment this line if timestamps not desired
#define LOG_LEVEL	LOG_TRACE	//no log output below this level

enum { LOG_TRACE, LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR, LOG_FATAL };

#define log_trace(...) log_log(LOG_TRACE, __FILE__, __LINE__, __VA_ARGS__)
#define log_debug(...) log_log(LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define log_info(...)  log_log(LOG_INFO,  __FILE__, __LINE__, __VA_ARGS__)
#define log_warn(...)  log_log(LOG_WARN,  __FILE__, __LINE__, __VA_ARGS__)
#define log_error(...) log_log(LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define log_fatal(...) log_log(LOG_FATAL, __FILE__, __LINE__, __VA_ARGS__)

void log_init();
void log_log(int level, const char *file, int line, const char *fmt, ...);

void log_toggle_active();
uint8_t log_is_active();

//Convert value to ascii to transmit onto a serial bus
void printInteger(uint32_t integer);
void printString(const char myString[]);
void printBinaryByte(uint8_t byte);
void printHexByte (uint8_t byte);
char nibbleToHexCharacter(uint8_t nibble);

//helper functions
const char *stripPath(const char *filename); //finds the final '/' in the path name
uint32_t tenPower (uint8_t i); //computes 10^i
uint8_t getNumDigits (uint32_t number); //find the number of digits in an integer
uint8_t bit_is_set(uint32_t byte, uint8_t bit);