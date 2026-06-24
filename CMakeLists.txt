# SPDX-FileCopyrightText: Copyright 2025 Vector Informatik GmbH
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.13)

project(VectorSilKitAdapterTap)

option(BUILD_LINUX_PACKAGE "Creates a SIL Kit Adapter TAP build suitable for package managers in Linux Distributions (.deb)" OFF)

if(BUILD_LINUX_PACKAGE)
    add_subdirectory(docs/man)
endif()

include(common/cmake/RepoTargets.cmake)
include(common/cmake/Building.cmake)

set(SILKIT_ADAPTER_TAP_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")
set(SILKIT_ADAPTER_TAP_LIBRARY_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/lib")

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

include(${CMAKE_CURRENT_LIST_DIR}/cmake/SilKitAdapterVersion.cmake)
configure_silkit_adapter_version(${PROJECT_NAME})

add_definitions(-DSILKIT_ADAPTER_VERSION="${PROJECT_VERSION}")

if (WIN32)
    if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
        # set the default install paths for Windows 32 and 64 bits
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            set(CMAKE_INSTALL_PREFIX "C:/Program Files/Vector SIL Kit Adapter TAP ${PROJECT_VERSION}" CACHE PATH "Default install path" FORCE)
        elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
            set(CMAKE_INSTALL_PREFIX "C:/Program Files (x86)/Vector SIL Kit Adapter TAP ${PROJECT_VERSION}" CACHE PATH "Default install path" FORCE)
        endif()
    endif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

    target_compile_definitions(asio INTERFACE _SILENCE_CXX17_ALLOCATOR_VOID_DEPRECATION_WARNING=1)
    target_compile_definitions(asio INTERFACE WINVER=0x0601 _WIN32_WINNT=0x0601) # Windows 7

endif()

enable_address_sanitizer(${ENABLE_ASAN})
enable_undefined_sanitizer(${ENABLE_UBSAN})
enable_thread_sanitizer(${ENABLE_THREADSAN})

add_subdirectory(tap/adapter)
add_subdirectory(tap/demos)
add_subdirectory(tap/Utility)

################################################################################
# Distribution of the source code and binaries
################################################################################
# Install sources and binaries
# Copy all files from the source and bin directory to the proper destination
# Leave out git repo related data

include(common/cmake/InstallFunctions.cmake)

install_adapter_source(            
    ${CMAKE_CURRENT_SOURCE_DIR}/tap
    ${CMAKE_CURRENT_SOURCE_DIR}/common
    ${CMAKE_CURRENT_SOURCE_DIR}/adaptive
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake
    ${CMAKE_CURRENT_SOURCE_DIR}/docs
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party
    ${CMAKE_CURRENT_SOURCE_DIR}/.devcontainer
    ${CMAKE_CURRENT_SOURCE_DIR}/.vscode
)

install_root_source_files()

install_scripts_to(tap/demos/DemoLinux
    ${CMAKE_CURRENT_SOURCE_DIR}/tap/demos/DemoLinux/start_adapter_and_ping_demo.sh
)

install_scripts_to(tap/demos/DemoLinux/tasks_scripts
    ${CMAKE_CURRENT_SOURCE_DIR}/tap/demos/DemoLinux/tasks_scripts/build_adapter.sh
    ${CMAKE_CURRENT_SOURCE_DIR}/tap/demos/DemoLinux/tasks_scripts/prepare_run_silkit.sh
    ${CMAKE_CURRENT_SOURCE_DIR}/tap/demos/DemoLinux/tasks_scripts/run_IcmpEchoDevice.sh
    ${CMAKE_CURRENT_SOURCE_DIR}/tap/demos/DemoLinux/tasks_scripts/stop_processes.sh
)

install_scripts_to(tap/demos/CANoe4SW_SE
    ${CMAKE_CURRENT_SOURCE_DIR}/tap/demos/CANoe4SW_SE/createEnvironment.sh
    ${CMAKE_CURRENT_SOURCE_DIR}/tap/demos/CANoe4SW_SE/run.sh
    ${CMAKE_CURRENT_SOURCE_DIR}/tap/demos/CANoe4SW_SE/run_all.sh
)

install_scripts_to(adaptive/helper-scripts
    ${CMAKE_CURRENT_SOURCE_DIR}/adaptive/helper-scripts/start_app_sil_kit_adapters_setup.sh
)

include(GNUInstallDirs)

install_silkit_lib(
    ${SILKIT_ADAPTER_TAP_OUTPUT_DIRECTORY}
    ${SILKIT_ADAPTER_TAP_LIBRARY_DIRECTORY}
)

install_demo(sil-kit-demo-ethernet-icmp-echo-device)
install_adapter(sil-kit-adapter-tap)

include(common/cmake/Packaging.cmake)

if(NOT DEFINED PACKAGE_FILENAME_SUFFIX)
    set(PACKAGE_FILENAME_SUFFIX "-${PROJECT_VERSION}")
endif()

declare_package(
    "SilKit-Adapter-TAP"
    "binary release of SIL Kit Adapter TAP"
)

set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})