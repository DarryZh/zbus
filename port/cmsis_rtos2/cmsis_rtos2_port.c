#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <zbus_config.h>
#include <zbus_kernel.h>

#include "cmsis_os2.h"

int k_mutex_init(struct k_mutex * mutex)
{
    if(mutex){
        if(mutex->mutex == NULL){
            mutex->mutex = osMutexNew(NULL);
            if(mutex->mutex == NULL){
                printf("[zbus] create mutex fail\r\n");
                return -1;
            }
        }
        return 0;
    }
    printf("[zbus] struct k_mutex is NULL\r\n");
    return -1;
}

int k_mutex_lock(struct k_mutex * mutex, uint32_t timeout)
{
    if(mutex){
        if(mutex->mutex == NULL){
            if(k_mutex_init(mutex) != 0){
                return -1;
            }
        }
        return osMutexAcquire(mutex->mutex, timeout);
    }
    printf("[zbus] struct k_mutex is NULL\r\n");
    return -1;
}

int k_mutex_unlock(struct k_mutex * mutex)
{
    if(mutex){
        if(mutex->mutex == NULL){
            if(k_mutex_init(mutex) != 0){
                return -1;
            }
        }
        return osMutexRelease(mutex->mutex);
    }
    printf("[zbus] struct k_mutex is NULL\r\n");
    return -1;
}

void k_mutex_destory(struct k_mutex * mutex)
{
    if(mutex){
        if(mutex->mutex){
            osMutexDelete(mutex->mutex);
            mutex->mutex = NULL;
        }
        return ;
    }
    printf("[zbus] struct k_mutex is NULL\r\n");
}


int k_msgq_init(struct k_msgq *msgq)
{
    if(msgq){
        if(msgq->mq == NULL){
            msgq->mq = osMessageQueueNew(msgq->msg_count, msgq->msg_size, NULL);
            if(msgq->mq == NULL){
                printf("[zbus] create msgq fail\r\n");
                return -1;
            }
        }
        return 0;
    }
    printf("[zbus] struct k_msgq is NULL\r\n");
    return -1;
}

int k_msgq_put(struct k_msgq *msgq, const void *data, uint32_t timeout)
{
    if(msgq){
        if(msgq->mq == NULL){
            int ret = k_msgq_init(msgq);
            if(ret != 0){
                return -1;
            }
        }
        return osMessageQueuePut(msgq->mq, data, 0, timeout);
    }
    printf("[zbus] struct k_msgq is NULL\r\n");
    return -1;
}

int k_msgq_get(struct k_msgq *msgq, void *data, uint32_t timeout)
{
    if(msgq){
        if(msgq->mq == NULL){
            int ret = k_msgq_init(msgq);
            if(ret != 0){
                return -1;
            }
        }
        return osMessageQueueGet(msgq->mq, data, 0, timeout);
    }
    printf("[zbus] struct k_msgq is NULL\r\n");
    return -1;
}

void k_msgq_destory(struct k_msgq *msgq)
{
    if(msgq){
        if(msgq->mq){
            osMessageQueueDelete(msgq->mq);
            msgq->mq = 0;
        }
        return ;
    }
    printf("[zbus] struct k_msgq is NULL\r\n");
}

void k_thread_init(struct k_thread *thread)
{
    if(thread){
        const osThreadAttr_t attr = {
            .priority = thread->thread_prio,
        };
        if(thread->thid == NULL){
            thread->thid = osThreadNew(thread->thread_entry, NULL, &attr);
            if(thread->thid == NULL){
                printf("[zbus] create thread fail\r\n");
            }
        }
        return ;
    }
    printf("[zbus] struct k_thread is NULL\r\n");
}

void k_thread_destory(struct k_thread *thread)
{
    if(thread){
        if(thread->thid){
            osThreadTerminate(thread->thid);
            thread->thid = 0;
        }
        return ;
    }
    printf("[zbus] struct k_thread is NULL\r\n");
}
