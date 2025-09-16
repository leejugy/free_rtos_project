/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    eth.h
  * @brief   This file contains all the function prototypes for
  *          the eth.c file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ETH_H__
#define __ETH_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */
#include "FreeRTOS_IP.h"
#include "rng.h"
/* USER CODE END Includes */

extern ETH_HandleTypeDef heth;

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

void MX_ETH_Init(void);

/* USER CODE BEGIN Prototypes */
/**
 * @brief get socket handle
 * 
 * @param[in] domain can be FREERTOS_AF_INET or something
 * @param[in] stream can be FREERTOS_SOCK_STREAM or someting
 * @param[in] prot can be FREERTOS_IPPROTO_TCP or something
 * @return Socket_t return socket handle or NULL
 * @ref FreeRTOS_Socket.h
 */
static inline Socket_t socket(BaseType_t domain, BaseType_t type, BaseType_t prot)
{
    Socket_t sk_handle = FreeRTOS_socket(domain, type, prot);
    if (sk_handle == FREERTOS_INVALID_SOCKET)
    {
        return NULL;
    }

    return sk_handle;
}

/**
 * @brief Set the socket options for the given socket.
 *
 * @param[in] fd The socket for which the options are to be set.
 * @param[in] lv Not used. Parameter is used to maintain the Berkeley sockets
 *                    standard.
 * @param[in] opt The name of the option to be set.
 * @param[in] opt_val The value of the option to be set.
 * @param[in] opt_len Not used. Parameter is used to maintain the Berkeley
 *                            sockets standard.
 *
 * @return If the option can be set with the given value, then 0 is returned. Else,
 *         an error code is returned.
 */
static inline int setsockopt(Socket_t handle, int32_t lv, int32_t opt, const void *opt_val, size_t opt_len)
{
    if (FreeRTOS_setsockopt(handle, lv, opt, opt_val, opt_len) != pdFREERTOS_ERRNO_NONE)
    {
        return -1;   
    }
    return 0;
}

/**
 * @brief Connect to a remote port.
 *
 * @note it send remote connect request to task
 * 
 * @param[in] handle The socket initiating the connection.
 * @param[in] sockaddr The address of the remote socket.
 * @param[in] addr_len This parameter is not used. It is kept in
 *                   the function signature to adhere to the Berkeley
 *                   sockets standard.
 *
 * @return 0 is returned on a successful connection, else a negative
 *         error code is returned.
 */
static inline int connect(Socket_t handle, const struct freertos_sockaddr *sockaddr, uint32_t addr_len)
{
    if (FreeRTOS_connect(handle, sockaddr, addr_len) != pdFREERTOS_ERRNO_NONE)
    {
        return -1;
    }
    return 0;
}

/**
 * @brief Accept a connection on an listening socket.
 *
 * @param[in] handle The socket in listening mode.
 * @param[out] sock_addr The address of the machine trying to connect to this node
 *                        is returned in this pointer.
 * @param[out] addr_len The length of the address of the remote machine.
 *
 * @return FreeRTOS_accept: can return a new connected socket if the server socket
 *         is in listen mode and receives a connection request. The new socket will
 *         be bound already to the same port number as the listening socket.
 */
static inline int accept(Socket_t handle, struct freertos_sockaddr *sock_addr, uint32_t *addr_len)
{
    Socket_t sk_handle = FreeRTOS_accept(handle, sock_addr, addr_len);
    if (sk_handle == FREERTOS_INVALID_SOCKET)
    {
        return -1;
    }
    return 1;
}

/**
 * @brief binds a socket to a local port number. If port 0 is provided,
 *        a system provided port number will be assigned. This function
 *        can be used for both UDP and TCP sockets. The actual binding
 *        will be performed by the IP-task to avoid mutual access to the
 *        bound-socket-lists (xBoundUDPSocketsList or xBoundTCPSocketsList).
 *
 * @param[in] handle The socket being bound.
 * @param[in] sock_addr The address struct carrying the port number to which
 *                       this socket is to be bound.
 * @param[in] addr_len This parameter is not used internally. The
 *                       function signature is used to adhere to standard
 *                       Berkeley sockets API.
 *
 * @return The return value is 0 if the bind is successful.
 *         If some error occurred, then a negative value is returned.
 */
static inline int bind(Socket_t handle, const struct freertos_sockaddr *sock_addr, uint32_t addr_len)
{
    if (FreeRTOS_bind(handle, sock_addr, addr_len) != pdFREERTOS_ERRNO_NONE)
    {
        return -1;
    }
    return 0;
}

/**
 * @brief Request to put a socket in listen mode.
 *
 * @param[in] handle the socket to be put in listening mode.
 * @param[in] backlog Maximum number of child sockets.
 *
 * @return 0 in case of success, or else a negative error code is
 *         returned.
 */
static inline int listen(Socket_t handle, BaseType_t backlog)
{
    if (FreeRTOS_listen(handle, backlog) != pdFREERTOS_ERRNO_NONE)
    {
        return -1;
    }
    return 0;
}

/**
 * @brief Send data using a TCP socket. It is not necessary to have the socket
 *        connected already. Outgoing data will be stored and delivered as soon as
 *        the socket gets connected.
 *
 * @param[in] handle  The socket owning the connection.
 * @param[in] buf The buffer containing the data. The value of this pointer
 *                      may be NULL in case zero-copy transmissions are used.
 *                      It is used in combination with 'FreeRTOS_get_tx_head()'.
 * @param[in] buf_size The length of the data to be added.
 * @param[in] flag This parameter is not used. (zero or FREERTOS_MSG_DONTWAIT).
 *
 * @return The number of bytes actually sent. Zero when nothing could be sent
 *         or a negative error code in case an error occurred.
 */
static inline int send(Socket_t handle, uint8_t *buf, size_t buf_size, int flag)
{
    int ret = FreeRTOS_send(handle, buf, buf_size, flag);
    if (ret < 0)
    {
        return -1;
    }
    return ret;
}

/**
 * @brief Read incoming data from a TCP socket. Only after the last
 *        byte has been read, a close error might be returned.
 *
 * @param[in] handle The socket owning the connection.
 * @param[out] buf The buffer to store the incoming data in.
 * @param[in] buf_size The length of the buffer so that the function
 *                            does not do out of bound access.
 * @param[in] flag The flags for conveying preference. The values
 *                    FREERTOS_MSG_DONTWAIT, FREERTOS_ZERO_COPY and/or
 *                    FREERTOS_MSG_PEEK can be used.
 *
 * @return The number of bytes actually received and stored in the pvBuffer.
 */
static inline int recv(Socket_t handle, uint8_t *buf, size_t buf_size, int flag)
{
    int ret = FreeRTOS_recv(handle, buf, buf_size, flag);
    if (ret < 0)
    {
        return -1;
    }
    return ret;
}

/**
 * @brief Close a socket and free the allocated space. In case of a TCP socket:
 *        the connection will not be closed automatically. Subsequent messages
 *        for the closed socket will be responded to with a RST. The IP-task
 *        will actually close the socket, after receiving a 'eSocketCloseEvent'
 *        message.
 *
 * @param[in] handle the socket being closed.
 * @warning close sequence must be called after invoke shutdown
 *
 * @return There are three distinct values which can be returned:
 *         0: If the xSocket is NULL/invalid.
 *         1: If the socket was successfully closed (read the brief above).
 *        -1: If the socket was valid but could not be closed because the message
 */
static inline int close(Socket_t handle)
{
    if (FreeRTOS_closesocket(handle) != 1)
    {
        return -1;
    }
    return 0;
}

/**
 * @brief Shutdown - This function will shut down the connection in both
 *        directions. However, it will first deliver all data queued for
 *        transmission, and also it will first wait to receive any missing
 *        packets from the peer.
 *
 * @param[in] handle The socket owning the connection.
 * @param[in] 0 Not used. Just present to stick to Berkeley standard.
 *
 * @return 0 on successful shutdown or else a negative error code.
 */
static inline int shutdown(Socket_t handle)
{
    if (FreeRTOS_shutdown(handle, 0) != pdFREERTOS_ERRNO_NONE)
    {
        return -1;
    }
    return 0;
}

static inline uint32_t inet_addr(char *addr)
{
    return FreeRTOS_inet_addr(addr);
}

static inline uint16_t htons(uint16_t val)
{
    return FreeRTOS_htons(val);
}

static inline uint32_t htosl(uint32_t val)
{
    return FreeRTOS_htonl(val);
}

static inline uint16_t ntohs(uint16_t val)
{
    return FreeRTOS_ntohs(val);
}

static inline uint32_t ntohl(uint32_t val)
{
    return FreeRTOS_ntohl(val);
}

#define chk_valid_ip(ip) (0 <= (ip) && (ip) <= 255)

typedef enum
{
    TCP_CLIENT1,
    TCP_CLIENT_MAX,
}TCP_CLIENT_IDX;

#define IP_LEN 32

typedef struct
{
    Socket_t handle;        /* free rtos server handle */
    TCP_CLIENT_IDX idx;     /* index of client_t */
    char sv_add[IP_LEN];    /* server address */
    int sv_port;            /* server port */
    bool valid;          /* connect try */
    bool close_cnt_flag; /*close count flags*/
    uint32_t close_cnt; /* close count */
}client_t;

#define client_get_status(cl) (((client_t *)cl)->handle->u.xTCP.eTCPState)

void eth_init();
void client_init();
void client_connect(client_t *cl);
int client_send(client_t *cl, uint8_t *sk_buf, size_t sk_buf_len);
int client_recv(client_t *cl, uint8_t *sk_buf, size_t sk_buf_len);
void __ETH_IRQHandler(void);
int check_valid_ip(char *ip);
client_t *client_get_head();
void eth_work();

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __ETH_H__ */

