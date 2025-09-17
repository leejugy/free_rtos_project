#include "tcp_client.h"
#include "usart.h"
#include "status.h"

tcp_client_t tcp_client[TCP_CLIENT_MAX] = {
    /* tcp tcp_client 1 */
    [TCP_CLIENT1].idx = TCP_CLIENT1,
    [TCP_CLIENT1].sv_port = 4096,
    [TCP_CLIENT1].sv_add = "58.181.17.62",

    /* tcp tcp_client 2 */
    [TCP_CLIENT2].idx = TCP_CLIENT2,
    [TCP_CLIENT2].sv_port = 4096,
    [TCP_CLIENT2].sv_add = "58.181.17.62",
};

static int tcp_client_init(tcp_client_t *cl)
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

    timeo = pdMS_TO_TICKS(CLIENT_SEND_RECV_TIMEO);
    ret = setsockopt(cl->handle, 0, FREERTOS_SO_SNDTIMEO, &timeo, sizeof(timeo));
    if (ret < 0)
    {
        printr("fail to set opt\r\n");
        return -1;
    }

    timeo = pdMS_TO_TICKS(CLIENT_SEND_RECV_TIMEO);
    ret = setsockopt(cl->handle, 0, FREERTOS_SO_RCVTIMEO, &timeo, sizeof(timeo));
    if (ret < 0)
    {
        printr("fail to set opt\r\n");
        return -1;
    }
    return 1;
}

/*
 * graceful close tcp_client
 * you must check null pointer that is handle of member of tcp_client_t.
 * tcp_client_deinit makes cl->handle as null.
*/
static void tcp_client_deinit(tcp_client_t *cl)
{
    if (cl->handle)
    {
        shutdown(cl->handle);
        close(cl->handle);
        cl->handle = NULL;
        cl->connected = false;
    }
}

static void tcp_client_check_connect(tcp_client_t *cl)
{
    if (!cl->handle)
    {
        return;
    }
    /* check if connect is done */
    if (cl->handle->u.xTCP.eTCPState == eESTABLISHED && !cl->connected)
    {
        print_dmesg(pbold("connect") " ip[%d] : %s, port : %d\r\n",
                    cl->idx, cl->sv_add, cl->sv_port);
        cl->connected = true;
    }
}

static void tcp_client_connect(tcp_client_t *cl)
{
    /* check if connect timeout is expired */
    if (tick_cur_gap(cl->conn_intv) < CLIENT_CONNECT_TIMEOUT)
    {
        return;
    }

    if (!cl->handle)
    {
        tcp_client_init(cl);
    }
    else
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
        return;
    }
}

static int tcp_client_send(tcp_client_t *cl, uint8_t *sk_buf, size_t sk_buf_len)
{
    if (!cl->handle)
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

static int tcp_client_recv(tcp_client_t *cl, uint8_t *sk_buf, size_t sk_buf_len)
{
    if (!cl->handle)
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

static void tcp_client_work_up()
{
    uint8_t recv_buf[256] = {0, };
    uint8_t send_buf[256] = {0, };
    tcp_client_t *cl = tcp_client;
    int idx = 0;
    int ret = 0;
    for (idx = 0; idx < TCP_CLIENT_MAX; idx++)
    {
        tcp_client_connect(&cl[idx]);
        tcp_client_check_connect(&cl[idx]);
        ret = tcp_client_recv(&cl[idx], recv_buf, sizeof(recv_buf));
        if (ret > 0)
        {
            print_dmesg("idx : %d\r\n", idx);
            strncpy((char *)send_buf, (char *)recv_buf, sizeof(send_buf));
            ret = tcp_client_send(&cl[idx], send_buf, (strnlen((char *)send_buf, sizeof(send_buf))));
        }
        if (ret < 0)
        {
            tcp_client_deinit(&cl[idx]);
        }
    }
}

static void tcp_client_work_down()
{
    int idx = 0;
    tcp_client_t *cl = tcp_client;
    for (idx = 0; idx < TCP_CLIENT_MAX; idx++)
    {
        tcp_client_deinit(&cl[idx]);
    }
    status_set_int(STATUS_INTEGER_TCP_CLIENT, STATUS_TCP_NONE);
}

void tcp_client_work()
{
    switch (status_get_int(STATUS_INTEGER_TCP_CLIENT))
    {
    case STATUS_TCP_UP:
        tcp_client_work_up();
        break;
        
    case STATUS_TCP_DOWN:
        tcp_client_work_down();
        break;
    case STATUS_TCP_NONE:
    default:
        break;
    }
}