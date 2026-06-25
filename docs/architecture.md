# Architecture

Experts import `RmqBridge.dll`, which uses [rabbitmq-c](https://github.com/alanxz/rabbitmq-c) to speak AMQP 0-9-1 to RabbitMQ on port **5672**.

## Flow

1. `RmqConnect(host, port, user, pass)` — TCP login, declare exchange/queue.
2. Publisher: `RmqPublish` → `basic.publish` on `mt.events` (topic).
3. Consumer: `RmqPoll` → non-blocking read from `mt.events.queue`.

## Builds

| Arch | DLL | Terminal |
|------|-----|----------|
| x86 | `Library/bin/x86/RmqBridge.dll` | MT4 |
| x64 | `Library/bin/x64/RmqBridge.dll` | MT5 |

First build fetches `rabbitmq-c` via CMake FetchContent (network required).
