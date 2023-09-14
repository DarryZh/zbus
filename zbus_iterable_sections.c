/*
 * Copyright (c) 2022 Rodrigo Peixoto <rodrigopex@gmail.com>
 * SPDX-License-Identifier: Apache-2.0
 */
#include <zbus_config.h>
#include <stdint.h>
#include <zbus_iterable_sections.h>
#include <zbus.h>

__weak void k_mutex_init(struct k_mutex * mutex){}

__weak int k_mutex_lock(struct k_mutex * mutex, uint32_t timeout){return 0;}

__weak int k_mutex_unlock(struct k_mutex * mutex){return 0;}

__weak int k_msgq_put(struct k_msgq *msgq, const void *data, uint32_t timeout){return 0;}

__weak int k_msgq_get(struct k_msgq *msgq, void *data, uint32_t timeout){return 0;}


bool zbus_iterate_over_channels(bool (*iterator_func)(const struct zbus_channel *chan))
{
	STRUCT_SECTION_FOREACH(zbus_channel, chan) {
		if (!(*iterator_func)(chan)) {
			return false;
		}
	}
	return true;
}

bool zbus_iterate_over_channels_with_user_data(
	bool (*iterator_func)(const struct zbus_channel *chan, void *user_data), void *user_data)
{
	STRUCT_SECTION_FOREACH(zbus_channel, chan) {
		if (!(*iterator_func)(chan, user_data)) {
			return false;
		}
	}
	return true;
}

bool zbus_iterate_over_observers(bool (*iterator_func)(const struct zbus_observer *obs))
{
	STRUCT_SECTION_FOREACH(zbus_observer, obs) {
		if (!(*iterator_func)(obs)) {
			return false;
		}
	}
	return true;
}

bool zbus_iterate_over_observers_with_user_data(
	bool (*iterator_func)(const struct zbus_observer *obs, void *user_data), void *user_data)
{
	STRUCT_SECTION_FOREACH(zbus_observer, obs) {
		if (!(*iterator_func)(obs, user_data)) {
			return false;
		}
	}
	return true;
}
