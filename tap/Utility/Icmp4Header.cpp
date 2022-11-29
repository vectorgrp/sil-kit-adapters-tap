// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "Icmp4Header.hpp"

#include <ostream>

namespace demo {

std::ostream& operator<<(std::ostream& ostream, const Icmp4Type& icmp4Type)
{
    switch (icmp4Type)
    {
    case demo::Icmp4Type::EchoReply: return ostream << "Icmp4Type::EchoReply";
    case demo::Icmp4Type::EchoRequest: return ostream << "Icmp4Type::EchoRequest";
    }
    return ostream << "Icmp4Type(" << unsigned(ToUnderlying(icmp4Type)) << ")";
}

std::ostream& operator<<(std::ostream& ostream, const Icmp4Header& icmp4Header)
{
    return ostream << "Icmp4Header(type=" << icmp4Header.type << ",code=" << icmp4Header.code
                   << ",checksum=" << icmp4Header.checksum << ")";
}

} // namespace demo
