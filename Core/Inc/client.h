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

#define CLIENT_CONNECT_TIMEOUT 3000 /* connect try with 3 second */

/*
 * you must check null pointer that is handle of member of client_t.
 * client_deinit makes cl->handle as null.
*/
typedef struct
{
    char sv_add[IP_LEN];    /* server address */
    bool close_cnt_flag;    /* close count flags */
    TCP_CLIENT_IDX idx;     /* index of client_t */
    eIPTCPState_t old_stat; /* old status */
    int sv_port;            /* server port */
    uint32_t close_cnt;     /* close count */
    uint32_t conn_intv;    /* connect interval */
    Socket_t handle;        /* free rtos server handle, null : handle is invalid or create fail */
}client_t;

static inline eIPTCPState_t client_get_status(client_t *cl)
{
    if (cl->handle != NULL)
    { 
        return cl->handle->u.xTCP.eTCPState;
    }

    return cl->old_stat;
}

int client_init(client_t *cl);
void client_connect(client_t *cl);
int client_send(client_t *cl, uint8_t *sk_buf, size_t sk_buf_len);
int client_recv(client_t *cl, uint8_t *sk_buf, size_t sk_buf_len);
client_t *client_get_head();
void client_work();

#endif
