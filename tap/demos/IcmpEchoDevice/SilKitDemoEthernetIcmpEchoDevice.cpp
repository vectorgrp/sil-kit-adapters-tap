// SPDX-FileCopyrightText: Copyright 2025 Vector Informatik GmbH
// SPDX-License-Identifier: MIT

#include "Device.hpp"

#include <iostream>
#include <string>
#include <vector>
#include <cstdint>
#include <future>

#include "silkit/SilKit.hpp"
#include "silkit/config/all.hpp"
#include "silkit/services/ethernet/all.hpp"
#include "silkit/services/ethernet/string_utils.hpp"
#include "../adapter/Parsing.hpp"
#include "../adapter/SignalHandler.hpp"
#include "../adapter/SilKitAdapterTap.hpp"

using namespace SilKit::Services::Ethernet;
using namespace SilKit::Services::Orchestration;
using namespace adapters;
using namespace exceptions;
using namespace std::chrono_literals;

const std::array<const std::string, 4> demoSwitchesWithArgument = {networkArg, regUriArg, logLevelArg,
                                                                   participantNameArg};
const std::array<const std::string, 1> demoSwitchesWithoutArgument = {helpArg};

void promptForExit()
{
    std::promise<int> signalPromise;
    auto signalValue = signalPromise.get_future();
    RegisterSignalHandler([&signalPromise](auto sigNum) { signalPromise.set_value(sigNum); });

    std::cout << "Press CTRL + C to stop the process..." << std::endl;

    signalValue.wait();

    std::cout << "\nSignal " << signalValue.get() << " received!" << std::endl;
    std::cout << "Exiting..." << std::endl;
}

void print_demo_help(bool userRequested)
{
    // clang-format off
    std::cout << "Usage (defaults in curly braces if you omit the switch):" << std::endl;
    std::cout << "sil-kit-demo-ethernet-icmp-echo-device [" << participantNameArg << " <participant's name{EthernetDevice}>]\n"
        "  [" << regUriArg << " silkit://<host{localhost}>:<port{8501}>]\n"
        "  [" << networkArg << " <SIL Kit Ethernet network name{tap_demo}>]\n"
        "  [" << logLevelArg << " <Trace|Debug|Warn|{Info}|Error|Critical|Off>]\n";
        std::cout << "\n"
        "Example:\n"
        "sil-kit-demo-ethernet-icmp-echo-device " << participantNameArg << " EchoDevice " << networkArg << " tap_network " << logLevelArg << " Off\n ";

    if (!userRequested)
        std::cout << "\n"
            "Pass "<<helpArg<<" to get this message.\n";
    // clang-format on
}

bool thereAreDemoUnknownArguments(int argc, char** argv)
{
    //skip the executable calling:
    argc -= 1;
    argv += 1;
    while (argc)
    {
        if (strncmp(*argv, "--", 2) != 0)
            return true;
        if (std::find(demoSwitchesWithArgument.begin(), demoSwitchesWithArgument.end(), *argv)
            != demoSwitchesWithArgument.end())
        {
            //switches with argument have an argument to ignore, so skip "2"
            argc -= 2;
            argv += 2;
        }
        else if (std::find(demoSwitchesWithoutArgument.begin(), demoSwitchesWithoutArgument.end(), *argv)
                 != demoSwitchesWithoutArgument.end())
        {
            //switches without argument don't have an argument to ignore, so skip "1"
            argc -= 1;
            argv += 1;
        }
        else
            return true;
    }
    return false;
}

/**************************************************************************************************
 * Main Function
 **************************************************************************************************/

int main(int argc, char** argv)
{
    if (findArg(argc, argv, "--help", argv) != nullptr)
    {
        print_demo_help(true);
        return NO_ERROR;
    }

    const std::string loglevel = getArgDefault(argc, argv, logLevelArg, "Info");
    const std::string participantName = getArgDefault(argc, argv, participantNameArg, "EthernetDevice");
    const std::string registryURI = getArgDefault(argc, argv, regUriArg, "silkit://localhost:8501");
    const std::string ethernetNetworkName = getArgDefault(argc, argv, networkArg, "tap_demo");

    const std::string ethernetControllerName = participantName + "_Eth1";
    const std::string participantConfigurationString =
        R"({ "Logging": { "Sinks": [ { "Type": "Stdout", "Level": ")" + loglevel + R"("} ] } })";

    try
    {
        throwInvalidCliIf(thereAreDemoUnknownArguments(argc, argv));

        auto participantConfiguration =
            SilKit::Config::ParticipantConfigurationFromString(participantConfigurationString);
        auto participant = SilKit::CreateParticipant(participantConfiguration, participantName, registryURI);

        auto logger = participant->GetLogger();

        logger->Info("Creating ethernet controller '" + ethernetControllerName + "'");
        auto* ethController = participant->CreateEthernetController(ethernetControllerName, ethernetNetworkName);

        static constexpr auto ethernetAddress = demo::EthernetAddress{0x52, 0x54, 0x56, 0x53, 0x4B, 0x55};
        static constexpr auto ip4Address = demo::Ip4Address{192, 168, 7, 35};
        auto demoDevice =
            demo::Device{ethernetAddress, ip4Address, logger, [&logger, ethController](std::vector<std::uint8_t> data) {
            const auto frameSize = data.size();
            static intptr_t transmitId = 0;
            ethController->SendFrame(EthernetFrame{std::move(data)}, reinterpret_cast<void*>(++transmitId));

            std::ostringstream SILKitDebugMessage;
            SILKitDebugMessage << "Demo >> SIL Kit: Ethernet frame (" << frameSize << " bytes, txId=" << transmitId
                               << ")";
            logger->Debug(SILKitDebugMessage.str());
        }};

        auto onReceivedEthernetMessageFromSILKit = [&logger, &demoDevice](IEthernetController* /*controller*/,
                                                                          const EthernetFrameEvent& msg) {
            auto rawFrame = msg.frame.raw;
            std::ostringstream SILKitDebugMessage;
            SILKitDebugMessage << "SIL Kit >> Demo: Ethernet frame (" << rawFrame.size() << " bytes)";
            logger->Debug(SILKitDebugMessage.str());
            demoDevice.Process(asio::buffer(rawFrame.data(), rawFrame.size()));
        };

        auto onEthernetAckCallback = [&logger](IEthernetController*, const EthernetFrameTransmitEvent& ack) {
            std::ostringstream SILKitDebugMessage;
            if (ack.status == EthernetTransmitStatus::Transmitted)
            {
                SILKitDebugMessage << "SIL Kit >> Demo: ACK for ETH Message with transmitId="
                                   << reinterpret_cast<intptr_t>(ack.userContext);
            }
            else
            {
                SILKitDebugMessage << "SIL Kit >> Demo: NACK for ETH Message with transmitId="
                                   << reinterpret_cast<intptr_t>(ack.userContext) << ": " << ack.status;
            }
            logger->Debug(SILKitDebugMessage.str());
        };

        ethController->AddFrameHandler(onReceivedEthernetMessageFromSILKit);
        ethController->AddFrameTransmitHandler(onEthernetAckCallback);

        // Setup lifecycle
        auto* lifecycleService = participant->CreateLifecycleService({OperationMode::Autonomous});
        auto* systemMonitor = participant->CreateSystemMonitor();
        std::promise<void> runningStatePromise;

        systemMonitor->AddParticipantStatusHandler(
            [&runningStatePromise, participantName](const ParticipantStatus& status) {
            if (participantName == status.participantName)
            {
                if (status.state == ParticipantState::Running)
                {
                    runningStatePromise.set_value();
                }
            }
        });

        // Called during startup
        lifecycleService->SetCommunicationReadyHandler([&ethController]() { ethController->Activate(); });

        auto finalStateFuture = lifecycleService->StartLifecycle();

        promptForExit();

        auto runningStateFuture = runningStatePromise.get_future();
        auto futureStatus = runningStateFuture.wait_for(15s);
        if (futureStatus != std::future_status::ready)
        {
            std::ostringstream SILKitDebugMessage;
            SILKitDebugMessage
                << "Lifecycle Service Stopping: timed out while checking if the participant is currently running.";
            logger->Debug(SILKitDebugMessage.str());
        }
        lifecycleService->Stop("Adapter stopped by the user.");

        auto finalState = finalStateFuture.wait_for(15s);
        if (finalState != std::future_status::ready)
        {
            std::ostringstream SILKitDebugMessage;
            SILKitDebugMessage << "Lifecycle service stopping: timed out";
            logger->Debug(SILKitDebugMessage.str());
        }
    }
    catch (const SilKit::ConfigurationError& error)
    {
        std::cerr << "Invalid configuration: " << error.what() << std::endl;
        return CONFIGURATION_ERROR;
    }
    catch (const InvalidCli&)
    {
        print_demo_help(false);
        std::cerr << std::endl << "Invalid command line arguments." << std::endl;
        return CLI_ERROR;
    }
    catch (const std::exception& error)
    {
        std::cerr << "Something went wrong: " << error.what() << std::endl;
        return OTHER_ERROR;
    }

    return NO_ERROR;
}
