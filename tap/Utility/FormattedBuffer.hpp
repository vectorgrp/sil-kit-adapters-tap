// SPDX-FileCopyrightText: Copyright 2025 Vector Informatik GmbH
// SPDX-License-Identifier: MIT

#pragma once

#include <iosfwd>

#include "asio/ts/buffer.hpp"

namespace demo {

struct FormattedBuffer
{
    asio::const_buffer buffer;
};

std::ostream& operator<<(std::ostream& ostream, const FormattedBuffer& formattedBuffer);

} // namespace demo
