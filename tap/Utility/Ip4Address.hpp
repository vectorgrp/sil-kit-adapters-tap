// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "Exceptions.hpp"

#include <array>
#include <iosfwd>

#include <asio/ts/buffer.hpp>

using namespace exceptions;

namespace demo {

struct Ip4Address
{
    std::array<std::uint8_t, 4> data;
};

inline bool operator==(const Ip4Address& lhs, const Ip4Address& rhs)
{
    return lhs.data == rhs.data;
}

inline auto ReadIp4Address(asio::const_buffer buffer) -> Ip4Address
{
    Ip4Address address = {};
    if (asio::buffer_copy(asio::buffer(address.data), buffer) != 4)
    {
        throw InvalidBufferSize{};
    }
    return address;
}

inline auto WriteIp4Address(asio::mutable_buffer target, const Ip4Address& ip4Address) -> std::size_t
{
    if (asio::buffer_copy(target, asio::buffer(ip4Address.data)) != 4)
    {
        throw InvalidBufferSize{};
    }
    return 4;
}

std::ostream& operator<<(std::ostream& ostream, const Ip4Address& ip4Address);

} // namespace demo
