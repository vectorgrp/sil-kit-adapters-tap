// SPDX-FileCopyrightText: Copyright 2025 Vector Informatik GmbH
// SPDX-License-Identifier: MIT

#pragma once

namespace adapters {

enum ReturnCode
{
#ifndef WIN32 // already defined in winerror.h
    NO_ERROR = 0,
#endif
    CLI_ERROR = 1,
    CONFIGURATION_ERROR,
    OTHER_ERROR,
    FILE_DESCRIPTOR_ERROR = -1
};
} // namespace adapters