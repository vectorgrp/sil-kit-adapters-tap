# SPDX-FileCopyrightText: Copyright 2025 Vector Informatik GmbH
# SPDX-License-Identifier: MIT

function(enable_address_sanitizer ENABLE_ASAN)
    if (ENABLE_ASAN)
        add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
        add_link_options(-fsanitize=address)
    endif()
endfunction()

function(enable_undefined_sanitizer ENABLE_UBSAN)
    if (ENABLE_UBSAN)
        add_compile_options(-fsanitize=undefined -fno-omit-frame-pointer)
        add_link_options(-fsanitize=undefined)
    endif()
endfunction()

function(enable_thread_sanitizer ENABLE_THREADSAN)
    if (ENABLE_THREADSAN)
        add_compile_options(-fsanitize=thread -fno-omit-frame-pointer -g3)
        add_link_options(-fsanitize=thread -fno-omit-frame-pointer -g3)
    endif()
endfunction()
