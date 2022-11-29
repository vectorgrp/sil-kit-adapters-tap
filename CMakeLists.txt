cmake_minimum_required(VERSION 3.5)
project(VectorSilKitAdaptersTap)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)

set(SK_QEMU_DEMOS_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")

find_package(Threads REQUIRED)
add_library(asio INTERFACE)
target_include_directories(asio INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/third_party/asio/asio/include)
target_compile_definitions(asio INTERFACE ASIO_STANDALONE=1)
target_link_libraries(asio INTERFACE Threads::Threads)

if (WIN32)
    target_compile_definitions(asio INTERFACE _SILENCE_CXX17_ALLOCATOR_VOID_DEPRECATION_WARNING=1)
    target_compile_definitions(asio INTERFACE WINVER=0x0601 _WIN32_WINNT=0x0601) # Windows 7
endif()

if(NOT SK_DIR)
    message(FATAL_ERROR "Please set the PATH to the SIL Kit in SK_DIR: e.g."
        " cmake -D SK_DIR=../SilKit-x.y.z-Platform/")
endif()
message(STATUS "Searching SilKit package in ${SK_DIR}")
find_package(SilKit 4.0.7
    REQUIRED
    CONFIG
    NO_CMAKE_PACKAGE_REGISTRY
    NO_DEFAULT_PATH
    PATHS "${SK_DIR}"
)
if(NOT TARGET SilKit::SilKit)
    message(FATAL_ERROR "Could not find SilKit package in ${SK_DIR}")
endif()

set(SK_QEMU_DEMOS_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")

add_subdirectory(tap/Utility)
add_subdirectory(tap/adapter)
add_subdirectory(tap/demos)
