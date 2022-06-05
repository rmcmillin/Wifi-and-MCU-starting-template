#define CONFIG_CONSOLE_PRINT_BUF_SIZE	240
//initialize console - run once
int32_t console_init();

//runs in superloop
//checks rx buffer, echos to tx buffer
//if detect /r or /n then send to cmd
int32_t console_run(void);

int32_t printc(const char* fmt, ...);