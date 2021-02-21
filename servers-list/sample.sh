#!/usr/bin/env bash
################################################################################
# Prepare sample server status report. It's useful for testing frontend.
################################################################################

set -o errexit
set -o nounset
set -o pipefail

on_exit() {
  local exit_code=$?
  if [ "$exit_code" -ne 0 ]; then
    echo "Failing with exit code: ${exit_code}" >&2
  fi
  exit "$exit_code"
}

main() {
  trap on_exit EXIT
  export PATH="${PATH}:$(pwd)"
  export OUTPUT_DIR=./sample-reports
  export SERVER_ADDRESS=teeworlds.eu
  mkdir -p "$OUTPUT_DIR"
  SERVER_PORT=8303 SERVER_NAME="Vanilla DM" ./update-list.sh
  SERVER_PORT=8304 SERVER_NAME="Vanilla CTF5" ./update-list.sh
  SERVER_PORT=8305 SERVER_NAME="Vanilla LTS" ./update-list.sh
  SERVER_PORT=8306 SERVER_NAME="Nonexisting server" ./update-list.sh
}

main "$@"
