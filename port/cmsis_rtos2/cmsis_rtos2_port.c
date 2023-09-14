#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <zbus_config.h>
#include <zbus_kernel.h>

#include "cmsis_os2.h"

int k_mutex_init(struct k_mutex * mutex)
{
    mutex->mutex = osSemaphoreNew(1, 1, NULL);
    if(mutex->mutex == NULL){
        printf("zbus create mutex fail\r\n");
        return -1;
    }
    // printf("zbus mutex id : %d\r\n", mutex->mutex);
    return 0;
}

int k_mutex_lock(struct k_mutex * mutex, uint32_t timeout)
{
    if(mutex->mutex == NULL){
        if(k_mutex_init(mutex) != 0){
            return -1;
        }
    }

    return osSemaphoreAcquire(mutex->mutex, timeout);
}

int k_mutex_unlock(struct k_mutex * mutex)
{
    if(mutex->mutex == NULL){
        if(k_mutex_init(mutex) != 0){
            return -1;
        }
    }

    return osSemaphoreRelease(mutex->mutex);
}

void k_mutex_destory(struct k_mutex * mutex)
{
    osMutexDelete(mutex->mutex);
}

int k_msgq_init(struct k_msgq *msgq)
{
    msgq->mq = osMessageQueueNew(msgq->msg_count, msgq->msg_size, NULL);
    if(msgq->mq == NULL){
        printf("zubs create msgq fail\r\n");
        return -1;
    }
    return 0;
}

int k_msgq_put(struct k_msgq *msgq, const void *data, uint32_t timeout)
{
    if(msgq->mq == NULL){
        int ret = k_msgq_init(msgq);
        if(ret != 0){
            return -1;
        }
    }

    return osMessageQueuePut(msgq->mq, data, 0, timeout);
}

int k_msgq_get(struct k_msgq *msgq, void *data, uint32_t timeout)
{
    if(msgq->mq == NULL){
        int ret = k_msgq_init(msgq);
        if(ret != 0){
            return -1;
        }
    }

    return osMessageQueueGet(msgq->mq, data, 0, timeout);
}

void k_msgq_destory(struct k_msgq *msgq)
{
    osMessageQueueDelete(msgq->mq);
}

void k_thread_init(struct k_thread *thread)
{
    // osThreadAttr_t attr = {
    //     .
    // };
    thread->thid = osThreadNew(thread->thread_entry, NULL, NULL);
    if(thread->thid == NULL){
        printf("zubs create thread fail\r\n");
    }
}

void k_thread_destory(struct k_thread *thread)
{

}