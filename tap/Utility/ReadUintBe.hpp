// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "Exceptions.hpp"

#include <type_traits>

#include <asio/ts/buffer.hpp>

namespace demo {

template <typename T>
auto ReadUintBe(asio::const_buffer buffer) -> std::enable_if_t<std::is_unsigned<T>::value, T>
{
    if (buffer.size() < sizeof(T))
    {
        throw InvalidBufferSize{};
    }

    const auto bytes = static_cast<const unsigned char*>(buffer.data());

    T result = 0;
    for (unsigned byte_index = 0; byte_index < sizeof(T); ++byte_index)
    {
        result |= static_cast<T>(bytes[sizeof(T) - 1 - byte_index]) << (byte_index * 8u);
    }

    return result;
}

template <typename T>
auto ReadUintBe(asio::const_buffer buffer) -> std::enable_if_t<std::is_enum<T>::value, T>
{
    return T{ReadUintBe<std::underlying_type_t<T>>(buffer)};
}

} // namespace demo
