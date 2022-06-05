#include <stdio.h>

#include "sam.h"
#include "rtc.h"
#include "log.h"

struct RTC_Settings{
	uint8_t clockMode; //!< 0: 24 hour, 1: AM/PM
	uint8_t calendarMode; //!< 0:Gregorian, 1: Persian
	uint8_t negativePPMCorrection; //!< 0: Positive Correction, 1: Negative Correction
	uint8_t correction; //!< 0=nocorrecction, 1-127ppm : slow clock correction uses formula based on ppmCorrection value
	uint8_t ppmCorrectionRange;	///< 0: low range ppm correction=, 1: high range ppm correction
	///< if absolute value of correction is less than 30ppm use HIGH_PPM
	///< lowppm correction = (3906/(20*ppm)) - 1
	///< highppm correction = 3906/(ppm) - 1
}rtc_settings;



void rtc_init (uint8_t clockMode, uint8_t calendarMode, uint8_t negativePPMCorrection, uint8_t correction, uint8_t ppmCorrectionRange){
	rtc_settings.clockMode = clockMode;
	rtc_settings.calendarMode = calendarMode;
	rtc_settings.negativePPMCorrection = negativePPMCorrection;
	rtc_settings.correction = correction;
	rtc_settings.ppmCorrectionRange = ppmCorrectionRange;
}

uint32_t rtc_start(){
	uint32_t registerValue = 0;
	int32_t rc = 0;
	
	if (rtc_settings.clockMode == RTC_12H_CLOCK){
		registerValue |= RTC_MR_HRMOD;
	}
	
	if (rtc_settings.calendarMode == RTC_PERSIAN_CALENDAR){
		registerValue |= RTC_MR_PERSIAN;
	}
	
	if (rtc_settings.negativePPMCorrection == RTC_NEGATIVE_PPM_CORRECTION){
		registerValue |= RTC_MR_NEGPPM;
	}
	
	if (rtc_settings.correction == 0){
		//no correction, default 0
	}
	else if (rtc_settings.correction > 0 && rtc_settings.correction <= 127){
		//correction
		registerValue |= RTC_MR_CORRECTION(rtc_settings.correction);
	}
	else{
		//bad value
		rc = -1;
	}
	
	if (rtc_settings.ppmCorrectionRange == RTC_HIGH_RANGE_PPM_CORRECTION){
		registerValue |= RTC_MR_HIGHPPM;
	}
	
	if (rc == 0){
		REG_RTC_MR = registerValue;
	}
	
	return rc;
}

//void rtc_stop(){}

//sets date/time struct with values and checks range. returns error on bad ranges. send struct to rtc_setTime to update registers
rtc_error_t rtc_configureNewTime (struct RTC_DateTime *dateTime, uint8_t day, uint8_t month, uint16_t year, uint8_t hour, uint8_t minute, uint8_t second, uint8_t dayOfWeek){
	int8_t rc = 0;
	
	//check year range
	switch (rtc_settings.calendarMode){
		case (RTC_GREGORIAN_CALENDAR):
		if (year < 1900 || year > 2099 ){
			rc = RTC_ERROR_YEAR;
		}
		break;
		case (RTC_PERSIAN_CALENDAR):
		if (year < 1300 || year > 1499 ){
			rc = RTC_ERROR_YEAR;
		}
		break;
		default:
		rc = RTC_ERROR_CONFIG;
	}
	
	//check month and days
	if (month == 1 || month == 3 || month == 5 || month == 7 || month == 8 || month == 10 || month == 12){
		if (day > 31){
			rc = RTC_ERROR_DAY;
		}
	}
	else if (month == 4 || month == 6 || month == 9 || month == 11){
		if (day > 30){
			rc = RTC_ERROR_DAY;
		}
	}
	else if (month == 2){
		if (day > 29){ //no leap year check
			rc = RTC_ERROR_DAY;
		}
	}
	else{
		rc = RTC_ERROR_MONTH; //bad month
	}
	
	//check seconds
	if (second > 59){
		rc = RTC_ERROR_SECOND;
	}
	//check minutes
	if (minute > 59){
		rc = RTC_ERROR_MINUTE;
	}
	//check hours
	switch (rtc_settings.clockMode){
		case (RTC_24H_CLOCK):
		if (hour > 23){
			rc = RTC_ERROR_HOUR;
		}
		break;
		case (RTC_12H_CLOCK):
		if (hour > 12){
			rc = RTC_ERROR_HOUR;
		}
		break;
		default:
		rc = RTC_ERROR_CONFIG; //settings not configured
	}
	
	//set struct values and return success
	dateTime->seconds = second;
	dateTime->minutes = minute;
	dateTime->hours = hour;
	dateTime->day = day;
	dateTime->month = month;
	dateTime->year = year;
	dateTime->dayOfWeek = dayOfWeek;
	
	return rc;
}

rtc_error_t rtc_setTime(struct RTC_DateTime *dateTime){
	uint8_t rc = 0;
	
	//prepare register contents	
	uint32_t timeRegisterValue = 0;
	uint32_t calendarRegisterValue = 0;
	
	timeRegisterValue |= decimalToBCD(dateTime->seconds) << RTC_SECONDS_bp;
	timeRegisterValue |= decimalToBCD(dateTime->minutes) << RTC_MINUTES_bp;
	timeRegisterValue |= decimalToBCD(dateTime->hours) << RTC_HOUR_bp; //needs to be updated for am/pm
	calendarRegisterValue |= decimalToBCD(dateTime->day) << RTC_DAY_bp;
	calendarRegisterValue |= decimalToBCD(dateTime->month) << RTC_MONTH_bp;
	calendarRegisterValue |= decimalToBCD(dateTime->year % 100) << RTC_YEAR_bp;
	calendarRegisterValue |= (decimalToBCD(dateTime->year - (dateTime->year % 100)))/100 << RTC_CENTURY_bp;
	calendarRegisterValue |= (decimalToBCD(dateTime->dayOfWeek) << RTC_DAY_OF_WEEK_bp);
	log_trace("Calendar Register: %d", calendarRegisterValue);
	
	//stop rtc date/time while updating
	REG_RTC_CR |= RTC_CR_UPDTIM | RTC_CR_UPDCAL;
	
	//wait for rtc acknowledge (max 1 second wait)
	while (!(REG_RTC_SR & RTC_SR_ACKUPD)); //needs timeout
	
	//clear acknowledge bit
	REG_RTC_SCCR |= RTC_SCCR_ACKCLR;
	
	//update time
	REG_RTC_TIMR = timeRegisterValue;
	REG_RTC_CALR = calendarRegisterValue;
	
	//restart rtc
	REG_RTC_CR &= ~(RTC_CR_UPDTIM | RTC_CR_UPDCAL);
	
	//check validity
	if (REG_RTC_SR & RTC_SR_TDERR){
		rc = RTC_ERROR_CONFIG;
	}
	return rc;
}

//Read time and date registers and store in struct
void rtc_getTime(struct RTC_DateTime *dateTime){
	uint32_t timeRegister, timeRegister2;
	uint32_t dateRegister, dateRegister2;
	
	//read registers 1-3 times to avoid roll over errors
	do{
		timeRegister = REG_RTC_TIMR;
		timeRegister2 = REG_RTC_TIMR;
	}while (!(timeRegister==timeRegister2));
	
	do{
		dateRegister = REG_RTC_CALR;
		dateRegister2 = REG_RTC_CALR;
	}while (!(dateRegister==dateRegister2));
	
	dateTime->seconds = bcdToDecimal((timeRegister & RTC_SECONDS_msk) >> RTC_SECONDS_bp);
	dateTime->minutes = bcdToDecimal((timeRegister & RTC_MINUTES_msk) >> RTC_MINUTES_bp);
	if (rtc_settings.clockMode == RTC_24H_CLOCK){
		dateTime->hours = bcdToDecimal((timeRegister & RTC_HOUR_msk) >> RTC_HOUR_bp);
	}
	else{
		dateTime->hours = (timeRegister & RTC_HOUR_msk) >> RTC_HOUR_bp;
		dateTime->am_pm = (dateTime->hours & RTC_AMPM_msk) >> RTC_AMPM_bp;
		dateTime->hours = bcdToDecimal (dateTime->hours & ~RTC_AMPM_msk);
	}
	
	dateTime->day = bcdToDecimal((dateRegister & RTC_DAY_msk) >> RTC_DAY_bp);
	dateTime->dayOfWeek = bcdToDecimal((dateRegister & RTC_DAY_OF_WEEK_msk) >> RTC_DAY_OF_WEEK_bp);
	dateTime->month = bcdToDecimal((dateRegister & RTC_MONTH_msk) >> RTC_MONTH_bp);
	dateTime->year = bcdToDecimal((dateRegister & RTC_CENTURY_msk) >> RTC_CENTURY_bp);
	dateTime->year *= 100;
	dateTime->year += bcdToDecimal((dateRegister & RTC_YEAR_msk) >> RTC_YEAR_bp);
}

//prints out date with leading zeros
void rtc_printTime(){
	struct RTC_DateTime dateTime;
	rtc_getTime(&dateTime);
	
	char timeString [22];
	
	//form string
	sprintf(timeString,"%02d/%02d/%d - %02d:%02d:%02d", dateTime.day, dateTime.month, dateTime.year, dateTime.hours, dateTime.minutes, dateTime.seconds);
	
	//print to UART
	printString(timeString);
}


//Helper Functions
/*Converts BCD to Decimal*/
uint8_t bcdToDecimal(uint8_t bcdByte){
	return ((((bcdByte & 0xF0) >> 4) * 10) + (bcdByte & 0x0F));
}

/*Converts Decimal to BCD*/
uint8_t decimalToBCD(uint8_t decByte){
	return (((decByte/10) <<4) | (decByte%10));
}