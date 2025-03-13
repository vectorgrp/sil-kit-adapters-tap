#!/bin/bash
# SPDX-FileCopyrightText: Copyright 2025 Vector Informatik GmbH
# SPDX-License-Identifier: MIT

scriptDir=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
silKitDir=/home/vector/SilKit/SilKit-4.0.55-ubuntu-18.04-x86_64-gcc

logDir=$scriptDir/logs # define a directory for .out files
mkdir -p $logDir # if it does not exist, create it

# if "exported_full_path_to_silkit" environment variable is set (in pipeline script), use it. Otherwise, use default value
silKitDir="${exported_full_path_to_silkit:-$silKitDir}"

# cleanup trap for child processes 
trap 'kill $(jobs -p); exit' EXIT SIGHUP;

if [ ! -d "$silKitDir" ]; then
    echo "The var 'silKitDir' needs to be set to actual location of your SilKit"
    exit 1
fi

# check if user is root
if [[ $EUID -ne 0 ]]; then
    echo "This script must be run as root / via sudo!"
    exit 1
fi

$silKitDir/SilKit/bin/sil-kit-registry --listen-uri 'silkit://0.0.0.0:8501' &> $logDir/sil-kit-registry.out &
sleep 1 # wait 1 second for the creation/existense of the .out file
timeout 30s grep -q 'Press Ctrl-C to terminate...' <(tail ---disable-inotify -f $logDir/sil-kit-registry.out -n +1) || { echo "[error] Timeout reached while waiting for sil-kit-registry to start"; exit 1; }

$scriptDir/../DemoLinux/start_adapter_and_ping_demo.sh &> $logDir/start_adapter_and_ping_demo.out &
timeout 30s grep -q 'Starting to ping the echo device...' <(tail ---disable-inotify -f $logDir/start_adapter_and_ping_demo.out -n +1) || { echo "[error] Timeout reached while waiting for start_adapter_and_ping_demo.sh to start"; exit 1; }

$scriptDir/../../../bin/sil-kit-demo-ethernet-icmp-echo-device &> $logDir/sil-kit-demo-ethernet-icmp-echo-device.out &
timeout 30s grep -q 'Press CTRL + C to stop the process...' <(tail ---disable-inotify -f $logDir/sil-kit-demo-ethernet-icmp-echo-device.out -n +1) || { echo "[error] Timeout reached while waiting for sil-kit-demo-ethernet-icmp-echo-device to start"; exit 1; }

$scriptDir/run.sh

#capture returned value of run.sh script
exit_status=$?

#exit run_all.sh with same exit_status
exit $exit_status
