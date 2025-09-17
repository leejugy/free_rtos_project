#include "client.h"
#include "usart.h"
#include "status.h"

client_t client[TCP_CLIENT_MAX] = {
    /* tcp client 1 */
    [TCP_CLIENT1].idx = TCP_CLIENT1,
    [TCP_CLIENT1].sv_port = 4096,
    [TCP_CLIENT1].sv_add = "58.181.17.62",

    /* tcp client 2 */
    [TCP_CLIENT2].idx = TCP_CLIENT2,
    [TCP_CLIENT2].sv_port = 4096,
    [TCP_CLIENT2].sv_add = "58.181.17.62",
};

int client_init(client_t *cl)
{
    if (status_get_int(STATUS_INTEGER_TCP_CLIENT) != STATUS_TCP_UP)
    {
        print_dmesg("network is down\r\n");
        return -1;
    }

    int ret = 0;
    TickType_t timeo = 0;

    cl->handle = socket(FREERTOS_AF_INET, FREERTOS_SOCK_STREAM, FREERTOS_IPPROTO_TCP);
    if (cl->handle == NULL)
    {
        printr("fail to socket init\r\n");
        return -1;
    }

    timeo = pdMS_TO_TICKS(5000);
    ret = setsockopt(cl->handle, 0, FREERTOS_SO_SNDTIMEO, &timeo, sizeof(timeo));
    if (ret < 0)
    {
        printr("fail to set opt\r\n");
        return -1;
    }

    timeo = pdMS_TO_TICKS(10);
    ret = setsockopt(cl->handle, 0, FREERTOS_SO_RCVTIMEO, &timeo, sizeof(timeo));
    if (ret < 0)
    {
        printr("fail to set opt\r\n");
        return -1;
    }
    return 1;
}

/*
 * graceful close client
 * you must check null pointer that is handle of member of client_t.
 * client_deinit makes cl->handle as null.
*/
void client_deinit(client_t *cl)
{
    if (cl->handle)
    {
        shutdown(cl->handle);
        close(cl->handle);
        cl->handle = NULL;
    }
}

void client_connect(client_t *cl)
{
    /* check if connect timeout is expired */
    if (osKernelGetTickCount() - cl->conn_intv < CLIENT_CONNECT_TIMEOUT)
    {
        return;
    }

    if (!cl->handle)
    {
        client_init(cl);
    }
    else if (client_get_status(cl) != eCLOSED)
    {
        return;
    }

    /* set connect timeout */
    cl->conn_intv = osKernelGetTickCount();

    struct freertos_sockaddr addr = {0, };

    addr.sin_address.ulIP_IPv4 = inet_addr(cl->sv_add);
    addr.sin_port = htons(cl->sv_port);
    addr.sin_family = FREERTOS_AF_INET;
    addr.sin_len = sizeof(addr);

    if (connect(cl->handle, &addr, sizeof(addr)) < 0)
    {
        printr("fail to connect : %s, port : %d", cl->sv_add, cl->sv_port);
    }
}

void client_status(client_t *cl)
{
    /* if client fail to init, cl is invalid socket */
    /* @note : if fail to make socket handle cl->handle will be null */
    if (!cl->handle)
    {
        return;
    }

    switch (client_get_status(cl))
    {
    case eESTABLISHED:
        if (cl->old_stat != eESTABLISHED)
        {
            print_dmesg(pbold("Connect") " : %s, port : %d\r\n", cl->sv_add, cl->sv_port);
        }
        break;

    case eLAST_ACK:
    case eFIN_WAIT_1:
    case eFIN_WAIT_2:
    case eTIME_WAIT:
        if (!cl->close_cnt_flag)
        {
           cl->close_cnt = osKernelGetTickCount();
           cl->close_cnt_flag = true;
        }
        /* wait for three second to timeout */
        if (osKernelGetTickCount() - cl->close_cnt > 3000 && cl->close_cnt_flag)
        {
            cl->close_cnt_flag = false;
        }
        else
        {
            break;
        }
        break;
    case eCLOSE_WAIT:
        cl->close_cnt_flag = false;
        client_deinit(cl);
        break;
    
    case eCLOSED:
    default:
        break;
    }
    cl->old_stat = client_get_status(cl);
}

int client_send(client_t *cl, uint8_t *sk_buf, size_t sk_buf_len)
{
    if (client_get_status(cl) != eESTABLISHED || !cl->handle)
    {
        return -1;
    }

    int len = send(cl->handle, sk_buf, sk_buf_len, 0);
    if (len < 0)
    {
        printr("fail to send data : %d bytes", sk_buf_len);
        return len;
    }
    else if (len != 0)
    {
        print_dmesg(pbold("send") " : %d bytes\r\n", len);
    }
    return len;
}

int client_recv(client_t *cl, uint8_t *sk_buf, size_t sk_buf_len)
{
    if (client_get_status(cl) != eESTABLISHED || !cl->handle)
    {
        return -1;
    }

    int len = recv(cl->handle, sk_buf, sk_buf_len, 0);
    if (len < 0)
    {
        printr("fail to recv data : %d bytes\r\n", sk_buf_len);
        return len;
    }
    else if (len != 0)
    {
        print_dmesg(pbold("recv") " : %d bytes\r\n", len);
        print_dmesg(pbold("data") " : %s\r\n", sk_buf);
    }
    return len;
}

client_t *client_get_head()
{
    return client;
}

void client_work()
{
    uint8_t recv[256] = {0, };
    uint8_t send[256] = {0, };
    client_t *cl = client_get_head();
    int idx = 0;
    int ret = 0;

    switch (status_get_int(STATUS_INTEGER_TCP_CLIENT))
    {
    case STATUS_TCP_UP:
        for (idx = 0; idx < TCP_CLIENT_MAX; idx++)
        {
            client_status(&cl[idx]);
            client_connect(&cl[idx]);
            ret = client_recv(&cl[idx], recv, sizeof(recv));
            if (ret > 0)
            {
                print_dmesg("idx : %d\r\n", idx);
                strncpy((char *)send, (char *)recv, sizeof(send));
                ret = client_send(&cl[idx], send, (strnlen((char *)send, sizeof(send))));
            }
            if (ret < 0 && client_get_status(cl) == eESTABLISHED)
            {
                client_deinit(&cl[idx]);
            }
        }
        break;
        
    case STATUS_TCP_DOWN:
        for (idx = 0; idx < TCP_CLIENT_MAX; idx++)
        {
            client_deinit(&cl[idx]);
        }
        status_set_int(STATUS_INTEGER_TCP_CLIENT, STATUS_TCP_NONE);
        break;
    case STATUS_TCP_NONE:
    default:
        break;
    }
}