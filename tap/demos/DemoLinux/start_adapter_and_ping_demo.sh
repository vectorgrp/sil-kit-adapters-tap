#!/bin/sh
# SPDX-FileCopyrightText: Copyright 2025 Vector Informatik GmbH
# SPDX-License-Identifier: MIT

scriptDir=$( dirname $(realpath $0) )

# check if user is root
if [ "$(id -u)" -ne 0 ]; then
    echo "[error] This script must be run as root / via sudo!"
    exit 1
fi

logDir=$scriptDir/../CANoe4SW_SE/logs # define a directory for .out files
mkdir -p $logDir # if it does not exist, create it

child_processes=""

# cleanup function called on exit
cleanup() {
  kill $child_processes > /dev/null 2>&1
  
  if ip netns list | grep -q tap_demo_ns; then
    ip netns delete tap_demo_ns
  fi

  ip tuntap del dev silkit_tap mode tap
  rm -f "$scriptDir/temp_fifo"
}

# cleanup trap 
trap 'cleanup; exit' EXIT HUP INT TERM;

echo "[info] Setting up network and ping the echo demo device via SIL Kit Adapter TAP..."

# check if running on Ubuntu, openSUSE or Android system
netns_path=
if cat /proc/version | grep -q -i "android"; then
  netns_path="/mnt/run"
elif cat /proc/version | grep -q -i -e "ubuntu" -e "SUSE"; then
  netns_path="/var/run/netns"
else
  echo "[error] Not supported OS. Please run this script on Ubuntu or Android system."
  exit 1
fi

echo "[info] Recreating tap_demo_ns network namespace"
if [ -f "${netns_path}/tap_demo_ns" ]; then
    ip netns delete tap_demo_ns
fi

echo "[info] Creating tap device silkit_tap"
ip tuntap add dev silkit_tap mode tap

echo "[info] Starting sil-kit-adapter-tap..."

$scriptDir/../../../bin/sil-kit-adapter-tap --configuration $scriptDir/../SilKitConfig_Adapter.silkit.yaml > $logDir/sil-kit-adapter-tap.out &
child_processes="$child_processes $!"

sleep 2 # wait 1 second for the creation/existense of the .out file

mkfifo "$scriptDir/temp_fifo"

tail -n +1 -f "$logDir/sil-kit-adapter-tap.out" > "$scriptDir/temp_fifo" &
child_processes="$child_processes $!"

timeout 30s grep -q 'Press CTRL + C to stop the process...' "$scriptDir/temp_fifo" || { echo "[error] Timeout reached while waiting for sil-kit-adapter-tap to start"; exit 1; }
echo "[info] sil-kit-adapter-tap has been started"

# Hint: It is important to establish the connection to the the adapter before moving the tap device to its separate namespace
echo "[info] Moving tap device 'silkit_tap' to network namespace 'tap_demo_ns'"
ip netns add tap_demo_ns
ip link set silkit_tap netns tap_demo_ns

echo "[info] Configuring tap device 'silkit_tap'"
# Hint: The IP address can be set to anything as long as it is in the same network as the echo device which is pinged
ip -netns tap_demo_ns addr add 192.168.7.2/16 dev silkit_tap
ip -netns tap_demo_ns link set silkit_tap up

echo "[info] Starting to ping the echo device..."
# Hint: The command 'ping 192.168.7.35' can be replaced by any other command or application which should send/receive data to/from SIL Kit via silkit_tap
sh -c "nsenter --net=${netns_path}/tap_demo_ns ping 192.168.7.35"
