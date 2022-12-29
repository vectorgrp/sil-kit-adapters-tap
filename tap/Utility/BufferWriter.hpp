// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "Exceptions.hpp"
#include "InternetChecksum.hpp"

#include <array>
#include <type_traits>

#include <cstddef>

#include <asio/ts/buffer.hpp>

using namespace exceptions;

namespace demo {

class BufferWriter
{
public:
    explicit BufferWriter(asio::mutable_buffer buffer)
        : _buffer{buffer}
    {
    }

    [[nodiscard]] auto GetOffset() const -> std::size_t { return _offset; }

    [[nodiscard]] auto GetChecksum() const -> std::uint16_t { return _checksum.GetChecksum(); }

    auto Skip(const std::size_t byteCount) -> std::size_t
    {
        const auto skipOffset = _offset;
        _offset += byteCount;
        return skipOffset;
    }

    template <typename ConstBufferSequence>
    auto Write(const ConstBufferSequence& constBufferSequence) -> std::size_t
    {
        const std::size_t putOffset = _offset;
        const std::size_t byteCount = Put(putOffset, constBufferSequence);
        _checksum.AddBuffer(asio::buffer(_buffer + putOffset, byteCount));
        _offset += byteCount;
        return putOffset;
    }

    template <typename T>
    auto WriteBe(const T value) -> std::size_t
    {
        const std::size_t putOffset = _offset;
        const std::size_t byteCount = PutBe(putOffset, value);
        _checksum.AddBuffer(asio::buffer(_buffer + putOffset, byteCount));
        _offset += byteCount;
        return putOffset;
    }

    template <typename ConstBufferSequence>
    auto Put(const std::size_t offset, const ConstBufferSequence& constBufferSequence) -> std::size_t
    {
        CheckPutAt(offset, asio::buffer_size(constBufferSequence));
        return asio::buffer_copy(_buffer + offset, constBufferSequence);
    }

    template <typename T>
    auto PutBe(const std::size_t offset, const T value) -> std::enable_if_t<std::is_unsigned<T>::value, std::size_t>
    {
        CheckPutAt(offset, sizeof(T));

        const auto bytes = static_cast<std::uint8_t*>((_buffer + offset).data());
        for (unsigned byte_index = 0; byte_index < sizeof(T); ++byte_index)
        {
            bytes[sizeof(T) - 1 - byte_index] = (value >> (byte_index * 8u)) & 0xFF;
        }

        return sizeof(T);
    }

    template <typename T>
    auto PutBe(const std::size_t offset, const T value) -> std::enable_if_t<std::is_enum<T>::value, std::size_t>
    {
        return PutBe(offset, std::underlying_type_t<T>(value));
    }

private:
    void CheckPutAt(std::size_t offset, std::size_t byteCount)
    {
        if ((_buffer + offset).size() < byteCount)
        {
            throw InvalidBufferSize{};
        }
    }

private:
    asio::mutable_buffer _buffer;
    std::size_t _offset = 0;

    InternetChecksum _checksum;
};

} // namespace demo
