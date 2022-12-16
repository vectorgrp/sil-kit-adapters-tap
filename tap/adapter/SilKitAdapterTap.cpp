// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "silkit/SilKit.hpp"
#include "silkit/config/all.hpp"
#include "silkit/services/ethernet/all.hpp"
#include "silkit/services/ethernet/string_utils.hpp"

#include "Exceptions.hpp"

#include <asio/ts/buffer.hpp>
#include <asio/ts/io_context.hpp>
#include <asio/ts/net.hpp>
#include <asio/posix/stream_descriptor.hpp>

#include <linux/if_tun.h>

using namespace SilKit::Services::Ethernet;

using namespace std::chrono_literals;

class TapConnection
{
public:
    TapConnection(asio::io_context& io_context, const std::string& tapDevName,
                     std::function<void(std::vector<uint8_t>)> onNewFrameHandler)
        : _tapDeviceStream{io_context}
        , _onNewFrameHandler(std::move(onNewFrameHandler))
    {
        _tapDeviceStream.assign(GetTapDeviceFileDescriptor(tapDevName.c_str()));
        DoReceiveFrameFromTapDevice();        
    }

    template<class container>
    void SendEthernetFrameToTapDevice(const container& data)
    {
        auto sizeSent = _tapDeviceStream.write_some(asio::buffer(data.data(), data.size()));
        if (data.size() != sizeSent )
        {                
            throw demo::InvalidBufferSize{};                                     
        }
    }

    private:
    void DoReceiveFrameFromTapDevice()
    {
        _tapDeviceStream.async_read_some(asio::buffer(_ethernetFrameBuffer.data(), _ethernetFrameBuffer.size()),                             
            [this](const std::error_code ec, const std::size_t bytes_received) {
                if (ec)
                {
                    throw demo::IncompleteReadError{};
                }

                auto frame_data = std::vector<std::uint8_t>(bytes_received);
                asio::buffer_copy(
                    asio::buffer(frame_data),
                    asio::buffer(_ethernetFrameBuffer.data(), _ethernetFrameBuffer.size()),
                    bytes_received);                
                
                _onNewFrameHandler(std::move(frame_data));                                     

                DoReceiveFrameFromTapDevice();
            });                                  
    }


private:
    int GetTapDeviceFileDescriptor(const char *tapDeviceName)
    {
        struct ifreq ifr;
        int tapFileDescriptor;
        int errorCode;
         
        if ((tapFileDescriptor = open("/dev/net/tun", O_RDWR)) < 0) 
        {            
            return tapFileDescriptor;
        }
        
        memset(&ifr, 0, sizeof(ifr));
        ifr.ifr_flags = (short int)IFF_TAP | IFF_NO_PI;

        if (*tapDeviceName)
        {         
            strncpy(ifr.ifr_name, tapDeviceName, IFNAMSIZ);
        }
        
        errorCode = ioctl(tapFileDescriptor, TUNSETIFF, reinterpret_cast<void*>(&ifr));
        if (errorCode < 0)
        {
            close(tapFileDescriptor);
            return errorCode;
        }
        
        std::cout << "TAP decvice succesfully opened" << std::endl;
        return tapFileDescriptor;
    }


private:   
    std::array<uint8_t, 1600> _ethernetFrameBuffer;
    asio::posix::stream_descriptor _tapDeviceStream;
    std::function<void(std::vector<uint8_t>)> _onNewFrameHandler;

};

void EthAckCallback(IEthernetController* /*controller*/, const EthernetFrameTransmitEvent& ack)
{
    if (ack.status == EthernetTransmitStatus::Transmitted)
    {
        std::cout << "SIL Kit >> Demo: ACK for ETH Message with transmitId=" 
                  << reinterpret_cast<intptr_t>(ack.userContext) << std::endl;
    }
    else
    {
        std::cout << "SIL Kit >> Demo: NACK for ETH Message with transmitId="
                  << reinterpret_cast<intptr_t>(ack.userContext)
                  << ": " << ack.status
                  << std::endl;
    }
}

int main(int argc, char** argv)
{
    const std::string participantConfigurationString =
        R"({ "Logging": { "Sinks": [ { "Type": "Stdout", "Level": "Info" } ] } })";

    const std::string participantName = "EthernetTapDevice";
    const std::string registryURI = "silkit://localhost:8501";

    const std::string ethernetControllerName = participantName + "_Eth1";
    const std::string ethernetNetworkName = "tap_demo";

    const std::string tapDevName = [argc, argv]() -> std::string {
        if (argc >= 2)
        {
            return argv[1];
        }
        return "silkit_tap";
    }(); 

    asio::io_context ioContext;

    try
    {
        auto participantConfiguration = SilKit::Config::ParticipantConfigurationFromString(participantConfigurationString);

        std::cout << "Creating participant '" << participantName << "' at " << registryURI << std::endl;
        auto participant = SilKit::CreateParticipant(participantConfiguration, participantName, registryURI);

        std::cout << "Creating ethernet controller '" << ethernetControllerName << "'" << std::endl;
        auto* ethController = participant->CreateEthernetController(ethernetControllerName,ethernetNetworkName);

        const auto onReceiveEthernetFrameFromTapDevice = [ethController](std::vector<std::uint8_t> data) {
            if (data.size() < 60)
            {
                data.resize(60, 0);
            }
            const auto frameSize = data.size();
            static intptr_t transmitId = 0;
            ethController->SendFrame(EthernetFrame{std::move(data)}, reinterpret_cast < void * >(++transmitId));
            std::cout << "TAP device >> SIL Kit: Ethernet frame (" << frameSize << " bytes, txId=" << transmitId << ")"
                      << std::endl;
        };

        std::cout << "Creating TAP device ethernet connector for '" << tapDevName << "'" << std::endl;
        TapConnection tapConnection{ioContext, tapDevName, onReceiveEthernetFrameFromTapDevice};

        const auto onReceiveEthernetMessageFromSilKit = [&tapConnection](IEthernetController* /*controller*/,
                                                              const EthernetFrameEvent& msg) {
            auto rawFrame = msg.frame.raw;
            tapConnection.SendEthernetFrameToTapDevice(rawFrame);

            std::cout << "SIL Kit >> TAP device: Ethernet frame (" << rawFrame.size() << " bytes)" << std::endl;
        };

        ethController->AddFrameHandler(onReceiveEthernetMessageFromSilKit);
        ethController->AddFrameTransmitHandler(&EthAckCallback);

        ethController->Activate();

        ioContext.run();

        std::cout << "Press enter to stop the process..." << std::endl;
        std::cin.ignore();
    }
    catch (const SilKit::ConfigurationError& error)
    {
        std::cerr << "Invalid configuration: " << error.what() << std::endl;
        std::cout << "Press enter to stop the process..." << std::endl;
        std::cin.ignore();
        return -2;
    }
    catch (const std::exception& error)
    {
        std::cerr << "Something went wrong: " << error.what() << std::endl;
        std::cout << "Press enter to stop the process..." << std::endl;
        std::cin.ignore();
        return -3;
    }

    return 0;
}
