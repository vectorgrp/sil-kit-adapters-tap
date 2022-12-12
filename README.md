# Vector SIL Kit Adapters for TAP devices
This collection of software is provided to illustrate how the [Vector SIL Kit](https://github.com/vectorgrp/sil-kit/)
can be attached to a TAP device.

This repository contains instructions to create, set up, and launch a QEMU image, and a minimal development environment.

The main contents are working examples of necessary software to connect the running system to a SIL Kit environment,
as well as complimentary demo applications for some communication to happen.

## Getting Started
This section specifies steps you should do if you have just cloned the repository.

Before any of those topics, please change your current directory to the top-level in the ``sil-kit-adapters-tap``
repository:

    cd /path/to/sil-kit-adapters-tap

Those instructions assume you use WSL (Ubuntu) or a Linux OS for running QEMU, and use ``bash`` as your interactive
shell.

### Fetch Third Party Software
The first thing that you should do is initializing the submodules to fetch the required third party software:

    git submodule update --init --recursive

Otherwise clone the standalone version of asio manually:

    git clone --branch asio-1-18-2 https://github.com/chriskohlhoff/asio.git third_party/asio


### Build QEMU image
Setup your WSL host (install ``virt-builder`` and a kernel image for use by ``virt-builder``):

    sudo ./tools/setup-host-wsl2-ubuntu.sh

Build the guest image:

    sudo chmod a+x tools/build-silkit-qemu-demos-guest
    sudo ./tools/build-silkit-qemu-demos-guest
    sudo chmod go+rw silkit-qemu-demos-guest.qcow2


### Run QEMU image
To start the guest image, run:

    sudo chmod a+x tools/run-silkit-qemu-demos-guest
    sudo ./tools/run-silkit-qemu-demos-guest

By default, the options in this script will spawn the guest system in the same terminal. The password for the ``root``
user is ``root``.

QEMU is also set up to forward the guests SSH port on ``localhost:10022``, any interaction with the guest can then
proceed via SSH:

    ssh -p10022 -oStrictHostKeyChecking=no -oUserKnownHostsFile=/dev/null root@localhost

**Note:** The options passed to SSH deactivate host-key checking for this connection, otherwise you will have to edit your
``known_hosts`` file if you decide to rebuild the guest image.

### Build the Adapters and Demos
To build the demos, you'll need SIL Kit packages ``SilKit-x.y.z-$platform`` for your platform. You can download them directly from [Vector SIL Kit Releases](https://github.com/vectorgrp/sil-kit/releases).

The adapters and demos are built using ``cmake``:

    mkdir build
    cmake -S. -Bbuild -DSILKIT_PACKAGE_DIR=/path/to/SilKit-x.y.z-$platform/
    cmake --build build --parallel

The adapters and demo executables will be available in ``build/bin`` (depending on the configured build directory).
Additionally the ``SilKit`` shared library (e.g., ``SilKit[d].dll`` on Windows) is copied to that directory
automatically.


## TAP Demo
The aim of this demo is to showcase a simple adapter forwarding ethernet traffic from and to the QEMU image via a TAP device through
Vector SIL Kit. Traffic being exchanged are ping (ICMP) requests, and the answering device replies only to them.

This demo is further explained in [tap/README.md](tap/README.md).

