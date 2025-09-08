#include "cli.h"
#include "usart.h"

static bool cmd_help(cli_data_t *cli_data);
static bool cmd_echo(cli_data_t *cli_data);
static bool cmd_clean(cli_data_t *cli_data);
static bool cmd_reboot(cli_data_t *cli_data);

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
    "clean  : clean the screen\r\n",
    [CMD_CLEAN].name = "clean",
    [CMD_CLEAN].func = cmd_clean,
    [CMD_CLEAN].opt = "",
    [CMD_CLEAN].opt_size = 0,

    /* reboot */
    [CMD_REBOOT].help = \
    "reboot  : restart the application\r\n",
    [CMD_REBOOT].name = "reboot",
    [CMD_REBOOT].func = cmd_reboot,
    [CMD_REBOOT].opt = "",
    [CMD_REBOOT].opt_size = 0,
};

static int cli_get_opt(cli_data_t *cli_data, char opt, char *opt_str)
{
    char opt_want[] = {'-', opt};
    char *str = strstr(cli_data->cmd_str, opt_want);
    int idx = 0;
    int len = 0;
    int opt_len = strlen(opt_want);

    if (str == NULL)
    {
        return -1;
    }

    if (opt_str == NULL)
    {
        return 1;
    }

    if (str[opt_len] == ' ')
    {
        opt_len++;
    }
    else
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

static bool cmd_reboot(cli_data_t *cli_data)
{
    /* address that pointing Reset_Handler function address's address */
    __IO void (**rst_func)() = (__IO void(**)())0x08000004;
    __IO uint32_t *iap_add = (__IO uint32_t *)0x08000000;

    print_dmesg("Reboot : check iap is available");

    if ((*iap_add & 0x20000000) == 0x20000000)
    {
        print_dmesg("Reboot : iap is detected");
        HAL_RCC_DeInit();
        HAL_DeInit();
        HAL_ICACHE_DeInit();
        __set_MSP((__IO uint32_t)iap_add);
        __disable_irq();
        (*rst_func)();
        return true;
    }
    else
    {
        print_dmesg("Reboot : iap is not detected");
        return false;
    }
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
        prints("%s", cli_cmd[idx].help);
        if (idx != CMD_IDX_MAX - 1)
        {
            prints("\r\n");
        }
    }
    prints("===================================================\r\n");

    return true;
}

static int cli_parser(char *rx)
{
    int idx = 0;
    CLI_EXEC_RESULT exe_cmd = EXEC_RESULT_NO_CMD;
    char pt_buf[CMD_MAX_LEN] = {0, }; 
    cli_data_t cli_data = {0, };

    for (idx = 0; idx < CMD_IDX_MAX; idx++)
    {
        if (strncmp(cli_cmd[idx].name, rx, strlen(cli_cmd[idx].name)) == 0)
        {
            cli_data.cmd_str = rx;
            if (cli_get_opt(&cli_data, 'h', NULL) < 0)
            {
                cli_data.out_str = pt_buf;
                cli_data.out_str_size = sizeof(pt_buf);
                cli_data.opt_size = cli_cmd[idx].opt_size;
                strcpy(cli_data.opt, cli_cmd[idx].opt);
                exe_cmd = cli_cmd[idx].func(&cli_data);
            }
            else
            {
                prints("%s", cli_cmd[idx].help);
                exe_cmd = EXEC_HELP;
            }
            break;
        }
    }

    switch (exe_cmd)
    {
    case EXEC_RESULT_ERR:
        printr("[%s]execution failed : try chat help to use command", cli_cmd[idx].name);
        return -1;

    case EXEC_RESULT_NO_CMD:
        printr("unkwon command : %s", rx);
        return -1;

    case EXEC_RESULT_OK:
        prints("%s", cli_data.out_str);
        return 1;
    
    case EXEC_HELP:
        return 1;
    }

    return -1;
}

static int cli_esc_work(char esc_chr, cli_work_t *cli_work)
{
    switch (esc_chr)
    {
    case CLI_ESC_MOVE_LEFT:
        if (cli_work->cur_pos <= 0)
        {
            goto out;
        }
        (cli_work->cur_pos)--;
        goto esc_out;
        
    case CLI_ESC_MOVE_RIGHT:
        if (cli_work->cur_pos >= cli_work->rx_cnt)
        {
            goto out;
        }
        (cli_work->cur_pos)++;
        goto esc_out;

    case CLI_ESC_MOVE_UP:
        if (cli_work->history_pos < cli_work->history_cnt)
        {
            cli_work->history_pos++;
        }
        goto history_out;

    case CLI_ESC_MOVE_DOWN:
        if (cli_work->history_pos > 1)
        {
            cli_work->history_pos--;
        }
        else if (cli_work->history_pos == 0)
        {
            goto out;
        }
        goto history_out;

    case CLI_ESC_SPECIAL:
        switch (cli_work->esc_num)
        {
        case '1': /* home key */
            cli_work->cur_pos = 0;
            goto mov_pos_out;

        case '4': /* end key */
            cli_work->cur_pos = cli_work->rx_cnt;
            goto mov_pos_out;

        default:
            return -1;
        }
        break;

    default:
        return -1;
    }

esc_out:
    prints("\x1b[%c", esc_chr);
    goto out;

mov_pos_out:
    prints("\x1b[%dG", 4 + cli_work->cur_pos); /* "~# " + cur_pos */
    goto out;

history_out:
    strcpy(cli_work->rx, cli_work->history[cli_work->history_pos - 1]);
    cli_work->cur_pos = strlen(cli_work->rx);
    cli_work->rx_cnt = strlen(cli_work->rx);
    prints("\x1b[0G~# \x1b[K%s", cli_work->rx);
    goto out;

out:
    return 1;
}

static void cli_del_str(cli_work_t *cli_work)
{
    int idx = 0;

    if (cli_work->cur_pos == 0)
    {
        return;
    }

    for (idx = cli_work->cur_pos; idx < cli_work->rx_cnt; idx++)
    {
        cli_work->rx[idx - 1] = cli_work->rx[idx];
    }
    cli_work->rx[idx - 1] = 0;
    cli_work->cur_pos--;
    cli_work->rx_cnt--;
}

static void cli_put_str(char put_str, cli_work_t *cli_work)
{
    int idx = (cli_work->rx_cnt > CMD_MAX_LEN - 1) ? CMD_MAX_LEN - 1 : cli_work->rx_cnt;

    for (; idx > cli_work->cur_pos; idx--)
    {
        cli_work->rx[idx] = cli_work->rx[idx - 1];
    }
    cli_work->rx[idx] = put_str;
    cli_work->cur_pos++;
    cli_work->rx_cnt++;
}

static void cli_history_insert(cli_work_t *cli_work)
{
    int idx = (cli_work->history_cnt > CLI_HISTORY_NUM - 1) ? CLI_HISTORY_NUM - 1 : cli_work->history_cnt;

    for (; idx > 0 ; idx--)
    {
        strcpy(cli_work->history[idx], cli_work->history[idx - 1]);
    }

    strcpy(cli_work->history[0], cli_work->rx);

    if (cli_work->history_cnt < CLI_HISTORY_NUM)
    {
        cli_work->history_cnt++;
    }
    cli_work->history_pos = 0;
}

CLI_STATUS cli_work(char *rx)
{
    static cli_work_t cli_work = {0, };
    static int esc_cnt = 0;
    static bool esc_seq = 0;

    int len = strlen(rx);
    int idx = 0;
    int ret = 0;
    
    for (idx = 0; idx < len; idx++)
    {
        if (esc_seq)
        {
            esc_cnt++;
            switch (esc_cnt)
            {
            case 2:
                if (rx[idx] != '[')
                {
                    esc_cnt = 0;
                    esc_seq = false;
                }
                ret = CLI_ESCAPE_SEQ;
                break;

            case 3:
                if (cli_esc_work(rx[idx], &cli_work) > 0)
                {
                    esc_cnt = 0;
                    esc_seq = false;
                }
                else if ('0' <= rx[idx] || rx[idx] <= '9')
                {
                    cli_work.esc_num = rx[idx];
                }
                else
                {
                    esc_cnt = 0;
                    esc_seq = false;
                }
                ret = CLI_ESCAPE_SEQ;
                break;

            case 4:
                cli_esc_work(rx[idx], &cli_work);
                esc_cnt = 0;
                esc_seq = false;
                ret = CLI_ESCAPE_SEQ;
                break;
            }
        }
        else if (rx[idx] == '\n' || rx[idx] == '\r')
        {
            prints("\r\n");
            cli_parser(cli_work.rx);
            cli_history_insert(&cli_work);
            memset(cli_work.rx, 0, sizeof(cli_work.rx));
            cli_work.rx_cnt = 0;
            cli_work.cur_pos = 0;
            ret = CLI_EXEC_CMD;
        }
        else if (rx[idx] == '\b')
        {
            if (cli_work.rx_cnt > 0 && cli_work.cur_pos > 0)
            {
                cli_del_str(&cli_work);
                prints("\b\x1b[P");
            }
            ret = CLI_ESCAPE_SEQ;
        }
        else if (rx[idx] == '\x1b')
        {
            esc_cnt = 1;
            esc_seq = true;
            ret = CLI_ESCAPE_SEQ;
        }
        else
        {
            /* If string length is 512, It will be cause of overflow at cli_history_insert.
             * cli_history_insert use strcpy to copy command to history. If string length is
             * 512, strcpy try to copy 513 byte by including null character
             */
            if (cli_work.rx_cnt < CMD_MAX_LEN - 1)
            {
                cli_put_str(rx[idx], &cli_work);
                prints("\x1b[@%c", rx[idx]);
                ret = CLI_INPUT;
            }
            else
            {
                printr("too many string");
                ret = CLI_ERR;
            }
        }
    }
    return ret;
}

void cli_proc()
{
    uint8_t rx_buf[UART_TRX_SIZE] = {0, };

    if (uart_read(&uart[UART1_IDX], rx_buf, sizeof(rx_buf)) > 0)
    {
        switch (cli_work((char *)rx_buf))
        {

        case CLI_EXEC_CMD:
            uart_send(&uart[UART1_IDX], rx_buf, strlen((char *)rx_buf));
            prints("~# ");
            break;
        case CLI_INPUT:
        case CLI_ESCAPE_SEQ:
        case CLI_ERR:
        default:
            break;
        }
    }
}