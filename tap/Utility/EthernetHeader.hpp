// SPDX-FileCopyrightText: Copyright 2025 Vector Informatik GmbH
// SPDX-License-Identifier: MIT

#pragma once

#include <optional>
#include <iosfwd>
#include <cstdint>
#include <vector>

#include "Enums.hpp"
#include "ReadUintBe.hpp"
#include "WriteUintBe.hpp"
#include "ParseResult.hpp"
#include "EthernetAddress.hpp"

#include "asio/ts/buffer.hpp"

namespace demo {

enum struct EtherType : std::uint16_t
{
    Ip4 = 0x0800,
    Arp = 0x0806,

    Vlan_802_1q = 0x8100,
    Vlan_802_1ad = 0x88A8,
};

struct EthernetVlanTag
{
    EtherType tpid;
    std::uint16_t data;
};

inline auto WriteEthernetVlanTag(asio::mutable_buffer target, const EthernetVlanTag& ethernetVlanTag) -> std::size_t
{
    target += WriteUintBe(target, ethernetVlanTag.tpid);
    target += WriteUintBe(target, ethernetVlanTag.data);
    return 4;
}

static_assert(sizeof(EthernetVlanTag) == 4 && std::is_trivial<EthernetVlanTag>::value);

struct EthernetHeader
{
    EthernetAddress destination;
    EthernetAddress source;
    std::optional<EthernetVlanTag> vlanTag8021ad;
    std::optional<EthernetVlanTag> vlanTag8021q;
    EtherType etherType;
};

inline auto ParseEthernetHeader(asio::const_buffer data) -> ParseResult<EthernetHeader>
{
    asio::const_buffer remaining = asio::buffer(data);

    EthernetHeader ethernetHeader = {};

    ethernetHeader.destination = ReadEthernetAddress(remaining);
    remaining += 6;

    ethernetHeader.source = ReadEthernetAddress(remaining);
    remaining += 6;

    if (const auto tpid = ReadUintBe<EtherType>(remaining); tpid == EtherType::Vlan_802_1ad)
    {
        ethernetHeader.vlanTag8021ad = EthernetVlanTag{tpid, ReadUintBe<std::uint16_t>(remaining + 2)};
        remaining += 4;
    }

    if (const auto tpid = ReadUintBe<EtherType>(remaining); tpid == EtherType::Vlan_802_1q)
    {
        ethernetHeader.vlanTag8021q = EthernetVlanTag{tpid, ReadUintBe<std::uint16_t>(remaining + 2)};
        remaining += 4;
    }

    ethernetHeader.etherType = ReadUintBe<EtherType>(remaining);
    remaining += 2;

    return {ethernetHeader, remaining};
}

inline auto WriteEthernetHeader(const asio::mutable_buffer target, EthernetHeader ethernetHeader) -> std::size_t
{
    asio::mutable_buffer dst = target;

    dst += WriteEthernetAddress(dst, ethernetHeader.destination);
    dst += WriteEthernetAddress(dst, ethernetHeader.source);

    if (ethernetHeader.vlanTag8021ad)
    {
        dst += WriteEthernetVlanTag(dst, ethernetHeader.vlanTag8021ad.value());
    }

    if (ethernetHeader.vlanTag8021q)
    {
        dst += WriteEthernetVlanTag(dst, ethernetHeader.vlanTag8021q.value());
    }

    dst += WriteUintBe(dst, ethernetHeader.etherType);

    return target.size() - dst.size();
}

std::ostream& operator<<(std::ostream& ostream, const EtherType& etherType);
std::ostream& operator<<(std::ostream& ostream, const EthernetVlanTag& ethernetVlanTag);
std::ostream& operator<<(std::ostream& ostream, const EthernetHeader& ethernetHeader);

} // namespace demo

namespace adapters {
namespace vlan {

// Injects an 802.1Q VLAN tag (TPID 0x8100, PCP=0, DEI=0) with the given VID
// into the frame at offset 12 (between source MAC and EtherType).
// Returns the modified frame.
inline auto InjectVlanTag(std::vector<std::uint8_t> frame, std::uint16_t vid) -> std::vector<std::uint8_t>
{
    // Need at least: Dst(6) + Src(6) + EtherType(2) = 14 bytes
    if (frame.size() < 14)
    {
        return frame;
    }

    const std::uint8_t vlanBytes[4] = {0x81, 0x00, static_cast<std::uint8_t>(vid >> 8),
                                       static_cast<std::uint8_t>(vid & 0xFF)};
    frame.insert(frame.begin() + 12, vlanBytes, vlanBytes + 4);
    return frame;
}

// Extracts the 802.1Q VLAN ID from a raw Ethernet frame.
// Returns the VID if an 802.1Q tag is present, or std::nullopt otherwise.
template <typename Container>
inline auto ExtractVlanId(const Container& frame) -> std::optional<std::uint16_t>
{
    // Need at least: Dst(6) + Src(6) + TPID(2) + TCI(2) + EtherType(2) = 18 bytes
    if (frame.size() < 18)
    {
        return std::nullopt;
    }

    const std::uint16_t tpid = (static_cast<std::uint16_t>(frame[12]) << 8) | static_cast<std::uint16_t>(frame[13]);
    if (tpid != static_cast<std::uint16_t>(demo::EtherType::Vlan_802_1q))
    {
        return std::nullopt;
    }

    const std::uint16_t tci = (static_cast<std::uint16_t>(frame[14]) << 8) | static_cast<std::uint16_t>(frame[15]);
    return static_cast<std::uint16_t>(tci & 0x0FFF);
}

// Removes the 4-byte 802.1Q VLAN tag from a raw Ethernet frame.
// Returns a new frame without the tag. The caller must ensure a VLAN tag is present.
template <typename Container>
inline auto RemoveVlanTag(const Container& frame) -> std::vector<std::uint8_t>
{
    std::vector<std::uint8_t> stripped;
    stripped.reserve(frame.size() - 4);
    stripped.insert(stripped.end(), frame.begin(), frame.begin() + 12);
    stripped.insert(stripped.end(), frame.begin() + 16, frame.end());
    return stripped;
}

} // namespace vlan
} // namespace adapters
