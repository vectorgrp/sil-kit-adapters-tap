# Ethernet Demo and Adapter Setup
This demo consists of a TAP device "silkit_tap" which is connected to the SIL Kit via ``SilKitAdapterTap`` as a SIL Kit participant. In a second step this TAP device is moved to a Linux network name space and gets a IP configured which is in the same range as the one of the ``SilKitDemoEthernetIcmpEchoDevice``. The Linux ping application is used within this created network name space to ping ``SilKitDemoEthernetIcmpEchoDevice`` through the TAP device via SIL Kit. The application ``SilKitDemoEthernetIcmpEchoDevice``, which is a SIL Kit participant as well, will reply to an ARP request and respond to ICMPv4 echo requests. 

The following sketch shows the general setup: 

    +-----[ Ping from NetNs ]----+                                               +------[ SIL Kit Adapter TAP ]------+
    |  silkit_tap added to NetNs | <= ----------- silkit_tap -------------   =>  |  TapConnection to silkit_tap      |
    |   <=> ping / response      |                                               |     <=> virtual (SIL Kit) Eth1    |
    +----------------------------+                                               +-----------------------------------+
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
  

## SilKitDemoEthernetIcmpEchoDevice
This demo application implements a very simple SIL Kit participant with a single simulated ethernet controller.
The application will reply to an ARP request and respond to ICMPv4 Echo Requests directed to it's hardcoded MAC address
(``52:54:56:53:4B:55``) and IPv4 address (``192.168.7.35``).


# Running the Demos

## Running the Demo Applications

Now is a good point to start the ``sil-kit-registry``, the ``SilKitDemoEthernetIcmpEchoDevice`` and the demo helper script ``start_adapter_and_ping_demo`` - which creates the TAP device, connects it to the adapter and afterwards adds it to the network namespace and starts pinging the echos device from there - in separate terminals:

    /path/to/SilKit-x.y.z-$platform/SilKit/bin/sil-kit-registry --listen-uri 'silkit://0.0.0.0:8501'
        
    ./bin/SilKitDemoEthernetIcmpEchoDevice --log Debug

    sudo ./tap/demos/DemoLinux/start_adapter_and_ping_demo.sh
    
The applications will produce output when they send and receive Ethernet frames from the TAP device or the Vector SIL Kit. The console output of ``SilKitAdapterTap`` is redirected to ``./bin/SilKitAdapterTap.out``.

## ICMP Ping and Pong
The ping requests should all receive responses.
    
You should see output similar to the following from the ``SilKitDemoEthernetIcmpEchoDevice`` application:

    [date time] [EthernetDevice] [debug] SIL Kit >> Demo: Ethernet frame (98 bytes)
    [date time] [EthernetDevice] [debug] EthernetHeader(destination=EthernetAddress(52:54:56:53:4b:55),source=EthernetAddress(9a:97:c4:83:d8:d0),etherType=EtherType::Ip4)
    [date time] [EthernetDevice] [debug] Ip4Header(totalLength=84,identification=21689,dontFragment=1,moreFragments=0,fragmentOffset=0,timeToLive=64,protocol=Ip4Protocol::ICMP,checksum=22138,sourceAddress=192.168.7.2,destinationAddress=192.168.7.35) + 64 bytes payload
    [date time] [EthernetDevice] [debug] Icmp4Header(type=Icmp4Type::EchoRequest,code=,checksum=47730) + 60 bytes payload
    [date time] [EthernetDevice] [debug] Reply: EthernetHeader(destination=EthernetAddress(9a:97:c4:83:d8:d0),source=EthernetAddress(52:54:56:53:4b:55),etherType=EtherType::Ip4)
    [date time] [EthernetDevice] [debug] Reply: Ip4Header(totalLength=84,identification=21689,dontFragment=1,moreFragments=0,fragmentOffset=0,timeToLive=64,protocol=Ip4Protocol::ICMP,checksum=22138,sourceAddress=192.168.7.35,destinationAddress=192.168.7.2)
    [date time] [EthernetDevice] [debug] Reply: Icmp4Header(type=Icmp4Type::EchoReply,code=,checksum=47730)
    [date time] [EthernetDevice] [debug] SIL Kit >> Demo: ACK for ETH Message with transmitId=3
    [date time] [EthernetDevice] [debug] Demo >> SIL Kit: Ethernet frame (98 bytes, txId=3)

## Adding CANoe (16 SP3 or newer) as a participant
If CANoe is connected to the SIL Kit, all ethernet traffic is visible there as well. You can also execute a test unit which checks if the ICMP Ping and Pong is happening as expected.

Before you can connect CANoe to the SIL Kit network you should adapt the ``RegistryUri`` in ``tap/demos/SilKitConfig_CANoe.silkit.yaml`` to the IP address of your system where your sil-kit-registry is running (in case of a WSL2 Ubuntu image e.g. the IP address of Eth0). The configuration file is referenced by both following CANoe use cases (Desktop Edition and Server Edition).

### CANoe Desktop Edition
Load the ``Tap_adapter_CANoe.cfg`` from the ``tap/demos/CANoe`` directory and start the measurement. Optionally you can also start the test unit execution of included test configuration. While the demo is running these tests should be successful.

### CANoe4SW Server Edition (Windows)
You can also run the same test set with ``CANoe4SW SE`` by executing the following PowerShell script ``tap/demos/CANoe4SW_SE/run.ps1``. The test cases are executed automatically and you should see a short test report in PowerShell after execution.

### CANoe4SW Server Edition (Linux)
You can also run the same test set with ``CANoe4SW SE (Linux)``. At first you have to execute the PowerShell script ``tap/demos/CANoe4SW_SE/createEnvForLinux.ps1`` on your windows system by using tools of ``CANoe4SW SE (Windows)`` to prepare your test environment for Linux. In ``tap/demos/CANoe4SW_SE/run.sh`` you should adapt ``canoe4sw_se_install_dir`` to the path of your ``CANoe4SW SE`` installation in your WSL2. Afterwards you can execute ``tap/demos/CANoe4SW_SE/run.sh`` in your WSL2. The test cases are executed automatically and you should see a short test report in your terminal after execution.
