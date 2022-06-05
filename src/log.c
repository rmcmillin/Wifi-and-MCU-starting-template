#include "sam.h"
#include <stdarg.h>
#include "uart.h"
#include "rtc.h"
#include "log.h"
#include "commonmacro.h"
//Nothing further to configure

struct LogSettings{
	uint8_t outputEnabled;
};

struct LogSettings logSettings;

void log_init(){
	logSettings.outputEnabled = ON;
}

//user provided function to transmit a single byte onto the serial port
void uartSendByte(uint8_t byte){
	transmitByte(byte);
}

//user provided function to get a time stamp or time offset
uint32_t getTimestamp (){
	rtc_printTime();
	return 0;
}

static const char *levelStrings[] = {
	"TRACE", "DEBUG", "INFO ", "WARN ", "ERROR", "FATAL"
};

#ifdef LOG_USE_COLOR
static const char *levelColours[] = {
	"\x1b[94m", "\x1b[36m", "\x1b[32m", "\x1b[33m", "\x1b[31m", "\x1b[35m"
};
#endif

void log_log(int level, const char *file, int line, const char *fmt, ...){
	if (level >= LOG_LEVEL && logSettings.outputEnabled == ON){
		//print time stamp
		#ifdef LOG_TIME
		getTimestamp();
		printString(" ");
		#endif
		
		//print log level
		#ifdef LOG_USE_COLOR
		printString(levelColours[level]);
		#endif
		printString(levelStrings[level]);
		printString(" ");
		
		//print file name
		#ifdef LOG_USE_COLOR
		printString("\x1b[0m\x1B[2m");
		#endif
		printString(stripPath(file));
		
		//print line number
		printString(":");
		printInteger(line);
		printString(" ");
		
		//deal with variable arguments here
		va_list valist;
		va_start(valist, fmt);
		printString("\x1b[0m");
		uint16_t i = 0;
		volatile uint32_t value;
		char *character;
		while (fmt[i]) {
			if (fmt[i] == '%'){
				i++;
				switch (fmt[i]){
					case 'd':{
						value = va_arg(valist, uint32_t);
						printInteger(value);
						break;
					}
					case 'l':{
						value = va_arg(valist, uint32_t);
						printInteger(value);
						break;
					}
					case 'i':{
						value = va_arg(valist, uint32_t);
						printBinaryByte((uint8_t)value);
						break;
					}
					case 'h':{
						value = va_arg(valist, uint32_t);
						printHexByte((uint8_t)value);
						break;
					}	
					case 's':{						
						character = va_arg(valist, char *);
						printString(character);
						//while (character){							
						//	transmitByte(character);
						//	character++;
						//}
						break;
					}
					case '%':{
						printString("%");
						break;
					}
				}
			}
			else{
				uartSendByte(fmt[i]);
			}
			
			i++;
		}
		va_end(valist);
		//end the line
		printString("\r\n");
	}
}

//toggle log output on and off
void log_toggle_active(){
	switch (logSettings.outputEnabled){
		case (ON):
		logSettings.outputEnabled = OFF;
		break;
		case (OFF):
		logSettings.outputEnabled = ON;		
		break;
	}
}

uint8_t log_is_active(){
	return logSettings.outputEnabled;
}

void printInteger(uint32_t integer){
	//get the number of digits
	uint8_t size = getNumDigits (integer);
	
	//print the digits starting MSB first
	for (int8_t i = size; i>0; i--){
		uartSendByte('0'+(integer/tenPower(i-1)) % 10);
	}
}

void printString(const char myString[]){
	uint8_t i = 0;
	while (myString[i]) {
		uartSendByte(myString[i]);
		i++;
	}
}

void printBinaryByte(uint8_t byte) {
	uint8_t bit;
	for (bit = 7; bit < 255; bit--) {
		if (bit_is_set(byte, bit))
		uartSendByte('1');
		else
		uartSendByte('0');
	}
}

void printHexByte (uint8_t byte){
	uint8_t nibble;
	nibble = (byte & 0b11110000) >> 4;
	uartSendByte(nibbleToHexCharacter(nibble));
	nibble = byte & 0b00001111;
	uartSendByte(nibbleToHexCharacter(nibble));
}


char nibbleToHexCharacter(uint8_t nibble) {
	if (nibble < 10) {
		return ('0' + nibble);
	}
	else {
		return ('A' + nibble - 10);
	}
}

uint32_t tenPower (uint8_t i){
	uint32_t result = 1;
	while (i){
		result *= 10;
		i--;
	}
	return result;
}

uint8_t getNumDigits (uint32_t number){
	if (number==0){
		return 1;
	}
	uint8_t size=0;
	for (uint8_t i=0; i<10; i++){
		if (number >= tenPower(i)){
			size++;
		}
	}
	return size;
}

const char *stripPath(const char *filename){
	uint8_t position = 0;
	uint8_t i = 0;
	while (filename[i]) {
		if (filename[i] == '/'){
			position = i;
		}
		i++;
	}
	return &filename[position+1];
}

uint8_t bit_is_set(uint32_t byte, uint8_t bit){
	return (byte >> bit) & 0b1;
}