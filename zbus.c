/*
 * Copyright (c) 2022 Rodrigo Peixoto <rodrigopex@gmail.com>
 * SPDX-License-Identifier: Apache-2.0
 */
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <zbus_config.h>
#include <zbus_kernel.h>
#include <zbus_iterable_sections.h>
#include <zbus.h>

extern void k_mutex_init(struct k_mutex * mutex);

extern int k_mutex_lock(struct k_mutex * mutex, uint32_t timeout);

extern int k_mutex_unlock(struct k_mutex * mutex);

extern int k_msgq_init(struct k_msgq *msgq);

extern int k_msgq_put(struct k_msgq *msgq, const void *data, uint32_t timeout);

extern int k_msgq_get(struct k_msgq *msgq, void *data, uint32_t timeout);

#define MSG_SUB_REF_CNT_BYTE_LEN			2
#define MSG_SUB_CHAN_BYTE_LEN				(sizeof(void *))

int _zbus_init(void)
{
	const struct zbus_channel *curr = NULL;
	const struct zbus_channel *prev = NULL;

	STRUCT_SECTION_FOREACH(zbus_channel_observation, observation) {
		curr = observation->chan;

		if (prev != curr) {
			if (prev == NULL) {
				curr->data->observers_start_idx = 0;
				curr->data->observers_end_idx = 0;
			} else {
				curr->data->observers_start_idx = prev->data->observers_end_idx;
				curr->data->observers_end_idx = prev->data->observers_end_idx;
			}
			prev = curr;
		}

		++(curr->data->observers_end_idx);

		if(observation->obs->type == ZBUS_OBSERVER_SUBSCRIBER_TYPE){
			k_msgq_init(observation->obs->queue);
		} else if(observation->obs->type == ZBUS_OBSERVER_MSG_SUBSCRIBER_TYPE){
			k_msgq_init(observation->obs->msg_queue);
		}
	}
	STRUCT_SECTION_FOREACH(zbus_channel, chan) {
		k_mutex_init(&chan->data->mutex);
		k_mutex_init(chan->msg_buf_mutex);

#if defined(CONFIG_ZBUS_RUNTIME_OBSERVERS)
		sys_slist_init(&chan->data->observers);
#endif /* CONFIG_ZBUS_RUNTIME_OBSERVERS */
	}

	return 0;
}

static uint32_t msg_buff_max = ZBUS_MSG_SUBSCRIBER_BUF_MAX_SIZE;
static inline void msg_buf_ref(void *buf)
{
	uint16_t *refcnt = (uint16_t *)buf;
	(*refcnt)++;
	// printf("[zbus] ref is %d\r\n", *refcnt);
}
static inline void msg_buf_unref(void *buf)
{
	uint16_t *refcnt = (uint16_t *)buf;
	if((*refcnt) > 0){
		(*refcnt)--;
		// printf("[zbus] ref is %d\r\n", *refcnt);
		if((*refcnt) == 0){
			// printf("[zbus] msg buf free %p\r\n", buf);
			if(buf){
				free(buf);	
				msg_buff_max++;
			}
		}
	} else {
		printf("[zbus] msg buf ref cnt error!\r\n");
	}
}
static inline void *msg_buf_get_chan(void *buf, void *chan)
{
	return memcpy(((uint8_t *)chan), ((uint8_t *)buf)+MSG_SUB_REF_CNT_BYTE_LEN, sizeof(chan));
}
static inline void *msg_buf_set_chan(void *buf, const void *chan)
{
	return memcpy(((uint8_t *)buf)+MSG_SUB_REF_CNT_BYTE_LEN, chan, sizeof(chan));
}
static inline void *msg_buf_get_data(void *dst, void *src, uint32_t len)
{
	return memcpy(((uint8_t *)dst), ((uint8_t *)src)+MSG_SUB_REF_CNT_BYTE_LEN+MSG_SUB_CHAN_BYTE_LEN, len);
}
static inline void *msg_buf_set_data(void *dst, void *src, uint32_t len)
{
	return memcpy(((uint8_t *)dst)+MSG_SUB_REF_CNT_BYTE_LEN+MSG_SUB_CHAN_BYTE_LEN, src, len);
}
static inline void *msg_buf_gen(int len)
{
	if(msg_buff_max == 0){
		printf("[zbus] msg buf size is overload\r\n");
		return NULL;
	}
	uint8_t *buf = malloc(len + MSG_SUB_REF_CNT_BYTE_LEN + MSG_SUB_CHAN_BYTE_LEN);
	if(buf == NULL){
		printf("[zbus] msg buf gen alloc fail, no mem\r\n");
		return NULL;
	}
	msg_buff_max--;
	memset(buf, 0, MSG_SUB_REF_CNT_BYTE_LEN+MSG_SUB_CHAN_BYTE_LEN);
	// printf("[zbus] msg buf alloc %p\r\n", buf);
	return buf;
}

static inline int _zbus_notify_observer(const struct zbus_channel *chan,
					const struct zbus_observer *obs, uint32_t end_time,
					void *buf)
{
	int err = 0;

	switch (obs->type) {
	case ZBUS_OBSERVER_LISTENER_TYPE: {
		obs->callback(chan);
		break;
	}
	case ZBUS_OBSERVER_SUBSCRIBER_TYPE: {
		return k_msgq_put(obs->queue, &chan, end_time);
	}
	case ZBUS_OBSERVER_MSG_SUBSCRIBER_TYPE: {
		err = k_mutex_lock(chan->msg_buf_mutex, K_FOREVER);
		if (err) {
			printf("[zbus] msg buf mutex lock fail\r\n");
			return err;
		}
		msg_buf_ref(buf);
		k_mutex_unlock(chan->msg_buf_mutex);
		k_msgq_put(obs->msg_queue, &buf, end_time);
		break;
	}
	default:
		_ZBUS_ASSERT(false, "Unreachable");
	}
	return 0;
}

static inline int _zbus_vded_exec(const struct zbus_channel *chan, uint32_t end_time)
{
	int err = 0;
	int last_error = 0;

	_ZBUS_ASSERT(chan != NULL, "chan is required");

	/* Static observer event dispatcher logic */
	struct zbus_channel_observation *observation;
	struct zbus_channel_observation_mask *observation_mask;

	err = k_mutex_lock(chan->msg_buf_mutex, K_FOREVER);
	uint8_t *buf = msg_buf_gen(zbus_chan_msg_size(chan));
	if(buf == NULL)
		return -1;

	if (err) {
		printf("[zbus] msg buf mutex lock fail\r\n");
		return err;
	}
	msg_buf_ref(buf);
	k_mutex_unlock(chan->msg_buf_mutex);
	msg_buf_set_chan(buf, &chan);
	msg_buf_set_data(buf, zbus_chan_msg(chan), zbus_chan_msg_size(chan));

	for (int16_t i = chan->data->observers_start_idx, limit = chan->data->observers_end_idx;
	     i < limit; ++i) {
		STRUCT_SECTION_GET(zbus_channel_observation, i, &observation);
		STRUCT_SECTION_GET(zbus_channel_observation_mask, i, &observation_mask);

		_ZBUS_ASSERT(observation != NULL, "observation must be not NULL");

		const struct zbus_observer *obs = observation->obs;

		if (!obs->enabled || observation_mask->enabled) {
			continue;
		}

		err = _zbus_notify_observer(chan, obs, end_time, buf);

		_ZBUS_ASSERT(err == 0,
			     "could not deliver notification to observer %s. Error code %d",
			     _ZBUS_OBS_NAME(obs), err);

		if (err) {
			last_error = err;
		}
	}

#if defined(CONFIG_ZBUS_RUNTIME_OBSERVERS)
	/* Dynamic observer event dispatcher logic */
	struct zbus_observer_node *obs_nd, *tmp;

	SYS_SLIST_FOR_EACH_CONTAINER_SAFE(&chan->data->observers, obs_nd, tmp, node) {

		_ZBUS_ASSERT(obs_nd != NULL, "observer node is NULL");

		const struct zbus_observer *obs = obs_nd->obs;

		if (!obs->enabled) {
			continue;
		}

		err = _zbus_notify_observer(chan, obs, end_time);

		if (err) {
			last_error = err;
		}
	}
#endif /* CONFIG_ZBUS_RUNTIME_OBSERVERS */

	err = k_mutex_lock(chan->msg_buf_mutex, K_FOREVER);
	if (err) {
		printf("[zbus] msg buf mutex lock fail\r\n");
		return err;
	}
	msg_buf_unref(buf);
	k_mutex_unlock(chan->msg_buf_mutex);
	return last_error;
}

int zbus_chan_pub(const struct zbus_channel *chan, const void *msg, uint32_t timeout)
{
	int err;

	_ZBUS_ASSERT(!k_is_in_isr(), "zbus cannot be used inside ISRs");
	_ZBUS_ASSERT(chan != NULL, "chan is required");
	_ZBUS_ASSERT(msg != NULL, "msg is required");

	uint32_t end_time = timeout;

	if (chan->validator != NULL && !chan->validator(msg, chan->message_size)) {
		return -ENOMSG;
	}

	err = k_mutex_lock(&chan->data->mutex, timeout);
	if (err) {
		return err;
	}

	memcpy(chan->message, msg, chan->message_size);

	err = _zbus_vded_exec(chan, end_time);

	k_mutex_unlock(&chan->data->mutex);

	return err;
}

int zbus_chan_read(const struct zbus_channel *chan, void *msg, uint32_t timeout)
{
	int err;

	_ZBUS_ASSERT(!k_is_in_isr(), "zbus cannot be used inside ISRs");
	_ZBUS_ASSERT(chan != NULL, "chan is required");
	_ZBUS_ASSERT(msg != NULL, "msg is required");

	err = k_mutex_lock(&chan->data->mutex, timeout);
	if (err) {
		return err;
	}

	memcpy(msg, chan->message, chan->message_size);

	return k_mutex_unlock(&chan->data->mutex);
}

int zbus_chan_notify(const struct zbus_channel *chan, uint32_t timeout)
{
	int err;

	_ZBUS_ASSERT(!k_is_in_isr(), "zbus cannot be used inside ISRs");
	_ZBUS_ASSERT(chan != NULL, "chan is required");

	uint32_t end_time = timeout;

	err = k_mutex_lock(&chan->data->mutex, timeout);
	if (err) {
		return err;
	}

	err = _zbus_vded_exec(chan, end_time);

	k_mutex_unlock(&chan->data->mutex);

	return err;
}

int zbus_chan_claim(const struct zbus_channel *chan, uint32_t timeout)
{
	_ZBUS_ASSERT(!k_is_in_isr(), "zbus cannot be used inside ISRs");
	_ZBUS_ASSERT(chan != NULL, "chan is required");

	int err = k_mutex_lock(&chan->data->mutex, timeout);

	if (err) {
		return err;
	}

	return 0;
}

int zbus_chan_finish(const struct zbus_channel *chan)
{
	_ZBUS_ASSERT(!k_is_in_isr(), "zbus cannot be used inside ISRs");
	_ZBUS_ASSERT(chan != NULL, "chan is required");

	int err = k_mutex_unlock(&chan->data->mutex);

	return err;
}

int zbus_sub_wait(const struct zbus_observer *sub, const struct zbus_channel **chan,
		  uint32_t timeout)
{
	_ZBUS_ASSERT(!k_is_in_isr(), "zbus cannot be used inside ISRs");
	_ZBUS_ASSERT(sub != NULL, "sub is required");
	_ZBUS_ASSERT(chan != NULL, "chan is required");

	if (sub->queue == NULL) {
		return -EINVAL;
	}
	return k_msgq_get(sub->queue, chan, timeout);
}

int zbus_sub_wait_msg(const struct zbus_observer *sub, const struct zbus_channel **chan, void *msg,
		      uint32_t timeout)
{
	_ZBUS_ASSERT(!k_is_in_isr(), "zbus subscribers cannot be used inside ISRs");
	_ZBUS_ASSERT(sub != NULL, "sub is required");
	_ZBUS_ASSERT(sub->type == ZBUS_OBSERVER_MSG_SUBSCRIBER_TYPE,
		     "sub must be a MSG_SUBSCRIBER");
	_ZBUS_ASSERT(sub->msg_queue != NULL, "sub msg_queue is required");
	_ZBUS_ASSERT(chan != NULL, "chan is required");
	_ZBUS_ASSERT(msg != NULL, "msg is required");

	void *buf;
	int err = k_msgq_get(sub->msg_queue, &buf, timeout);
	if (buf == NULL) {
		return -ENOMSG;
	}
	if(err) {
		return err;
	}

	msg_buf_get_chan(buf, chan);
	// printf("[zbus] zbus_sub_wait_msg size is %d\r\n", zbus_chan_msg_size(*chan));
	msg_buf_get_data(msg, buf, zbus_chan_msg_size(*chan));

	err = k_mutex_lock((*chan)->msg_buf_mutex, K_FOREVER);
	if (err) {
		printf("[zbus] msg buf mutex lock fail\r\n");
		return err;
	}
	msg_buf_unref(buf);
	k_mutex_unlock((*chan)->msg_buf_mutex);

	return 0;
}

int zbus_obs_set_chan_notification_mask(const struct zbus_observer *obs,
					const struct zbus_channel *chan, bool masked)
{
	_ZBUS_ASSERT(obs != NULL, "obs is required");
	_ZBUS_ASSERT(chan != NULL, "chan is required");

	struct zbus_channel_observation *observation;
	struct zbus_channel_observation_mask *observation_mask;

	for (int16_t i = chan->data->observers_start_idx, limit = chan->data->observers_end_idx;
	     i < limit; ++i) {
		STRUCT_SECTION_GET(zbus_channel_observation, i, &observation);
		STRUCT_SECTION_GET(zbus_channel_observation_mask, i, &observation_mask);

		_ZBUS_ASSERT(observation != NULL, "observation must be not NULL");

		if (observation->obs == obs) {
			observation_mask->enabled = masked;
			return 0;
		}
	}
	return -ESRCH;
}

int zbus_obs_is_chan_notification_masked(const struct zbus_observer *obs,
					 const struct zbus_channel *chan, bool *masked)
{
	_ZBUS_ASSERT(obs != NULL, "obs is required");
	_ZBUS_ASSERT(chan != NULL, "chan is required");

	struct zbus_channel_observation *observation;
	struct zbus_channel_observation_mask *observation_mask;

	for (int16_t i = chan->data->observers_start_idx, limit = chan->data->observers_end_idx;
	     i < limit; ++i) {
		STRUCT_SECTION_GET(zbus_channel_observation, i, &observation);
		STRUCT_SECTION_GET(zbus_channel_observation_mask, i, &observation_mask);

		_ZBUS_ASSERT(observation != NULL, "observation must be not NULL");

		if (observation->obs == obs) {
			*masked = observation_mask->enabled;
			return 0;
		}
	}
	return -ESRCH;
}
