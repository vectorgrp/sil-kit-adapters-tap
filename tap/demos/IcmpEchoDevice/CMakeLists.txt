add_executable(SilKitDemoEthernetIcmpEchoDevice
    "SilKitDemoEthernetIcmpEchoDevice.cpp"
    Device.hpp
    Device.cpp
    ${CMAKE_SOURCE_DIR}/tap/adapter/Parsing.cpp
    ${CMAKE_SOURCE_DIR}/tap/adapter/SignalHandler.cpp
)
target_link_libraries(SilKitDemoEthernetIcmpEchoDevice
    PRIVATE
        Utility
        SilKit::SilKit
)
set_target_properties(SilKitDemoEthernetIcmpEchoDevice 
    PROPERTIES
    #ensure SilKit shared libraries can be loaded
    INSTALL_RPATH "$ORIGIN/../lib:$ORIGIN"
    BUILD_RPATH "$ORIGIN"
    RUNTIME_OUTPUT_DIRECTORY $<$<BOOL:ALWAYS>:${SILKIT_ADAPTERS_TAP_OUTPUT_DIRECTORY}>
)
