// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "Parsing.hpp"
#include <iostream>
#include <algorithm>


void adapters::print_help(bool userRequested)
{
    std::cout << "Usage (defaults in curly braces if you omit the switch):" << std::endl
              << "SilKitAdapterTap [--name <participant's name{EthernetTapDevice}>]\n"
                 "  [--registry-uri silkit://<host{localhost}>:<port{8501}>]\n"
                 "  [--log <Trace|Debug|Warn|{Info}|Error|Critical|off>]\n"
                 "  [--tap-name <tap device's name{silkit_tap}>]\n"
                 "  [--network <SIL Kit ethernet network{tap_demo}>]\n";
    std::cout << "\n"
                 "Example:\n"
                 "SilKitAdapterTap --name EthernetTapBridge "
                 "--network tap_bridge";
    if (!userRequested)
        std::cout << "\n"
                     "Pass --help to get this message.\n";
};

char** adapters::findArg(int argc, char** argv, const std::string& argument, char** args)
{
    auto found = std::find_if(args, argv + argc, [argument](const char* arg) -> bool {
        return argument == arg;
    });
    if (found < argv + argc)
    {
        return found;
    }
    return NULL;
};
char** adapters::findArgOf(int argc, char** argv, const std::string& argument, char** args)
{
    auto found = findArg(argc, argv, argument, args);
    if (found != NULL && found + 1 < argv + argc)
    {
        return found + 1;
    }
    return NULL;
};

std::string adapters::getArgDefault(int argc, char** argv, const std::string& argument, const std::string& defaultValue)
{
    auto found = findArgOf(argc, argv, argument, argv);
    if (found != NULL)
        return *(found);
    else
        return defaultValue;
};

