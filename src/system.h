
typedef enum
{
	GENERAL,
	BACKUP,
	WATCHDOG,
	SOFTWARE,
	USER,
} resetSource_t;

resetSource_t getResetSource();
void system_reset();

void system_resetCPU();
void system_resetPeripheral();
void system_externalReset();