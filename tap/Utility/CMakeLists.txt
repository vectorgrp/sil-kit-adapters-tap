# SPDX-FileCopyrightText: Copyright 2025 Vector Informatik GmbH
# SPDX-License-Identifier: MIT

add_library(
    Utility STATIC

    BufferReader.cpp
    BufferReader.hpp
    BufferWriter.cpp
    BufferWriter.hpp
    Enums.hpp
    Enums.cpp
    Exceptions.hpp
    Exceptions.cpp
    FormattedBuffer.cpp
    FormattedBuffer.hpp
    InternetChecksum.cpp
    InternetChecksum.hpp
    ParseResult.hpp
    ParseResult.cpp
    ReadUintBe.hpp
    ReadUintBe.cpp
    WriteUintBe.hpp
    WriteUintBe.cpp

    EthernetAddress.hpp
    EthernetAddress.cpp
    EthernetHeader.hpp
    EthernetHeader.cpp

    ArpIp4Packet.hpp
    ArpIp4Packet.cpp

    Ip4Address.hpp
    Ip4Address.cpp
    Ip4Header.hpp
    Ip4Header.cpp
    Icmp4Header.hpp
    Icmp4Header.cpp
)

target_include_directories(Utility PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(Utility
    PUBLIC
        asio
)

if(MINGW)
    target_compile_definitions(Utility INTERFACE _WIN32_WINNT=0x0601)
    target_link_libraries(Utility PUBLIC -lwsock32 -lws2_32) #windows socket/ wsa
endif()

