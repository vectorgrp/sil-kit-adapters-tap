#!/bin/sh

virt-builder debian-11                                             \
    --output silkit-qemu-demos-guest.qcow2                            \
    --format qcow2                                                 \
    --hostname silkit-qemu-demos-guest                                \
    --root-password password:root                                  \
    --run ./tools/virt-builder/debian/install.100.setup-network.sh \
    --run ./tools/virt-builder/debian/install.110.setup-sshd.sh

# shrink disk image to minimum:
#qemu-img convert input.too-big.qcow2 -O qcow2 output.qcow2

