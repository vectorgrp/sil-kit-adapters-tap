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

if(WIN32)
    # Copy the SilKit.dll next to the executables
    add_custom_command(
        TARGET sil-kit-adapter-tap
        POST_BUILD
        COMMAND ${CMAKE_COMMAND}
        ARGS -E copy $<TARGET_FILE:SilKit::SilKit> ${SILKIT_ADAPTER_TAP_OUTPUT_DIRECTORY}/SilKit.dll
    )
    # Copy the SilKit.lib in the lib folder
    get_target_property(SILKIT_DIR_LIB SilKit::SilKit LOCATION) # Get the SilKit.dll full path
    get_filename_component(SILKIT_DIR_LIB ${SILKIT_DIR_LIB} DIRECTORY) # Get the SilKit.dll directory path
    add_custom_command(
        TARGET sil-kit-adapter-tap
        POST_BUILD
        COMMAND ${CMAKE_COMMAND}
        ARGS -E copy ${SILKIT_DIR_LIB}/../lib/SilKit.lib ${SILKIT_ADAPTER_TAP_LIBRARY_DIRECTORY}/SilKit.lib
    )
else()
    # Copy the libSilKit.so in the lib folder
    add_custom_command(
        TARGET sil-kit-adapter-tap
        POST_BUILD
        COMMAND ${CMAKE_COMMAND}
        ARGS -E copy $<TARGET_FILE:SilKit::SilKit> ${SILKIT_ADAPTER_TAP_LIBRARY_DIRECTORY}/libSilKit.so
    )
endif()
