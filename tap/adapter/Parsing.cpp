// SPDX-FileCopyrightText: Copyright 2025 Vector Informatik GmbH
// SPDX-License-Identifier: MIT

#include "Parsing.hpp"
#include <iostream>

const std::string adapters::tapNameArg = "--tap-name";
const std::string adapters::networkArg = "--network";
const std::string adapters::vlanTagArg = "--vlan-tag";

void adapters::print_help(bool userRequested)
{
    // clang-format off
    std::cout << "Usage (defaults in curly braces if you omit the switch):" << std::endl
              << "sil-kit-adapter-tap\n"
                 "  ["<<participantNameArg<<" <participant's name{SilKitAdapterTap}>]\n"
                 "  ["<<configurationArg<<" <path to .silkit.yaml or .json configuration file>]\n"
                 "  ["<<regUriArg<<" silkit://<host{localhost}>:<port{8501}>]\n"
                 "  ["<<logLevelArg<<" <Trace|Debug|Warn|{Info}|Error|Critical|Off>]\n"
                 "  ["<<tapNameArg<<" <tap device's name{silkit_tap}>]\n"
                 "  ["<<networkArg<<" <SIL Kit ethernet network{Ethernet1}>]\n"
                 "  ["<<vlanTagArg<<" <VLAN ID to inject on frames>]\n"
                 "\n"
                 "SIL Kit-specific CLI arguments will be overwritten by the config file passed by " << configurationArg << ".\n";
    std::cout << "\n"
                 "Example:\n"
                 "sil-kit-adapter-tap " << participantNameArg << " EthernetTapBridge "
              <<  networkArg << " tap_bridge " << vlanTagArg << " 4\n";

    std::cout << "\n"
                     "Pass "<<versionArg<<" to get the version of the Adapter.\n";

    if (!userRequested)
        std::cout << "\n"
                     "Pass "<<helpArg<<" to get this message.\n";
    // clang-format on
};

void adapters::print_version()
{
    std::cout << "SIL Kit Adapter for TAP devices - version: " << SILKIT_ADAPTER_VERSION << std::endl;
}
