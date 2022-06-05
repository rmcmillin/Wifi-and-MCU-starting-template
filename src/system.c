#include "sam.h"

#include "system.h"
#include "log.h"

#define REGISTER_KEY	0xA5




static const char *resetSourceStrings[] = {
	"GENERAL", "BACKUP", "WATCHDOG ", "SOFTWARE ", "USER"
};

resetSource_t getResetSource(){
	resetSource_t rc;
	rc = (REG_RSTC_SR & RSTC_SR_RSTTYP_Msk) >> RSTC_SR_RSTTYP_Pos;
	log_warn("Reset Source: %s", resetSourceStrings[rc]);
	return rc;
}

void system_resetCPU(){
	REG_RSTC_CR = RSTC_CR_KEY(REGISTER_KEY) & RSTC_CR_PROCRST;
}

void system_resetPeripheral(){
	REG_RSTC_CR = RSTC_CR_KEY(REGISTER_KEY) & RSTC_CR_PERRST;
}

void system_externalReset(){
	REG_RSTC_CR = RSTC_CR_KEY(REGISTER_KEY) & RSTC_CR_EXTRST;
}