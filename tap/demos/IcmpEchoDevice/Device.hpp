// SPDX-FileCopyrightText: Copyright 2025 Vector Informatik GmbH
// SPDX-License-Identifier: MIT

#pragma once

#include "Enums.hpp"
#include "Exceptions.hpp"
#include "FormattedBuffer.hpp"
#include "ReadUintBe.hpp"
#include "WriteUintBe.hpp"

#include "EthernetAddress.hpp"
#include "Ip4Address.hpp"

#include "EthernetHeader.hpp"

#include "ArpIp4Packet.hpp"

#include "Ip4Header.hpp"
#include "Icmp4Header.hpp"

#include <iostream>
#include <vector>
#include <functional>
#include <cstdint>

#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>
#include <asio/ts/socket.hpp>
#include "silkit/services/logging/all.hpp"

using namespace exceptions;

namespace demo {

class Device
{
public:
    Device(EthernetAddress ethernetAddress, Ip4Address ip4Address, SilKit::Services::Logging::ILogger* logger,
           std::function<void(std::vector<std::uint8_t>)> sendFrameCallback)
        : _ethernetAddress{ethernetAddress}
        , _ip4Address{ip4Address}
        , _logger(logger)
        , _sendFrameCallback{std::move(sendFrameCallback)}
    {
    }

public:
    void Process(asio::const_buffer incomingData);

public:
    std::vector<std::uint8_t> AllocateBuffer(std::size_t size)
    {
        return std::vector<std::uint8_t>(size);
    }

private:
    EthernetAddress _ethernetAddress;
    Ip4Address _ip4Address;
    std::function<void(std::vector<std::uint8_t>)> _sendFrameCallback;
    SilKit::Services::Logging::ILogger* _logger;
};

} // namespace demo
