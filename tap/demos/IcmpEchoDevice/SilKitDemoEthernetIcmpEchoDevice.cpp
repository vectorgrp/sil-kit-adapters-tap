// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "Device.hpp"

#include <iostream>
#include <string>
#include <vector>

#include "silkit/SilKit.hpp"
#include "silkit/config/all.hpp"
#include "silkit/services/ethernet/all.hpp"
#include "silkit/services/ethernet/string_utils.hpp"


using namespace SilKit::Services::Ethernet;

using namespace std::chrono_literals;

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

/**************************************************************************************************
 * Main Function
 **************************************************************************************************/

int main(int argc, char**)
{
    const std::string participantConfigurationString =
        R"({ "Logging": { "Sinks": [ { "Type": "Stdout", "Level": "Info" } ] } })";

    const std::string participantName = "EthernetDevice";
    const std::string registryURI = "silkit://localhost:8501";

    const std::string ethernetControllerName = participantName + "_Eth1";
    const std::string ethernetNetworkName ="qemu_demo";

    try
    {
        auto participantConfiguration = SilKit::Config::ParticipantConfigurationFromString(participantConfigurationString);

        std::cout << "Creating participant '" << participantName << "' at " << registryURI << std::endl;
        auto participant = SilKit::CreateParticipant(participantConfiguration, participantName, registryURI);

        std::cout << "Creating ethernet controller '" << ethernetControllerName << "'" << std::endl;
        auto* ethController = participant->CreateEthernetController(ethernetControllerName, ethernetNetworkName);

        static constexpr auto ethernetAddress = demo::EthernetAddress{0x01, 0x23, 0x45, 0x67, 0x89, 0xab};
        static constexpr auto ip4Address = demo::Ip4Address{192, 168, 12, 35};
        auto demoDevice = demo::Device{ethernetAddress, ip4Address, [ethController](std::vector<std::uint8_t> data) {
                                           const auto frameSize = data.size();
                                           static intptr_t transmitId = 0;
                                           ethController->SendFrame(EthernetFrame{std::move(data)},
                                                                    reinterpret_cast<void*>(++transmitId));
                                           std::cout << "Demo >> SIL Kit: Ethernet frame (" << frameSize
                                                     << " bytes, txId=" << transmitId << ")" << std::endl;
                                       }};

        ethController->AddFrameHandler(
            [&demoDevice](IEthernetController* /*controller*/, const EthernetFrameEvent& msg) {
                auto rawFrame = msg.frame.raw;
                std::cout << "SIL Kit >> Demo: Ethernet frame (" << rawFrame.size() << " bytes)" << std::endl;
                demoDevice.Process(asio::buffer(rawFrame.data(),rawFrame.size()));
            });

        ethController->AddFrameTransmitHandler(&EthAckCallback);

        ethController->Activate();

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
