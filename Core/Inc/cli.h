#ifndef __CLI_H__
#define __CLI_H__

#include "main.h"

#define CLI_OPTION_MAX (1 << 6)
#define CMD_MAX_LEN (1 << 9)
#define CLI_HISTORY_NUM (1 << 2)

typedef enum
{
    CMD_HELP,
    CMD_ECHO,
    CMD_CLEAR,
    CMD_REBOOT,
    CMD_DATE,
    CMD_IDX_MAX,
}CLI_COMMAND_IDX;

typedef enum
{
    EXEC_RESULT_NO_CMD = -2,
    EXEC_RESULT_ERR = -1,
    EXEC_HELP = 0,
    EXEC_RESULT_OK = 1,
    EXEC_NONE = 2,
}CLI_EXEC_RESULT;

typedef enum
{
    CLI_ERR = -1,
    CLI_INPUT = 0,
    CLI_EXEC_CMD = 1,
    CLI_ESCAPE_SEQ = 2,
}CLI_STATUS;

typedef enum
{
    CLI_ESC_MOVE_LEFT = 'D',
    CLI_ESC_MOVE_RIGHT = 'C',
    CLI_ESC_MOVE_UP = 'A',
    CLI_ESC_MOVE_DOWN = 'B',
    CLI_ESC_SPECIAL = '~'
}CLI_ESC_TYPE;

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
    char rx[CMD_MAX_LEN];
    char history[CLI_HISTORY_NUM][CMD_MAX_LEN];
    char esc_num;
    int history_cnt;
    int history_pos;
    int rx_cnt;
    int cur_pos;
}cli_work_t;

typedef struct
{
    char *name;
    char *help;
    char opt[CLI_OPTION_MAX];
    int opt_size;
    bool (*func)(cli_data_t *cli_data);
}cli_command_t;

CLI_STATUS cli_work(char *rx);

void cli_proc();
#endif