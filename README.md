# zbus pure version

## API

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