add_executable(SilKitAdapterTap
    "SilKitAdapterTap.cpp"
    "Parsing.cpp")
target_link_libraries(SilKitAdapterTap
    PRIVATE
        Utility
        asio
        SilKit::SilKit
)
set_target_properties(SilKitAdapterTap 
    PROPERTIES
    #ensure SilKit shared libraries can be loaded
    INSTALL_RPATH "$ORIGIN"
    BUILD_RPATH "$ORIGIN"
    RUNTIME_OUTPUT_DIRECTORY $<$<BOOL:ALWAYS>:${SILKIT_ADAPTERS_TAP_OUTPUT_DIRECTORY}>
)
add_custom_command(
    TARGET SilKitAdapterTap
    POST_BUILD
    COMMAND ${CMAKE_COMMAND}
    ARGS -E copy $<TARGET_FILE:SilKit::SilKit> ${SILKIT_ADAPTERS_TAP_OUTPUT_DIRECTORY}
)

