# Vector SIL Kit Adapter for TAP devices
This collection of software is provided to illustrate how the [Vector SIL Kit](https://github.com/vectorgrp/sil-kit/)
can be attached to a TAP device.

This repository contains instructions to create, set up, and launch such a minimal TAP device setup.

The main contents are working examples of necessary software to connect the running system to a SIL Kit environment,
as well as complimentary demo application for some communication to happen.

Those instructions assume you use WSL2 (Ubuntu) or a Linux OS for building and running the Linux version of the adapter together with ``bash`` as your interactive shell. In case of the Windows version of the adapter ``PowerShell`` is assumed.

## a) Getting Started with self-built Adapter and Demos
This section specifies steps you should do if you have just cloned the repository.

Before any of those topics, please change your current directory to the top-level in the ``sil-kit-adapters-tap``
repository:

    cd /path/to/sil-kit-adapters-tap

### Fetch Third Party Software
The first thing that you should do is initializing the submodules to fetch the required third party software:

    git submodule update --init --recursive

Otherwise clone the standalone version of asio manually:

    git clone --branch asio-1-24-0 https://github.com/chriskohlhoff/asio.git third_party/asio

### Build the Adapter and Demos
To build the demos, you'll need SIL Kit packages ``SilKit-x.y.z-$platform`` for your platform. You can download them directly from [Vector SIL Kit Releases](https://github.com/vectorgrp/sil-kit/releases).

The adapter and demos are built using ``cmake``. If you want to build the adapter against a specific downloaded release of SIL Kit, you can follow these steps:

    mkdir build
    cmake -S. -Bbuild -DSILKIT_PACKAGE_DIR=/path/to/SilKit-x.y.z-$platform/ -D CMAKE_BUILD_TYPE=Release
    cmake --build build --parallel --config Release

**Note 1:** If you have a self-built or pre-built version of SIL Kit, you can build the adapter against it by setting SILKIT_PACKAGE_DIR to the path, where the bin, include and lib directories are.

**Note 2:** If you have SIL Kit installed on your system, you can build the adapter against it, even by not providing SILKIT_PACKAGE_DIR to the installation path at all. Hint: Be aware, if you are using WSL2 this may result in issue where your Windows installation of SIL Kit is found. To avoid this specify SILKIT_PACKAGE_DIR.

**Note 3:** If you don't provide a specific path for SILKIT_PACKAGE_DIR and there is no SIL Kit installation on your system, a SIL Kit release package (the default version listed in CMakeLists.txt) will be fetched from github.com and the adapter will be built against it.

**Note 4:** The Adapter can be used to connect to TAP devices in QNX environments. In order to acheive that, you can cross-build the adapter for QNX systems using the provided CMake toolchain files inside the `cmake` folder.  

The adapter and demo executables will be available in the ``bin`` directory as well as the ``SilKit.dll`` if you are on Windows. Additionally the ``SilKit.lib`` on Windows and the ``libSilKit.so`` on Linux are automatically copied to the ``lib`` directory.

## b) Getting Started with pre-built Adapter and Demos
Download a preview or release of the adapter directly from [Vector SIL Kit Adapter for TAP devices Releases](https://github.com/vectorgrp/sil-kit-adapters-tap/releases).

If not already existent on your system you should also download a SIL Kit Release directly from [Vector SIL Kit Releases](https://github.com/vectorgrp/sil-kit/releases). You will need this for being able to start a sil-kit-registry.

## Install the sil-kit-adapter-tap (optional)
Be aware that SIL Kit itself also needs to be installed to run the adapter.

Run the following command to install the sil-kit-adapter-tap (can be done for self-built and pre-built package after cmake configure):

### Linux installation

    sudo cmake --build build --target install

**Note:** After installing the adapter on Linux, the following command  ``sil-kit-adapter-tap`` can be called from everywhere without defining a path. The default installation path will be ``/usr/local/bin``.

### Windows installation

    cmake --build build --target install --config Release

**Note 1:** Elevated rights are needed to install the adapter under its default location. This can be achieved by running the command in a PowerShell opened as administrator.

**Note 2:** The default installation path will be ``C:\Program Files\Vector SIL Kit Adapter TAP <TAP_ADAPTER_VERSION>``, with <TAP_ADAPTER_VERSION> as the version of the TAP adapter you install. 
Depending on your system this default path can be ``Program Files (x86)``.

## Run the sil-kit-adapter-tap
This application allows the user to attach a TAP device of a Linux or Windows system to the Vector SIL Kit.

Before you start the adapter there always needs to be a sil-kit-registry running already. Start it e.g. like this:

    /path/to/SilKit-x.y.z-$platform/SilKit/bin/sil-kit-registry --listen-uri 'silkit://0.0.0.0:8501'

It is also necessary that the TAP device exists before the ``sil-kit-adapter-tap`` is started. 

**Hint:** If your TAP device has been created by a third party application (you want the SIL Kit to connect to) it is possible that this TAP device resource is 'flagged' as busy/blocked. In this case you just can create another TAP device for usage with the ``sil-kit-adapter-tap`` by yourself and bridge (``brctl`` on Linux, ``netsh bridge`` on Windows) it with the TAP device of your third party application.

The application *optionally* takes the following command line arguments (default between curly braces):

    sil-kit-adapter-tap [--name <participant's name{SilKitAdapterTap}>]
      [--configuration <path to .silkit.yaml or .json configuration file>]
      [--registry-uri silkit://<host{localhost}>:<port{8501}>]
      [--log <Trace|Debug|Warn|{Info}|Error|Critical|Off>]
      [--tap-name <tap device's name{silkit_tap}>]
      [--network <SIL Kit ethernet network{tap_demo}>]
      [--help]

**Note:** SIL Kit-specific CLI arguments will be overwritten by the config file specified by ``--configuration``.

## Linux TAP Demo
The aim of this demo is to showcase a simple adapter forwarding ethernet traffic from and to a Linux TAP device through
Vector SIL Kit. Traffic being exchanged are ping (ICMP) requests, and the answering device replies to them.

This demo is further explained in [tap/demos/DemoLinux/README.md](tap/demos/DemoLinux/README.md).

## Windows TAP Demo
The aim of this demo is to showcase a simple adapter forwarding ethernet traffic from and to a Windows TAP device through
Vector SIL Kit. Traffic being exchanged are ping (ICMP) requests, and the answering device replies to them.

This demo is further explained in [tap/demos/DemoWindows/README.md](tap/demos/DemoWindows/README.md).

## Connecting an Adaptive executable to CANoe
The Vector SIL Kit Adapter TAP allows you to connect Adaptive executables to CANoe. A step-by-step guide on how to do this can be found [here](adaptive/README.md).
