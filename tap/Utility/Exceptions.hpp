// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <stdexcept>

namespace exceptions {

struct IncompleteReadError : std::runtime_error
{
    IncompleteReadError()
        : std::runtime_error("incomplete read")
    {
    }
};

struct InvalidBufferSize : std::runtime_error
{
    InvalidBufferSize()
        : std::runtime_error("invalid buffer size")
    {
    }
};

struct InvalidFrameSizeError : std::runtime_error
{
    InvalidFrameSizeError()
        : std::runtime_error("invalid frame size")
    {
    }
};

struct InvalidEthernetFrameError : std::runtime_error
{
    InvalidEthernetFrameError()
        : std::runtime_error("invalid ethernet frame")
    {
    }
};

struct InvalidArpPacketError : std::runtime_error
{
    InvalidArpPacketError()
        : std::runtime_error("invalid arp packet")
    {
    }
};

struct InvalidIp4PacketError : std::runtime_error
{
    InvalidIp4PacketError()
        : std::runtime_error("invalid ip v4 packet")
    {
    }
};

class InvalidCli : public std::exception
{
};

template <class exception>
void throwIf(bool b)
{
    if (b)
        throw exception();
}

inline auto& throwInvalidCliIf = throwIf<InvalidCli>;

} // namespace demo

