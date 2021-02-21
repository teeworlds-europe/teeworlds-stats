#!/usr/bin/env bash
################################################################################
# Get Teeworlds server status and construct JSON summary of server statuses.
# This script is configured with environment variables:
#
#   - OUTPUT_DIR - output directory for reports
#   - PREPARE_REPORT - "yes", if all JSON reports should be concatenated into
#                      single document to be served online
#   - SERVER_ADDRESS - server's address
#   - SERVER_PORT - server's port
#   - SERVER_NAME - server's placeholder name displayed when description can't
#                   be fetched. This must be unique across monitored servers.
#   - RETRY_COUNT - try to fetch status this many times before failing
#   - TIMEOUT - status fetching timeout for single try
#   - REPORT_VALIDITY - records older than this(in minutes) are deleted
################################################################################

set -o errexit
set -o nounset
set -o pipefail

RETRY_COUNT=${RETRY_COUNT:-3}
TIMEOUT=${TIMEOUT:-3s}
OUTPUT_DIR=${OUTPUT_DIR:-./}
PREPARE_REPORT=${PREPARE_REPORT:-yes}
SERVER_ADDRESS=${SERVER_ADDRESS:-127.0.0.1}
SERVER_PORT=${SERVER_PORT:-8303}
SERVER_NAME=${SERVER_NAME:-"Test Teeworlds server"}
REPORT_VALIDITY=${REPORT_VALIDITY:-10}

on_exit() {
  local exit_code=$?
  if [ "$exit_code" -ne 0 ]; then
    echo "Failing with exit code: ${exit_code}" >&2
  fi
  exit "$exit_code"
}

create_server_report() {
  local stats=""
  local connected=0
  for try in $(seq "$RETRY_COUNT"); do
    echo -n "Gathering statistics from ${SERVER_ADDRESS}:${SERVER_PORT}: ${SERVER_NAME}..."
    stats=$(timeout "$TIMEOUT" teeworlds-stat-generate "$SERVER_ADDRESS" "$SERVER_PORT") && {
      connected=1
      echo "ok"
      break
    }
    echo "fail"
  done
  if [ "$connected" -eq 0 ]; then
    stats=$(
      echo '{}' \
      | jq -c '. | .name=$name | .state=$state' \
        --arg name "${SERVER_NAME}" \
        --arg state 'unreachable'
    )
  else
    stats=$(
      echo "$stats" \
      | jq -c '. | .state=$state' \
        --arg state 'online'
    )
  fi
  stats=$(
    echo "$stats" \
    | jq -c '. | .address=$address | .port=($port | tonumber) | .timestamp=$timestamp' \
      --arg address "$SERVER_ADDRESS" \
      --arg port "$SERVER_PORT" \
      --arg timestamp "$(date +"%s")"
  )
  local report_name
  report_file="report-$(echo "${SERVER_NAME}" | md5sum | cut -d ' ' -f 1).json"
  echo "Saving statistics for ${SERVER_ADDRESS}:${SERVER_PORT} in ${OUTPUT_DIR}/${report_file}"
  echo "$stats" > "${OUTPUT_DIR}/${report_file}"
}

remove_outdated_reports() {
  echo "Removing old reports"
  find "$OUTPUT_DIR" -mtime "$REPORT_VALIDITY" -delete -print
}

create_summary_report() {
  echo "Creating summary report in ${OUTPUT_DIR}/summary.json"
  find "${OUTPUT_DIR}" -name 'report-*.json' -print0 \
    | xargs -0r cat > "${OUTPUT_DIR}/summary.json"
}

main() {
  trap on_exit EXIT
  create_server_report
  if [ "$PREPARE_REPORT" = 'yes' ]; then
    {
      flock --exclusive 9
      remove_outdated_reports
      create_summary_report
    } 9>"${OUTPUT_DIR}/lockfile"
  fi
}

main "$@"
