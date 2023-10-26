// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

namespace adapters {

enum ReturnCode
{
    NO_ERROR = 0,
    CLI_ERROR,
    CONFIGURATION_ERROR,
    OTHER_ERROR,
    FILE_DESCRIPTOR_ERROR = -1
};
}