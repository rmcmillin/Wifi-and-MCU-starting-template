
#define RTC_CLOCK_CORRECTION_PPM 9 //based on selected external crystal

//Init Settings
#define RTC_24H_CLOCK	0
#define RTC_12H_CLOCK	1
#define RTC_GREGORIAN_CALENDAR	0
#define RTC_PERSIAN_CALENDAR	1
#define RTC_POSITIVE_PPM_CORRECTION	0
#define RTC_NEGATIVE_PPM_CORRECTION 1
#define RTC_LOW_RANGE_PPM_CORRECTION 0
#define RTC_HIGH_RANGE_PPM_CORRECTION 1

//Time Register Masks
#define RTC_SECONDS_msk	0x0000007f
#define RTC_SECONDS_bp	0
#define RTC_MINUTES_msk	0x00007f00
#define RTC_MINUTES_bp	8
#define RTC_HOUR_msk	0x007f0000
#define RTC_HOUR_bp		16

//Calendar Register Masks
#define RTC_DAY_msk			0x3f000000
#define RTC_DAY_bp			24
#define RTC_DAY_OF_WEEK_msk	0x00e00000
#define RTC_DAY_OF_WEEK_bp	21
#define RTC_MONTH_msk		0x001f0000
#define RTC_MONTH_bp		16
#define RTC_YEAR_msk		0x0000ff00
#define RTC_YEAR_bp			8
#define RTC_CENTURY_msk		0x0000007f
#define RTC_CENTURY_bp		0
#define RTC_AMPM_msk		0b01000000
#define RTC_AMPM_bp			7

struct RTC_DateTime{
	uint8_t seconds;
	uint8_t minutes;
	uint8_t hours;
	uint8_t am_pm; //!< 0: am, 1: pm
	uint8_t dayOfWeek;
	uint8_t day;
	uint8_t month;
	uint16_t year;
};

typedef enum
{
	RTC_ERROR_YEAR,
	RTC_ERROR_MONTH,
	RTC_ERROR_DAY,
	RTC_ERROR_DAY_OF_WEEK,
	RTC_ERROR_HOUR,
	RTC_ERROR_MINUTE,
	RTC_ERROR_SECOND,
	RTC_ERROR_CONFIG	
} rtc_error_t;

void rtc_init (uint8_t clockMode, uint8_t calendarMode, uint8_t negativePPMCorrection, uint8_t correction, uint8_t ppmCorrectionRange);
uint32_t rtc_start();
void rtc_stop();
uint32_t rtc_getStatus();
rtc_error_t rtc_configureNewTime (struct RTC_DateTime *dateTime, uint8_t day, uint8_t month, uint16_t year, uint8_t hour, uint8_t minute, uint8_t second,uint8_t dayOfWeek);
rtc_error_t rtc_setTime();
void rtc_getTime();
void rtc_printTime();

//helper functions
uint8_t bcdToDecimal(uint8_t bcdByte);
uint8_t decimalToBCD(uint8_t decByte);