// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "TapConnection.hpp"

#include "Parsing.hpp"
#include "SignalHandler.hpp"
#include "SilKitAdapterTap.hpp"

using namespace exceptions;
using namespace adapters;

TapConnection::TapConnection(asio::io_context& io_context, const std::string& tapDevName,
                  std::function<void(std::vector<std::uint8_t>)> onNewFrameHandler,
                  SilKit::Services::Logging::ILogger* logger)
    : _tapDeviceStream{io_context}
    , _onNewFrameHandler(std::move(onNewFrameHandler))
    , _logger(logger)
{
    _fileDescriptor = GetTapDeviceFileDescriptor(tapDevName.c_str());
#if WIN32
    throwInvalidFileDescriptorIf(_fileDescriptor == nullptr);
#else // UNIX
    throwInvalidFileDescriptorIf(_fileDescriptor < 0);
#endif
    _tapDeviceStream.assign(_fileDescriptor);
    ReceiveEthernetFrameFromTapDevice();
}

void TapConnection::ReceiveEthernetFrameFromTapDevice()
{
    _tapDeviceStream.async_read_some(
        asio::buffer(_ethernetFrameBuffer.data(), _ethernetFrameBuffer.size()),
        [this](const std::error_code ec, const std::size_t bytes_received) {
            try
            {
                if (ec)
                {
                    std::string SILKitErrorMessage = "Unable to receive data from TAP device.\n"
                                                        "Error code: "+ std::to_string(ec.value()) + " (" + ec.message()+ ")\n"
                                                        "Error category: " + ec.category().name();
                    _logger->Error(SILKitErrorMessage);
                }
                else
                {
                    auto frame_data = std::vector<std::uint8_t>(bytes_received);
                    asio::buffer_copy(asio::buffer(frame_data),
                                        asio::buffer(_ethernetFrameBuffer.data(), _ethernetFrameBuffer.size()),
                                        bytes_received);

                    _onNewFrameHandler(std::move(frame_data));
                }
            }
            catch (const std::exception& ex)
            {
                // Handle any exception that might occur
                std::string SILKitErrorMessage = "Exception occurred: " + std::string(ex.what());
                _logger->Error(SILKitErrorMessage);
            }
            // Continue with the next read

            ReceiveEthernetFrameFromTapDevice();
        });
}

#if WIN32
TapConnection::~TapConnection()
{
    _logger->Debug("Disable network media of TAP adapter");
    SetMediaStatus(_fileDescriptor, 0);
}

auto TapConnection::GetConnection(const char* tapDeviceName, WinTapConnection& winTapConnection, std::string& errorCmd, LONG& errorCode) -> int
{
    bool connectionFound = false;

    auto checkCmdResult = [&errorCmd, &errorCode](const std::string& cmd, const LONG resultCode) -> int {
        if (resultCode != ERROR_SUCCESS)
        {
            errorCmd = cmd;
            errorCode = resultCode;
            
            return -1;
        }
        return 0;
    };

    // Open registry keys
    HKEY adapterKey;
    LONG rcOpenAdapter = RegOpenKeyExW(HKEY_LOCAL_MACHINE,
        L"SYSTEM\\CurrentControlSet\\Control\\Class\\{4D36E972-E325-11CE-BFC1-08002BE10318}",
        0, KEY_READ, &adapterKey);

    if (checkCmdResult("RegOpenKeyExW (open adapter)", rcOpenAdapter) == -1) return FILE_DESCRIPTOR_ERROR;

    HKEY connectionKey;
    LONG rcOpenConnection = RegOpenKeyExW(HKEY_LOCAL_MACHINE,
        L"SYSTEM\\CurrentControlSet\\Control\\Network\\{4D36E972-E325-11CE-BFC1-08002BE10318}",
        0, KEY_READ, &connectionKey);

    if (checkCmdResult("RegOpenKeyExW (open connection)", rcOpenConnection) == -1) return FILE_DESCRIPTOR_ERROR;

    DWORD numberOfSubKeys = 0;
    {
        LONG rcQuery = RegQueryInfoKey(adapterKey, nullptr, nullptr, nullptr, &numberOfSubKeys, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
        if (rcQuery != ERROR_SUCCESS)
        {
            numberOfSubKeys = 0;
        }
    }

    // For each key found, try to find the ComponentId "tap0901" for Windows TAP adapter
    for (DWORD keyIndex = 0; keyIndex < numberOfSubKeys; ++keyIndex)
    {
        bool isTap = false;
        wchar_t adapterSubkey[256];
        DWORD keyNameSize = sizeof(adapterSubkey);
        LONG rcEnumKey = RegEnumKeyExW(adapterKey, keyIndex, adapterSubkey, &keyNameSize, nullptr, nullptr, nullptr, nullptr);

        if (checkCmdResult("RegEnumKeyExW", rcEnumKey) == -1) return FILE_DESCRIPTOR_ERROR;

        wchar_t value[1024];
        DWORD  valueSize = sizeof(value);
        DWORD valueType;
        LONG rcGetValue = RegGetValueW(adapterKey, adapterSubkey, L"ComponentId", RRF_RT_REG_SZ, &valueType, value, &valueSize);

        if (checkCmdResult("RegGetValueW (ComponentId)", rcGetValue) == -1) return FILE_DESCRIPTOR_ERROR;

        // The Windows TAP are installed with the devcon.exe tool by specifying the hardware ID tap0901
        if (wcscmp(value, L"tap0901") == 0)
        {
            isTap = true;
        }

        if (isTap)
        {
            wchar_t netCfgInstanceId[1024];
            DWORD  value1Size = sizeof(netCfgInstanceId);
            DWORD value1Type;
            LONG rcGetValue1 = RegGetValueW(adapterKey, adapterSubkey, L"NetCfgInstanceId", RRF_RT_REG_SZ, &value1Type, netCfgInstanceId, &value1Size);

            if (checkCmdResult("RegGetValueW (NetCfgInstanceIdp)", rcGetValue1) == -1) return FILE_DESCRIPTOR_ERROR;

            wchar_t deviceInstanceID[1024];
            DWORD  value2Size = sizeof(deviceInstanceID);
            DWORD value2Type;
            LONG rcGetValue2 = RegGetValueW(adapterKey, adapterSubkey, L"DeviceInstanceID", RRF_RT_REG_SZ, &value2Type, deviceInstanceID, &value2Size);

            if (checkCmdResult("RegGetValueW (DeviceInstanceID)", rcGetValue2) == -1) return FILE_DESCRIPTOR_ERROR;

            wchar_t connectionSubKey[1024];
            wcscpy_s(connectionSubKey, 1024, netCfgInstanceId);
            wcscat_s(connectionSubKey, 1024, L"\\Connection");

            wchar_t connectionName[1024];
            DWORD  value3Size = sizeof(connectionName);
            DWORD value3Type;
            LONG rcGetValue3 = RegGetValueW(connectionKey, connectionSubKey, L"Name", RRF_RT_REG_SZ, &value3Type, connectionName, &value3Size);

            if (checkCmdResult("RegGetValueW (Name)", rcGetValue3) == -1) return FILE_DESCRIPTOR_ERROR;

            if (connectionName == ToWString(tapDeviceName))
            {
                wchar_t filepath[1024];
                wcscpy_s(filepath, 1024, L"\\\\.\\Global\\");
                wcscat_s(filepath, 1024, netCfgInstanceId);
                wcscat_s(filepath, 1024, L".tap");

                winTapConnection.ConnectionName = connectionName;
                winTapConnection.UserModeFilePath = filepath;
                winTapConnection.NetCfgInstanceId = netCfgInstanceId;
                winTapConnection.DeviceInstanceID = deviceInstanceID;

                connectionFound = true;
                break;
            }
        }
    }

    RegCloseKey(adapterKey);
    RegCloseKey(connectionKey);

    if (connectionFound)
    {
        return NO_ERROR;
    }
    else
    {
        return OTHER_ERROR;
    }
}

auto TapConnection::SetMediaStatus(HANDLE hDevice, uint32_t status) -> bool
{
    // Function 6: SET_MEDIA_STATUS
    DWORD dwIoControlCode = CTL_CODE(FILE_DEVICE_UNKNOWN, 6, METHOD_BUFFERED, FILE_ANY_ACCESS);
    DWORD len;
    OVERLAPPED overlapped;
    memset(&overlapped, 0, sizeof(overlapped));
    BOOL rcControl = DeviceIoControl(hDevice, dwIoControlCode, &status, sizeof(status), &status, sizeof(len), &len, &overlapped);
    if (rcControl != 0)
    {
        return true;
    }
    else
    {
        DWORD errorControl = GetLastError();
        if (errorControl == ERROR_IO_PENDING)
        {
            BOOL rcWait = GetOverlappedResult(hDevice, &overlapped, &len, TRUE);
            if (rcWait != 0)
            {
                return true; // IoControl succeeded
            }
            else
            {
                return false;
            }
        }
        else
        {
            return false;
        }
    }
}

auto TapConnection::ToWString(const char* str) const -> std::wstring
{
    std::wstring result;

    int len = ::MultiByteToWideChar(CP_ACP, 0, str, -1, nullptr, 0);
    if (len > 0)
    {
        result.resize(len); // len includes the terminating null character
        ::MultiByteToWideChar(CP_ACP, 0, str, -1, &(result[0]), len);
        result.pop_back(); // remove the terminating null character
    }

    return result;
}

auto TapConnection::ToString(const std::wstring& wstring) const -> std::string
{
    std::string str;
    std::transform(wstring.begin(), wstring.end(), std::back_inserter(str), [](wchar_t c) {
        return static_cast<char>(c);
    });
    return str;
}   

auto TapConnection::GetTapDeviceFileDescriptor(const char* tapDeviceName) -> HANDLE
{
    HANDLE tapFileHandle;
    WinTapConnection mTapConnections;
    std::string errorCmd;
    LONG errorCode;

    const int res = GetConnection(tapDeviceName, mTapConnections, errorCmd, errorCode);
    if (res == FILE_DESCRIPTOR_ERROR)
    {
        _logger->Error("Failed to execute Windows system call " + errorCmd
                       + ". Error code: " + std::to_string(errorCode) + ", " + extractErrorMessage(errorCode)
                       + "(Hint): Ensure that the network interface \"" + std::string(tapDeviceName)
                       + "\" specified in [--tap-name] exists and is operational.");
        return nullptr;
    }
    else if (res == OTHER_ERROR)
    {
        // If no error on Windows registry access but tapDeviceName not found
        _logger->Error(std::string(tapDeviceName) + " not found in Windows network connections."
                       + "\n(Hint): Ensure that the network interface \"" + std::string(tapDeviceName)
                       + "\" specified in [--tap-name] exists and is operational.");
        return nullptr;
    }

    _logger->Debug("Windows TAP adapter found at " + ToString(mTapConnections.UserModeFilePath));

    tapFileHandle = CreateFileW(mTapConnections.UserModeFilePath.c_str(), GENERIC_WRITE | GENERIC_READ, 0, nullptr,
                                OPEN_EXISTING, FILE_ATTRIBUTE_SYSTEM | FILE_FLAG_OVERLAPPED, nullptr);

    if (tapFileHandle == INVALID_HANDLE_VALUE)
    {
        _logger->Error("Error while opening Windows TAP adapter " + std::string(tapDeviceName) + ": "
                       + std::to_string(GetLastError()));
        return nullptr;
    }
    else
    {
        if (SetMediaStatus(tapFileHandle, 1))
        {
            _logger->Debug("Enable network media of TAP adapter " + std::string(tapDeviceName));
        }
        else
        {
            _logger->Error("Failed to enable network media of TAP adapter " + std::string(tapDeviceName) + ". The adapter typically remains in "
                            "state 'unconnected'.");
            return nullptr;
        }
    }

    return tapFileHandle;
}

#else // UNIX
auto TapConnection::GetTapDeviceFileDescriptor(const char *tapDeviceName) -> int
{
    struct ifreq ifr;
    int tapFileDescriptor;
    
    if ((tapFileDescriptor = open("/dev/net/tun", O_RDWR)) < 0)
    {
        int fdError = errno;
        _logger->Error("File descriptor openning failed with error code: " + std::to_string(fdError) + extractErrorMessage(fdError));
        return FILE_DESCRIPTOR_ERROR;
    }

    // Check if tapDeviceName is null, empty, or too long, IFNAMSIZ is a constant that defines the maximum possible buffer size for an interface name (including its terminating zero byte)
    if (tapDeviceName == nullptr || strlen(tapDeviceName) >= IFNAMSIZ)
    {
        _logger->Error("Invalid TAP device name used for [--tap-name] arg.\n"
        "(Hint): Ensure that the name provided is within a valid length between (1 and " + std::to_string(IFNAMSIZ - 1) + ") characters.");
        return FILE_DESCRIPTOR_ERROR;
    }

    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_flags = (short int)IFF_TAP | IFF_NO_PI;

    strncpy(ifr.ifr_name, tapDeviceName, IFNAMSIZ);

    // Path to the tap device in network interfaces
    std::stringstream pathToTapDevice;
    pathToTapDevice << "/sys/class/net/" << tapDeviceName;

    // Check if tapDeviceName exists in the list of all network interfaces
    if ((access(pathToTapDevice.str().c_str(), F_OK) != 0) || (ioctl(tapFileDescriptor, TUNSETIFF, reinterpret_cast<void*>(&ifr)) < 0))
    {
        int ioctlError = errno;
        _logger->Error("Failed to execute IOCTL system call with error code: " + std::to_string(ioctlError) + extractErrorMessage(ioctlError) + 
            "\n(Hint): Ensure that the network interface \"" + std::string(tapDeviceName) + "\" specified in [--tap-name] exists and is operational.");
        close(tapFileDescriptor);
        return FILE_DESCRIPTOR_ERROR;
    }

    _logger->Info("TAP device successfully opened");
    return tapFileDescriptor;
}
#endif
