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
        return EXEC_NONE;
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
        return EXEC_NONE;
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
        SCB->VTOR = BOOT_LOADER_ADD;
        reboot_deinit_apps();
        HAL_RCC_DeInit();
        HAL_DeInit();
        HAL_ICACHE_DeInit();
        __set_MSP((__IO uint32_t)iap_add);
        __disable_irq();
        (*rst_func)();
        return EXEC_NONE;
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

    return EXEC_NONE;
}

static void ping_result(STATUS_PING status, int *idx, int *ping_cnt, int *fail_cnt)
{
    status_set_int(STATUS_INTEGER_PING, STATUS_PING_NONE);
    if (status == STATUS_PING_FAIL)
    {
        (*fail_cnt)++;
    }

    if ((*idx)++ != *ping_cnt)
    {
        osDelay(1000);
    }
}

static CLI_EXEC_RESULT cmd_ping(cli_data_t *cli_data)
{
    int len = 0;
    int idx = 0;
    int opt_idx = 0;
    int ping_cnt = 5;
    int seq = 0;
    int fail_cnt = 0;
    uint32_t os_tick = 0;
    uint32_t os_tick_tot = 0;
    uint32_t net_add = 0;
    char ip[IP_LEN] = {0, };
    cli_arg_t cli_arg = {0, };

    cli_arg.opt.get_ret = 1;
    cli_arg.cli_get.num = 0;
    len = cli_get_arg(cli_data, &cli_arg);
    if (len < 0)
    {
        printr("fail to get argument");
        return EXEC_RESULT_ERR;
    }

    if (check_valid_ip(cli_arg.arg) < 0)
    {
        printr("invalid ip address : %s", cli_arg.arg);
        return EXEC_RESULT_ERR;
    }

    strcpy(ip, cli_arg.arg);
    cli_arg.cli_get.opt = cli_cmd[CMD_PING].opt[opt_idx++];
    if (cli_get_opt(cli_data, &cli_arg) > 0)
    {
        ping_cnt = atoi(cli_arg.arg);
    }

    if (ping_cnt <= 0 || ping_cnt > 10)
    {
        printr("invalid range arg input : %d", ping_cnt);
        return EXEC_RESULT_ERR;
    }

    net_add = inet_addr(ip);
    os_tick_tot = osKernelGetTickCount();
    while(idx != ping_cnt)
    {
        switch (status_get_int(STATUS_INTEGER_PING))
        {
        case STATUS_PING_NONE:
            os_tick = osKernelGetTickCount();
            status_set_int(STATUS_INTEGER_PING, STATUS_PING_WAIT);
            seq = FreeRTOS_SendPingRequest(net_add, 1, PING_TIMEOUT);
            break;

        case STATUS_PING_FAIL:
            printr("icmp fail to %s: icmp_seq=%d time=%d ms\r\n", ip, seq, tick_cur_gap(os_tick));
            ping_result(STATUS_PING_FAIL, &idx, &ping_cnt, &fail_cnt);
            break;

        case STATUS_PING_OK:
            prints("1 bytes to %s: icmp_seq=%d time=%d ms\r\n", ip, seq, tick_cur_gap(os_tick));
            ping_result(STATUS_PING_OK, &idx, &ping_cnt, &fail_cnt);
            break;

        case STATUS_PING_WAIT:
            if (tick_cur_gap(os_tick) > PING_TIMEOUT)
            {
                printr("icmp fail to %s: icmp_seq=%d time=%d ms\r\n", ip, seq, tick_cur_gap(os_tick));
                ping_result(STATUS_PING_FAIL, &idx, &ping_cnt, &fail_cnt);
            }
            break;
        default:
            break;
        }
    }

    prints("--- %s ping statistics ---\r\n", ip);
    prints("%d packets transmitted, %d received, %d%% packet loss, time %ldms\r\n", 
        ping_cnt, PING_RECV(ping_cnt, fail_cnt), PIGN_FAIL_PERCENT(ping_cnt, fail_cnt),
        tick_cur_gap(os_tick_tot));
    return EXEC_NONE;
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
    return EXEC_NONE;
}

static CLI_EXEC_RESULT cmd_clear(cli_data_t *cli_data)
{
    prints("\x1B[2J");
    return EXEC_NONE;
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

    return EXEC_NONE;
}

static int cli_parser(char *rx)
{
    int idx = 0;
    CLI_EXEC_RESULT exe_cmd = EXEC_RESULT_NO_CMD;
    char pt_buf[CMD_MAX_LEN] = {0, }; 
    cli_data_t cli_data = {0, };
    cli_arg_t cli_arg = {0, };

    if (strlen(rx) == 0)
    {
        exe_cmd = EXEC_NONE;
        goto result_out;
    }
    
    for (idx = 0; idx < CMD_IDX_MAX; idx++)
    {
        if (cli_cmp_cmd(rx, cli_cmd[idx].name) > 0)
        {
            cli_data.cmd_str = rx;
            cli_arg.opt.get_ret = false;
            cli_arg.cli_get.opt = 'h';
            if (cli_get_opt(&cli_data, &cli_arg) != 0)
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

result_out:
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
    case EXEC_NONE:
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