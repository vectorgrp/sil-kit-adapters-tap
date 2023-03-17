// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <string>
#include "Exceptions.hpp"

namespace adapters {
/// <summary>
/// Prints the help message containing all switches and arguments.
/// 
///   The --help switch will be omitted if the user requested it explicitely.
/// </summary>
/// <param name="userRequested">Set this to true to signify the user requested the printing.</param>
void print_help(bool userRequested = false);

/// <summary>
/// Searches [argv,argv+argc[ for a string matching argument, starting at args.
/// </summary>
/// <param name="argc">length of the available char**.</param>
/// <param name="argv">start of the available char**.</param>
/// <param name="argument">exemplar to search.</param>
/// <param name="args">starting point of the search.</param>
/// <returns>pointer to the found argument, or NULL otherwise</returns>
char** findArg(int argc, char** argv, const std::string& argument, char** args);

/// <summary>
/// Searches [argv,argv+argc[ for a string following a string matching argument, starting at args.
/// </summary>
/// <param name="argc">length of the available char**.</param>
/// <param name="argv">start of the available char**.</param>
/// <param name="argument">exemplar to search.</param>
/// <param name="args">starting poing of the search.</param>
/// <returns>pointer to the next char* found after argument, or NULL otherwise</returns>
char** findArgOf(int argc, char** argv, const std::string& argument, char** args);

/// <summary>
/// Searches [argv,argv+argc[ for a string following a string matching argument.
/// Returns defaultValue if not found
/// </summary>
/// <param name="argc">length of the available char**.</param>
/// <param name="argv">start of the available char**.</param>
/// <param name="argument">exemplar to search.</param>
/// <param name="defaultValue">value returned if argument is not present in [argv,argv+argc[.</param>
/// <returns>string containing the string following argument if argument is present, or defaultValue otherwise.</returns>
std::string getArgDefault(int argc, char** argv, const std::string& argument, const std::string& defaultValue);

/// <summary>
/// Small utility function to quickly check if "it" is not "cont.end()"
/// </summary>
template <typename iterator, typename container>
void assertAdditionalIterator(const iterator& it, const container& cont)
{
    throwInvalidCliIf(it == cont.end());
}

/// <summary>
/// Searches [argv,argv+argc[ for all strings following a string matching argument, and calls action(those strings)
/// </summary>
/// <typeparam name="Action">lambda or function callable on a char*</typeparam>
/// <param name="argc">number of char**</param>
/// <param name="argv">pointer to first char*</param>
/// <param name="argument">examplar to search</param>
/// <param name="action">Action to call with strings following each exemplar found in [argv,argv+argc[</param>
template<typename Action>
void foreachArgDo(int argc, char** argv, const std::string& argument, const Action& action)
{
    for (char** arg = findArgOf(argc, argv, argument, argv); 
                arg != NULL; 
                arg = findArgOf(argc, argv, argument, arg + 1))
    {
        action(*arg);
    }
}

/// <summary>
/// string containing the argument preceding ethernet adaptation information.
/// </summary>
extern std::string ethArg;

/// <summary>
/// string containing the argument preceding chardev adaptation information.
/// </summary>
extern std::string chardevArg;

} // namespace adapters
