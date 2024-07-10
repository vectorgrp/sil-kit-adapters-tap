# Ethernet Demo and Adapter Setup
This demo consists of a TAP device "silkit_tap" which is connected to the SIL Kit via ``sil-kit-adapter-tap`` as a SIL Kit participant. This TAP device gets 
a IP configured which is in the same range as the one of the ``sil-kit-demo-ethernet-icmp-echo-device``.
The Windows ping application is used to ping ``sil-kit-demo-ethernet-icmp-echo-device`` through the TAP device via SIL Kit. 
The application ``sil-kit-demo-ethernet-icmp-echo-device``, which is a SIL Kit participant as well, will reply to an ARP request and respond to ICMPv4 echo requests. 

The following sketch shows the general setup: 

    +-----------[ Ping ]----------+                                            +------[ SIL Kit Adapter TAP ]------+
    | Source: Windows TAP adapter | <= ----------- silkit_tap ------------- => |  TapConnection to silkit_tap      |
    |    <=> ping / response      |                                            |     <=> virtual (SIL Kit) Eth1    |
    +-----------------------------+                                            +-----------------------------------+
                                                                                             <=>
                                                                                           SIL Kit
                                                                                             <=>                 
    +--[ SilKitDemoEthernetIcmpEchoDevice ]--+                                +-------[ SIL Kit Registry ]--------+
    |                                        | <= ------- SIL Kit -------- => |                                   |
    +----------------------------------------+                                |                                   |
                                                                              |                                   |
    +------------[ Vector CANoe ]------------+                                |                                   |
    |                                        | <= ------- SIL Kit -------- => |                                   |
    +----------------------------------------+                                +-----------------------------------+
  

## sil-kit-demo-ethernet-icmp-echo-device
This demo application implements a very simple SIL Kit participant with a single simulated ethernet controller.
The application will reply to an ARP request and respond to ICMPv4 Echo Requests directed to it's hardcoded MAC address
(``52:54:56:53:4B:55``) and IPv4 address (``192.168.7.35``).

## Creating a Windows TAP Adapter
A Windows TAP Adapter can be created by using the open source tool provided by [OpenVPN on Github](https://github.com/OpenVPN/tap-windows6). The releases are
available [here](https://github.com/OpenVPN/tap-windows6/releases) (the ``dist.win10.zip`` is used in the demo).
After unzipping the folder, run the following commands in an administrator shell:
```
cd /path/to/dist.win10/dist.win10/amd64/
.\devcon.exe install .\OemVista.inf tap0901
```

Then configure the Windows TAP Adapter in the ``Network Connections`` Windows panel. Rename it to ``silkit_tap``, 
then right click to properties and configure the IP, or by command line:
```
netsh interface ip set address silkit_tap static 192.168.7.2 255.255.255.0
```

# Running the Demos

## Running the Demo Applications

Now is a good point to start the ``sil-kit-registry``, the ``sil-kit-demo-ethernet-icmp-echo-device`` in separate terminals:

    /path/to/SilKit-x.y.z-$platform/SilKit/bin/sil-kit-registry.exe --listen-uri 'silkit://0.0.0.0:8501'
    
    ./bin/sil-kit-adapter-tap.exe --log Debug

    ./bin/sil-kit-demo-ethernet-icmp-echo-device.exe --log Debug
    
The applications will produce output when they send and receive Ethernet frames from the TAP device or the Vector SIL Kit.

## ICMP Ping and Pong
To ping the ``sil-kit-demo-ethernet-icmp-echo-device`` and make sure the route over the Windows TAP is taken, specify the source as follow:
```
ping 192.168.7.35 -S 192.168.7.2 -t
```
    
You should see output similar to the following from the ``sil-kit-demo-ethernet-icmp-echo-device`` application:

    [date time] [EthernetDevice] [debug] SIL Kit >> Demo: Ethernet frame (98 bytes)
    [date time] [EthernetDevice] [debug] EthernetHeader(destination=EthernetAddress(52:54:56:53:4b:55),source=EthernetAddress(9a:97:c4:83:d8:d0),etherType=EtherType::Ip4)
    [date time] [EthernetDevice] [debug] Ip4Header(totalLength=84,identification=21689,dontFragment=1,moreFragments=0,fragmentOffset=0,timeToLive=64,protocol=Ip4Protocol::ICMP,checksum=22138,sourceAddress=192.168.7.2,destinationAddress=192.168.7.35) + 64 bytes payload
    [date time] [EthernetDevice] [debug] Icmp4Header(type=Icmp4Type::EchoRequest,code=,checksum=47730) + 60 bytes payload
    [date time] [EthernetDevice] [debug] Reply: EthernetHeader(destination=EthernetAddress(9a:97:c4:83:d8:d0),source=EthernetAddress(52:54:56:53:4b:55),etherType=EtherType::Ip4)
    [date time] [EthernetDevice] [debug] Reply: Ip4Header(totalLength=84,identification=21689,dontFragment=1,moreFragments=0,fragmentOffset=0,timeToLive=64,protocol=Ip4Protocol::ICMP,checksum=22138,sourceAddress=192.168.7.35,destinationAddress=192.168.7.2)
    [date time] [EthernetDevice] [debug] Reply: Icmp4Header(type=Icmp4Type::EchoReply,code=,checksum=47730)
    [date time] [EthernetDevice] [debug] SIL Kit >> Demo: ACK for ETH Message with transmitId=3
    [date time] [EthernetDevice] [debug] Demo >> SIL Kit: Ethernet frame (98 bytes, txId=3)

## Adding CANoe (17 SP3 or newer) as a participant
If CANoe is connected to the SIL Kit, all ethernet traffic is visible there as well. You can also execute a test unit which checks if the ICMP Ping and Pong is happening as expected.

If you encounter connection issues by connecting CANoe to the SIL Kit network you may have to adapt the ``RegistryUri`` in ``tap/demos/SilKitConfig_CANoe.silkit.yaml`` to the IP address of your system where your sil-kit-registry is running. However if you run it on the same system in most cases ``localhost`` should be sufficient. The configuration file is referenced by both following CANoe use cases (Desktop Edition and Server Edition).

### CANoe Desktop Edition
Load the ``Tap_adapter_CANoe.cfg`` from the ``tap/demos/CANoe`` directory and start the measurement. Optionally you can also start the test unit execution of included test configuration. While the demo is running these tests should be successful.

### CANoe4SW Server Edition (Windows)
You can also run the same test set with ``CANoe4SW SE`` by executing the following PowerShell script ``tap/demos/CANoe4SW_SE/run.ps1``. The test cases are executed automatically and you should see a short test report in PowerShell after execution.
