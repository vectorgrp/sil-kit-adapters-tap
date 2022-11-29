// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "EthernetAddress.hpp"

#include "FormattedBuffer.hpp"

#include <ostream>

namespace demo {

std::ostream& operator<<(std::ostream& ostream, const EthernetAddress& ethernetAddress)
{
    return ostream << "EthernetAddress(" << FormattedBuffer{asio::buffer(ethernetAddress.data)} << ")";
}

} // namespace demo
