# MetaTrader setup

## 1. RabbitMQ

```powershell
.\scripts\run-rabbitmq.ps1
```

## 2. Build & install DLL

```powershell
.\scripts\build-library.ps1
```

| Terminal | Copy from | Copy to |
|----------|-----------|---------|
| MT4 | `Library\bin\x86\RmqBridge.dll` | `<Data>\MQL4\Libraries\` |
| MT5 | `Library\bin\x64\RmqBridge.dll` | `<Data>\MQL5\Libraries\` |

**File → Open Data Folder** in MetaTrader.

## 3. Experts

Copy `Experts/mt4/*.mq4` or `Experts/mt5/*.mq5` into `Experts\`, compile in MetaEditor.

## 4. Terminal settings

**Tools → Options → Expert Advisors** — enable automated trading and **Allow DLL imports**.

## 5. Run

Attach **RmqPublisher** on one chart (e.g. MT4) and **RmqConsumer** on another (e.g. MT5).

Inputs: `RmqHost=127.0.0.1`, `RmqPort=5672`, `RmqUser=rmq4mt`, `RmqPass=rmq4mt`

## Troubleshooting

| Issue | Fix |
|-------|-----|
| `connect failed` | Run `.\scripts\run-rabbitmq.ps1`; check `RmqGetLastError` in Experts log |
| `cannot load dll` | Wrong arch — MT4 = x86, MT5 = x64 |
| Garbled `????` errors | Old DLL or wrong arch — rebuild, copy DLL, recompile EA |
| No messages | Publisher must be running on the other terminal |

After any DLL rebuild: replace `RmqBridge.dll` in `Libraries\` and recompile the expert.
