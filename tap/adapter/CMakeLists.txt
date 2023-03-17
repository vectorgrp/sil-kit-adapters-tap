
add_executable(SilKitAdapterTap
    "SilKitAdapterTap.cpp"
    "Parsing.cpp")
target_link_libraries(SilKitAdapterTap
    PRIVATE
        Utility
        asio
        SilKit::SilKit
)
set_target_properties(SilKitAdapterTap PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY $<$<BOOL:ALWAYS>:${SILKIT_ADAPTERS_TAP_OUTPUT_DIRECTORY}>
)
add_custom_command(
    TARGET SilKitAdapterTap
    POST_BUILD
    COMMAND ${CMAKE_COMMAND}
    ARGS -E copy $<TARGET_PROPERTY:SilKit::SilKit,LOCATION> ${SILKIT_ADAPTERS_TAP_OUTPUT_DIRECTORY}
)

