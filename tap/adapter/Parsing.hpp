// SPDX-FileCopyrightText: Copyright 2025 Vector Informatik GmbH
// SPDX-License-Identifier: MIT

#pragma once

#include <string>

#include "common/Parsing.hpp"

namespace adapters {

/// <summary>
/// Prints the help message containing all switches and arguments.
///
///   The --help switch will be omitted if the user requested it explicitely.
/// </summary>
/// <param name="userRequested">Set this to true to signify the user requested the printing.</param>
void print_help(bool userRequested = false);

/// <summary>
/// Prints the version of the adapter.
/// </summary>
void print_version();

/// <summary>
/// string containing the argument preceding the TAP device name.
/// </summary>
extern const std::string tapNameArg;

/// <summary>
/// string containing the argument preceding the SIL Kit Ethernet network name.
/// </summary>
extern const std::string networkArg;

/// <summary>
/// string containing the argument preceding the optional VLAN tag ID.
/// </summary>
extern const std::string vlanTagArg;

} // namespace adapters
