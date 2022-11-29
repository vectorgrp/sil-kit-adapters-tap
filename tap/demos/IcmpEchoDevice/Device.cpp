// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "Device.hpp"

#include <iostream>

namespace demo {

void Device::Process(asio::const_buffer incomingData)
{
    const auto [ethernetHeader, ethernetPayload] = ParseEthernetHeader(asio::buffer(incomingData));
    std::cout << ethernetHeader << std::endl;

    switch (ethernetHeader.etherType)
    {
    case EtherType::Arp:
    {
        const auto arpPacket = ParseArpIp4Packet(ethernetPayload);
        std::cout << arpPacket << std::endl;

        if (arpPacket.operation == ArpOperation::Request)
        {
            if (arpPacket.targetProtocolAddress == _ip4Address)
            {
                EthernetHeader replyEthernetHeader = ethernetHeader;
                replyEthernetHeader.destination = arpPacket.senderHardwareAddress;
                replyEthernetHeader.source = _ethernetAddress;
                std::cout << "Reply: " << replyEthernetHeader << std::endl;

                ArpIp4Packet replyArpPacket{
                    ArpOperation::Reply,
                    _ethernetAddress,
                    _ip4Address,
                    arpPacket.senderHardwareAddress,
                    arpPacket.senderProtocolAddress,
                };
                std::cout << "Reply: " << replyArpPacket << std::endl;

                auto reply = AllocateBuffer(incomingData.size());
                auto dst = asio::buffer(reply);

                dst += WriteEthernetHeader(dst, replyEthernetHeader);
                dst += WriteArpIp4Packet(dst, replyArpPacket);

                _sendFrameCallback(std::move(reply));
            }
        }
        break;
    }
    case EtherType::Ip4:
    {
        const auto [ip4Header, ip4Payload] = ParseIp4Header(ethernetPayload);
        std::cout << ip4Header << " + " << ip4Payload.size() << " bytes payload" << std::endl;

        switch (ip4Header.protocol)
        {
        case Ip4Protocol::ICMP:
        {
            const auto [icmp4Header, icmp4Payload] = ParseIcmp4Header(ip4Payload);
            std::cout << icmp4Header << " + " << icmp4Payload.size() << " bytes payload" << std::endl;

            if (icmp4Header.type == Icmp4Type::EchoRequest)
            {
                if (ip4Header.destinationAddress == _ip4Address)
                {
                    EthernetHeader replyEthernetHeader = ethernetHeader;
                    replyEthernetHeader.destination = replyEthernetHeader.source;
                    replyEthernetHeader.source = _ethernetAddress;
                    std::cout << "Reply: " << replyEthernetHeader << std::endl;

                    Ip4Header replyIp4Header = ip4Header;
                    replyIp4Header.destinationAddress = replyIp4Header.sourceAddress;
                    replyIp4Header.sourceAddress = _ip4Address;
                    std::cout << "Reply: " << replyIp4Header << std::endl;

                    Icmp4Header replyIcmp4Header = icmp4Header;
                    replyIcmp4Header.type = Icmp4Type::EchoReply;
                    std::cout << "Reply: " << replyIcmp4Header << std::endl;

                    auto reply = AllocateBuffer(incomingData.size());
                    auto dst = asio::buffer(reply);

                    dst += WriteEthernetHeader(dst, replyEthernetHeader);
                    dst += WriteIp4Header(dst, replyIp4Header);
                    auto icmp4Dst = dst;
                    dst += WriteIcmp4Header(dst, replyIcmp4Header);
                    asio::buffer_copy(dst, icmp4Payload);

                    InternetChecksum checksum;
                    checksum.AddBuffer(asio::buffer(icmp4Dst));
                    WriteUintBe(icmp4Dst + 2, checksum.GetChecksum());

                    _sendFrameCallback(std::move(reply));
                }
            }

            break;
        }
        default: break;
        }

        break;
    }
    default: break;
    }
}

} // namespace demo
