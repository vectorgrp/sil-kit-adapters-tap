# Ethernet Demo and Adapter Setup
This demo consists of two separate components: the QEMU based guest image contains a live
Linux kernel that reacts to ICMP echo requests on its virtual network interface.

    +-------[ QEMU ]---------+                                +------[ SIL Kit ]------+
    | Debian 11              |<==   [listening to tap10] ==>  |  PosixBasic_Descriptor???|
    |   virtual NIC silkit0  |                                |   <=> virtual Eth1    |
    +------------------------+                                +----------+------------+
                                                                         |
                                                           Vector CANoe <=> 

