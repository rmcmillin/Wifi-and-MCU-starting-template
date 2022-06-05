#include "sam.h"
#include <string.h>
#include <ctype.h>

#include "console.h"
#include "cmd.h"
#include "commonmacro.h"

#define MAX_TOKENS 10
#define MAX_CLIENTS 5

static const struct ClientInfo *clientInfo[MAX_CLIENTS];

int32_t cmd_register(const struct ClientInfo *newClient){
	//find next free client in array of clients	and assign to library calling this
	for (uint8_t i = 0; i < MAX_CLIENTS; i++){
		if (clientInfo[i] == 0 || strcasecmp(clientInfo[i]->name, newClient->name) == 0) {
			clientInfo[i] = newClient;
			return 0;
		}
	}
	return ERR_NO_RESOURCE;
}

int32_t cmd_execute(char* buffer){
	int32_t num_tokens = 0;
	const char* tokens[MAX_TOKENS];
	char* p = buffer;
	const struct ClientInfo *ci;
	const struct CommandInfo *cmdi;

	// Tokenize the command line in-place.
	while (1) {
		//ignore white space at beginning of command (' ', \n, \t, \v, \f, \r)
		while (*p && isspace((unsigned char)*p)){
			p++;
		}
		
		if (*p == '\0') {//end of command
			break;
		}
		else {
			if (num_tokens >= MAX_TOKENS) {
				return ERR_BAD_COMMAND;
			}
			//find end of command
			tokens[num_tokens++] = p;
			while (*p && !isspace((unsigned char)*p)){
				p++;
			}
			
			if (*p) {
				//Terminate token.
				*p++ = '\0';
				} 
			else {
				//Found end of line.
				break;
			}
		}
	}

	// If there are no tokens, nothing to do.
	if (num_tokens == 0)
	return 0;	

	// Handle top-level help.
	if (strcasecmp("help", tokens[0]) == 0 ||
	strcasecmp("?", tokens[0]) == 0) {
		for (uint8_t i = 0; i < MAX_CLIENTS && clientInfo[i] != 0; i++) {
			ci = clientInfo[i];
			if (ci->num_cmds == 0)
			continue;
			printc("%s (", ci->name);
			for (uint8_t j = 0; j < ci->num_cmds; j++) {
				cmdi = &ci->cmds[j];
				printc("%s%s", j == 0 ? "" : ", ", cmdi->name);
			}

			printc(")\n");
		}
		//printc("\nLog levels are: %s\n", LOG_LEVEL_NAMES);
		return 0;
	}

	// Find and execute the command.
	for (uint8_t i = 0; i < MAX_CLIENTS && clientInfo[i] != 0;	i++) {
		ci = clientInfo[i];
		if (strcasecmp(tokens[0], ci->name) != 0)
		continue; //skip and go to next client

		// If there is no command, treat it as help.
		if (num_tokens == 1)
		tokens[1] = "?";

		// Handle help command directly.
		if (strcasecmp(tokens[1], "help") == 0 ||
		strcasecmp(tokens[1], "?") == 0) {
			//log_debug("Handle client help\n");
			for (uint8_t j = 0; j < ci->num_cmds; j++) {
				cmdi = &ci->cmds[j];
				printc("%s %s: %s\n", ci->name, cmdi->name, cmdi->help);
			}			
			return 0;
		}			

		// Find the command
		for (uint8_t i = 0; i < ci->num_cmds; i++) {
			if (strcasecmp(tokens[1], ci->cmds[i].name) == 0) {
				//log_debug("Handle command\n");
				ci->cmds[i].func(num_tokens, tokens);
				return 0;
			}
		}
		printc("No such command (%s %s)\n", tokens[0], tokens[1]);
		return ERR_BAD_COMMAND;
	}
	printc("No such command (%s)\n", tokens[0]);
	return ERR_BAD_COMMAND;
}

/*
int32_t cmd_parse_args(int32_t argc, const char** argv, const char* fmt, struct cmd_arg_val* arg_vals)
{
	int32_t arg_cnt = 0;
	char* endptr;
	bool opt_args = false;
	bool allow_extra_args = false;

	while (*fmt) {
		if (*fmt == '[') {
			opt_args = true;
			fmt++;
			continue;
		}
		if (*fmt == ']') {
			fmt++;
			continue;
		}
		if (*fmt == '+') {
			allow_extra_args = true;
			fmt++;
			continue;
		}
		if (arg_cnt >= argc) {
			if (opt_args) {
				return arg_cnt;
			}
			printc("Insufficient arguments\n");
			return MOD_ERR_BAD_CMD;
		}

		// These error conditions should not occur, but we check them for
		// safety.
		if (*argv == NULL || **argv == '\0') {
			printc("Invalid empty arguments\n");
			return MOD_ERR_BAD_CMD;
		}

		switch (*fmt) {
			case 'i':
			arg_vals->val.i = strtol(*argv, &endptr, 0);
			if (*endptr) {
				printc("Argument '%s' not a valid integer\n", *argv);
				return MOD_ERR_ARG;
			}
			break;
			case 'u':
			arg_vals->val.u = strtoul(*argv, &endptr, 0);
			if (*endptr) {
				printc("Argument '%s' not a valid unsigned integer\n", *argv);
				return MOD_ERR_ARG;
			}
			break;
			case 'p':
			arg_vals->val.p = (void*)strtoul(*argv, &endptr, 16);
			if (*endptr) {
				printc("Argument '%s' not a valid pointer\n", *argv);
				return MOD_ERR_ARG;
			}
			break;
			case 's':
			arg_vals->val.s = *argv;
			break;
			case 'f': {
				const char* p = *argv;
				float f = 0.0F;
				float factor = 1.0F;
				bool point_seen = false;

				if (*p == '-') {
					p++;
					factor = -1.0F;
				};
				for (; *p; p++) {
					if (*p == '.') {
						point_seen = true;
						continue;
					}
					if (!isdigit((int)*p)) {
						printc("Argument '%s' not a valid float\n", *argv);
						return MOD_ERR_ARG;
					}
					if (point_seen)
					factor /= 10.0F;
					f = f * 10.0f + (float)(*p - '0');
				}
				arg_vals->val.f = f * factor;
				break;
			}
			default:
			printc("Bad argument format '%c'\n", *fmt);
			return MOD_ERR_ARG;
		}
		arg_vals->type = *fmt;
		arg_vals++;
		arg_cnt++;
		argv++;
		fmt++;
		opt_args = false;
	}
	if (arg_cnt < argc && (!allow_extra_args)) {
		printc("Too many arguments\n");
		return MOD_ERR_BAD_CMD;
	}
	return arg_cnt;
}

*/