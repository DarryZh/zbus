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
    //检查消息队列是否存在
	msgq->mq = msgget((int)msgq->name, IPC_EXCL);//(键名,权限)
	if (msgq->mq < 0)
	{
		//创建消息队列
		msgq->mq = msgget((int)msgq->name, IPC_CREAT | 0666);
		if (msgq->mq <0)
		{
			printf("failed to create msq | errno=%d [%s]\n", errno, strerror(errno));
			return -1;
		}
	}
        
    return 0;
}

int k_msgq_put(struct k_msgq *msgq, const void *data, uint32_t timeout)
{
    int ret_value;

    if(msgq->mq <= 0){
        if(k_msgq_init(msgq)){
            return -1;
        }
    }

    //发送消息队列(sizeof消息的长度，而不是整个结构体的长度)
    ret_value = msgsnd(msgq->mq, data, msgq->attr.mq_msgsize, IPC_NOWAIT);
    if (ret_value < 0)
    {
        printf("msgsnd() write msg failed,errno=%d[%s]\n", errno, strerror(errno));
    }
    return ret_value;
}

int k_msgq_get(struct k_msgq *msgq, void *data, uint32_t timeout)
{
    int ret_value;

    if(msgq->mq <= 0){
        if(k_msgq_init(msgq)){
            return -1;
        }
    }

    //发送消息队列(sizeof消息的长度，而不是整个结构体的长度)
    ret_value = msgrcv(msgq->mq, data, msgq->attr.mq_msgsize, 0, IPC_NOWAIT);
    if (ret_value > 0)
    {
        // printf("read msg:%s\n", msgs.msgtext);
        return 0;
    }

    return -1;
}

void k_msgq_destory(struct k_msgq *msgq)
{
    if(msgq->mq > 0){
        msgctl(msgq->mq, IPC_RMID, 0);
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
