#ifndef __CLIENT_H__
#define __CLIENT_H__

#include "main.h"
#include "eth.h"
#include "app_freertos.h"

typedef enum
{
    TCP_CLIENT1,
    TCP_CLIENT2,
    TCP_CLIENT_MAX,
}TCP_CLIENT_IDX;

#define CLIENT_SEND_RECV_TIMEO 50
#define CLIENT_SELECT_TIMEO 10
#define CLIENT_CONNECT_TIMEOUT 3000 /* connect try with 3 second */

/*
 * you must check null pointer that is sk of member of tcp_client_t.
 * tcp_client_deinit makes cl->sk as null.
*/
typedef struct
{
    char sv_add[IP_LEN];    /* server address */
    TCP_CLIENT_IDX idx;     /* index of tcp_client_t */
    bool connected;         /* is connected */
    int sv_port;            /* server port */
    uint32_t conn_intv;     /* connect interval */
    Socket_t sk;        /* free rtos server sk, null : sk is invalid or create fail */
    SocketSet_t sel;
} tcp_client_t;
void tcp_client_work();
#endif
