#ifndef __CLI_H__
#define __CLI_H__

#include "main.h"

#define CLI_OPTION_MAX 64

typedef enum
{
    CMD_HELP,
    CMD_ECHO,
    CMD_CLEAN,
    CMD_IDX_MAX,
}CLI_COMMAND_IDX;

typedef enum
{
    CLI_ERR = -1,
    CLI_INPUT = 0,
    CLI_EXEC_CMD = 1,
}CLI_STATUS;

typedef struct
{
    char *cmd_str;
    char *out_str;
    size_t out_str_size;
    char opt[CLI_OPTION_MAX];
    int opt_size;
}cli_data_t;

typedef struct
{
    char *name;
    char *help;
    char opt[CLI_OPTION_MAX];
    int opt_size;
    bool (*func)(cli_data_t *cli_data);
}cli_command_t;

CLI_STATUS cli_work(char *rx);
#define CMD_MAX_LEN 512

void cli_proc();
#endif