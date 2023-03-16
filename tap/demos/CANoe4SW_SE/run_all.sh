#!/bin/bash
scriptDir=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
silKitDir=/home/vector/SilKit/SilKit-4.0.19-ubuntu-18.04-x86_64-gcc/

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

# Make a temporary fifo to use as std:cin which is not fd#0 (this shell's std:cin) to prevent unintended closure of the background-launched processes
tmp_fifo=$(mktemp -u)
mkfifo $tmp_fifo
exec 3<>$tmp_fifo
rm $tmp_fifo

<&3 $silKitDir/SilKit/bin/sil-kit-registry --listen-uri 'silkit://0.0.0.0:8501' &> $scriptDir/sil-kit-registry.out &

# give sil-kit-registry time for startup
sleep 1

<&3 $scriptDir/../start_adapter_and_ping_demo.sh &> $scriptDir/start_adapter_and_ping_demo.out &

<&3 $scriptDir/../../../build/bin/SilKitDemoEthernetIcmpEchoDevice &> $scriptDir/SilKitDemoEthernetIcmpEchoDevice.out &

$scriptDir/run.sh
