// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ReadUintBe.hpp"
#include "WriteUintBe.hpp"

#include "EthernetAddress.hpp"
#include "EthernetHeader.hpp"
#include "Ip4Address.hpp"

#include <iosfwd>

namespace demo {

enum struct ArpOperation : std::uint16_t
{
    Request = 1,
    Reply = 2,
};

struct ArpIp4Packet
{
    ArpOperation operation;
    EthernetAddress senderHardwareAddress;
    Ip4Address senderProtocolAddress;
    EthernetAddress targetHardwareAddress;
    Ip4Address targetProtocolAddress;
};

inline auto ParseArpIp4Packet(asio::const_buffer data) -> ArpIp4Packet
{
    const auto htype = ReadUintBe<std::uint16_t>(data + 0);
    if (htype != 1)
    {
        throw InvalidArpPacketError{};
    }

    const auto ptype = ReadUintBe<std::uint16_t>(data + 2);
    if (ptype != 0x0800)
    {
        throw InvalidArpPacketError{};
    }

    const auto hlen = ReadUintBe<std::uint8_t>(data + 4);
    if (hlen != 6)
    {
        throw InvalidArpPacketError{};
    }

    const auto plen = ReadUintBe<std::uint8_t>(data + 5);
    if (plen != 4)
    {
        throw InvalidArpPacketError{};
    }

    const auto operation = ReadUintBe<ArpOperation>(data + 6);
    switch (operation)
    {
    case ArpOperation::Request:
    case ArpOperation::Reply: break;
    default: throw InvalidArpPacketError{};
    }

    const auto sha = ReadEthernetAddress(data + 8);
    const auto spa = ReadIp4Address(data + 14);
    const auto tha = ReadEthernetAddress(data + 18);
    const auto tpa = ReadIp4Address(data + 24);

    return {
        operation, sha, spa, tha, tpa,
    };
}

inline auto WriteArpIp4Packet(const asio::mutable_buffer target, const ArpIp4Packet& arpIp4Packet) -> std::size_t
{
    asio::mutable_buffer dst = target;

    dst += WriteUintBe<std::uint16_t>(dst, 1);
    dst += WriteUintBe(dst, EtherType::Ip4);
    dst += WriteUintBe<std::uint8_t>(dst, 6);
    dst += WriteUintBe<std::uint8_t>(dst, 4);
    dst += WriteUintBe(dst, arpIp4Packet.operation);
    dst += WriteEthernetAddress(dst, arpIp4Packet.senderHardwareAddress);
    dst += WriteIp4Address(dst, arpIp4Packet.senderProtocolAddress);
    dst += WriteEthernetAddress(dst, arpIp4Packet.targetHardwareAddress);
    dst += WriteIp4Address(dst, arpIp4Packet.targetProtocolAddress);

    return target.size() - dst.size();
}

std::ostream& operator<<(std::ostream& ostream, const ArpOperation& arpOperation);
std::ostream& operator<<(std::ostream& ostream, const ArpIp4Packet& arpIp4Packet);

} // namespace demo
