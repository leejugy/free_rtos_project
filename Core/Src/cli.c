#include "cli.h"
#include "rtc.h"
#include "eth.h"
#include "usart.h"
#include "status.h"

static CLI_EXEC_RESULT cmd_help(cli_data_t *cli_data);
static CLI_EXEC_RESULT cmd_echo(cli_data_t *cli_data);
static CLI_EXEC_RESULT cmd_clear(cli_data_t *cli_data);
static CLI_EXEC_RESULT cmd_reboot(cli_data_t *cli_data);
static CLI_EXEC_RESULT cmd_date(cli_data_t *cli_data);
static CLI_EXEC_RESULT cmd_ping(cli_data_t *cli_data);
static CLI_EXEC_RESULT cmd_dmesg(cli_data_t *cli_data);

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
    [CMD_CLEAR].help = \
    "clear  : clean the screen\r\n",
    [CMD_CLEAR].name = "clear",
    [CMD_CLEAR].func = cmd_clear,
    [CMD_CLEAR].opt = "",
    [CMD_CLEAR].opt_size = 0,

    /* reboot */
    [CMD_REBOOT].help = \
    "reboot  : restart the application\r\n",
    [CMD_REBOOT].name = "reboot",
    [CMD_REBOOT].func = cmd_reboot,
    [CMD_REBOOT].opt = "",
    [CMD_REBOOT].opt_size = 0,

    /* date */
    [CMD_DATE].help = \
    "date       : set or load date.\r\n" \
    "<load use> : date\r\n" \
    "<set use>  : date -s \"yyyy-mm-dd hh:mm:ss\"\r\n",
    [CMD_DATE].name = "date",
    [CMD_DATE].func = cmd_date,
    [CMD_DATE].opt = "s",
    [CMD_DATE].opt_size = 1,

    /* ping */
    [CMD_PING].help = \
    "ping       : send icmp packet to <ip> address\r\n" \
    "             -c option set count to send, default : 5, max : 10\r\n"\
    "<use>      : ping <ip> -c <count>\r\n",
    [CMD_PING].name = "ping",
    [CMD_PING].func = cmd_ping,
    [CMD_PING].opt = "c",
    [CMD_PING].opt_size = 1,

    /* dmesg */
    [CMD_DMESG].help = \
    "dmesg      : turn on or off debug message\r\n" \
    "<use>      : dmesg <on/off>\r\n" \
    "<toggle>   : dmesg\r\n",
    [CMD_DMESG].name = "dmesg",
    [CMD_DMESG].func = cmd_dmesg,
    [CMD_DMESG].opt = "",
    [CMD_DMESG].opt_size = 0,
};

/**
 * @brief Get command option. The command option is meaning -[character]
 * @note If you want to get argument you must set bit cli_arg->opt.get_ret
 * then you can get argument at cli_arg->arg
 * 
 * But if you don't want to get argument and only want to get result that
 * argument is exist, unset cli->opt.get_arg.
 * 
 * @param cli_data received command data
 * @param cli_arg has argument to be stored, and settngs to get argument
 * @return int 0 : argument exist (only retured at cli->opt.get_arg = 0)
 * 
 * int argument string length : when success to get argument
 * 
 * int -1 : fail to get argument
 */
static int cli_get_opt(cli_data_t *cli_data, cli_arg_t *cli_arg)
{
    char opt_want[] = {'-', cli_arg->cli_get.opt, '\0'};
    char *str = strstr(cli_data->cmd_str, opt_want);
    int idx = 0;
    int len = 0;
    int opt_len = strlen(opt_want);

    if (str == NULL)
    {
        return -1; /* no option detected */
    }

    if (!cli_arg->opt.get_ret)
    {
        return 0; /* if you don't want to use argument, use this. String must have option string like "-h" */
    }

    if (str[opt_len] == ' ')
    {
        opt_len++;
    }
    else
    {
        return -1; /* no argument input */
    }


    cli_arg->len = 0;

    str = str + opt_len;
    len = strlen(str);

    if((str[idx] == '\'' || str[idx] == '\"') && (cli_arg->quotes == false))
    {
        cli_arg->quotes = true;
        str++;
    }

    for (idx = 0; idx < len; idx++)
    {
        if (cli_arg->quotes)
        {
            if(str[idx] == '\'' || str[idx] == '\"')
            {
                break;
            }
            else if (str[idx] == '\0')
            {
                break;
            }
            else
            {
                cli_arg->arg[cli_arg->len++] = str[idx];
            }
        }
        else
        {
            if (str[idx] == '\0')
            {
                break;
            }
            else if(str[idx] == ' ')
            {
                break;
            }
            else
            {
                cli_arg->arg[cli_arg->len++] = str[idx];
            }
        }
    }
    cli_arg->arg[cli_arg->len] = 0;
    return cli_arg->len; /* option detect and store option argument to opt_str */
}

/**
 * @brief Get command arugment. The command arugment is next to the space character
 * @note If you want to get argument you must set bit cli_arg->opt.get_ret
 * then you can get argument at cli_arg->arg
 * 
 * But if you don't want to get argument and only want to get result that
 * argument is exist, unset cli->opt.get_arg.
 * 
 * @param cli_data received command data
 * @param cli_arg has argument to be stored, and settngs to get argument
 * @return int 0 : argument exist (only retured at cli->opt.get_arg = 0)
 * 
 * int argument string length : when success to get argument
 * 
 * int -1 : fail to get argument
 */
static int cli_get_arg(cli_data_t *cli_data, cli_arg_t *cli_arg)
{
    char *str = strchr(cli_data->cmd_str, ' ');
    int idx = 0;

    if (str == NULL)
    {
        return -1; /* Get argument failed, no argument after space */
    }

    if (!cli_arg->opt.get_ret)
    {
        return 0; /* just find argument */
    }
    
    for (idx = 0; idx < cli_arg->cli_get.num; idx++)
    {
        str = strchr(&str[1], ' ');
        if (str == NULL)
        {
            return -1; /* Get arg_num argument failed */
        }
    }
    
    str++;
    cli_arg->len = 0;
    int len = strlen(str);

    if((str[idx] == '\'' || str[idx] == '\"') && (cli_arg->quotes == false))
    {
        cli_arg->quotes = true;
        str++;
    }
    
    for (idx = 0; idx < len; idx++)
    {
        if (cli_arg->quotes)
        {
            if(str[idx] == '\'' || str[idx] == '\"')
            {
                break;
            }
            else if (str[idx] == '\0')
            {
                break;
            }
            else
            {
                cli_arg->arg[cli_arg->len++] = str[idx];
            }
        }
        else
        {
            if (str[idx] == '\0')
            {
                break;
            }
            else if(str[idx] == ' ')
            {
                break;
            }
            else
            {
                cli_arg->arg[cli_arg->len++] = str[idx];
            }
        }
    }
    cli_arg->arg[cli_arg->len] = 0;
    return cli_arg->len; /* Get argument success and copy argument to arg */
}

static int cli_cmp_cmd(char *rx, char *cmd_name)
{
    int idx = 0;

    for (idx = 0; idx < strlen(rx); idx++)
    {
        if (rx[idx] == 0 || rx [idx] == ' ')
        {
            break;
        }
        else if (rx[idx] != cmd_name[idx])
        {
            return -1; /* command is different */
        }
    }

    if (idx == strlen(cmd_name))
    {
        return 1; /* command is same */
    }

    return -1; /* this case is not exist */
}

static CLI_EXEC_RESULT cmd_date(cli_data_t *cli_data)
{   
    cli_arg_t cli_arg = {0, };
    struct tm __tm = {0, };
    int idx = 0;

    /* find 's' */
    cli_arg.opt.get_ret = true;
    cli_arg.cli_get.opt = cli_cmd[CMD_DATE].opt[idx++];
    if (cli_get_opt(cli_data, &cli_arg) > 0)
    {
        if (rtc_string_to_tm(cli_arg.arg, &__tm) != RTC_OKAY)
        {
            printr("invalid cmd date : %s", cli_arg.arg);
            return EXEC_RESULT_ERR;
        }
        prints("set date : %04d-%02d-%02d %02d:%02d:%02d\r\n", 
            __tm.tm_year + 1900, __tm.tm_mon + 1, __tm.tm_mday,
            __tm.tm_hour, __tm.tm_min, __tm.tm_sec);
        rtc_set_time(&__tm);
        return EXEC_RESULT_OK;
    }

    /* check argument exist */
    cli_arg.opt.get_ret = false;
    if (cli_get_arg(cli_data, &cli_arg) != 0) /* no args */
    {
        if (rtc_get_time(&__tm) != RTC_OKAY)
        {
            printr("fail to get rtc data");
            return EXEC_RESULT_ERR;
        }

        prints("date : %04d-%02d-%02d %02d:%02d:%02d\r\n", 
            __tm.tm_year + 1900, __tm.tm_mon + 1, __tm.tm_mday,
            __tm.tm_hour, __tm.tm_min, __tm.tm_sec);
        return EXEC_RESULT_OK;
    }
    else
    {
        printr("undefined command usage");
    }
    return EXEC_RESULT_ERR;
}

static void reboot_deinit_apps()
{
    status_set_int(STATUS_INTEGER_TCP_CLIENT, STATUS_TCP_DOWN);
    while (status_get_int(STATUS_INTEGER_TCP_CLIENT) != STATUS_TCP_NONE)
    {
        osDelay(50);
    }
    print_dmesg("Reboot : close client");

    status_set_int(STATUS_INTEGER_TCP_SERVER1, STATUS_TCP_DOWN);
    while (status_get_int(STATUS_INTEGER_TCP_SERVER1) != STATUS_TCP_NONE)
    {
        osDelay(50);
    }
    print_dmesg("Reboot : close server");
}

static CLI_EXEC_RESULT cmd_reboot(cli_data_t *cli_data)
{
    /* address that pointing Reset_Handler function address's address */
    __IO void (**rst_func)() = (__IO void(**)())(BOOT_LOADER_ADD + 4);
    __IO uint32_t *iap_add = (__IO uint32_t *)BOOT_LOADER_ADD;

    print_dmesg("Reboot : check iap is available");

    /* check iap is valid */
    if ((*iap_add & 0x20000000) == 0x20000000)
    {
        print_dmesg("Reboot : iap is detected");
        /* waiting deinit apps */
        reboot_deinit_apps();
        /* set vector table to boot loader */
        SCB->VTOR = BOOT_LOADER_ADD;
        /* deinit all */
        HAL_RCC_DeInit();
        HAL_DeInit();
        HAL_ICACHE_DeInit();
        __set_MSP((__IO uint32_t)iap_add);
        __disable_irq();
        /* jump to bootloader's reset handler */
        (*rst_func)();
        return EXEC_RESULT_OK;
    }
    else
    {
        print_dmesg("Reboot : iap is not detected");
    }
    return EXEC_RESULT_ERR;
}

static CLI_EXEC_RESULT cmd_echo(cli_data_t *cli_data)
{
    int idx = 0;
    int len = 0;
    cli_arg_t cli_arg = {0, };

    cli_arg.opt.get_ret = true;
    while(1)
    {
        len = cli_get_arg(cli_data, &cli_arg);
        if (len > 0)
        {
            prints("%s ", cli_arg.arg);
            cli_arg.cli_get.num++;
            memset(cli_arg.arg, 0, sizeof(cli_arg.arg));
        }
        else
        {
            break;
        }
        if (cli_arg.quotes)
        {
            break;
        }
        idx++;
    }
    prints("\r\n");

    return EXEC_RESULT_OK;
}

static void ping_result(STATUS_PING status, cmd_ping_t *cmd_ping)
{
    status_set_int(STATUS_INTEGER_PING, STATUS_PING_NONE);
    if (status == STATUS_PING_FAIL)
    {
        cmd_ping->fail_cnt++;
    }

    cmd_ping->req_intv = osKernelGetTickCount();
    cmd_ping->idx++;
    cmd_ping->os_tick_tot += tick_cur_gap(cmd_ping->os_tick);
}

static int cmd_ping_get_arg(cli_data_t *cli_data, cmd_ping_t *cmd_ping)
{
    int len = 0;
    int opt_idx = 0;
    cli_arg_t cli_arg = {0, };

    cli_arg.opt.get_ret = 1;
    cli_arg.cli_get.num = 0;
    len = cli_get_arg(cli_data, &cli_arg);
    if (len < 0)
    {
        printr("fail to get argument");
        return -1;
    }

    if (check_valid_ip(cli_arg.arg) < 0)
    {
        printr("invalid ip address : %s", cli_arg.arg);
        return -1;
    }

    strcpy(cmd_ping->ip, cli_arg.arg);
    cli_arg.cli_get.opt = cli_cmd[CMD_PING].opt[opt_idx++];
    if (cli_get_opt(cli_data, &cli_arg) > 0)
    {
        cmd_ping->ping_cnt = atoi(cli_arg.arg);
    }
    else
    {
        cmd_ping->ping_cnt = 5;
    }

    if (cmd_ping->ping_cnt <= 0 || cmd_ping->ping_cnt > 10)
    {
        printr("invalid range arg input : %d", cmd_ping->ping_cnt);
        return -1;
    }

    cmd_ping->net_add = inet_addr(cmd_ping->ip);
    return 1;
}

static void cmd_ping_req(cli_data_t *cli_data, cmd_ping_t *cmd_ping)
{
    switch (status_get_int(STATUS_INTEGER_PING))
    {
    /* device can send ping */
    case STATUS_PING_NONE:
        if (check_expired(cmd_ping->req_intv, 1000))
        {
            cmd_ping->os_tick = osKernelGetTickCount();
            cmd_ping->req_intv = osKernelGetTickCount();
            status_set_int(STATUS_INTEGER_PING, STATUS_PING_WAIT);
            cmd_ping->seq = FreeRTOS_SendPingRequest(cmd_ping->net_add, 1, PING_TIMEOUT);
        }
        break;
    /* ping send fail -> cause of the problem of icmp packet (checksum or data) */
    case STATUS_PING_FAIL:
        ping_result(STATUS_PING_FAIL, cmd_ping);
        printr("icmp fail to %s: icmp_seq=%d time=%d ms\r\n", 
                cmd_ping->ip, cmd_ping->seq, tick_cur_gap(cmd_ping->os_tick));
        break;
    /* icmp packet is received */
    case STATUS_PING_OK:
        ping_result(STATUS_PING_OK, cmd_ping);
        prints("1 bytes to %s: icmp_seq=%d time=%d ms\r\n", 
                cmd_ping->ip, cmd_ping->seq, tick_cur_gap(cmd_ping->os_tick));
        break;
    /* wait to receive icmp packet */
    case STATUS_PING_WAIT:
        /* icmp packet received timeout */
        if (check_expired(cmd_ping->os_tick,PING_TIMEOUT))
        {
            ping_result(STATUS_PING_FAIL, cmd_ping);
            printr("icmp fail to %s: icmp_seq=%d time=%d ms\r\n", 
                    cmd_ping->ip, cmd_ping->seq, tick_cur_gap(cmd_ping->os_tick));
        }
        break;
    default:
        break;
    }
}

static CLI_EXEC_RESULT cmd_ping(cli_data_t *cli_data)
{
    static cmd_ping_t cmd_ping = {0, };
    CLI_EXEC_RESULT ret = EXEC_WAIT;

    switch (cli_data->work)
    {
    /* ping is initial status */
    case CLI_WORK_NONE:
        if (cmd_ping_get_arg(cli_data, &cmd_ping) < 0)
        {
            return EXEC_RESULT_ERR;
        }
        cli_data->work = CLI_WORK_CONTINUE;
        cmd_ping.req_intv = osKernelGetTickCount();
        break;
    /* device has a remained ping count that be sent */
    case CLI_WORK_CONTINUE:
        cmd_ping_req(cli_data, &cmd_ping);
        if (cmd_ping.idx == cmd_ping.ping_cnt)
        {
            cli_data->work = CLI_WORK_END;
        }
        break;
    /* ping command end or receive CTRL + C */
    case CLI_WORK_END:
    case CLI_WORK_STOP: /* CTRL + C */
        ret = EXEC_RESULT_OK;
        cli_data->work = CLI_WORK_NONE;
        prints("--- %s ping statistics ---\r\n", cmd_ping.ip);
        prints("%d packets transmitted, %d received, %d%% packet loss, time %ldms\r\n", 
                cmd_ping.idx, PING_RECV(&cmd_ping), 
                PIGN_FAIL_PERCENT(&cmd_ping), cmd_ping.os_tick_tot);
        memset(&cmd_ping, 0, sizeof(cmd_ping));
        status_set_int(STATUS_INTEGER_PING, STATUS_PING_NONE);
        break;
    }
    return ret;
}

static CLI_EXEC_RESULT cmd_dmesg(cli_data_t *cli_data)
{
    cli_arg_t cli_arg = {0, };
    char *msg = NULL;

    cli_arg.cli_get.num = 0;
    cli_arg.opt.get_ret = 1;

    if (cli_get_arg(cli_data, &cli_arg) < 0) /* toggle */
    {
        switch(status_get_int(STATUS_INTEGER_DMESG))
        {
            case STATUS_DMESG_ON:
                status_set_int(STATUS_INTEGER_DMESG, STATUS_DMESG_OFF);
                msg = "OFF";
                break;

            case STATUS_DMESG_OFF:
                status_set_int(STATUS_INTEGER_DMESG, STATUS_DMESG_ON);
                msg = "ON";
                break;
        }
    }
    else
    {
        if (strcmp(cli_arg.arg, "on") == 0)
        {
            status_set_int(STATUS_INTEGER_DMESG, STATUS_DMESG_ON);
            msg = "ON";
        }
        else if (strcmp(cli_arg.arg, "off") == 0)
        {
            status_set_int(STATUS_INTEGER_DMESG, STATUS_DMESG_OFF);
            msg = "OFF";
        }
        else
        {
            printr("unkown argument : %s", cli_arg.arg);
            return EXEC_RESULT_ERR;
        }
    }

    prints("debug message : %s\r\n", msg);
    return EXEC_RESULT_OK;
}

static CLI_EXEC_RESULT cmd_clear(cli_data_t *cli_data)
{
    prints("\x1B[2J");
    return EXEC_RESULT_OK;
}

static CLI_EXEC_RESULT cmd_help(cli_data_t *cli_data)
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

    return EXEC_RESULT_OK;
}

static CLI_EXEC_RESULT __cli_work(char *rx, cli_data_t *cli_data)
{
    static int idx = 0;
    CLI_EXEC_RESULT exe_cmd = EXEC_RESULT_NO_CMD; 
    cli_arg_t cli_arg = {0, };

    /* command is still working */
    if (cli_data->work != CLI_WORK_NONE)
    {
        exe_cmd = cli_cmd[idx].func(cli_data);
        goto result_out;
    }
    else
    {
        /*
         * command is not working, set index zero 
         * to search command at bottom
         */
        idx = 0;
    }

    /* only input enter */
    if (strlen(rx) == 0)
    {
        exe_cmd = EXEC_NONE;
        goto result_out;
    }
    
    /* searching command at cli_cmd's member name */
    for (idx = 0; idx < CMD_IDX_MAX; idx++)
    {
        if (cli_cmp_cmd(rx, cli_cmd[idx].name) > 0)
        {
            cli_data->cmd_str = rx;
            cli_arg.opt.get_ret = false;
            cli_arg.cli_get.opt = 'h';
            /* check user input help option */
            if (cli_get_opt(cli_data, &cli_arg) != 0)
            {
                /* it hasn't help option */
                cli_data->opt_size = cli_cmd[idx].opt_size;
                strcpy(cli_data->opt, cli_cmd[idx].opt);
                exe_cmd = cli_cmd[idx].func(cli_data);
            }
            else
            {
                /* it has help option, print help */
                prints("%s", cli_cmd[idx].help);
                exe_cmd = EXEC_HELP;
            }
            break;
        }
    }

result_out:
    switch (exe_cmd)
    {
    /* command execution failed in some reason */
    case EXEC_RESULT_ERR:
        printr("[%s]execution failed : try chat help to use command", cli_cmd[idx].name);
        return exe_cmd;
    /* can't find any command at cli_cmd's member name */
    case EXEC_RESULT_NO_CMD:
        printr("unkwon command : %s", rx);
        return exe_cmd;
    /* just returning result */
    case EXEC_HELP:
    case EXEC_RESULT_OK:
    case EXEC_NONE:
    case EXEC_WAIT:
        return exe_cmd;
    /* unkown error */
    default:
        return EXEC_UNKOWN;
    }
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

    /* ignore case that enter only */
    if (strlen(cli_work->rx) == 0)
    {
        return;
    }

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
    static cli_data_t cli_data = {0, };
    static int esc_cnt = 0;
    static bool esc_seq = 0;

    int len = strlen(rx);
    int idx = 0;
    int ret = CLI_NONE;

    switch (cli_data.work)
    {
    /* when CTRL + C input. */        
    case CLI_WORK_STOP: 
    /* command execution end */
    case CLI_WORK_END:
        goto exec_cmd;
    /* command must need to be worked continuously. */
    case CLI_WORK_CONTINUE: 
        /* when CTRL + C input. */
        if (strchr(rx, 0x03))
        {
            /* set cli work stop it will make stop command execution */
            cli_data.work = CLI_WORK_STOP;
            prints("^C\r\n");
        }
        goto exec_cmd;
    
    default:
    case CLI_WORK_NONE:
        break;
    }
    
    for (idx = 0; idx < len; idx++)
    {
        /* 
         * escape sequence like Home, End, right arrow left arrow 
         * up, down arrow.
         */
        if (esc_seq)
        {
            esc_cnt++;
            /* parse escape sequence character */
            switch (esc_cnt)
            {
            case 2:
                /* check invalid escape sequence format */
                if (rx[idx] != '[')
                {
                    esc_cnt = 0;
                    esc_seq = false;
                }
                ret = CLI_ESCAPE_SEQ;
                break;

            case 3:
                /* if fail to check escape sequence */
                if (cli_esc_work(rx[idx], &cli_work) > 0)
                {
                    esc_cnt = 0;
                    esc_seq = false;
                }
                /* 
                 * check the escape sequence's number to get home and end 
                 * home -> 0x1b, '[', '1', '~'
                 * end -> 0x1b, '[', '4', '~'
                 */
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
        /* CTRL + C */
        else if (rx[idx] == 0x03)
        {
            prints("^C\r\n");
            ret = CLI_CANCEL;
        }
        /* enter -> command execute */
        else if (rx[idx] == '\n' || rx[idx] == '\r')
        {
            prints("\r\n");
            cli_history_insert(&cli_work);
            goto exec_cmd;
        }
        /* delete string */
        else if (rx[idx] == '\b')
        {
            if (cli_work.rx_cnt > 0 && cli_work.cur_pos > 0)
            {
                cli_del_str(&cli_work);
                prints("\b\x1b[P");
            }
            ret = CLI_ESCAPE_SEQ;
        }
        /* escape sequence detected */
        else if (rx[idx] == '\x1b')
        {
            esc_cnt = 1;
            esc_seq = true;
            ret = CLI_ESCAPE_SEQ;
        }
        /* put string */
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
exec_cmd:
    CLI_EXEC_RESULT cli_ret = __cli_work(cli_work.rx, &cli_data);
    switch (cli_ret)
    {
    /* command execution end */
    case EXEC_RESULT_ERR:
    case EXEC_RESULT_NO_CMD:
    case EXEC_HELP:
    case EXEC_RESULT_OK:
        ret = CLI_EXEC_CMD;
        break;
    /* you input only enter key */
    case EXEC_NONE:
        ret = CLI_ENTER;
        break;
    /* wait or error, do noting */
    default:
    case EXEC_WAIT:
        ret = CLI_NONE; 
        break;
    }
    memset(cli_work.rx, 0, sizeof(cli_work.rx));
    cli_work.rx_cnt = 0;
    cli_work.cur_pos = 0;
    return ret;
}

void cli_proc()
{
    uint8_t rx_buf[UART_TRX_SIZE] = {0, };

    uart_read(&uart[UART1_IDX], rx_buf, sizeof(rx_buf));
    switch (cli_work((char *)rx_buf))
    {
    /* command is executed */
    case CLI_EXEC_CMD:
        uart_send(&uart[UART1_IDX], rx_buf, strlen((char *)rx_buf));
    /* only enter input */
    case CLI_ENTER:
    /* cancel command */
    case CLI_CANCEL:
        prints("~# ");
        break;

    case CLI_NONE:
    case CLI_INPUT:
    case CLI_ESCAPE_SEQ:
    case CLI_ERR:
    default:
        break;
    }
}