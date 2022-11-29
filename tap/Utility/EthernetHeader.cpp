// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "EthernetHeader.hpp"

#include <ostream>

namespace demo {

std::ostream& operator<<(std::ostream& ostream, const EtherType& etherType)
{
    switch (etherType)
    {
    case demo::EtherType::Ip4: return ostream << "EtherType::Ip4";
    case demo::EtherType::Arp: return ostream << "EtherType::Arp";
    case demo::EtherType::Vlan_802_1q: return ostream << "EtherType::Vlan_802_1q";
    case demo::EtherType::Vlan_802_1ad: return ostream << "EtherType::Vlan_802_1ad";
    }
    return ostream << "EtherType(" << ToUnderlying(etherType) << ")";
}

std::ostream& operator<<(std::ostream& ostream, const EthernetVlanTag& ethernetVlanTag)
{
    return ostream << "EthernetVlanTag(tpid=" << ethernetVlanTag.tpid << ",data=" << ethernetVlanTag.data << ")";
}

std::ostream& operator<<(std::ostream& ostream, const EthernetHeader& ethernetHeader)
{
    return ostream << "EthernetHeader(destination=" << ethernetHeader.destination << ",source=" << ethernetHeader.source
                   << ",etherType=" << ethernetHeader.etherType << ")";
}

} // namespace demo
