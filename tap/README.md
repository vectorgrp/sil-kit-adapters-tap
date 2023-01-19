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

Now is a good point to start the ``sil-kit-registry``, the ``SilKitDemoEthernetIcmpEchoDevice`` and the demo helper script ``start_ping_demo`` - which creates the TAP device, connects it to the adapter and afterwards adds it to the network namespace and starts pinging the echos device from there - in separate terminals:

    ./path/to/SilKit-x.y.z-$platform/SilKit/bin/sil-kit-registry --listen-uri 'silkit://127.0.0.1:8501'
        
    ./build/bin/SilKitDemoEthernetIcmpEchoDevice

    sudo ./tap/demos/start_ping_demo.sh
    
The applications will produce output when they send and receive Ethernet frames from the TAP device or the Vector SIL Kit. The console output of ``SilKitAdapterTap`` is redirected to ``/build/bin/SilKitAdapterTap.out``.

## Starting CANoe 16
You can also start ``CANoe 16 SP3`` or newer and load the ``Tap_adapter_CANoe.cfg`` from the ``CANoe`` directory and start the
measurement.

## ICMP Ping and Pong
The ping requests should all receive responses.
    
You should see output output similar to the following from the ``SilKitDemoEthernetIcmpEchoDevice`` application:

    SIL Kit >> Demo: Ethernet frame (98 bytes)
    EthernetHeader(destination=EthernetAddress(52:54:56:53:4b:55),source=EthernetAddress(02:d5:de:c1:7f:82),etherType=EtherType::Ip4)
    Ip4Header(totalLength=84,identification=28274,dontFragment=1,moreFragments=0,fragmentOffset=0,timeToLive=64,protocol=Ip4Protocol::ICMP,checksum=15553,sourceAddress=192.168.7.2,destinationAddress=192.168.7.35) + 64 bytes payload
    Icmp4Header(type=Icmp4Type::EchoRequest,code=,checksum=33586) + 60 bytes payload
    Reply: EthernetHeader(destination=EthernetAddress(02:d5:de:c1:7f:82),source=EthernetAddress(52:54:56:53:4b:55),etherType=EtherType::Ip4)
    Reply: Ip4Header(totalLength=84,identification=28274,dontFragment=1,moreFragments=0,fragmentOffset=0,timeToLive=64,protocol=Ip4Protocol::ICMP,checksum=15553,sourceAddress=192.168.7.35,destinationAddress=192.168.7.2)
    Reply: Icmp4Header(type=Icmp4Type::EchoReply,code=,checksum=33586)
    SIL Kit >> Demo: ACK for ETH Message with transmitId=718
    Demo >> SIL Kit: Ethernet frame (98 bytes, txId=718)

If CANoe is connected to the SIL Kit, all Ethernet traffic should be visible there as well.
