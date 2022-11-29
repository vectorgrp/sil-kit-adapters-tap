// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "BufferWriter.hpp"
#include "BufferReader.hpp"
#include "ReadUintBe.hpp"
#include "WriteUintBe.hpp"
#include "ParseResult.hpp"

#include "EthernetAddress.hpp"
#include "EthernetHeader.hpp"
#include "Ip4Address.hpp"

#include <iosfwd>

namespace demo {

enum struct Ip4Protocol : std::uint8_t
{
    ICMP = 1,
    TCP = 6,
    UDP = 17,
};

struct Ip4Header
{
    std::uint16_t totalLength;
    std::uint16_t identification;
    bool dontFragment;
    bool moreFragments;
    std::uint16_t fragmentOffset;
    std::uint8_t timeToLive;
    Ip4Protocol protocol;
    std::uint16_t checksum;
    Ip4Address sourceAddress;
    Ip4Address destinationAddress;
};

inline auto ParseIp4Header(asio::const_buffer data) -> ParseResult<Ip4Header>
{
    auto reader = BufferReader{data};

    const auto byte0 = reader.ReadBe<std::uint8_t>();
    const std::uint8_t ipVersion = (byte0 & 0xF0) >> 4u;
    const std::uint32_t internetHeaderLength = (byte0 & 0x0F) * 4u;

    if (ipVersion != 4)
    {
        throw InvalidIp4PacketError{};
    }

    if (internetHeaderLength < 20 || internetHeaderLength > 60 || data.size() < internetHeaderLength)
    {
        throw InvalidIp4PacketError{};
    }

    // NOTE: We ignore byte1 (typeOfService)
    (void)reader.ReadBe<std::uint8_t>();

    const auto totalLength = reader.ReadBe<std::uint16_t>();
    if (internetHeaderLength <= totalLength && data.size() < totalLength)
    {
        throw InvalidIp4PacketError{};
    }

    const auto identification = reader.ReadBe<std::uint16_t>();

    const auto flagsAndFragmentOffset = reader.ReadBe<std::uint16_t>();
    if ((flagsAndFragmentOffset & 0b001) != 0)
    {
        throw InvalidIp4PacketError{};
    }

    const bool dontFragment = (flagsAndFragmentOffset & 0b0100'0000'0000'0000) != 0;
    const bool moreFragments = (flagsAndFragmentOffset & 0b0010'0000'0000'0000) != 0;

    const std::uint16_t fragmentOffset = flagsAndFragmentOffset & 0b0001'1111'1111'1111;

    const auto timeToLive = reader.ReadBe<std::uint8_t>();
    const auto protocol = reader.ReadBe<Ip4Protocol>();
    const auto checksum = reader.ReadBe<std::uint16_t>();
    Ip4Address sourceAddress, destinationAddress;
    reader.ReadInto(asio::buffer(sourceAddress.data));
    reader.ReadInto(asio::buffer(destinationAddress.data));

    return {
        Ip4Header{
            totalLength,
            identification,
            dontFragment,
            moreFragments,
            fragmentOffset,
            timeToLive,
            protocol,
            checksum,
            sourceAddress,
            destinationAddress,
        },
        data + internetHeaderLength,
    };
}

inline auto WriteIp4Header(const asio::mutable_buffer target, const Ip4Header &ip4Header) -> std::size_t
{
    BufferWriter writer{target};

    writer.WriteBe(std::uint8_t((4u << 4u) | (20u / 4u)));
    writer.WriteBe(std::uint8_t(0)); // Type of Service / DSCP+ECN
    writer.WriteBe(ip4Header.totalLength);
    writer.WriteBe(ip4Header.identification);
    writer.WriteBe(
        std::uint16_t((ip4Header.dontFragment << 14u) | (ip4Header.moreFragments << 13u) | ip4Header.fragmentOffset));
    writer.WriteBe(ip4Header.timeToLive);
    writer.WriteBe(ip4Header.protocol);
    const auto checksumOffset = writer.WriteBe(std::uint16_t(0)); // checksum
    writer.Write(asio::buffer(ip4Header.sourceAddress.data));
    writer.Write(asio::buffer(ip4Header.destinationAddress.data));

    writer.PutBe(checksumOffset, writer.GetChecksum());

    return writer.GetOffset();
}

std::ostream &operator<<(std::ostream &ostream, const Ip4Protocol &protocol);
std::ostream &operator<<(std::ostream &ostream, const Ip4Header &ip4Header);

} // namespace demo
