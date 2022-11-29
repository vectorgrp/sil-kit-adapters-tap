// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ReadUintBe.hpp"
#include "WriteUintBe.hpp"
#include "ParseResult.hpp"

#include "Ip4Address.hpp"
#include "Ip4Header.hpp"

#include <iosfwd>

namespace demo {

enum struct Icmp4Type : std::uint8_t
{
    EchoReply = 0,
    EchoRequest = 8,
};

struct Icmp4Header
{
    Icmp4Type type;
    std::uint8_t code;
    std::uint16_t checksum;
};

inline auto ParseIcmp4Header(asio::const_buffer data) -> ParseResult<Icmp4Header>
{
    const auto type = ReadUintBe<Icmp4Type>(data + 0);
    const auto code = ReadUintBe<std::uint8_t>(data + 1);
    const auto checksum = ReadUintBe<std::uint16_t>(data + 2);

    return {
        Icmp4Header{
            type,
            code,
            checksum,
        },
        data + 4,
    };
}

inline auto WriteIcmp4Header(const asio::mutable_buffer target, const Icmp4Header& icmp4Header) -> std::size_t
{
    BufferWriter writer{target};

    writer.WriteBe(icmp4Header.type);
    writer.WriteBe(icmp4Header.code);
    writer.WriteBe(std::uint16_t(0));

    return writer.GetOffset();
}

std::ostream& operator<<(std::ostream& ostream, const Icmp4Type& icmp4Type);
std::ostream& operator<<(std::ostream& ostream, const Icmp4Header& icmp4Header);

} // namespace demo
