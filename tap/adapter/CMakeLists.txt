add_executable(sil-kit-adapter-tap
    "SilKitAdapterTap.cpp"
    "TapConnection.cpp"
    "Parsing.cpp"
    "SignalHandler.cpp"
)
target_link_libraries(sil-kit-adapter-tap
    PRIVATE
        Utility
        asio
        SilKit::SilKit
)
set_target_properties(sil-kit-adapter-tap 
    PROPERTIES
    #ensure SilKit shared libraries can be loaded
    INSTALL_RPATH "$ORIGIN/../lib:$ORIGIN"
    BUILD_RPATH "$ORIGIN"
    RUNTIME_OUTPUT_DIRECTORY $<$<BOOL:ALWAYS>:${SILKIT_ADAPTER_TAP_OUTPUT_DIRECTORY}>    
)
add_custom_command(
    TARGET sil-kit-adapter-tap
    POST_BUILD
    COMMAND ${CMAKE_COMMAND}
    ARGS -E copy $<TARGET_FILE:SilKit::SilKit> ${SILKIT_ADAPTER_TAP_LIBRARY_DIRECTORY}/libSilKit.so
)
