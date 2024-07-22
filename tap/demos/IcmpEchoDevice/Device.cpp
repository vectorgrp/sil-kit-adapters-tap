// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "Device.hpp"

#include <iostream>

namespace demo {

void Device::Process(asio::const_buffer incomingData)
{
    const auto [ethernetHeader, ethernetPayload] = ParseEthernetHeader(asio::buffer(incomingData));
    std::ostringstream SILKitDebugMessage;

    SILKitDebugMessage << ethernetHeader;
    _logger->Debug(SILKitDebugMessage.str());

    switch (ethernetHeader.etherType)
    {
    case EtherType::Arp:
    {
        const auto arpPacket = ParseArpIp4Packet(ethernetPayload);
        SILKitDebugMessage.str("");
        SILKitDebugMessage << arpPacket;
        _logger->Debug(SILKitDebugMessage.str());

        if (arpPacket.operation == ArpOperation::Request)
        {
            if (arpPacket.targetProtocolAddress == _ip4Address)
            {
                EthernetHeader replyEthernetHeader = ethernetHeader;
                replyEthernetHeader.destination = arpPacket.senderHardwareAddress;
                replyEthernetHeader.source = _ethernetAddress;

                SILKitDebugMessage.str("");
                SILKitDebugMessage << "Reply: " << replyEthernetHeader;
                _logger->Debug(SILKitDebugMessage.str());

                ArpIp4Packet replyArpPacket{
                    ArpOperation::Reply,
                    _ethernetAddress,
                    _ip4Address,
                    arpPacket.senderHardwareAddress,
                    arpPacket.senderProtocolAddress,
                };

                SILKitDebugMessage.str("");
                SILKitDebugMessage << "Reply: " << replyArpPacket;
                _logger->Debug(SILKitDebugMessage.str());

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

        SILKitDebugMessage.str("");
        SILKitDebugMessage << ip4Header << " + " << ip4Payload.size() << " bytes payload";
        _logger->Debug(SILKitDebugMessage.str());

        switch (ip4Header.protocol)
        {
        case Ip4Protocol::ICMP:
        {
            const auto [icmp4Header, icmp4Payload] = ParseIcmp4Header(ip4Payload);

            SILKitDebugMessage.str("");
            SILKitDebugMessage << icmp4Header << " + " << icmp4Payload.size() << " bytes payload";
            _logger->Debug(SILKitDebugMessage.str());

            if (icmp4Header.type == Icmp4Type::EchoRequest)
            {
                if (ip4Header.destinationAddress == _ip4Address)
                {
                    EthernetHeader replyEthernetHeader = ethernetHeader;
                    replyEthernetHeader.destination = replyEthernetHeader.source;
                    replyEthernetHeader.source = _ethernetAddress;

                    SILKitDebugMessage.str("");
                    SILKitDebugMessage << "Reply: " << replyEthernetHeader;
                    _logger->Debug(SILKitDebugMessage.str());

                    Ip4Header replyIp4Header = ip4Header;
                    replyIp4Header.destinationAddress = replyIp4Header.sourceAddress;
                    replyIp4Header.sourceAddress = _ip4Address;

                    SILKitDebugMessage.str("");
                    SILKitDebugMessage << "Reply: " << replyIp4Header;
                    _logger->Debug(SILKitDebugMessage.str());

                    Icmp4Header replyIcmp4Header = icmp4Header;
                    replyIcmp4Header.type = Icmp4Type::EchoReply;

                    SILKitDebugMessage.str("");
                    SILKitDebugMessage << "Reply: " << replyIcmp4Header;
                    _logger->Debug(SILKitDebugMessage.str());


                    auto reply = AllocateBuffer(incomingData.size());
                    auto dst = asio::buffer(reply);

                    dst += WriteEthernetHeader(dst, replyEthernetHeader);
                    dst += WriteIp4Header(dst, replyIp4Header);
                    asio::mutable_buffer icmp4Dst(dst);
                    dst += WriteIcmp4Header(dst, replyIcmp4Header);
                    asio::buffer_copy(dst, icmp4Payload);

                    InternetChecksum checksum;
                    checksum.AddBuffer(icmp4Dst);
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
