// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "Exceptions.hpp"

#include <type_traits>

#include <asio/ts/buffer.hpp>

namespace demo {

template <typename T>
auto WriteUintBe(asio::mutable_buffer buffer, const T value)
    -> std::enable_if_t<std::is_unsigned<T>::value, std::size_t>
{
    if (buffer.size() < sizeof(T))
    {
        throw InvalidBufferSize{};
    }

    const auto bytes = static_cast<unsigned char*>(buffer.data());

    for (unsigned byte_index = 0; byte_index < sizeof(T); ++byte_index)
    {
        bytes[sizeof(T) - 1 - byte_index] = (value >> (byte_index * 8u)) & 0xFF;
    }

    return sizeof(T);
}

template <typename T>
auto WriteUintBe(asio::mutable_buffer buffer, const T value) -> std::enable_if_t<std::is_enum<T>::value, std::size_t>
{
    return WriteUintBe(buffer, static_cast<std::underlying_type_t<T>>(value));
}

} // namespace demo
