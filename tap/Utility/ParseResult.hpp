// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <asio/ts/buffer.hpp>

namespace demo {

template <typename Header>
struct ParseResult
{
    Header header;
    asio::const_buffer remaining;
};

} // namespace demo