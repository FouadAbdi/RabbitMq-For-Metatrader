#!/usr/bin/env bash
# Run RabbitMQ via Docker Compose inside WSL.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

if ! command -v docker >/dev/null 2>&1; then
  echo "Error: docker not found in WSL." >&2
  echo "Install Docker in WSL2 — see infrastructure/README.md" >&2
  exit 1
fi

ensure_docker() {
  if docker info >/dev/null 2>&1; then
    return 0
  fi
  echo "Starting Docker in WSL..."
  if command -v service >/dev/null 2>&1; then
    sudo service docker start >/dev/null 2>&1 || true
  fi
  local i
  for i in $(seq 1 30); do
    if docker info >/dev/null 2>&1; then
      echo "Docker is running."
      return 0
    fi
    sleep 1
  done
  echo "Error: Docker daemon is not running in WSL." >&2
  echo "Start it: sudo service docker start   (or open Docker Desktop with WSL integration)" >&2
  exit 1
}

ensure_docker

if [[ ! -f .env ]]; then
  cp .env.example .env
  echo "Created infrastructure/.env from .env.example"
fi

ACTION="${1:-up}"

export COMPOSE_PROJECT_NAME=rmq4mt

wait_for_rabbit() {
  echo "Waiting for RabbitMQ to accept connections..."
  local i
  for i in $(seq 1 6); do
    if docker compose exec -T rabbitmq rabbitmqctl await_startup --timeout 120 \
      && docker compose exec -T rabbitmq rabbitmqctl list_users 2>/dev/null | grep -q rmq4mt; then
      echo "RabbitMQ is ready."
      return 0
    fi
    echo "Still starting, retrying ($i/6)..."
    sleep 5
  done
  echo "Error: RabbitMQ did not become ready in time." >&2
  docker compose logs --tail 30 rabbitmq >&2
  exit 1
}

case "$ACTION" in
  up)
    docker compose up -d
    wait_for_rabbit
    echo ""
    docker compose ps
    echo ""
    echo "AMQP:        localhost:${RMQ_PORT:-5672}"
    echo "Management:  http://localhost:${RMQ_MGMT_PORT:-15672}"
    ;;
  down) docker compose down ;;
  logs) docker compose logs -f rabbitmq ;;
  ps)   docker compose ps ;;
  *)
    echo "Usage: $0 [up|down|logs|ps]"
    exit 1
    ;;
esac
