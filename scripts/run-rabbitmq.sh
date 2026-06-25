#!/usr/bin/env bash
# Wrapper: run RabbitMQ via WSL Docker (from WSL) or delegate to infrastructure/run.sh
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
ACTION="${1:-up}"

exec bash "$ROOT/infrastructure/run.sh" "$ACTION"
