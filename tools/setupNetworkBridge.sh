#!/bin/bash
echo "Setting up network bridge for QEMU images..."
ip tuntap add dev silkit_tap mode tap
# create bridge network in Linux
sudo brctl addbr br1
# add tap devices to this network bridge
sudo brctl addif br1 qemu_demo_tap
sudo brctl addif br1 silkit_tap
# Activate the bridge and interfaces
sudo ip link set br1 up
sudo ip link set qemu_demo_tap up
sudo ip link set silkit_tap up
