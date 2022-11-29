
add_executable(SilKitDemoEthernetIcmpEchoDevice
    "SilKitDemoEthernetIcmpEchoDevice.cpp"
    Device.hpp
    Device.cpp
)
target_link_libraries(SilKitDemoEthernetIcmpEchoDevice
    PRIVATE
        Utility
        SilKit::SilKit
)
set_target_properties(SilKitDemoEthernetIcmpEchoDevice PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY $<$<BOOL:ALWAYS>:${SK_QEMU_DEMOS_RUNTIME_OUTPUT_DIRECTORY}>
)

