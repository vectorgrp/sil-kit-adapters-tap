cmake_minimum_required(VERSION 3.5)
project(VectorSilKitAdapterTap)
include(FetchContent)

set(CMAKE_BUILD_RPATH_USE_ORIGIN ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)

set(SILKIT_ADAPTER_TAP_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")
set(SILKIT_ADAPTER_TAP_LIBRARY_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/lib")

set(SILKIT_ADAPTER_TAP_VERSION "1.0.3")
add_definitions(-DSILKIT_ADAPTER_TAP_VERSION="${SILKIT_ADAPTER_TAP_VERSION}")

set(SILKIT_VERSION "4.0.50" CACHE STRING "If no SIL Kit package is specified, this version will be downloaded")
set(SILKIT_FLAVOR "ubuntu-18.04-x86_64-gcc" CACHE STRING "If no SIL Kit package is specified, this package flavor will be downloaded")

find_package(Threads REQUIRED)
add_library(asio INTERFACE)
target_include_directories(asio INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/third_party/asio/asio/include)
target_compile_definitions(asio INTERFACE ASIO_STANDALONE=1)
target_link_libraries(asio INTERFACE Threads::Threads)

if (WIN32)
    # set the default install paths for Windows 32 and 64 bits
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(CMAKE_INSTALL_PREFIX "C:/Program Files/Vector SIL Kit Adapter TAP ${SILKIT_ADAPTER_TAP_VERSION}" CACHE PATH "Default install path" FORCE)
    elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
        set(CMAKE_INSTALL_PREFIX "C:/Program Files (x86)/Vector SIL Kit Adapter TAP ${SILKIT_ADAPTER_TAP_VERSION}" CACHE PATH "Default install path" FORCE)
    endif()
    
    target_compile_definitions(asio INTERFACE _SILENCE_CXX17_ALLOCATOR_VOID_DEPRECATION_WARNING=1)
    target_compile_definitions(asio INTERFACE WINVER=0x0601 _WIN32_WINNT=0x0601) # Windows 7

    # Set SIL Kit lib name
    set(SILKIT_DLL_NAME $<IF:$<CONFIG:Release>,SilKit.dll,SilKitd.dll>)
    set(SILKIT_LIB_NAME $<IF:$<CONFIG:Release>,SilKit.lib,SilKitd.lib>)
else()
    # Set SIL Kit lib name
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        set(SILKIT_SO_NAME "libSilKit.so")
    else() # Debug or RelWithDebInfo
        set(SILKIT_SO_NAME "libSilKitd.so")
    endif()
endif()

if(DEFINED SILKIT_PACKAGE_DIR)
    # if the user forces using a specific SIL Kit package, use it
	message(STATUS "SILKIT_PACKAGE_DIR has been set to: ${SILKIT_PACKAGE_DIR}, CMake will look for a SIL Kit package in that directory")
    find_package(SilKit 4.0.40
        REQUIRED
        CONFIG
        NO_CMAKE_PACKAGE_REGISTRY
        NO_DEFAULT_PATH
        PATHS "${SILKIT_PACKAGE_DIR}"
    )
else()
# otherwise, look for an installed version of SIL Kit
    message(STATUS "SILKIT_PACKAGE_DIR has not been set by user. Attempting to find an installed version of SIL Kit")
    find_package(SilKit 4.0.40
        CONFIG
    )
    
    if(NOT SilKit_FOUND)
	# if installed SIL Kit is not found, fetch it from github.com
		message(STATUS "No installed version of SIL Kit was found. Attempting to fetch [SilKit-${SILKIT_VERSION}-${SILKIT_FLAVOR}] from github.com")
        FetchContent_Declare(
            silkit
            URL https://github.com/vectorgrp/sil-kit/releases/download/sil-kit%2Fv${SILKIT_VERSION}/SilKit-${SILKIT_VERSION}-${SILKIT_FLAVOR}.zip
            DOWNLOAD_DIR ${CMAKE_CURRENT_LIST_DIR}/Downloads
        )

        message(STATUS "SIL Kit: fetching [SilKit-${SILKIT_VERSION}-${SILKIT_FLAVOR}]")
        FetchContent_MakeAvailable(silkit)

        set(SILKIT_SOURCE_DIR ${silkit_SOURCE_DIR}/SilKit-Source)
        message(STATUS "SIL Kit: using source code from: \"${SILKIT_SOURCE_DIR}\"")
        message(STATUS "SIL Kit: using pre-built binaries from: ${silkit_SOURCE_DIR}")

        message(STATUS "Searching SilKit package which has been fetched from github.com")
        find_package(SilKit 4.0.40
            REQUIRED
            CONFIG
            NO_CMAKE_PACKAGE_REGISTRY
            NO_DEFAULT_PATH
            PATHS "${silkit_SOURCE_DIR}"
        )
    endif()
endif()

if(TARGET SilKit::SilKit)
	message(STATUS "SIL Kit package has been successfully imported as a CMake target. [version : ${SilKit_VERSION}]")
else()
	message(FATAL_ERROR "Something went wrong : Could not find SIL Kit package.")
endif()

add_subdirectory(tap/Utility)
add_subdirectory(tap/adapter)
add_subdirectory(tap/demos)

################################################################################
# Distribution of the source code and binaries
################################################################################
# Install sources and binaries
# Copy all files from the source and bin directory to the proper destination
# Leave out git repo related data

install(
    DIRECTORY            
        ${CMAKE_CURRENT_SOURCE_DIR}/tap
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party
        ${CMAKE_CURRENT_SOURCE_DIR}/adaptive
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake
        ${CMAKE_CURRENT_SOURCE_DIR}/.devcontainer
        ${CMAKE_CURRENT_SOURCE_DIR}/.vscode
    DESTINATION .
    COMPONENT source
    EXCLUDE_FROM_ALL   
    REGEX "\.git$" EXCLUDE
    REGEX "\.github$" EXCLUDE    
)

install(
    FILES            
        ${CMAKE_CURRENT_SOURCE_DIR}/tap/demos/DemoLinux/start_adapter_and_ping_demo.sh
    DESTINATION tap/demos/DemoLinux
    PERMISSIONS
        OWNER_READ OWNER_WRITE OWNER_EXECUTE
        GROUP_READ GROUP_EXECUTE
        WORLD_READ WORLD_EXECUTE
    COMPONENT source
    EXCLUDE_FROM_ALL
)

install(
    FILES            
        ${CMAKE_CURRENT_SOURCE_DIR}/tap/demos/DemoLinux/tasks_scripts/build_adapter.sh
        ${CMAKE_CURRENT_SOURCE_DIR}/tap/demos/DemoLinux/tasks_scripts/prepare_run_silkit.sh
        ${CMAKE_CURRENT_SOURCE_DIR}/tap/demos/DemoLinux/tasks_scripts/run_IcmpEchoDevice.sh
        ${CMAKE_CURRENT_SOURCE_DIR}/tap/demos/DemoLinux/tasks_scripts/stop_processes.sh
    DESTINATION tap/demos/DemoLinux/tasks_scripts
    PERMISSIONS
        OWNER_READ OWNER_WRITE OWNER_EXECUTE
        GROUP_READ GROUP_EXECUTE
        WORLD_READ WORLD_EXECUTE
    COMPONENT source
    EXCLUDE_FROM_ALL
)


install(
    FILES            
        ${CMAKE_CURRENT_SOURCE_DIR}/tap/demos/CANoe4SW_SE/run.sh
        ${CMAKE_CURRENT_SOURCE_DIR}/tap/demos/CANoe4SW_SE/run_all.sh
    DESTINATION tap/demos/CANoe4SW_SE
    PERMISSIONS
        OWNER_READ OWNER_WRITE OWNER_EXECUTE
        GROUP_READ GROUP_EXECUTE
        WORLD_READ WORLD_EXECUTE
    COMPONENT source
    EXCLUDE_FROM_ALL
)

install(
    FILES            
        ${CMAKE_CURRENT_SOURCE_DIR}/adaptive/helper-scripts/start_app_sil_kit_adapters_setup.sh
    DESTINATION adaptive/helper-scripts
    PERMISSIONS
        OWNER_READ OWNER_WRITE OWNER_EXECUTE
        GROUP_READ GROUP_EXECUTE
        WORLD_READ WORLD_EXECUTE
    COMPONENT source
    EXCLUDE_FROM_ALL
)

install(
    FILES
        CMakeLists.txt
        CMakePresets.json
        LICENSE
        README.md
        SECURITY.md
        CONTRIBUTING.md
    DESTINATION .
    COMPONENT source
    EXCLUDE_FROM_ALL
)

include(GNUInstallDirs)

if(WIN32)
    install(
        FILES
            ${SILKIT_ADAPTER_TAP_OUTPUT_DIRECTORY}/${SILKIT_DLL_NAME}
        DESTINATION ${CMAKE_INSTALL_BINDIR}
        COMPONENT lib
        EXCLUDE_FROM_ALL
    )
    install(
        FILES
            ${SILKIT_ADAPTER_TAP_LIBRARY_DIRECTORY}/${SILKIT_LIB_NAME}
        DESTINATION ${CMAKE_INSTALL_LIBDIR}
        COMPONENT lib
        EXCLUDE_FROM_ALL
    )
else()
    install(
        FILES
            ${SILKIT_ADAPTER_TAP_LIBRARY_DIRECTORY}/${SILKIT_SO_NAME}
        DESTINATION ${CMAKE_INSTALL_LIBDIR}
        PERMISSIONS
            OWNER_READ OWNER_WRITE
            GROUP_READ
            WORLD_READ
        COMPONENT lib
        EXCLUDE_FROM_ALL
    )
endif()

install(
    TARGETS
        sil-kit-demo-ethernet-icmp-echo-device
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT Runtime
    PERMISSIONS
        OWNER_READ OWNER_WRITE OWNER_EXECUTE
        GROUP_READ GROUP_EXECUTE
        WORLD_READ WORLD_EXECUTE    
    EXCLUDE_FROM_ALL
)

# only sil-kit-adapter-tap should be installed to /usr/local/bin by calling --target install (therefore it is not excluded)
install(
    TARGETS
        sil-kit-adapter-tap
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT Runtime
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT Runtime
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/sil-kit-adapter-tap COMPONENT Development   
    PERMISSIONS
        OWNER_READ OWNER_WRITE OWNER_EXECUTE
        GROUP_READ GROUP_EXECUTE
        WORLD_READ WORLD_EXECUTE
)

if (DEFINED CMAKE_INSTALL_MANDIR)
    if (UNIX)
        install(
            FILES
                ${CMAKE_CURRENT_SOURCE_DIR}/docs/man/sil-kit-adapter-tap.1
            DESTINATION ${CMAKE_INSTALL_MANDIR}/man1/
            COMPONENT docs
        )
    endif()
endif()


###############################################################################
# Packaging
###############################################################################

set(CPACK_GENERATOR "ZIP")

set(CPACK_PACKAGE_DESCRIPTION "binary release of SIL Kit Adapter TAP")
set(CPACK_PACKAGE_NAME "SilKit-Adapter-TAP")
set(CPACK_PACKAGE_VENDOR "Vector Informatik")
set(CPACK_PACKAGE_CONTACT "support@vector.com")
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}${PACKAGE_FILENAME_SUFFIX}")

set(CPACK_ARCHIVE_COMPONENT_INSTALL ON)
set(CPACK_COMPONENTS_ALL_IN_ONE_PACKAGE ON)
set(CPACK_COMPONENT_INCLUDE_TOPLEVEL_DIRECTORY ON)

include(CPack)
