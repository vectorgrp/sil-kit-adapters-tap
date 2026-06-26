// SPDX-FileCopyrightText: Copyright 2025 Vector Informatik GmbH
// SPDX-License-Identifier: MIT

#pragma once

#include <stdexcept>

#include "common/Exceptions.hpp"

namespace adapters {

struct InvalidFileDescriptor : public std::runtime_error
{
    InvalidFileDescriptor()
        : std::runtime_error("an invalid TAP device has been passed to the adapter.")
    {
    }
};

inline void throwInvalidFileDescriptorIf(bool b)
{
    return throwIf<InvalidFileDescriptor>(b);
}

} // namespace adapters
