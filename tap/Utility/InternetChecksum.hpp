// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ReadUintBe.hpp"

#include <optional>

#include <cstdint>
#include <cassert>

#include <asio/ts/buffer.hpp>

namespace demo {

class InternetChecksum
{
public:
    void AddBuffer(asio::const_buffer buffer)
    {
        if (buffer.size() == 0)
        {
            return;
        }

        if (_partial)
        {
            AddByte(*static_cast<const std::uint8_t *>(buffer.data()));
            buffer += 1;
        }

        const std::size_t bytePairCount = buffer.size() / std::size_t(2);
        for (std::size_t bytePairIndex = 0; bytePairIndex < bytePairCount; ++bytePairIndex)
        {
            const auto bytes = static_cast<const std::uint8_t *>(buffer.data());
            AddPair(bytes[bytePairIndex * 2 + 0], bytes[bytePairIndex * 2 + 1]);
        }
        buffer += bytePairCount * 2;

        if (buffer.size() == 1)
        {
            AddByte(*static_cast<const std::uint8_t *>(buffer.data()));
            buffer += 1;
        }

        assert(buffer.size() == 0);
    }

    [[nodiscard]] std::uint16_t GetChecksum() const
    {
        const std::uint64_t accumulator = _accumulator + (_partial.value_or(0) << 8u);
        return ~(std::uint16_t(accumulator >> 16u) + std::uint16_t(accumulator));
    }

private:
    void AddByte(const std::uint8_t x)
    {
        if (_partial)
        {
            AddPair(*_partial, x);
            _partial.reset();
        }
        else
        {
            _partial = x;
        }
    }

    void AddPair(const std::uint8_t a, const std::uint8_t b) { _accumulator += (a << 8u) | b; }

private:
    std::uint64_t _accumulator = 0;
    std::optional<std::uint8_t> _partial;
};

} // namespace demo