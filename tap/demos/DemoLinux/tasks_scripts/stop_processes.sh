# SPDX-FileCopyrightText: Copyright 2025 Vector Informatik GmbH
# SPDX-License-Identifier: MIT

#!/bin/bash
set -e

echo "[Info] Stopping processes"

# List of processes to be stopped
processes=("sil-kit-registry" "sil-kit-demo-ethernet-icmp-echo-device" "start_adapter_and_ping_demo.sh")

# Loop through each process and check if it is running and send SIGINT if it is
for process in "${processes[@]}"; do
    if pgrep -f "$process" > /dev/null; then
        pkill -f "$process"
        echo "[Info] $process has been stopped"
    else
        echo "[Info] $process is not running"
    fi
done