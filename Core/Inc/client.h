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

typedef struct
{
    char sv_add[IP_LEN];    /* server address */
    bool close_cnt_flag;    /* close count flags */
    TCP_CLIENT_IDX idx;     /* index of client_t */
    eIPTCPState_t old_stat; /* old status */
    int sv_port;            /* server port */
    uint32_t close_cnt;     /* close count */
    Socket_t handle;        /* free rtos server handle, null : handle is invalid or create fail */
}client_t;

#define client_get_status(cl) (((client_t *)cl)->handle->u.xTCP.eTCPState)

int client_init(client_t *cl);
void client_connect(client_t *cl);
int client_send(client_t *cl, uint8_t *sk_buf, size_t sk_buf_len);
int client_recv(client_t *cl, uint8_t *sk_buf, size_t sk_buf_len);
client_t *client_get_head();
void client_work();

#endif
