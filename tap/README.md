# Ethernet Demo and Adapter Setup
This demo consists of two separate components: the QEMU based guest image contains a live
Linux kernel that reacts to ICMP echo requests on its virtual network interface.
The SIL Kit component contains a tap connection which is bridged to the tap device representing the virtual QEMU network interface
and implements a transport to a virtual SIL Kit Ethernet bus named "tap_demo".

    +-------[ QEMU ]---------+                                               +------[ SIL Kit ]-------------+
    | Debian 11              | <== [qemu_demo_tap bridged to silkit_tap] ==> |  TapConnection to silkit_tap |
    |   virtual NIC silkit0  |                                               |   <=> virtual Eth1           |
    +------------------------+                                               +---------------+--------------+
                                                                                             |
                                                                               Vector CANoe <=> 

## SilKitAdapterTap
This application allows the user to attach simulated ethernet interface (``nic``) of a QEMU virtual machine to the
SIL Kit.

The application uses the tap device backend provided by QEMU.
It can be configured for the QEMU virtual machine using the following command line argument of QEMU:

    -netdev tap,id=mynet0,ifname=qemu_demo_tap,script=no,downscript=no

The argument of ``ifname=`` specifies a tap device for the QEMU ethernet traffic happening on its virtual ethernet interface.

All *outgoing* ethernet frames on that particular virtual ethernet interface inside of the virtual machine are sent to
the defined tap device.
Any *incoming* data to the tap device is presented to the virtual machine as an incoming ethernet frame on the
virtual interface.

It is necessary to create a second tap device in your linux host system and bridge it to the tap device of QEMU. 
This is needed because the QEMU execution blocks its self created device. Therefore the SIL Kit Adapter needs t own tap device to connect to.

There is a helper script available which can help you with with this bridging.    

The application *optionally* takes the following optional arguments as command line argument:

    ./build/bin/SilKitAdapterTap [--tap-name 'silkit_tap'][--registry-uri 'silkit://localhost:8501'][--participant-name 'EthernetTapDevice'][--network-name 'tap_demo']    

## SilKitDemoEthernetIcmpEchoDevice
This demo application implements a very simple SIL Kit participant with a single simulated ethernet controller.
The application will reply to an ARP request and respond to ICMPv4 Echo Requests directed to it's hardcoded MAC address
(``52:54:56:53:4B:55``) and IPv4 address (``192.168.7.35``).




# Running the Demos

## Running the Demo Applications

Now is a good point to start the ``sil-kit-registry``, ``SilKitAdapterTap`` - which connects the QEMU virtual ethernet
interface with the SIL Kit - and the ``SilKitDemoEthernetIcmpEchoDevice`` in separate terminals:

    wsl$ ./path/to/SilKit-x.y.z-$platform/SilKit/bin/sil-kit-registry --listen-uri 'silkit://127.0.0.1:8501'
    
    wsl$ ./tools/setupNetworkBridge.sh
    
    wsl$ ./build/bin/SilKitAdapterTap
    
    wsl$ ./build/bin/SilKitDemoEthernetIcmpEchoDevice
    
The demo applications will produce output when they send and receive Ethernet frames from QEMU or the Vector SIL Kit.

## Starting CANoe 16
You can also start ``CANoe 16 SP3`` or newer and load the ``Tap_adapter_CANoe.cfg`` from the ``CANoe`` directory and start the
measurement.

## ICMP Ping and Pong
When the virtual machine boots, the network interface created for hooking up with the Vector SIL Kit (``silkit0``) is ``up``.
It automatically assigns the static IP ``192.168.7.2/24`` to the interface.

Apart from SSH you can also log into the QEMU guest with the user ``root`` with password ``root``.

Then ping the demo device four times:

    root@silkit-qemu-demos-guest:~# ping -c4 192.168.7.35

The ping requests should all receive responses.

You should see output similar to the following from the ``SilKitAdapterTap`` application:

    SIL Kit >> Demo: ACK for ETH Message with transmitId=5
    TAP device >> SIL Kit: Ethernet frame (98 bytes, txId=5)
    SIL Kit >> TAP device: Ethernet frame (98 bytes)
    SIL Kit >> Demo: ACK for ETH Message with transmitId=6
    TAP device >> SIL Kit: Ethernet frame (98 bytes, txId=6)
    SIL Kit >> TAP device: Ethernet frame (98 bytes)
    SIL Kit >> Demo: ACK for ETH Message with transmitId=7
    TAP device >> SIL Kit: Ethernet frame (98 bytes, txId=7)
    SIL Kit >> TAP device: Ethernet frame (98 bytes)
    SIL Kit >> Demo: ACK for ETH Message with transmitId=8
    TAP device >> SIL Kit: Ethernet frame (70 bytes, txId=8)
    SIL Kit >> Demo: ACK for ETH Message with transmitId=9
    TAP device >> SIL Kit: Ethernet frame (98 bytes, txId=9)
    SIL Kit >> TAP device: Ethernet frame (98 bytes)

    
And output similar to the following from the ``SilKitDemoEthernetIcmpEchoDevice`` application:

    SIL Kit >> Demo: Ethernet frame (98 bytes)
    EthernetHeader(destination=EthernetAddress(52:54:56:53:4b:55),source=EthernetAddress(52:54:56:53:4b:51),etherType=EtherType::Ip4)
    Ip4Header(totalLength=84,identification=26992,dontFragment=1,moreFragments=0,fragmentOffset=0,timeToLive=64,protocol=Ip4Protocol::ICMP,checksum=16835,sourceAddress=192.168.7.2,destinationAddress=192.168.7.35) + 64 bytes payload
    Icmp4Header(type=Icmp4Type::EchoRequest,code=,checksum=35916) + 60 bytes payload
    Reply: EthernetHeader(destination=EthernetAddress(52:54:56:53:4b:51),source=EthernetAddress(52:54:56:53:4b:55),etherType=EtherType::Ip4)
    Reply: Ip4Header(totalLength=84,identification=26992,dontFragment=1,moreFragments=0,fragmentOffset=0,timeToLive=64,protocol=Ip4Protocol::ICMP,checksum=16835,sourceAddress=192.168.7.35,destinationAddress=192.168.7.2)
    Reply: Icmp4Header(type=Icmp4Type::EchoReply,code=,checksum=35916)
    SIL Kit >> Demo: ACK for ETH Message with transmitId=4
    Demo >> SIL Kit: Ethernet frame (98 bytes, txId=4)

If CANoe is connected to the SIL Kit, all Ethernet traffic should be visible there as well.
