#ifndef __STATUS_H__
#define __STATUS_H__

#include "main.h"
#include "app_freertos.h"

void status_init();

#define status_integer_use 1
#define status_string_use 0

#if (status_integer_use)
typedef enum
{
    STATUS_INTEGER_PING,
    STATUS_INTEGER_TCP_CLIENT,
    STATUS_INTEGER_TCP_SERVER,
    STATUS_INTEGER_DMESG,
    STATUS_INTEGER_MAX
}STATUS_INTEGER;

typedef enum
{
    STATUS_PING_FAIL = -1,
    STATUS_PING_NONE,
    STATUS_PING_WAIT,
    STATUS_PING_OK,
}STATUS_PING;

typedef enum
{
    STATUS_TCP_NONE = 0,
    STATUS_TCP_UP,
    STATUS_TCP_DOWN,
}STATUS_TCP;

typedef enum
{
    STATUS_DMESG_OFF,
    STATUS_DMESG_ON,
}STATUS_DMESG;

typedef struct
{
    osSemaphoreId_t *sem;
    int status[STATUS_INTEGER_MAX];
}int_status_t;

void status_set_int(STATUS_INTEGER val, int set);
int status_get_int(STATUS_INTEGER val);
#endif

#if (status_string_use)
#define STATUS_STRING_MAX_LEN (1 << 8)
typedef enum
{
    STATUS_STRING_MAX = 0,
}STATUS_STRING;

typedef struct
{
    osSemaphoreId_t *sem;
    char status[STATUS_STRING_MAX][STATUS_STRING_MAX_LEN];
}string_status_t;

void status_get_string(STATUS_STRING val, char *buf, size_t buf_size);
void status_set_string(STATUS_INTEGER val, char *buf, size_t buf_size);
#endif

#endif