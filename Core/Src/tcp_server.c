#include "tcp_server.h"
#include "status.h"

tcp_server_t tcp_server[TCP_SERVER_MAX] = 
{
    /* tcp server 1 */
    [TCP_SERVER1].cl = {{0, }},
    [TCP_SERVER1].sk = NULL,
    [TCP_SERVER1].accept_intv = 0,
    [TCP_SERVER1].port = 4096  
};

/**
 * @note :eSELECT_READ will be set when current socket status is eTCP_LISTEN.
 * If peer socket was is null or socket has received data (it's likely SYN)
 * from client, eSELECT_READ bit is returned by FreeRTOS_select.
 * If you want to get client socket accepted with 3 way handshake,
 * use accept function after you get eSELECT_READ from select.
 *
 * 3 way handshake
 * (client) SYN -> (server)
 * (server) SYN-ACK -> (client)
 * (client) ACK -> (server)
 * */
static int tcp_server_select_init(tcp_server_t *sv)
{
    if (!sv->sel)
    {
        sv->sel = FreeRTOS_CreateSocketSet();
        if (sv->sel == NULL)
        {
            printr("fail to create socket set");
            return -1;
        }
    }
    FreeRTOS_FD_SET(sv->sk, sv->sel, eSELECT_READ);
    return 1;
}

static int tcp_server_client_select_init(tcp_server_client_t *cl)
{
    if (!cl->sel)
    {
        cl->sel = FreeRTOS_CreateSocketSet();
        if (cl->sel == NULL)
        {
            printr("fail to create socket set");
            return -1;
        }
    }
    /* note : eCLOSE_WAIT or eCLOSE status bit set eSELECT_EXCEPT will be returned at FreeRTOS_select */
    FreeRTOS_FD_SET(cl->sk, cl->sel, eSELECT_READ | eSELECT_WRITE | eSELECT_EXCEPT);
    return 1;
}

static int tcp_server_init(tcp_server_t *sv)
{
    if (sv->sk)
    {
        return -1;
    }

    sv->sk = socket(FREERTOS_AF_INET, FREERTOS_SOCK_STREAM, FREERTOS_IPPROTO_TCP);
    if (sv->sk == NULL)
    {
        printr("fail to init socket");
        return -1;
    }

    TickType_t timeo = 0;
    int ret = 0;

    timeo = pdMS_TO_TICKS(SERVER_SEND_RECV_TIMEO);
    ret = setsockopt(sv->sk, 0, FREERTOS_SO_SNDTIMEO, &timeo, sizeof(timeo));
    if (ret < 0)
    {
        printr("fail to set opt\r\n");
        return -1;
    }

    timeo = pdMS_TO_TICKS(SERVER_SEND_RECV_TIMEO);
    ret = setsockopt(sv->sk, 0, FREERTOS_SO_RCVTIMEO, &timeo, sizeof(timeo));
    if (ret < 0)
    {
        printr("fail to set opt\r\n");
        return -1;
    }

    struct freertos_sockaddr addr = {0, };

    addr.sin_address.ulIP_IPv4 = htonl(INADDR_ANY);
    addr.sin_port = htons(sv->port);
    addr.sin_family = FREERTOS_AF_INET;
    addr.sin_len = sizeof(addr);

    if (bind(sv->sk, &addr, sizeof(addr)) < 0)
    {
        printr("fail to bind socket");
        return -1;
    }

    if (listen(sv->sk, 10) < 0)
    {
        printr("fail to listen socket");
        return -1;
    }

    if (!sv->sel)
    {
        if (tcp_server_select_init(sv) < 0)
        {
            return -1;
        }
    }

    int idx = 0;

    for (idx = 0; idx < SERVER_CLIENT_MAX; idx++)
    {
        sv->cl[idx].idx = idx;
    }
    return 1;
}

static void tcp_server_deinit(tcp_server_t *sv)
{
    if (sv->sk)
    {
        shutdown(sv->sk);
        close(sv->sk);
        if (sv->sel)
        {
            FreeRTOS_FD_CLR(sv->sk, sv->sel, eSELECT_ALL);
        }
        sv->sk = NULL;
    }
}

static void tcp_server_client_deinit(tcp_server_client_t *cl)
{
    if (cl->sk)
    {
        shutdown(cl->sk);
        close(cl->sk);
        if (cl->sel)
        {
            FreeRTOS_FD_CLR(cl->sk, cl->sel, eSELECT_ALL);
        }
        cl->sk = NULL;
    }
}

static int __tcp_server_accept(tcp_server_t *sv, tcp_server_client_t *cl)
{
    if (cl->sk)
    {
        return -1;
    }

    if (!sv->sel)
    {
        if (tcp_server_select_init(sv) < 0)
        {
            return -1;
        }
    }

    int ret = FreeRTOS_select(sv->sel, SERVER_SELECT_TIMEO);
    if (!(ret & eSELECT_READ))
    {
        return -1;
    }

    uint32_t addr_len = sizeof(struct freertos_sockaddr);
    char ip[IP_LEN] = {0, };

    cl->sk = accept(sv->sk, &cl->addr, &addr_len);
    if (cl->sk == NULL)
    {
        return -1;
    }
    inet_ntoa(cl->addr.sin_address.ulIP_IPv4, ip);
    print_dmesg(pbold("accept") " ip : %s, port : %d\r\n", ip, ntohs(cl->addr.sin_port));
    if (tcp_server_client_select_init(cl) < 0)
    {
        return -1;
    }
    return 1;
}

static int tcp_server_accept(tcp_server_t *sv)
{
    if (!sv->sk)
    {
        tcp_server_init(sv);
    }

    if (tick_cur_gap(sv->accept_intv) < ACCEPT_INTERVAL)
    {
        return -1;
    }

    int idx = 0;
    bool reject = true;
    tcp_server_client_t *cl = NULL;
    /* find empty socket */
    for (idx = 0; idx < SERVER_CLIENT_MAX; idx++)
    {
        cl = &sv->cl[idx];
        if (cl->sk == NULL)
        {
            reject = false;
            break;
        }
    }

    if (reject)
    {
        tcp_server_client_t cl_del = {0, };
        __tcp_server_accept(sv, &cl_del);
        tcp_server_client_deinit(&cl_del);
        return -1;
    }

    if (__tcp_server_accept(sv, cl) < 0)
    {
        return -1;
    }
    return 1;
}

static int tcp_server_client_send(tcp_server_client_t *cl, uint8_t *sk_buf, size_t sk_buf_len)
{
    if (!cl->sk)
    {
        return -1;
    }

    if (!cl->sel)
    {
        if (tcp_server_client_select_init(cl) < 0)
        {
            return -1;
        }
    }

    int ret = FreeRTOS_select(cl->sel, SERVER_SELECT_TIMEO);
    if (ret & eSELECT_EXCEPT)
    {
        return -1;
    }
    else if (!(ret & eSELECT_WRITE))
    {
        return 0;
    }

    int len = send(cl->sk, sk_buf, sk_buf_len, 0);
    if (len < 0)
    {
        printr("fail to send data : %d bytes\r\n", sk_buf_len);
        return len;
    }
    else if (len != 0)
    {
        print_dmesg(pbold("send") " : %d bytes\r\n", len);
    }
    return len;
}

static int tcp_server_client_recv(tcp_server_client_t *cl, uint8_t *sk_buf, size_t sk_buf_len)
{
    if (!cl->sk)
    {
        return -1;
    }

    if (!cl->sel)
    {
        if (tcp_server_client_select_init(cl) < 0)
        {
            return -1;
        }
    }

    int ret = FreeRTOS_select(cl->sel, SERVER_SELECT_TIMEO);
    if (ret & eSELECT_EXCEPT)
    {
        return -1;
    }
    else if (!(ret & eSELECT_READ))
    {
        return 0;
    }

    int len = recv(cl->sk, sk_buf, sk_buf_len, 0);
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

static void tcp_server_work_up()
{
    uint8_t recv_buf[256] = {0, };
    uint8_t send_buf[256] = {0, };
    tcp_server_t *sv = tcp_server;
    tcp_server_client_t *cl = NULL;
    int sv_idx = 0;
    int cl_idx = 0;
    int ret = 0;

    for (sv_idx = 0; sv_idx < TCP_SERVER_MAX; sv_idx++)
    {
        tcp_server_accept(&sv[sv_idx]);
        for (cl_idx = 0; cl_idx < SERVER_CLIENT_MAX; cl_idx++)
        {
            cl = &sv[sv_idx].cl[cl_idx];
            ret = tcp_server_client_recv(cl, recv_buf, sizeof(recv_buf));
            if (ret > 0)
            {
                print_dmesg("idx : %d\r\n", cl_idx);
                strncpy((char *)send_buf, (char *)recv_buf, sizeof(send_buf));
                ret = tcp_server_client_send(cl, send_buf, (strnlen((char *)send_buf, sizeof(send_buf))));
            }
            if (ret < 0)
            {
                tcp_server_client_deinit(cl);
            }
        }
    }
}

static void tcp_server_work_down()
{
    int sv_idx = 0;
    int cl_idx = 0;
    tcp_server_t *sv = tcp_server;
    tcp_server_client_t *cl = NULL;
    for (sv_idx = 0; sv_idx < TCP_SERVER_MAX; sv_idx++)
    {
        for (cl_idx = 0; cl_idx < SERVER_CLIENT_MAX; cl_idx++)
        {
            cl = &sv[sv_idx].cl[cl_idx];
            tcp_server_client_deinit(cl);
        }
        tcp_server_deinit(&sv[sv_idx]);
    }
    status_set_int(STATUS_INTEGER_TCP_SERVER, STATUS_TCP_NONE);
}

void tcp_server_work()
{
    switch (status_get_int(STATUS_INTEGER_TCP_SERVER))
    {
    case STATUS_TCP_UP:
        tcp_server_work_up();
        break;

    case STATUS_TCP_DOWN:
        tcp_server_work_down();
        break;

    case STATUS_TCP_NONE:
    default:
        break;
    }
}