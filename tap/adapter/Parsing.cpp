// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "Parsing.hpp"
#include <iostream>
#include <algorithm>
#include <array>
#include <cstring>


const std::array<std::string, 6> switchesWithArgument = {adapters::networkArg, adapters::tapNameArg, adapters::regUriArg, adapters::logLevelArg, 
                                                         adapters::participantNameArg, adapters::configurationArg};

const std::array<std::string, 1> switchesWithoutArguments = {adapters::helpArg};

bool adapters::thereAreUnknownArguments(int argc, char** argv)
{
    //skip the executable calling:
    argc -= 1;
    argv += 1;
    while (argc)
    {
        if (strncmp(*argv, "--", 2) != 0)
            return true;
        if (std::find(switchesWithArgument.begin(), switchesWithArgument.end(), *argv) != switchesWithArgument.end())
        {
            //switches with argument have an argument to ignore, so skip "2"
            argc -= 2;
            argv += 2;
        }
        else if (std::find(switchesWithoutArguments.begin(), switchesWithoutArguments.end(), *argv)
                 != switchesWithoutArguments.end())
        {
            //switches without argument don't have an argument to ignore, so skip "1"
            argc -= 1;
            argv += 1;
        }
        else
            return true;
    }
    return false;
}

void adapters::print_help(bool userRequested)
{
    std::cout << "Usage (defaults in curly braces if you omit the switch):" << std::endl
              << "sil-kit-adapter-tap ["<<participantNameArg<<" <participant's name{SilKitAdapterTap}>]\n"
                 "  ["<<configurationArg<<" <path to .silkit.yaml or .json configuration file>]\n"
                 "  ["<<regUriArg<<" silkit://<host{localhost}>:<port{8501}>]\n"
                 "  ["<<logLevelArg<<" <Trace|Debug|Warn|{Info}|Error|Critical|Off>]\n"
                 "  ["<<tapNameArg<<" <tap device's name{silkit_tap}>]\n"
                 "  ["<<networkArg<<" <SIL Kit ethernet network{tap_demo}>]\n"
                 "\n"
                 "SIL Kit-specific CLI arguments will be overwritten by the config file passed by " << configurationArg << ".\n";
    std::cout << "\n"
                 "Example:\n"
                 "sil-kit-adapter-tap "<<participantNameArg<<" EthernetTapBridge "
             <<  networkArg<<" tap_bridge\n";
    if (!userRequested)
        std::cout << "\n"
                     "Pass "<<helpArg<<" to get this message.\n";
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

