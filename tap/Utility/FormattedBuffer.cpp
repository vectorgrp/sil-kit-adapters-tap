// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "FormattedBuffer.hpp"

#include <array>
#include <ostream>

namespace {

constexpr auto to_hex(std::byte value) -> std::array<char, 2>
{
    constexpr auto hex =
        std::array<char, 16>{'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

    const auto lo_value = static_cast<unsigned>(value) & 0xF;
    const auto hi_value = (static_cast<unsigned>(value) >> 4u) & 0xF;

    return {hex[hi_value], hex[lo_value]};
}

} // namespace

namespace demo {

std::ostream& operator<<(std::ostream& ostream, const FormattedBuffer& formattedBuffer)
{
    const auto bytes = static_cast<const std::byte*>(formattedBuffer.buffer.data());
    auto it = std::ostreambuf_iterator<char>{ostream};

    for (std::size_t index = 0; index < formattedBuffer.buffer.size(); ++index)
    {
        if (index > 0)
        {
            *it++ = ':';
        }

        const auto hex = to_hex(bytes[index]);
        *it++ = hex[0];
        *it++ = hex[1];
    }

    return ostream;
}

} // namespace demo
