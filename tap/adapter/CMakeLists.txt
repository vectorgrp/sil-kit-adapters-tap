# SPDX-FileCopyrightText: Copyright 2025 Vector Informatik GmbH
# SPDX-License-Identifier: MIT

add_executable(sil-kit-adapter-tap
    "SilKitAdapterTap.cpp"
    "TapConnection.cpp"
    "Parsing.cpp"
)
target_link_libraries(sil-kit-adapter-tap
    PRIVATE
        Utility
        asio
        SilKit::SilKit
        sil-kit-adapters-common
)
set_target_properties(sil-kit-adapter-tap 
    PROPERTIES
    #ensure SilKit shared libraries can be loaded
    INSTALL_RPATH "$ORIGIN/../lib:$ORIGIN"
    BUILD_RPATH "$ORIGIN"
    RUNTIME_OUTPUT_DIRECTORY $<$<BOOL:ALWAYS>:${SILKIT_ADAPTER_TAP_OUTPUT_DIRECTORY}>    
)

copy_sil_kit_to_bin(sil-kit-adapter-tap ${SILKIT_ADAPTER_TAP_OUTPUT_DIRECTORY} ${SILKIT_ADAPTER_TAP_LIBRARY_DIRECTORY})