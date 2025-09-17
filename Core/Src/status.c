#include "status.h"
#include "usart.h"
#if (status_integer_use)
int_status_t int_status = {
    .status[STATUS_INTEGER_PING] = STATUS_PING_NONE,
    .status[STATUS_INTEGER_TCP_CLIENT] = STATUS_TCP_NONE,
    .status[STATUS_INTEGER_DMESG] = STATUS_DMESG_ON,
};
#endif
#if (status_string_use)
string_status_t string_status = {0, };
#endif

void status_init()
{
    #if (status_integer_use)
    {
        int_status.sem = &status_semHandle;
    }
    #endif

    #if (status_string_use)
    {
        string_status.sem = &status_semHandle;
    }
    #endif

    printok("STATUS : init stm32 status");
}

#if (status_integer_use)
int status_get_int(STATUS_INTEGER val)
{
    int ret = 0;

    sem_wait(int_status.sem);
    ret = int_status.status[val];
    sem_post(int_status.sem);
    return ret;
}

void status_set_int(STATUS_INTEGER val, int set)
{
    sem_wait(int_status.sem);
    int_status.status[val] = set;
    sem_post(int_status.sem);
}
#endif

#if (status_string_use)
void status_get_string(STATUS_STRING val, char *buf, size_t buf_size)
{
    sem_wait(string_status.sem);
    strncpy(buf, string_status.status[val], buf_size);
    sem_post(string_status.sem);
}

void status_set_string(STATUS_INTEGER val, char *buf, size_t buf_size)
{
    sem_wait(string_status.sem);
    strncpy(string_status.status[val], buf, sizeof(string_status.status[val]));
    sem_post(string_status.sem);
}
#endif