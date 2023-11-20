#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <zbus_config.h>
#include <zbus_kernel.h>

#include <pthread.h>

#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>

void k_mutex_init(struct k_mutex * mutex)
{
    if(mutex){
        printf("[port] mutex is %p\r\n", &(mutex->mutex));
        pthread_mutex_init(&(mutex->mutex), NULL);
        return ;
    }
    printf("[zbus] struct k_mutex is NULL\r\n");
}

int k_mutex_lock(struct k_mutex * mutex, uint32_t timeout)
{
    if(mutex){
        // printf("[port] mutex lock %p\r\n", &(mutex->mutex));
        return pthread_mutex_lock(&(mutex->mutex));
    }
    printf("[zbus] struct k_mutex is NULL\r\n");
    return -1;
}

int k_mutex_unlock(struct k_mutex * mutex)
{
    if(mutex){
        // printf("[port] mutex unlock %p\r\n", &(mutex->mutex));
        return pthread_mutex_unlock(&(mutex->mutex));
    }
    printf("[zbus] struct k_mutex is NULL\r\n");
    return -1;
}

void k_mutex_destory(struct k_mutex * mutex)
{
    if(mutex){
        pthread_mutex_destroy(&(mutex->mutex));
        return ;
    }
    printf("[zbus] struct k_mutex is NULL\r\n");
}

int k_msgq_init(struct k_msgq *msgq)
{
    static int key = 0;
    if(msgq){
        if(msgq->mq == 0){
            //检查消息队列是否存在
            key++;
            msgq->mq = msgget(key, IPC_EXCL);//(键名,权限)
            if (msgq->mq < 0)
            {
                //创建消息队列
                msgq->mq = msgget(key, IPC_CREAT | 0666);
                if (msgq->mq < 0)
                {
                    printf("failed to create msq | errno=%d [%s]\n", errno, strerror(errno));
                    return -1;
                }
            }
        }
        return 0;
    }
    printf("[zbus] struct k_msgq is NULL\r\n");
    return -1;
}

int k_msgq_put(struct k_msgq *msgq, const void *data, uint32_t timeout)
{
    int ret_value;
    if(msgq){
        if(msgq->mq <= 0){
            printf("[port] msgq is not init\r\n");
            return -1;
        }

        //发送消息队列(sizeof消息的长度，而不是整个结构体的长度)
        ret_value = msgsnd(msgq->mq, data, msgq->attr.mq_msgsize, 0);
        if (ret_value < 0)
        {
            printf("msgsnd() write msg failed,errno=%d[%s]\n", errno, strerror(errno));
        }
        return ret_value;
    }
    printf("[zbus] struct k_msgq is NULL\r\n");
    return -1;
}

int k_msgq_get(struct k_msgq *msgq, void *data, uint32_t timeout)
{
    int ret_value;
    if(msgq){
        if(msgq->mq <= 0){
            // printf("[port] msgq is not init\r\n");
            return -1;
        }

        //发送消息队列(sizeof消息的长度，而不是整个结构体的长度)
        ret_value = msgrcv(msgq->mq, data, msgq->attr.mq_msgsize, 0, 0);
        if (ret_value > 0)
        {
            return 0;
        } else {
            return -1;
        }
    }
    printf("[zbus] struct k_msgq is NULL\r\n");
    return -1;
}

void k_msgq_destory(struct k_msgq *msgq)
{
    if(msgq){
        if(msgq->mq > 0){
            msgctl(msgq->mq, IPC_RMID, 0);
            msgq->mq = 0;
        }
        return ;
    }
    printf("[zbus] struct k_msgq is NULL\r\n");
}

void k_thread_init(struct k_thread *thread)
{
    if(thread){
        if(thread->thid == 0){
            pthread_attr_t attr;
            if (pthread_attr_init(&attr) != 0){
                printf("pthread_attr_init() error\r\n");
            }
            if (pthread_attr_setstacksize(&attr, thread->stack_size) != 0){
                printf("pthread_attr_setstacksize() error, thread->stack_size is %d\r\n", thread->stack_size);
            }
            if (pthread_create(&(thread->thid), &attr, thread->thread_entry, NULL) != 0) {
                printf("pthread_create() error\r\n");
            }
            if (pthread_detach(thread->thid) != 0) {
                printf("pthread_detach() error\r\n");
            }
            return ;
        }
    }
    printf("[zbus] struct k_thread is NULL\r\n");
}

void k_thread_destory(struct k_thread *thread)
{
    if(thread){
        if(thread->thid != 0){
            pthread_cancel(thread->thid);
            thread->thid = 0;
        }
        return ;
    }
    printf("[zbus] struct k_thread is NULL\r\n");
}
