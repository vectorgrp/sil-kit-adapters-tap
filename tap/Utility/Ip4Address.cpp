// SPDX-FileCopyrightText: Copyright 2025 Vector Informatik GmbH
// SPDX-License-Identifier: MIT

#include "Ip4Address.hpp"

#include <ostream>

#include "asio/ts/net.hpp"

namespace demo {

std::ostream &operator<<(std::ostream &ostream, const Ip4Address &ip4Address)
{
    return ostream << asio::ip::address_v4{ip4Address.data};
}

} // namespace demo
