#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "sam.h"
//#include "string.h"
#include "cmd.h"
#include "uart.h"
#include "console.h"
#include "log.h"
#include "commonmacro.h"

#define PROMPT "> "

#define CONSOLE_CMD_BFR_SIZE 80
#define DATA_PRINT_BYTES_PER_LINE 32

struct ConsoleState {
	//struct console_cfg cfg;
	char cmdBuffer[CONSOLE_CMD_BFR_SIZE]; //used
	uint32_t data_print_bytes_left;
	uint32_t data_print_offset;
	uint8_t* data_print_ptr;
	uint16_t num_cmd_bfr_chars;
	uint8_t first_run_done;				//used
};

static struct ConsoleState consoleState;

int32_t console_init(){
	//if (cfg == NULL) {
	//	return MOD_ERR_ARG;
	//}
	log_trace("Console Init()");
	memset(&consoleState, 0, sizeof(consoleState));
	//state.cfg = *cfg;
	return 0;
}

int32_t console_run(){
	int32_t rc = 0;
	uint8_t c;

	if (!consoleState.first_run_done && !log_is_active()) {
		consoleState.first_run_done = TRUE;
		printc("%s", PROMPT);
	}
	
	//if characters to send && uart_txIdle
	//transmit characters printc()
	
	while (uart_getCharacter(&c)) {
		// Handle processing completed command line.
		if (c == '\n' || c == '\r') {
			consoleState.cmdBuffer [consoleState.num_cmd_bfr_chars] = '\0';
			printc("\n");
			cmd_execute(consoleState.cmdBuffer);
			//printc("command: %s\r\n", consoleState.cmdBuffer); //debug
			consoleState.num_cmd_bfr_chars = 0;
			if (consoleState.data_print_bytes_left == 0)
				printc("%s", PROMPT);
			continue;
		}

		// Handle backspace/delete.
		if (c == '\b' || c == '\x7f') {
			if (consoleState.num_cmd_bfr_chars > 0) {
				// Overwrite last character with a blank.
				printc("\b \b");
				consoleState.num_cmd_bfr_chars--;
			}
			continue;
		}

		// Handle logging on/off toggle.
		if (c == LOG_TOGGLE_CHAR) {
			log_toggle_active();
			printc("\n<Logging %s>\n", log_is_active() ? "on" : "off");
			if (log_is_active() == TRUE){
				consoleState.first_run_done = FALSE;
			}
			continue;
		}

		// Echo the character back.
		if (isprint(c)) {
			if (consoleState.num_cmd_bfr_chars < (CONSOLE_CMD_BFR_SIZE-1)) {
				consoleState.cmdBuffer[consoleState.num_cmd_bfr_chars++] = c;
				printc("%c", c);
				} 
			else {
				// No space in buffer for the character, so ring the bell.
				printc("\a");
			}
			continue;
		}
		
	}
	
	return rc;
}


int32_t printc(const char* fmt, ...){
	va_list args;
	char buf[CONFIG_CONSOLE_PRINT_BUF_SIZE];
	int rc;
	int idx;

	va_start(args, fmt);
	rc = vsnprintf(buf, CONFIG_CONSOLE_PRINT_BUF_SIZE, fmt, args);
	va_end(args);
	for (idx = 0; idx < rc; idx++) {
		transmitByte ( buf[idx]);
		
		if (buf[idx] == '\0')
		break;
		if (buf[idx] == '\n')
		transmitByte ('\r');
	}
	if (rc >= CONFIG_CONSOLE_PRINT_BUF_SIZE)
	printc("[!]\n");
	return rc;
}