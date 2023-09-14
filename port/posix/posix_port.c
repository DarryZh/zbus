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
    // printf("%s    \r\n", destination);
    msgq->mq = mq_open(destination, O_CREAT|O_EXCL, 0777, &(msgq->attr));
    if(msgq->mq != -1){
        mq_close(msgq->mq);
        printf("mqueue creat succ qid:%d\r\n", msgq->mq);
        return 0;
    } else {
        printf("mqueue creat fail\r\n");
        return -1;
    }
}

int k_msgq_put(struct k_msgq *msgq, const void *data, uint32_t timeout)
{
    if(msgq->mq == 0){
        k_msgq_init(msgq);
    }

    char destination[64]={"/"};
    strcat(destination, msgq->name);
    // printf("%s    \r\n", destination);
    msgq->mq = mq_open(destination, O_WRONLY, 0777, &(msgq->attr));

    struct timespec tv;
    clock_gettime(CLOCK_REALTIME, &tv);
    tv.tv_sec += 1;

    // int ret = mq_timedsend(msgq->mq, data, sizeof(void *), 0, &tv);
    // printf("send data : %x, size : %d\r\n", data, sizeof(void *));
    int ret = mq_send(msgq->mq, data, sizeof(void *), 0);
    // int ret = -1;
    if(ret == -1){
        printf("msgq send fail\r\n");
    } else {
        // printf("msgq send ret %d\r\n", ret);
    }
    mq_close(msgq->mq);
    return ret;
}

int k_msgq_get(struct k_msgq *msgq, void *data, uint32_t timeout)
{
    if(msgq->mq == 0){
        k_msgq_init(msgq);
    }

    char destination[64]={"/"};
    strcat(destination, msgq->name);
    // printf("%s    \r\n", destination);
    msgq->mq = mq_open(destination, O_RDONLY, 0777, &(msgq->attr));
    if(msgq->mq != -1){
        // mq_close(msgq->mq);
        // printf("mqueue get open succ qid:%d\r\n", msgq->mq);
        // return 0;
    } else {
        // printf("mqueue get open fail\r\n");
        // return -1;
    }

    struct timespec tv;
    clock_gettime(CLOCK_REALTIME, &tv);
    tv.tv_sec += 10;
    unsigned int priority = 0;

    // int ret = mq_timedreceive(msgq->mq, data, sizeof(void *), NULL, &tv);
    int ret = mq_receive(msgq->mq, data, sizeof(void *), &priority);
    // printf("recev data : %x\r\n");
    // int ret = -1;
    if(ret == -1){
        // printf("msgq receive fail\r\n");    
    } else {
        // printf("msgq receive ret %d\r\n", ret);
    }
    mq_close(msgq->mq);
    return ret;
}

void k_msgq_destory(struct k_msgq *msgq)
{

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