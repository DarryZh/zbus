/*
 * Copyright (c) 2022 Rodrigo Peixoto <rodrigopex@gmail.com>
 * SPDX-License-Identifier: Apache-2.0
 */
#include <stdint.h>
#include <zbus.h>

static bool print_channel_data_iterator(const struct zbus_channel *chan, void *user_data)
{
	int *count = user_data;

	printf("%d - Channel %s:\r\n", *count, zbus_chan_name(chan));
	printf("      Message size: %d\r\n", zbus_chan_msg_size(chan));
	printf("      Observers:\r\n");

	++(*count); 	

	struct zbus_channel_observation *observation;

	for (int16_t i = chan->data->observers_start_idx, limit = chan->data->observers_end_idx;
	     i < limit; ++i) {
		STRUCT_SECTION_GET(zbus_channel_observation, i, &observation);

		__ASSERT(observation != NULL, "observation must be not NULL");

		printf("      - %s\r\n", observation->obs->name);
	}

#ifdef CONFIG_ZBUS_RUNTIME_OBSERVERS
	struct zbus_observer_node *obs_nd, *tmp;

	SYS_SLIST_FOR_EACH_CONTAINER_SAFE(&chan->data->observers, obs_nd, tmp, node) {
		printf("      - %s", obs_nd->obs->name);
	}
#endif
	return true;
}

static bool print_observer_data_iterator(const struct zbus_observer *obs, void *user_data)
{
	int *count = user_data;

	printf("%d - %s %s\r\n", *count,
		obs->type == ZBUS_OBSERVER_LISTENER_TYPE ? "Listener" : "Subscriber",
		zbus_obs_name(obs));

	++(*count);

	return true;
}

#define CONSUMER_STACK_SIZE (1024*2)
#define PRODUCER_STACK_SIZE (1024*2)

#define CONFIG_BM_ONE_TO	16
#define CONFIG_BM_ASYNC		1

#define MSEC_PER_SEC		1000

#define GET_ARCH_TIME_MS()			osKernelGetTickCount()

#define CONFIG_BM_MESSAGE_SIZE		256


/** @brief Number of bytes in @p x kibibytes */
#define KB(x) (((size_t)x) << 10)
/** @brief Number of bytes in @p x mebibytes */
#define MB(x) (KB(x) << 10)
/** @brief Number of bytes in @p x gibibytes */
#define GB(x) (MB(x) << 10)

/** @brief Number of Hz in @p x kHz */
#define KHZ(x) ((x) * 1000)
/** @brief Number of Hz in @p x MHz */
#define MHZ(x) (KHZ(x) * 1000)

struct bm_msg {
	uint8_t bytes[CONFIG_BM_MESSAGE_SIZE];
};

ZBUS_CHAN_DEFINE(bm_channel,	/* Name */
		 struct bm_msg, /* Message type */

		 NULL, /* Validator */
		 NULL, /* User data */
		 ZBUS_OBSERVERS(s1

#if (CONFIG_BM_ONE_TO >= 2LLU)
				,
				s2
#if (CONFIG_BM_ONE_TO > 2LLU)
				,
				s3, s4
#if (CONFIG_BM_ONE_TO > 4LLU)
				,
				s5, s6, s7, s8
#if (CONFIG_BM_ONE_TO > 8LLU)
				,
				s9, s10, s11, s12, s13, s14, s15, s16
#endif
#endif
#endif
#endif
				), /* observers */
		 ZBUS_MSG_INIT(0)  /* Initial value {0} */
);

#define BYTES_TO_BE_SENT (256LLU * 1024LLU)
static long count;

#if (CONFIG_BM_ASYNC == 1)
ZBUS_SUBSCRIBER_DEFINE(s1, 4);
#if (CONFIG_BM_ONE_TO >= 2LLU)
ZBUS_SUBSCRIBER_DEFINE(s2, 4);
#if (CONFIG_BM_ONE_TO > 2LLU)
ZBUS_SUBSCRIBER_DEFINE(s3, 4);
ZBUS_SUBSCRIBER_DEFINE(s4, 4);
#if (CONFIG_BM_ONE_TO > 4LLU)
ZBUS_SUBSCRIBER_DEFINE(s5, 4);
ZBUS_SUBSCRIBER_DEFINE(s6, 4);
ZBUS_SUBSCRIBER_DEFINE(s7, 4);
ZBUS_SUBSCRIBER_DEFINE(s8, 4);
#if (CONFIG_BM_ONE_TO > 8LLU)
ZBUS_SUBSCRIBER_DEFINE(s9, 4);
ZBUS_SUBSCRIBER_DEFINE(s10, 4);
ZBUS_SUBSCRIBER_DEFINE(s11, 4);
ZBUS_SUBSCRIBER_DEFINE(s12, 4);
ZBUS_SUBSCRIBER_DEFINE(s13, 4);
ZBUS_SUBSCRIBER_DEFINE(s14, 4);
ZBUS_SUBSCRIBER_DEFINE(s15, 4);
ZBUS_SUBSCRIBER_DEFINE(s16, 4);
#endif
#endif
#endif
#endif

#define S_TASK(name)                                                                               \
	void name##_task(void)                                                                     \
	{                                                                                          \
		const struct zbus_channel *chan;                                                   \
		struct bm_msg *msg_received;                                                       \
                                                                                                   \
		while (!zbus_sub_wait(&name, &chan, K_FOREVER)) {                                  \
			zbus_chan_claim(chan, 0);                                          \
                                                                                                   \
			msg_received = zbus_chan_msg(chan);                                        \
                                                                                                   \
			zbus_chan_finish(chan);                                                    \
			/*zbus_chan_read(chan, &msg_received, 100);*/	\
                                                                                                   \
			count += CONFIG_BM_MESSAGE_SIZE;                                \
		}                                                                                  \
	}                                                                                          \
                                                                                                   \
	K_THREAD_DEFINE(name##_id, CONSUMER_STACK_SIZE, name##_task, 3);

S_TASK(s1)
#if (CONFIG_BM_ONE_TO >= 2LLU)
S_TASK(s2)
#if (CONFIG_BM_ONE_TO > 2LLU)
S_TASK(s3)
S_TASK(s4)
#if (CONFIG_BM_ONE_TO > 4LLU)
S_TASK(s5)
S_TASK(s6)
S_TASK(s7)
S_TASK(s8)
#if (CONFIG_BM_ONE_TO > 8LLU)
S_TASK(s9)
S_TASK(s10)
S_TASK(s11)
S_TASK(s12)
S_TASK(s13)
S_TASK(s14)
S_TASK(s15)
S_TASK(s16)
#endif
#endif
#endif
#endif

#else /* SYNC */

static void s_cb(const struct zbus_channel *chan);

ZBUS_LISTENER_DEFINE(s1, s_cb);

#if (CONFIG_BM_ONE_TO >= 2LLU)
ZBUS_LISTENER_DEFINE(s2, s_cb);
#if (CONFIG_BM_ONE_TO > 2LLU)
ZBUS_LISTENER_DEFINE(s3, s_cb);
ZBUS_LISTENER_DEFINE(s4, s_cb);
#if (CONFIG_BM_ONE_TO > 4LLU)
ZBUS_LISTENER_DEFINE(s5, s_cb);
ZBUS_LISTENER_DEFINE(s6, s_cb);
ZBUS_LISTENER_DEFINE(s7, s_cb);
ZBUS_LISTENER_DEFINE(s8, s_cb);
#if (CONFIG_BM_ONE_TO > 8LLU)
ZBUS_LISTENER_DEFINE(s9, s_cb);
ZBUS_LISTENER_DEFINE(s10, s_cb);
ZBUS_LISTENER_DEFINE(s11, s_cb);
ZBUS_LISTENER_DEFINE(s12, s_cb);
ZBUS_LISTENER_DEFINE(s13, s_cb);
ZBUS_LISTENER_DEFINE(s14, s_cb);
ZBUS_LISTENER_DEFINE(s15, s_cb);
ZBUS_LISTENER_DEFINE(s16, s_cb);
#endif
#endif
#endif
#endif

static void s_cb(const struct zbus_channel *chan)
{
	const struct bm_msg *actual_message_data = zbus_chan_const_msg(chan);

	/* It only illustrates the message is ready to be consumed */
	ARG_UNUSED(actual_message_data);

	count += CONFIG_BM_MESSAGE_SIZE;
}

#endif /* CONFIG_BM_ASYNC */

static void producer_thread(void)
{
	printf("Benchmark 1 to %d: Dynamic memory, %sSYNC transmission and message size %u\r\n",
		CONFIG_BM_ONE_TO, IS_ENABLED(CONFIG_BM_ASYNC) ? "A" : "", CONFIG_BM_MESSAGE_SIZE);

	struct bm_msg msg;

	for (uint64_t i = (CONFIG_BM_MESSAGE_SIZE - 1); i > 0; --i) {
		msg.bytes[i] = i;
	}

	uint64_t start_ns = GET_ARCH_TIME_MS();

	for (uint64_t internal_count = BYTES_TO_BE_SENT / CONFIG_BM_ONE_TO; internal_count > 0;
	     internal_count -= CONFIG_BM_MESSAGE_SIZE) {
		zbus_chan_pub(&bm_channel, &msg, (200));
	}

	uint64_t end_ns = GET_ARCH_TIME_MS();

	uint64_t duration = end_ns - start_ns;

	if (duration == 0) {
		printf("Something wrong. Duration is zero!\r\n");
		assert(0);
	}
	uint64_t i = ((BYTES_TO_BE_SENT * MSEC_PER_SEC) / MB(1)) / duration;
	uint64_t f = ((BYTES_TO_BE_SENT * MSEC_PER_SEC * 100) / MB(1) / duration) % 100;

	printf("Bytes sent = %lld, received = %lu\r\n", BYTES_TO_BE_SENT, count);
	printf("Average data rate: %llu.%lluMB/s\r\n", i, f);
	printf("Duration: %lld.%09llus\r\n", duration / MSEC_PER_SEC, duration % MSEC_PER_SEC);

	count = 0;
	printf("\r\n@%llu\r\n", duration);
	osThreadExit();
}

K_THREAD_DEFINE(producer_thread_id, PRODUCER_STACK_SIZE, producer_thread, 3);

extern int _zbus_init(void);
extern void k_thread_init(struct k_thread *thread);

void zbus_benchmark(void){
	k_thread_init(&producer_thread_id);
	k_thread_init(&s1_id);
	#if (CONFIG_BM_ONE_TO >= 2LLU)
	k_thread_init(&s2_id);
	#if (CONFIG_BM_ONE_TO > 2LLU)
	k_thread_init(&s3_id);
	k_thread_init(&s4_id);
	#if (CONFIG_BM_ONE_TO > 4LLU)
	k_thread_init(&s5_id);
	k_thread_init(&s6_id);
	k_thread_init(&s7_id);
	k_thread_init(&s8_id);
	#if (CONFIG_BM_ONE_TO > 8LLU)
	k_thread_init(&s9_id);
	k_thread_init(&s10_id);
	k_thread_init(&s11_id);
	k_thread_init(&s12_id);
	k_thread_init(&s13_id);
	k_thread_init(&s14_id);
	k_thread_init(&s15_id);
	k_thread_init(&s16_id);
	#endif
	#endif
	#endif
	#endif
}

void zbus_dump(void){
	int count = 0;

	printf("Channel list:\r\n");
	zbus_iterate_over_channels_with_user_data(print_channel_data_iterator, &count);

	count = 0;

	printf("Observers list:\r\n");
	zbus_iterate_over_observers_with_user_data(print_observer_data_iterator, &count);
}

void main(void)
{
  	_zbus_init();

	zbus_dump();
	zbus_benchmark();
	osDelay(osWaitForever);
	while(1);
}
