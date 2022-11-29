// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "Exceptions.hpp"
#include "InternetChecksum.hpp"

#include <array>
#include <type_traits>

#include <cstddef>

#include <asio/ts/buffer.hpp>

namespace demo {

class BufferReader
{
public:
    explicit BufferReader(asio::const_buffer buffer)
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

    template <typename MutableBufferSequence>
    auto ReadInto(const MutableBufferSequence& mutableBufferSequence) -> std::size_t
    {
        const std::size_t getOffset = _offset;
        const std::size_t byteCount = Get(getOffset, mutableBufferSequence);
        _checksum.AddBuffer(asio::buffer(_buffer + getOffset, byteCount));
        _offset += byteCount;
        return getOffset;
    }

    template <typename T>
    auto ReadBeInto(T& value) -> std::size_t
    {
        const std::size_t getOffset = _offset;
        const std::size_t byteCount = GetBe(getOffset, value);
        _checksum.AddBuffer(asio::buffer(_buffer + getOffset, byteCount));
        _offset += byteCount;
        return getOffset;
    }

    template <typename T>
    auto ReadBe() -> T
    {
        T result;
        ReadBeInto(result);
        return result;
    }

    template <typename MutableBufferSequence>
    auto Get(const std::size_t offset, const MutableBufferSequence& mutableBufferSequence) -> std::size_t
    {
        CheckGetAt(offset, asio::buffer_size(mutableBufferSequence));
        return asio::buffer_copy(mutableBufferSequence, _buffer + offset);
    }

    template <typename T>
    auto GetBe(const std::size_t offset, T& value) -> std::enable_if_t<std::is_unsigned<T>::value, std::size_t>
    {
        CheckGetAt(offset, sizeof(T));

        const auto bytes = static_cast<const std::uint8_t*>((_buffer + offset).data());
        value = 0;
        for (unsigned byte_index = 0; byte_index < sizeof(T); ++byte_index)
        {
            value |= static_cast<T>(bytes[sizeof(T) - 1 - byte_index]) << (byte_index * 8u);
        }

        return sizeof(T);
    }

    template <typename T>
    auto GetBe(const std::size_t offset, T& value) -> std::enable_if_t<std::is_enum<T>::value, std::size_t>
    {
        auto underlyingValue = std::underlying_type_t<T>(value);
        const auto byteCount = GetBe(offset, underlyingValue);
        value = static_cast<T>(underlyingValue);
        return byteCount;
    }

private:
    void CheckGetAt(std::size_t offset, std::size_t byteCount)
    {
        if ((_buffer + offset).size() < byteCount)
        {
            throw InvalidBufferSize{};
        }
    }

private:
    asio::const_buffer _buffer;
    std::size_t _offset = 0;

    InternetChecksum _checksum;
};

} // namespace demo
