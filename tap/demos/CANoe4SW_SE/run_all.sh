#!/bin/bash
scriptDir=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
silKitDir=/home/vector/SilKit/SilKit-4.0.43-ubuntu-18.04-x86_64-gcc/

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

$silKitDir/SilKit/bin/sil-kit-registry --listen-uri 'silkit://0.0.0.0:8501' -s &> $scriptDir/sil-kit-registry.out &

# give sil-kit-registry time for startup
sleep 1

$scriptDir/../start_adapter_and_ping_demo.sh &> $scriptDir/start_adapter_and_ping_demo.out &

$scriptDir/../../../bin/SilKitDemoEthernetIcmpEchoDevice &> $scriptDir/SilKitDemoEthernetIcmpEchoDevice.out &

$scriptDir/run.sh
