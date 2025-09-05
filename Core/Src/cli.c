#include "cli.h"
#include "usart.h"

static bool cmd_help(cli_data_t *cli_data);
static bool cmd_echo(cli_data_t *cli_data);
static bool cmd_clean(cli_data_t *cli_data);

cli_command_t cli_cmd[CMD_IDX_MAX] = 
{
    /* help */
    [CMD_HELP].help = \
    "help   : show command list\r\n",
    [CMD_HELP].name = "help",
    [CMD_HELP].func = cmd_help,
    [CMD_HELP].opt = "",
    [CMD_HELP].opt_size = 0,

    /* echo */
    [CMD_ECHO].help = \
    "echo   : printing message to terminal\r\n" \
    "<use>  : echo [argument to print]\r\n",
    [CMD_ECHO].name = "echo",
    [CMD_ECHO].func = cmd_echo,
    [CMD_ECHO].opt = "",
    [CMD_ECHO].opt_size = 0,

    /* clean */
    [CMD_CLEAN].help = \
    "clean  : clean the screen",
    [CMD_CLEAN].name = "clean",
    [CMD_CLEAN].func = cmd_clean,
    [CMD_CLEAN].opt = "",
    [CMD_CLEAN].opt_size = 0,
};

static int cli_get_opt(cli_data_t *cli_data, char opt, char *opt_str)
{
    char opt_want[] = {'-', opt, ' '};
    char *str = strstr(cli_data->cmd_str, opt_want);
    int idx = 0;
    int len = 0;
    int opt_len = strlen(opt_want);

    if (str == NULL)
    {
        return -1;
    }

    len = strlen(str);
    for (idx = 0; idx < len; idx++)
    {
        opt_str[idx] = str[opt_len + idx];
        if (opt_str[idx] == '\0')
        {
            break;
        }
        else if(opt_str[idx] == ' ')
        {
            opt_str[idx] = 0;
            break;
        }
    }
    return 1;
}

static int cli_get_arg(cli_data_t *cli_data, int arg_num, char *arg)
{
    char *str = strchr(cli_data->cmd_str, ' ');
    int idx = 0;
    
    for (idx = 0; idx < arg_num; idx++)
    {
        str = strchr(&str[1], ' ');
        if (str == NULL)
        {
            return -1;
        }
    }
    
    str = &str[1];
    int len = strlen(str);
    
    for (idx = 0; idx < len; idx++)
    {
        arg[idx] = str[idx];
        if (arg[idx] == '\0')
        {
            break;
        }
        else if(arg[idx] == ' ')
        {
            arg[idx] = 0;
            break;
        }
    }
    return 1;
}

static bool cmd_echo(cli_data_t *cli_data)
{
    int idx = 0;
    char arg[CMD_MAX_LEN] = {0, };

    while(1)
    {
        if (cli_get_arg(cli_data, idx, arg) > 0)
        {
            prints("%s ", arg);
            memset(arg, 0, sizeof(arg));
        }
        else
        {
            break;
        }
        idx++;
    }
    prints("\r\n");

    return true;
}

static bool cmd_clean(cli_data_t *cli_data)
{
    prints("\x1B[2J");
    return true;
}

static bool cmd_help(cli_data_t *cli_data)
{
    int idx = 0;
    
    prints("===============[stm32 help cmd list]===============\r\n");
    for (idx = 0; idx < CMD_IDX_MAX; idx++)
    {
        prints("%s\r\n", cli_cmd[idx].help);
    }
    prints("===================================================\r\n");

    return true;
}

static int cli_parser(char *rx)
{
    int idx = 0;
    int exe_cmd = -1;
    char pt_buf[CMD_MAX_LEN] = {0, }; 
    cli_data_t cli_data = {0, };

    for (idx = 0; idx < CMD_IDX_MAX; idx++)
    {
        if (strncmp(cli_cmd[idx].name, rx, strlen(cli_cmd[idx].name)) == 0)
        {
            cli_data.cmd_str = rx;
            cli_data.out_str = pt_buf;
            cli_data.out_str_size = sizeof(pt_buf);
            cli_data.opt_size = cli_cmd[idx].opt_size;
            strcpy(cli_data.opt, cli_cmd[idx].opt);
            exe_cmd = cli_cmd[idx].func(&cli_data);
            break;
        }
    }

    if (exe_cmd == true)
    {
        prints("%s", cli_data.out_str);
        return 1;
    }
    else if (exe_cmd == -1)
    {
        printr("unkwon command", cli_cmd[idx].name);
        return -1;
    }
    else /* false */
    {
        printr("[%s]execution failed : try chat help to use command", cli_cmd[idx].name);
        return -1;
    }
}

CLI_STATUS cli_work(char *rx)
{
    static char cli_rx[CMD_MAX_LEN] = {0, };
    static int cli_rx_cnt = 0;
    int len = strlen(rx);
    int idx = 0;
    
    for (idx = 0; idx < len; idx++)
    {
        if (rx[idx] == '\n' || rx[idx] == '\r')
        {
            prints("\r\n");
            cli_parser(cli_rx);
            cli_rx_cnt = 0;
            memset(cli_rx, 0, sizeof(cli_rx));
            return CLI_EXEC_CMD;
        }
        else if (rx[idx] == '\b')
        {
            if (cli_rx_cnt > 0)
            {
                cli_rx[cli_rx_cnt - 1] = '0';
                cli_rx_cnt--;
            }
        }
        else
        {
            if (cli_rx_cnt < CMD_MAX_LEN)
            {
                cli_rx[cli_rx_cnt++] = rx[idx];
            }
            else
            {
                printr("too many string");
                return CLI_ERR;
            }
        }
    }
    return CLI_INPUT;
}

void cli_proc()
{
    uint8_t rx_buf[UART_TRX_SIZE] = {0, };

    if (uart_read(&uart[UART1_IDX], rx_buf, sizeof(rx_buf)) > 0)
    {
        switch (cli_work((char *)rx_buf))
        {
        case CLI_INPUT:
            uart_send(&uart[UART1_IDX], rx_buf, strlen((char *)rx_buf));
            break;

        case CLI_EXEC_CMD:
            uart_send(&uart[UART1_IDX], rx_buf, strlen((char *)rx_buf));
            prints("~# ");
            break;

        case CLI_ERR:
        default:
            break;
        }
    }
}