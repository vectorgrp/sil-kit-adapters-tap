# SPDX-FileCopyrightText: Copyright 2025 Vector Informatik GmbH
# SPDX-License-Identifier: MIT

#!/bin/bash
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

export AMSR_DISABLE_INTEGRITY_CHECK=1
export SILKIT_ADAPTER_TAP_PATH=/home/vector/SilKit/sil-kit-adapters-tap/bin

# check if user is root
if [[ $EUID -ne 0 ]]; then
    echo "This script must be run as root / via sudo -E!"
    exit 1
fi

# check if AMSR_SRC_DIR is set
if [[ -z $AMSR_SRC_DIR ]]; then
    echo "Make sure \$AMSR_SRC_DIR is set to your BSW Package folder and this script is run with sudo -E!"
    exit 1
fi

startapp_cm_server1_pid=
# cleanup trap for child processes
trap 'kill $startapp_cm_server1_pid $(jobs -p); exit' EXIT SIGINT;

echo "Recreating tap_demo_ns network namespace"
if test -f "/run/netns/tap_demo_ns"; then
    ip netns delete tap_demo_ns
fi

echo "Creating tap device silkit_tap"
#Add device with name silkit_tap with mode tap
ip tuntap add dev silkit_tap mode tap

echo "Starting sil-kit-adapter-tap..."
$SILKIT_ADAPTER_TAP_PATH/sil-kit-adapter-tap --name 'SilKit_TapDevice' --tap-name 'silkit_tap'  --registry-uri 'silkit://localhost:8501' --network 'Ethernet1' &> /$SCRIPT_DIR/sil-kit-adapter-tap.out &
sleep 1 # wait 1 second for the creation/existense of the .out file
timeout 30s grep -q 'Press CTRL + C to stop the process...' <(tail -f /$SCRIPT_DIR/sil-kit-adapter-tap.out) || { echo "[error] Timeout reached while waiting for sil-kit-adapter-tap to start"; exit 1; }
echo "sil-kit-adapter-tap has been started"

# Hint: It is important to establish the connection to the the adapter before moving the tap device to its separate namespace
echo "Moving tap device 'silkit_tap' to network namespace 'tap_demo_ns'"
#network namespace, means : add network namespace called tap_demo_ns
ip netns add tap_demo_ns
#established link between our silkit_tap device and network namespace tap_demo_ns
ip link set silkit_tap netns tap_demo_ns

echo "Configuring tap device 'silkit_tap'"
ip -netns tap_demo_ns addr add 192.168.7.2/16 dev silkit_tap
ip -netns tap_demo_ns link set silkit_tap up

# start crypto daemon
cd $AMSR_SRC_DIR/Examples/startapplication/build/gcc7_linux_x86_64/install/opt/amsr_crypto_daemon
bash -c "nsenter --net=/run/netns/tap_demo_ns ./bin/amsr_crypto_daemon &" &> $SCRIPT_DIR/crypto.out
sleep 2
echo "crypto daemon has been started."

# start someip daemon
cd $AMSR_SRC_DIR/Examples/startapplication/build/gcc7_linux_x86_64/install/opt/amsr_someipd_daemon
bash -c "nsenter --net=/run/netns/tap_demo_ns ./bin/amsr_someipd_daemon -c ./etc/someipd-posix.json &" &> $SCRIPT_DIR/someipd.out
sleep 2
echo "someip daemon has been started."

# start server application
cd $AMSR_SRC_DIR/Examples/startapplication/build/gcc7_linux_x86_64/install/opt/startapp_cm_server1
echo "starting server application..."
./bin/startapp_cm_server1 &
startapp_cm_server1_pid=$!
wait $startapp_cm_server1_pid
