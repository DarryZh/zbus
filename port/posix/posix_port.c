#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <zbus_config.h>
#include <zbus_kernel.h>

#include <time.h>
#include <pthread.h>
#include <mqueue.h>

void k_mutex_init(struct k_mutex * mutex)
{
    pthread_mutex_init(&(mutex->mutex), NULL);
}

int k_mutex_lock(struct k_mutex * mutex, uint32_t timeout)
{
    return pthread_mutex_lock(&(mutex->mutex));
}

int k_mutex_unlock(struct k_mutex * mutex)
{
    return pthread_mutex_unlock(&(mutex->mutex));
}

void k_mutex_destory(struct k_mutex * mutex)
{
    pthread_mutex_destroy(&(mutex->mutex));
}

int k_msgq_init(struct k_msgq *msgq)
{
    char destination[64]={"/"};
    strcat(destination, msgq->name);

    // printf("[k_msg_init] name:%s, max msg:%ld, msg size:%ld\r\n", destination, msgq->attr.mq_maxmsg, msgq->attr.mq_msgsize);

    msgq->mq = mq_open(destination, O_CREAT|O_TRUNC, 0666, &(msgq->attr));
    if(msgq->mq > 0){
        mq_close(msgq->mq);
        // printf("mqueue creat succ qid:%d\r\n", msgq->mq);
        return 0;
    } else {
        perror("[k_msgq_init]");
        return -1;
    }
}

int k_msgq_put(struct k_msgq *msgq, const void *data, uint32_t timeout)
{
    if(msgq->mq <= 0){
        if(k_msgq_init(msgq)){
            return -1;
        }
    }

    char destination[64]={"/"};
    strcat(destination, msgq->name);
    msgq->mq = mq_open(destination, O_WRONLY, 0666, &(msgq->attr));

    if(msgq->mq <= 0){
        printf("mqueue put open fail\r\n");
        return -1;
    }

    int ret = mq_send(msgq->mq, data, msgq->attr.mq_msgsize, 0);
    if(ret < 0){
        printf("msgq send fail, ret = %d\r\n", ret);
    }
    mq_close(msgq->mq);
    return ret;
}

int k_msgq_get(struct k_msgq *msgq, void *data, uint32_t timeout)
{
    if(msgq->mq <= 0){
        if(k_msgq_init(msgq)){
            return -1;
        }
    }

    char destination[64]={"/"};
    strcat(destination, msgq->name);
    msgq->mq = mq_open(destination, O_RDONLY, 0666, &(msgq->attr));
    if(msgq->mq <= 0){
        printf("mqueue get open fail\r\n");
        return -1;
    }

    unsigned int priority = 0;
    int ret = mq_receive(msgq->mq, data, msgq->attr.mq_msgsize, &priority);
    // printf("mq recev data len: %d\r\n", ret);
    if(ret < 0){
        printf("msgq receive fail, ret = %d\r\n", ret);    
    } else {
        ret = 0;
    }
    mq_close(msgq->mq);
    return ret;
}

void k_msgq_destory(struct k_msgq *msgq)
{
    if(msgq->mq > 0){
        char destination[64]={"/"};
        strcat(destination, msgq->name);
        mq_unlink(destination);
    }
}

void k_thread_init(struct k_thread *thread)
{
    if (pthread_create(&(thread->thid), NULL, thread->thread_entry, NULL) != 0) {
        printf("pthread_create() error");
    }
    if (pthread_detach(thread->thid) != 0) {
        printf("pthread_detach() error");
    }
}

void k_thread_destory(struct k_thread *thread)
{

}