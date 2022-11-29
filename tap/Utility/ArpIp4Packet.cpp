// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "ArpIp4Packet.hpp"

#include <ostream>

namespace demo {

std::ostream& operator<<(std::ostream& ostream, const ArpOperation& arpOperation)
{
    switch (arpOperation)
    {
    case demo::ArpOperation::Request: return ostream << "ArpOperation::Request";
    case demo::ArpOperation::Reply: return ostream << "ArpOperation::Reply";
    }
    return ostream << "ArpOperation(" << ToUnderlying(arpOperation) << ")";
}

std::ostream& operator<<(std::ostream& ostream, const ArpIp4Packet& arpIp4Packet)
{
    return ostream << "ArpIp4Packet(operation=" << arpIp4Packet.operation
                   << ",senderHardwareAddress=" << arpIp4Packet.senderHardwareAddress
                   << ",senderProtocolAddress=" << arpIp4Packet.senderProtocolAddress
                   << ",targetHardwareAddress=" << arpIp4Packet.targetHardwareAddress
                   << ",targetProtocolAddress=" << arpIp4Packet.targetProtocolAddress << ")";
}

} // namespace demo
