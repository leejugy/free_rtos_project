#ifndef __SERVER_H__
#define __SERVER_H__

#include "main.h"
#include "app_freertos.h"
#include "eth.h"

#define SERVER_SEND_RECV_TIMEO 50

typedef enum
{
    TCP_SERVER1,
    TCP_SERVER_MAX,
}TCP_SERVER_IDX;

#define SERVER_CLIENT_MAX 3
#define SERVER_SELECT_TIMEO 10
#define INADDR_ANY 0x00000000
#define ACCEPT_INTERVAL 100 /* 100ms try accept socket */
/*
 * you must check null pointer that is sk of member of tcp_server_t.
 * tcp_server_client_deinit makes sv->cl->sk as null.
*/
typedef struct
{
    int idx;
    Socket_t sk;
    struct freertos_sockaddr addr;
    eIPTCPState_t old_stat;
    SocketSet_t sel;
}tcp_server_client_t;

typedef struct
{
    tcp_server_client_t cl[SERVER_CLIENT_MAX]; /* old status */
    Socket_t sk;                       /* free rtos server sk, null : sk is invalid or create fail */
    int port;
    uint32_t accept_intv;
    SocketSet_t sel;
}tcp_server_t;

static inline eIPTCPState_t server_get_client_status(tcp_server_client_t *cl)
{
    if (cl->sk != NULL)
    { 
        return cl->sk->u.xTCP.eTCPState;
    }

    return cl->old_stat;
}

void tcp_server1_work();
#endif