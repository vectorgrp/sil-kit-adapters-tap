// SPDX-FileCopyrightText: Copyright 2025 Vector Informatik GmbH
// SPDX-License-Identifier: MIT

#include "EthernetAddress.hpp"

#include <ostream>

#include "FormattedBuffer.hpp"

namespace demo {

std::ostream& operator<<(std::ostream& ostream, const EthernetAddress& ethernetAddress)
{
    return ostream << "EthernetAddress(" << FormattedBuffer{asio::buffer(ethernetAddress.data)} << ")";
}

} // namespace demo
