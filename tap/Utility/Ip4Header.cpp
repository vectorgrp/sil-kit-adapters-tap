// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "Ip4Header.hpp"

#include <ostream>

namespace demo {

std::ostream& operator<<(std::ostream& ostream, const Ip4Protocol& protocol)
{
    switch (protocol)
    {
    case demo::Ip4Protocol::ICMP: return ostream << "Ip4Protocol::ICMP";
    case demo::Ip4Protocol::TCP: return ostream << "Ip4Protocol::TCP";
    case demo::Ip4Protocol::UDP: return ostream << "Ip4Protocol::UDP";
    }
    return ostream << "Ip4Protocol(" << unsigned(ToUnderlying(protocol)) << ")";
}

std::ostream& operator<<(std::ostream& ostream, const Ip4Header& ip4Header)
{
    return ostream << "Ip4Header(totalLength=" << ip4Header.totalLength
                   << ",identification=" << ip4Header.identification << ",dontFragment=" << ip4Header.dontFragment
                   << ",moreFragments=" << ip4Header.moreFragments << ",fragmentOffset=" << ip4Header.fragmentOffset
                   << ",timeToLive=" << unsigned(ip4Header.timeToLive) << ",protocol=" << ip4Header.protocol
                   << ",checksum=" << ip4Header.checksum << ",sourceAddress=" << ip4Header.sourceAddress
                   << ",destinationAddress=" << ip4Header.destinationAddress << ")";
}

} // namespace demo
