#!/bin/bash

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
HLC_DIR="$(cd -- "${SCRIPT_DIR}/.." && pwd)"
COMPOSE_FILE="${HLC_DIR}/docker-compose.yaml"

if docker compose version >/dev/null 2>&1; then
    docker compose -f "${COMPOSE_FILE}" build
    docker compose -f "${COMPOSE_FILE}" up
else
    docker-compose -f "${COMPOSE_FILE}" build
    docker-compose -f "${COMPOSE_FILE}" up
fi
