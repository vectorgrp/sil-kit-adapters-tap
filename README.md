# Vector SIL Kit Adapters for TAP devices (Linux only)
This collection of software is provided to illustrate how the [Vector SIL Kit](https://github.com/vectorgrp/sil-kit/)
can be attached to a TAP device.

This repository contains instructions to create, set up, and launch such a minimal TAP device setup.

The main contents are working examples of necessary software to connect the running system to a SIL Kit environment,
as well as complimentary demo application for some communication to happen.

Those instructions assume you use WSL (Ubuntu) or a Linux OS for building and running the adapter, and use ``bash`` as your interactive
shell.

## a) Getting Started with self build Adapters and Demos
This section specifies steps you should do if you have just cloned the repository.

Before any of those topics, please change your current directory to the top-level in the ``sil-kit-adapters-tap``
repository:

    cd /path/to/sil-kit-adapters-tap

### Fetch Third Party Software
The first thing that you should do is initializing the submodules to fetch the required third party software:

    git submodule update --init --recursive

Otherwise clone the standalone version of asio manually:

    git clone --branch asio-1-18-2 https://github.com/chriskohlhoff/asio.git third_party/asio

### Build the Adapters and Demos
To build the demos, you'll need SIL Kit packages ``SilKit-x.y.z-$platform`` for your platform. You can download them directly from [Vector SIL Kit Releases](https://github.com/vectorgrp/sil-kit/releases).

The adapters and demos are built using ``cmake``. If you want to build the adapter against a specific downloaded release of SIL Kit, you can follow these steps:

    mkdir build
    cmake -S. -Bbuild -DSILKIT_PACKAGE_DIR=/path/to/SilKit-x.y.z-$platform/ -D CMAKE_BUILD_TYPE=Release
    cmake --build build --parallel

**Note 1:** If you have installed a self-built version of SIL Kit, you can build the adapter against it by setting SILKIT_PACKAGE_DIR to the installation path, where the bin, include and lib directories are.

**Note 2:** If you don't provide a specific path for SILKIT_PACKAGE_DIR, a SIL Kit release package [SilKit-4.0.17-ubuntu-18.04-x86_64-gcc] will be fetched from github.com and the adapter will be built against it.

  
The adapters and demo executables will be available in ``build/bin`` (depending on the configured build directory).
Additionally the ``SilKit`` shared library is copied to that directory automatically.

## b) Getting Started with pre-built Adapters and Demos
Download a preview or release of the Adapters directly from [Vector SIL Kit Adapters Releases](https://github.com/vectorgrp/sil-kit-adapters-tap/releases).

If not already existent on your system you should also download a SIL Kit Release directly from [Vector SIL Kit Releases](https://github.com/vectorgrp/sil-kit/releases). You will need this for being able to start a sil-kit-registry.

## Run the SilKitAdapterTap
This application allows the user to attach a TAP device of any Linux system to the Vector SIL Kit.

Before you start the adapter there always needs to be a sil-kit-registry running already. Start it e.g. like this:

    ./path/to/SilKit-x.y.z-$platform/SilKit/bin/sil-kit-registry --listen-uri 'silkit://0.0.0.0:8501'

It is also necessary that the TAP device exists before the the ``SilKitAdapterTap`` is started. 

**Hint:** If your TAP device has been created by a third party application (you want the SIL Kit to connect to) it is possible that this TAP device resource is 'flagged' as busy/blocked. In this case you just can create another TAP device for usage with the ``SilKitAdapterTap`` by yourself and bridge (``brctl``) it with the TAP device of your third party application.

The application *optionally* takes the following command line arguments (default between curly braces):

    SilKitAdapterTap [--name <participant's name{EthernetTapDevice}>]
      [--registry-uri silkit://<host{localhost}>:<port{8501}>]
      [--log <Trace|Debug|Warn|{Info}|Error|Critical|off>]
      [--tap-name <tap device's name{silkit_tap}>]
      [--network <SIL Kit ethernet network{tap_demo}>]
      [--help]

## TAP Demo
The aim of this demo is to showcase a simple adapter forwarding ethernet traffic from and to a TAP device through
Vector SIL Kit. Traffic being exchanged are ping (ICMP) requests, and the answering device replies to them.

This demo is further explained in [tap/README.md](tap/README.md).

