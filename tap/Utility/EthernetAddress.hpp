// SPDX-FileCopyrightText: Copyright 2025 Vector Informatik GmbH
// SPDX-License-Identifier: MIT

#pragma once

#include <array>
#include <iosfwd>
#include <cstdint>
#include <asio/ts/buffer.hpp>

#include "Exceptions.hpp"

using namespace exceptions;

namespace demo {

struct EthernetAddress
{
    std::array<std::uint8_t, 6> data;
};

inline bool operator==(const EthernetAddress& lhs, const EthernetAddress& rhs)
{
    return lhs.data == rhs.data;
}

inline auto ReadEthernetAddress(asio::const_buffer buffer) -> EthernetAddress
{
    EthernetAddress address = {};
    if (asio::buffer_copy(asio::buffer(address.data), buffer) != 6)
    {
        throw InvalidBufferSize{};
    }
    return address;
}

inline auto WriteEthernetAddress(asio::mutable_buffer target, const EthernetAddress& ethernetAddress) -> std::size_t
{
    if (asio::buffer_copy(target, asio::buffer(ethernetAddress.data)) != 6)
    {
        throw InvalidBufferSize{};
    }

    return 6;
}

std::ostream& operator<<(std::ostream& ostream, const EthernetAddress& ethernetAddress);

} // namespace demo
