#ifndef __ZBUS_CONFIG_H__
#define __ZBUS_CONFIG_H__

#define CONFIG_ZBUS_POSIX
// #define CONFIG_ZBUS_CMSIS_RTOS2

#if ((!defined CONFIG_ZBUS_POSIX) && (!defined CONFIG_ZBUS_CMSIS_RTOS2))
#define CONFIG_ZBUS_POSIX
#endif

#define ZBUS_MSG_SUBSCRIBER_BUF_MAX_SIZE    65535

#define CONFIG_ZBUS_CHANNEL_NAME
#define CONFIG_ZBUS_OBSERVER_NAME

#define __ASSERT(test, fmt, ...) { }

#endif
