// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <string>
#include <vector>
#include <cstdint>

#include "Exceptions.hpp"

#include "asio/ts/buffer.hpp"
#include "asio/ts/io_context.hpp"

#include "silkit/SilKit.hpp"
#include "silkit/services/logging/all.hpp"

#if WIN32
#include <windows.h>
#include <winioctl.h>
#include "asio/windows/stream_handle.hpp"
#elif defined(__QNX__)
#include <net/if.h>
#include <net/if_tap.h>
#include "asio/posix/stream_descriptor.hpp"
#else // UNIX
#include <linux/if_tun.h>
#include "asio/posix/stream_descriptor.hpp"
#endif

class TapConnection
{
public:
    TapConnection(asio::io_context& io_context, const std::string& tapDevName,
                  std::function<void(std::vector<std::uint8_t>)> onNewFrameHandler,
                  SilKit::Services::Logging::ILogger* logger);

    template<class container>
    auto SendEthernetFrameToTapDevice(const container& data)
    {
        auto sizeSent = _tapDeviceStream.write_some(asio::buffer(data.data(), data.size()));
        if (data.size() != sizeSent )
        {
            throw exceptions::InvalidFrameSizeError{};
        }
    }

private:
    std::array<std::uint8_t, 1600> _ethernetFrameBuffer;
    std::function<void(std::vector<std::uint8_t>)> _onNewFrameHandler;
    SilKit::Services::Logging::ILogger* _logger;

    void ReceiveEthernetFrameFromTapDevice();
    inline auto extractErrorMessage(const int errorCode) -> std::string;

#if WIN32
public:
    // Specific destructor to disable Windows TAP adapter
    ~TapConnection();

private:
    asio::windows::stream_handle _tapDeviceStream;
    HANDLE _fileDescriptor;

    struct WinTapConnection
    {
        // Name of the network connection, that is visible to the user in the Windows control panel
        std::wstring ConnectionName;
        // File path for user mode access
        std::wstring UserModeFilePath;
        std::wstring NetCfgInstanceId;
        std::wstring DeviceInstanceID;
    };

    // Select TAP connection from the Windows registry
    auto GetConnection(const char* tapDeviceName, WinTapConnection& winTapConnection, std::string& errorCmd,
                       LONG& errorCode) -> int;

    // Set the network interface status to connected (status=1) or disconnected (status=0)
    auto SetMediaStatus(HANDLE hDevice, uint32_t status) -> bool;

    // WString converters
    auto ToWString(const char* str) const -> std::wstring;
    auto ToString(const std::wstring& wstring) const -> std::string;

    auto GetTapDeviceFileDescriptor(const char* tapDeviceName) -> HANDLE;

#else // QNX OR UNIX
private:
    asio::posix::stream_descriptor _tapDeviceStream;
    int _fileDescriptor;

    auto GetTapDeviceFileDescriptor(const char *tapDeviceName) -> int;
#endif
};

////////////////////////////
// Inline implementations //
////////////////////////////
auto TapConnection::extractErrorMessage(const int errorCode) -> std::string
{
#ifdef WIN32
    LPTSTR errorText = NULL;

    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS, NULL,
                  errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&errorText, 0, NULL);

    if (errorText != NULL)
    {
        std::string strError(errorText);

        // release memory allocated by FormatMessage()
        LocalFree(errorText);
        errorText = NULL;

        return strError;
    }
    return "";
#else // QNX OR UNIX
    const char* errorMessage = strerror(errorCode);

    if (errorMessage != nullptr)
    {
        return ("\t(" + std::string(errorMessage) + ")");
    }
    return "";
#endif
}