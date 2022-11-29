// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <type_traits>

namespace demo {

template <typename T>
auto ToUnderlying(const T t) -> std::enable_if_t<std::is_enum<T>::value, std::underlying_type_t<T>>
{
    return static_cast<std::underlying_type_t<T>>(t);
}

} // namespace demo
