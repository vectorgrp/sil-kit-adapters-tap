#!/bin/bash
scriptDir=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
# Hint: needs to be adapted to actual location
silKitDir=/home/vector/SilKit/SilKit-4.0.17-ubuntu-18.04-x86_64-gcc/

# check if user is root
if [[ $EUID -ne 0 ]]; then
    echo "This script must be run as root / via sudo!"
    exit 1
fi

#Make a temporary fifo to use as std:cin which is not fd#0 (this shell's std:cin) to prevent unintended closure of the background-launched processes
tmp_fifo=$(mktemp -u)
mkfifo $tmp_fifo
exec 3<>$tmp_fifo
rm $tmp_fifo

<&3 $silKitDir/SilKit/bin/sil-kit-registry --listen-uri 'silkit://0.0.0.0:8501' &> $scriptDir/sil-kit-registry.out &

<&3 $scriptDir/../start_ping_demo.sh &> $scriptDir/start_ping_demo.out &

<&3 $scriptDir/../../../build/bin/SilKitDemoEthernetIcmpEchoDevice &> $scriptDir/SilKitDemoEthernetIcmpEchoDevice.out &

$scriptDir/run.sh

#cleanup of launched processes
kill -s 15 $(jobs -p)
