# RabbitMq4Metatrader

**Quick + Easy and Native** — natively connect **MetaTrader 4** and **MetaTrader 5** to **RabbitMQ**.

No Node.js bridge. No middleware. Expert advisors `#import` `RmqBridge.dll`, which speaks **AMQP 0-9-1** straight to RabbitMQ on port **5672** via [rabbitmq-c](https://github.com/alanxz/rabbitmq-c).

```
MT4/MT5 EA  →  RmqBridge.dll  →  RabbitMQ :5672
```

## Quick start

### 1. Start RabbitMQ (WSL Docker)

```powershell
.\scripts\run-rabbitmq.ps1
```

AMQP: `127.0.0.1:5672` · Management: http://localhost:15672 · User/pass: `rmq4mt` / `rmq4mt`

### 2. Build the DLL

```powershell
.\scripts\build-library.ps1
```

| Terminal | DLL |
|----------|-----|
| MT4 | `Library\bin\x86\RmqBridge.dll` |
| MT5 | `Library\bin\x64\RmqBridge.dll` |

Copy into `MQL4\Libraries\` or `MQL5\Libraries\`.

### 3. Compile & run experts

Copy `Experts/mt4/*.mq4` or `Experts/mt5/*.mq5` into MetaEditor, compile, attach to chart.

Default inputs: `RmqHost=127.0.0.1`, `RmqPort=5672`, `RmqUser=rmq4mt`, `RmqPass=rmq4mt`

- **Publisher** — sends JSON tick payloads to exchange `mt.events`
- **Consumer** — reads from queue `mt.events.queue`

Enable **Allow DLL imports** in terminal settings.

## DLL API

| Export | Description |
|--------|-------------|
| `RmqConnect(host, port, user, pass)` | Connect to RabbitMQ (returns 1 on success) |
| `RmqDisconnect()` | Close connection |
| `RmqPublish(body)` | Publish JSON/text to `mt.events` |
| `RmqPoll(buffer, size)` | Read next message from `mt.events.queue` |
| `RmqGetLastError(err, size)` | Last error message |

String parameters use `wchar_t*` (Unicode). After rebuilding the DLL, recompile experts and replace the file in `Libraries\`.

## Project layout

```
RabbitMq4Metatrader/
├── Experts/          # MT4 + MT5 expert sources
├── Library/          # C++ DLL (rabbitmq-c)
├── infrastructure/   # RabbitMQ Docker (WSL)
├── docs/             # Setup notes
└── scripts/          # Build + run helpers
```

## Requirements

- Windows 10/11, Visual Studio 2022, CMake 3.16+
- WSL2 + Docker (local RabbitMQ)
- MT4 (32-bit) or MT5 (64-bit)

## Contributing

Contributions are welcome — bug reports, docs, C++ DLL work, MQL experts, and infrastructure improvements.

### Ways to help

| Area | Examples |
|------|----------|
| **Issues** | Repro steps, logs from Experts tab, `RmqGetLastError` output |
| **DLL (`Library/`)** | Connection handling, publish/consume, new exports |
| **Experts (`Experts/`)** | Sample EAs, input validation, JSON payloads |
| **Docs (`docs/`)** | Setup guides, architecture notes, screenshots |
| **Infrastructure** | Docker compose, RabbitMQ config, run scripts |

Open an issue before large changes so we can align on design.

### Development setup

1. **Fork & clone** the repo.
2. **Install requirements** (see above). On Linux/WSL you can use `scripts/build-library.sh` and `scripts/run-rabbitmq.sh` instead of the `.ps1` scripts.
3. **Start RabbitMQ** locally:

   ```powershell
   .\scripts\run-rabbitmq.ps1
   ```

4. **Build the DLL** (first run downloads `rabbitmq-c` via CMake — network required):

   ```powershell
   .\scripts\build-library.ps1          # both MT4 (x86) and MT5 (x64)
   .\scripts\build-library.ps1 -Target MT4
   .\scripts\build-library.ps1 -Target MT5
   ```

5. **Install in MetaTrader** — copy the built DLL and expert sources as in [docs/setup-mt4-mt5.md](docs/setup-mt4-mt5.md). Enable **Allow DLL imports**.

### Development workflow

Typical loop when changing the bridge or experts:

1. Edit sources under `Library/` and/or `Experts/`.
2. Rebuild the DLL and copy `RmqBridge.dll` into `MQL4\Libraries\` or `MQL5\Libraries\`.
3. Recompile experts in MetaEditor (required after DLL or `#import` changes).
4. **Smoke test** — attach **RmqPublisher** on one terminal and **RmqConsumer** on another; confirm messages flow through `mt.events` / `mt.events.queue`. Check the Management UI at http://localhost:15672 if needed.

For DLL internals and AMQP flow, see [docs/architecture.md](docs/architecture.md).

### Where to change what

| Path | Purpose |
|------|---------|
| `Library/include/rmq4mt/` | Public C++ API used by the DLL |
| `Library/src/client.cpp` | Connect, publish, poll logic |
| `Library/src/dll_exports.cpp` | `RmqConnect`, `RmqPublish`, … exports for MQL |
| `Library/RmqBridge.def` | Export names (keep in sync with experts) |
| `Experts/mt4/`, `Experts/mt5/` | Reference expert advisors |
| `infrastructure/` | Local RabbitMQ via Docker |

**MT4 vs MT5:** MT4 needs the **x86** DLL; MT5 needs **x64**. Wrong architecture is the most common `cannot load dll` cause.

### Pull requests

1. Create a **focused branch** from `main` (one feature or fix per PR when possible).
2. **Test** your change with RabbitMQ running and at least one terminal (both if you touch shared DLL code).
3. **Update docs** if behavior, API exports, or setup steps change.
4. Open a PR with a short description, what you tested, and any related issue number.

This project is licensed under **GPL-3.0** — contributions are accepted under the same license.

## License

GPL-3.0 — see [LICENSE](LICENSE).
