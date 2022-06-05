
typedef int32_t (*cmd_func)(int32_t argc, const char** argv);

struct CommandInfo {
	const char *const name; // Name of command
	const cmd_func func;    // Command function
	const char *const help; // Command help string
};

struct ClientInfo {
	const char *const name;          // Client name (first command line token)
	const int32_t num_cmds;          // Number of commands.
	const struct CommandInfo *const cmds; // Pointer to array of command info
	//int32_t* const log_level_ptr;    // Pointer to log level variable (or NULL)
	//const int32_t num_u16_pms;       // Number of pm values.
	//uint16_t* const u16_pms;         // Pointer to array of pm values
	//const char* const* const u16_pm_names; // Pointer to array of pm names
};

int32_t cmd_register(const struct ClientInfo *newClient);

int32_t cmd_execute(char *buffer);

