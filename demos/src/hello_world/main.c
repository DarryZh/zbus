/*
 * Copyright (c) 2022 Rodrigo Peixoto <rodrigopex@gmail.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <zbus_config.h>
#include <zbus_kernel.h>
#include <zbus.h>

struct version_msg {
	uint8_t major;
	uint8_t minor;
	uint16_t build;
};

struct acc_msg {
	int x;
	int y;
	int z;
};

ZBUS_CHAN_DEFINE(version_chan,       /* Name */
		 struct version_msg, /* Message type */

		 NULL,                 /* Validator */
		 NULL,                 /* User data */
		 ZBUS_OBSERVERS_EMPTY, /* observers */
		 ZBUS_MSG_INIT(.major = 0, .minor = 1,
			       .build = 2) /* Initial value major 0, minor 1, build 2 */
);

ZBUS_CHAN_DEFINE(acc_data_chan,  /* Name */
		 struct acc_msg, /* Message type */

		 NULL,                                 /* Validator */
		 NULL,                                 /* User data */
		 ZBUS_OBSERVERS(foo_lis, bar_sub),     /* observers */
		 ZBUS_MSG_INIT(.x = 0, .y = 0, .z = 0) /* Initial value */
);

static bool simple_chan_validator(const void *msg, size_t msg_size)
{
	ARG_UNUSED(msg_size);

	const int *simple = msg;

	if ((*simple >= 0) && (*simple < 10)) {
		return true;
	}

	return false;
}

ZBUS_CHAN_DEFINE(simple_chan, /* Name */
		 int,         /* Message type */

		 simple_chan_validator, /* Validator */
		 NULL,                  /* User data */
		 ZBUS_OBSERVERS_EMPTY,  /* observers */
		 0                      /* Initial value is 0 */
);

static void listener_callback_example(const struct zbus_channel *chan)
{
	const struct acc_msg *acc = zbus_chan_const_msg(chan);

	printf("From listener -> Acc x=%d, y=%d, z=%d\r\n", acc->x, acc->y, acc->z);
}

ZBUS_LISTENER_DEFINE(foo_lis, listener_callback_example);

ZBUS_SUBSCRIBER_DEFINE(bar_sub, 4);

static void subscriber_task(void)
{
	printf("subscriber thread start\r\n");
	const struct zbus_channel *chan;
	while(1){
		while (!zbus_sub_wait(&bar_sub, &chan, K_FOREVER)) {
			struct acc_msg acc;
			if (&acc_data_chan == chan) {
				zbus_chan_read(&acc_data_chan, &acc, 100);

				printf("From subscriber -> Acc x=%d, y=%d, z=%d\r\n", acc.x, acc.y, acc.z);
			}
		}
	}
}
K_THREAD_DEFINE(subscriber_user_task, 2048, subscriber_task, 3);

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


extern int _zbus_init(void);
extern void k_thread_init(struct k_thread *thread);
extern void k_msgq_destory(struct k_msgq *msgq);
int main(void)
{
	_zbus_init();
	k_thread_init(&subscriber_user_task);
	usleep(10);

	int err, value;
	struct acc_msg acc1 = {.x = 1, .y = 1, .z = 1};
	const struct version_msg *v = zbus_chan_const_msg(&version_chan);

	printf("Sensor sample started raw reading, version %u.%u-%u!\r\n", v->major, v->minor,
		v->build);

	int count = 0;

	printf("Channel list:\r\n");
	zbus_iterate_over_channels_with_user_data(print_channel_data_iterator, &count);

	count = 0;

	printf("Observers list:\r\n");
	zbus_iterate_over_observers_with_user_data(print_observer_data_iterator, &count);

	zbus_chan_pub(&acc_data_chan, &acc1, 100);

	acc1.x = 3;
	acc1.y = 3;
	acc1.z = 3;
	zbus_chan_pub(&acc_data_chan, &(acc1), 100);

	value = 5;
	err = zbus_chan_pub(&simple_chan, &value, 100);

	if (err == 0) {
		printf("Pub a valid value to a channel with validator successfully.\r\n");
	}

	value = 15;
	err = zbus_chan_pub(&simple_chan, &value, 100);

	if (err == -ENOMSG) {
		printf("Pub an invalid value to a channel with validator successfully.\r\n");
	}

	usleep(10);
	k_msgq_destory(&_zbus_observer_queue_bar_sub);

	return 0;
}

