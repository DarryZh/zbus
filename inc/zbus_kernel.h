/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
#ifndef __ZBUS_KERNEL_H__
#define __ZBUS_KERNEL_H__

#include <toolchain/zbus_gcc.h>
#include <zbus_util_macro.h>
#include <zbus_errno_base.h>

#ifdef	CONFIG_ZBUS_POSIX
#include <pthread.h>
#include <mqueue.h>
#elif defined CONFIG_ZBUS_CMSIS_RTOS2
#include "cmsis_os2.h"
#else 
#error	Please choose a config (CONFIG_ZBUS_POSIX CONFIG_ZBUS_CMSIS_RTOS2) 
#endif

#define	ENOMSG		42	/* No message of desired type */

#define K_FOREVER	-1

struct k_mutex {
#ifdef	CONFIG_ZBUS_POSIX
	pthread_mutex_t mutex;
#elif defined CONFIG_ZBUS_CMSIS_RTOS2
	osMutexId_t mutex;
#else 
#error	Please choose a config (CONFIG_ZBUS_POSIX CONFIG_ZBUS_CMSIS_RTOS2) 
#endif
};

/**
 * @brief Message Queue Structure
 */
struct k_msgq {
#ifdef	CONFIG_ZBUS_POSIX
	const char *name;
	mqd_t mq;
	struct mq_attr attr;
#elif defined CONFIG_ZBUS_CMSIS_RTOS2
	osMessageQueueId_t mq;
	uint32_t msg_count;
	uint32_t msg_size;
#else 
#error	Please choose a config (CONFIG_ZBUS_POSIX CONFIG_ZBUS_CMSIS_RTOS2) 
#endif
};

/**
 * @ingroup thread_apis
 * Thread Structure
 */
struct k_thread {
#ifdef	CONFIG_ZBUS_POSIX
	const char *name;
	pthread_t thid;
	uint32_t stack_size;
	void *thread_entry;
	uint32_t thread_prio;
#elif defined CONFIG_ZBUS_CMSIS_RTOS2
	const char *name;
	osThreadId_t thid;
	uint32_t stack_size;
	void *thread_entry;
	uint32_t thread_prio;
#else 
#error	Please choose a config (CONFIG_ZBUS_POSIX CONFIG_ZBUS_CMSIS_RTOS2) 
#endif
};

/**
 * @cond INTERNAL_HIDDEN
 */
#ifdef	CONFIG_ZBUS_POSIX	
#define Z_MUTEX_INITIALIZER(obj) \
	{ \
		0,	\
	}
#elif defined CONFIG_ZBUS_CMSIS_RTOS2	
#define Z_MUTEX_INITIALIZER(obj) \
	{ \
		NULL,	\
	}											
#else 	
#error	Please choose a config (CONFIG_ZBUS_POSIX CONFIG_ZBUS_CMSIS_RTOS2)	
#endif

/**
 * INTERNAL_HIDDEN @endcond
 */

/**
 * @brief Statically define and initialize a mutex.
 *
 * The mutex can be accessed outside the module where it is defined using:
 *
 * @code extern struct k_mutex <name>; @endcode
 *
 * @param name Name of the mutex.
 */
#define K_MUTEX_DEFINE(name) \
	struct k_mutex name = \
		 Z_MUTEX_INITIALIZER(name)

/**
 * @cond INTERNAL_HIDDEN
 */

#ifdef	CONFIG_ZBUS_POSIX	
#define Z_MSGQ_INITIALIZER(obj, q_buffer, q_msg_size, q_max_msgs) \
	{ \
		.name = STRINGIFY(obj),		\
		.mq = 0,			\
		.attr.mq_flags = 0,	\
		.attr.mq_maxmsg = q_max_msgs,	\
		.attr.mq_msgsize = q_msg_size,	\
		.attr.mq_curmsgs = 0,				\
	}
#elif defined CONFIG_ZBUS_CMSIS_RTOS2	
#define Z_MSGQ_INITIALIZER(obj, q_buffer, q_msg_size, q_max_msgs) \
	{ \
		.mq = NULL,			\
		.msg_count = q_max_msgs,	\
		.msg_size = q_msg_size,		\
	}		
#else 	
#error	Please choose a config (CONFIG_ZBUS_POSIX CONFIG_ZBUS_CMSIS_RTOS2)	
#endif

/**
 * @brief Statically define and initialize a message queue.
 *
 * The message queue's ring buffer contains space for @a q_max_msgs messages,
 * each of which is @a q_msg_size bytes long. The buffer is aligned to a
 * @a q_align -byte boundary, which must be a power of 2. To ensure that each
 * message is similarly aligned to this boundary, @a q_msg_size must also be
 * a multiple of @a q_align.
 *
 * The message queue can be accessed outside the module where it is defined
 * using:
 *
 * @code extern struct k_msgq <name>; @endcode
 *
 * @param q_name Name of the message queue.
 * @param q_msg_size Message size (in bytes).
 * @param q_max_msgs Maximum number of messages that can be queued.
 * @param q_align Alignment of the message queue's ring buffer.
 *
 */
#define K_MSGQ_DEFINE(q_name, q_max_msgs, q_msg_size, q_align)	\
	struct k_msgq q_name = \
			Z_MSGQ_INITIALIZER(q_name, NULL, (q_msg_size), (q_max_msgs))

#ifdef	CONFIG_ZBUS_POSIX	
#define Z_THREAD_INITIALIZER(t_name, t_stack_size, entry, prio) \
	{ \
		.name = STRINGIFY(t_name),		\
		.thid = 0,	\
		.stack_size = t_stack_size,	\
		.thread_entry = entry,	\
		.thread_prio = prio,	\
	}
#elif defined CONFIG_ZBUS_CMSIS_RTOS2	
#define Z_THREAD_INITIALIZER(t_name, t_stack_size, entry, prio) \
	{ \
		.name = STRINGIFY(t_name),		\
		.thid = NULL,	\
		.stack_size = t_stack_size,	\
		.thread_entry = entry,	\
		.thread_prio = prio,	\
	}											
#else 	
#error	Please choose a config (CONFIG_ZBUS_POSIX CONFIG_ZBUS_CMSIS_RTOS2)	
#endif

#define K_THREAD_DEFINE(t_name, t_stack_size, entry, prio)	\
	struct k_thread t_name = \
			Z_THREAD_INITIALIZER(t_name, t_stack_size, entry, prio)                          	


#endif

