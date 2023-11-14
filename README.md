# zbus pure version

## demos
 - demos/hello_world:  zbus基础用法
 - demos/benchmark:    zbus性能和压力测试

## API
ZBUS_MSG_INIT(_val, ...)

ZBUS_OBSERVERS(...)

ZBUS_CHAN_DEFINE(_name, _type, _validator, _user_data, _observers, _init_val) 

ZBUS_SUBSCRIBER_DEFINE(_name, _queue_size)

ZBUS_LISTENER_DEFINE(_name, _cb)

int zbus_chan_pub(const struct zbus_channel *chan, const void *msg, uint32_t timeout);

int zbus_chan_read(const struct zbus_channel *chan, void *msg, uint32_t timeout);

int zbus_chan_claim(const struct zbus_channel *chan, uint32_t timeout);

int zbus_chan_finish(const struct zbus_channel *chan);

const void *zbus_chan_const_msg(const struct zbus_channel *chan);

uint16_t zbus_chan_msg_size(const struct zbus_channel *chan);

int zbus_sub_wait(const struct zbus_observer *sub, const struct zbus_channel **chan,
		  uint32_t timeout);

## benchmark
Xtensa  160Mhz  gcc -O0

测试参数：消息的数据块大小为256字节，一条通道，16个订阅者，单/多拷贝模式

```
[2023-09-15 15:27:08]  [D] Benchmark 1 to 16: Dynamic memory, ASYNC transmission and message size 256
[2023-09-15 15:27:08]  [D] Bytes sent = 262144, received = 262144
[2023-09-15 15:27:08]  [D] Average data rate: 6.9MB/s
[2023-09-15 15:27:08]  [D] Duration: 0.000041s
```

测试参数：消息的数据块大小为256字节，一条通道，16个监听者，单拷贝模式

```
[2023-09-15 15:52:53]  [D] Benchmark 1 to 16: Dynamic memory, SYNC transmission and message size 256
[2023-09-15 15:52:53]  [D] Bytes sent = 262144, received = 262144
[2023-09-15 15:52:53]  [D] Average data rate: 125.0MB/s
[2023-09-15 15:52:53]  [D] Duration: 0.000002s
```
