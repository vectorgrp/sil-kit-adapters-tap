// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <linux/if_tun.h>

#include <asio/ts/buffer.hpp>
#include <asio/ts/io_context.hpp>
#include <asio/posix/stream_descriptor.hpp>

#include "silkit/SilKit.hpp"
#include "silkit/config/all.hpp"
#include "silkit/services/ethernet/all.hpp"
#include "silkit/services/ethernet/string_utils.hpp"
#include "silkit/services/logging/all.hpp"

#include "Exceptions.hpp"
#include "Parsing.hpp"

#include "SilKitAdapterTap.hpp"

using namespace SilKit::Services::Ethernet;
using namespace exceptions;

class TapConnection
{
public:
    TapConnection(asio::io_context& io_context, const std::string& tapDevName,
                     std::function<void(std::vector<uint8_t>)> onNewFrameHandler,
                  SilKit::Services::Logging::ILogger* logger)
        : _tapDeviceStream{io_context}
        , _onNewFrameHandler(std::move(onNewFrameHandler))
        , _logger(logger)
    {
        _tapDeviceStream.assign(GetTapDeviceFileDescriptor(tapDevName.c_str()));
        ReceiveEthernetFrameFromTapDevice();        
    }

    template<class container>
    void SendEthernetFrameToTapDevice(const container& data)
    {
        auto sizeSent = _tapDeviceStream.write_some(asio::buffer(data.data(), data.size()));
        if (data.size() != sizeSent )
        {                
            throw InvalidFrameSizeError{};                                     
        }
    }

private:
    void ReceiveEthernetFrameFromTapDevice()
    {
        _tapDeviceStream.async_read_some(asio::buffer(_ethernetFrameBuffer.data(), _ethernetFrameBuffer.size()),                             
            [this](const std::error_code ec, const std::size_t bytes_received) {
                if (ec)
                {
                    throw IncompleteReadError{};
                }

                auto frame_data = std::vector<std::uint8_t>(bytes_received);
                asio::buffer_copy(
                    asio::buffer(frame_data),
                    asio::buffer(_ethernetFrameBuffer.data(), _ethernetFrameBuffer.size()),
                    bytes_received);                
                
                _onNewFrameHandler(std::move(frame_data));                                     

                ReceiveEthernetFrameFromTapDevice();
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
        
        _logger->Info("TAP device successfully opened");
        return tapFileDescriptor;
    }


private:   
    std::array<uint8_t, 1600> _ethernetFrameBuffer;
    asio::posix::stream_descriptor _tapDeviceStream;
    std::function<void(std::vector<uint8_t>)> _onNewFrameHandler;
    SilKit::Services::Logging::ILogger* _logger;
};

using namespace adapters;

void promptForExit()
{
    std::cout << "Press enter to stop the process..." << std::endl;
    std::cin.ignore();
}

template <class exception>
void throwIf(bool b)
{
    if (b)
        throw exception();
}

inline auto& throwInvalidCliIf = throwIf<InvalidCli>;

int main(int argc, char** argv)
{
    if (findArg(argc, argv, helpArg, argv) != NULL)
    {
        print_help(true);
        return NO_ERROR;
    }

    const std::string loglevel = getArgDefault(argc, argv, logLevelArg, "Info");
    const std::string participantConfigurationString =
        R"({ "Logging": { "Sinks": [ { "Type": "Stdout", "Level": ")" + loglevel + R"("} ] } })";
    const std::string registryURI = getArgDefault(argc, argv, regUriArg, "silkit://localhost:8501");

    const std::string tapDevName = getArgDefault(argc, argv, tapNameArg, "silkit_tap");
    const std::string participantName = getArgDefault(argc, argv, participantNameArg, "EthernetTapDevice");
    const std::string ethernetNetworkName = getArgDefault(argc, argv, networkArg, "tap_demo");
    const std::string ethernetControllerName = participantName + "_Eth1";

    asio::io_context ioContext;

    try
    {
        throwInvalidCliIf(thereAreUnknownArguments(argc, argv));  

        auto participantConfiguration = SilKit::Config::ParticipantConfigurationFromString(participantConfigurationString);        

        std::cout << "Creating participant '" << participantName << "' at " << registryURI << std::endl;
        auto participant = SilKit::CreateParticipant(participantConfiguration, participantName, registryURI);
        
        auto logger = participant->GetLogger();

        std::ostringstream SILKitInfoMessage;
        SILKitInfoMessage << "Creating ethernet controller '" << ethernetControllerName << "'";
        logger->Info(SILKitInfoMessage.str());
        auto* ethController = participant->CreateEthernetController(ethernetControllerName,ethernetNetworkName);

        const auto onReceiveEthernetFrameFromTapDevice = [&logger, ethController](std::vector<std::uint8_t> data) {
            if (data.size() < 60)
            {
                data.resize(60, 0);
            }
            const auto frameSize = data.size();
            static intptr_t transmitId = 0;
            ethController->SendFrame(EthernetFrame{std::move(data)}, reinterpret_cast < void * >(++transmitId));

            std::ostringstream SILKitDebugMessage;
            SILKitDebugMessage << "TAP device >> SIL Kit: Ethernet frame (" << frameSize << " bytes, txId=" << transmitId << ")";
            logger->Debug(SILKitDebugMessage.str());
        };

        SILKitInfoMessage.str("");
        SILKitInfoMessage << "Creating TAP device ethernet connector for '" << tapDevName << "'";
        logger->Info(SILKitInfoMessage.str());
        TapConnection tapConnection{ioContext, tapDevName, onReceiveEthernetFrameFromTapDevice, logger};

        const auto onReceiveEthernetMessageFromSilKit = [&logger, &tapConnection](IEthernetController* /*controller*/,
                                                              const EthernetFrameEvent& msg) {
            auto rawFrame = msg.frame.raw;
            tapConnection.SendEthernetFrameToTapDevice(rawFrame);

            std::ostringstream SILKitDebugMessage;
            SILKitDebugMessage << "SIL Kit >> TAP device: Ethernet frame (" << rawFrame.size() << " bytes)" << std::endl;
            logger->Debug(SILKitDebugMessage.str());
        };

        const auto onEthAckCallback = [&logger](IEthernetController* /*controller*/, 
                                        const EthernetFrameTransmitEvent& ack) {
            std::ostringstream SILKitDebugMessage;
            if (ack.status == EthernetTransmitStatus::Transmitted)
            {
                SILKitDebugMessage << "SIL Kit >> TAP device: ACK for ETH Message with transmitId=" 
                        << reinterpret_cast<intptr_t>(ack.userContext);
            }
            else
            {
                SILKitDebugMessage << "SIL Kit >> TAP device: NACK for ETH Message with transmitId="
                        << reinterpret_cast<intptr_t>(ack.userContext) << ": " << ack.status;
            }
            logger->Debug(SILKitDebugMessage.str());
        };

        ethController->AddFrameHandler(onReceiveEthernetMessageFromSilKit);
        ethController->AddFrameTransmitHandler(onEthAckCallback);
        ethController->Activate();

        ioContext.run();

        promptForExit();
    }
    catch (const SilKit::ConfigurationError& error)
    {
        std::cerr << "Invalid configuration: " << error.what() << std::endl;
        promptForExit();
        return CLI_ERROR;
    }
    catch (const InvalidCli&)
    {
        adapters::print_help();
        std::cerr << std::endl << "Invalid command line arguments." << std::endl;
        promptForExit();
        return CLI_ERROR;
    }
    catch (const std::exception& error)
    {
        std::cerr << "Something went wrong: " << error.what() << std::endl;
        promptForExit();
        return OTHER_ERROR;
    }

    return NO_ERROR;
}
