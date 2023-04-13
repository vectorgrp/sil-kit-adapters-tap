cmake_minimum_required(VERSION 3.5)
project(VectorSilKitAdaptersTap)
include(FetchContent)

set(CMAKE_BUILD_RPATH_USE_ORIGIN ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)

set(SILKIT_ADAPTERS_TAP_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")

find_package(Threads REQUIRED)
add_library(asio INTERFACE)
target_include_directories(asio INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/third_party/asio/asio/include)
target_compile_definitions(asio INTERFACE ASIO_STANDALONE=1)
target_link_libraries(asio INTERFACE Threads::Threads)

if(NOT SILKIT_PACKAGE_DIR)
    FetchContent_Declare(
        silkit
        URL https://github.com/vectorgrp/sil-kit/releases/download/sil-kit%2Fv4.0.17/SilKit-4.0.17-ubuntu-18.04-x86_64-gcc.zip
        DOWNLOAD_DIR ${CMAKE_CURRENT_LIST_DIR}/Downloads
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )

    message(STATUS "SIL Kit: fetching: silkit")
    FetchContent_MakeAvailable(silkit)

    message(STATUS "SIL Kit: using source code from: \"${silkit_SOURCE_DIR}\"")
    set(SILKIT_SOURCE_DIR ${silkit_SOURCE_DIR}/SilKit-Source)
    message(STATUS "SIL Kit: using pre-built binaries from: ${silkit_SOURCE_DIR}")

    find_package(SilKit 4.0.7
        HINTS ${silkit_SOURCE_DIR}
        REQUIRED
    )

    if(NOT TARGET SilKit::SilKit)
        message(FATAL_ERROR "Could not find SilKit package by fetching it from github.com")
    endif()
endif()

message(STATUS "Searching SilKit package in ${SILKIT_PACKAGE_DIR}")
find_package(SilKit 4.0.7
    REQUIRED
    CONFIG
    NO_CMAKE_PACKAGE_REGISTRY
    NO_DEFAULT_PATH
    PATHS "${SILKIT_PACKAGE_DIR}"
)
if(NOT TARGET SilKit::SilKit)
    message(FATAL_ERROR "Could not find SilKit package in ${SILKIT_PACKAGE_DIR}")
endif()

add_subdirectory(tap/Utility)
add_subdirectory(tap/adapter)
add_subdirectory(tap/demos)

################################################################################
# Distribution of the source code and binaries
################################################################################
# Install sources and binaries
# Copy all files from the source and bin directory to the proper destination
# Filter by file types

install(
    DIRECTORY            
        ${CMAKE_CURRENT_SOURCE_DIR}/tap            
    DESTINATION Sources
    COMPONENT source
    FILES_MATCHING
        PATTERN *.cpp
        PATTERN *.cxx
        PATTERN *.c
        PATTERN *.rc
        PATTERN *.h
        PATTERN *.hpp
        PATTERN *.hpp.in
        PATTERN *.json
        PATTERN *.yaml
        PATTERN *.cmake
        PATTERN *.cmake.in
        PATTERN CMakeLists.txt
        PATTERN Readme-*.txt
        PATTERN *.txt.in
        PATTERN *.md
        PATTERN *.xml
    REGEX "/ci/.*" EXCLUDE
    REGEX "/ci$" EXCLUDE
    REGEX "\.git$" EXCLUDE
    REGEX "\.github$" EXCLUDE
)

install(
    DIRECTORY
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party
    DESTINATION Sources
    COMPONENT source
    REGEX "\.git$" EXCLUDE
    REGEX "\.github$" EXCLUDE
)

install(
    FILES
        CMakeLists.txt
        CMakePresets.json
    DESTINATION Sources
    COMPONENT source
)

install(
    FILES
        LICENSE        
    DESTINATION .
    COMPONENT source
)

install(
    DIRECTORY
        ${SILKIT_ADAPTERS_TAP_OUTPUT_DIRECTORY}
    DESTINATION .
    COMPONENT bin
)

###############################################################################
# Packaging
###############################################################################

set(CPACK_GENERATOR "ZIP")

set(CPACK_PACKAGE_DESCRIPTION "binary release of SIL Kit Adapters TAP")
set(CPACK_PACKAGE_NAME "SilKit-Adapters-TAP")
set(CPACK_PACKAGE_VENDOR "Vector Informatik")
set(CPACK_PACKAGE_CONTACT "support@vector.com")
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${SILKIT_ADAPTERS_TAG_VERSION}-ubuntu-20.04-x86_64-gcc")

include(CPack)
