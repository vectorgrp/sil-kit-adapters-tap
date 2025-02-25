// SPDX-FileCopyrightText: Copyright 2025 Vector Informatik GmbH
// SPDX-License-Identifier: MIT

#pragma once

#include "asio/ts/buffer.hpp"

namespace demo {

template <typename Header>
struct ParseResult
{
    Header header;
    asio::const_buffer remaining;
};

} // namespace demo