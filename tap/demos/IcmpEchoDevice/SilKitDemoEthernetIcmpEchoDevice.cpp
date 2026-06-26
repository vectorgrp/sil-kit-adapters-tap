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

#include "common/Parsing.hpp"
#include "common/Cli.hpp"

using namespace SilKit::Services::Ethernet;
using namespace SilKit::Services::Orchestration;
using namespace util;
using namespace adapters;
using namespace std::chrono_literals;

void print_demo_help(bool userRequested)
{
    // clang-format off
    std::cout << "Usage (defaults in curly braces if you omit the switch):" << std::endl;
    std::cout << "sil-kit-demo-ethernet-icmp-echo-device [" << participantNameArg << " <participant's name{EthernetDevice}>]\n"
        "  [" << regUriArg << " silkit://<host{localhost}>:<port{8501}>]\n"
        "  [" << networkArg << " <SIL Kit Ethernet network name{tap_demo}>]\n"
        "  [" << logLevelArg << " <Trace|Debug|Warn|{Info}|Error|Critical|Off>]\n"
        "  [" << vlanTagArg << " (log VLAN tag information from received frames)]\n";
        std::cout << "\n"
        "Example:\n"
        "sil-kit-demo-ethernet-icmp-echo-device " << participantNameArg << " EchoDevice " << networkArg << " tap_network " << logLevelArg << " Off\n ";

    if (!userRequested)
        std::cout << "\n"
            "Pass "<<helpArg<<" to get this message.\n";
    // clang-format on
}

/**************************************************************************************************
 * Main Function
 **************************************************************************************************/

int main(int argc, char** argv)
{
    if (findArg(argc, argv, helpArg, argv) != nullptr)
    {
        print_demo_help(true);
        return CodeSuccess;
    }

    const std::string loglevel = getArgDefault(argc, argv, logLevelArg, "Info");
    const std::string participantName = getArgDefault(argc, argv, participantNameArg, "EthernetDevice");
    const std::string registryURI = getArgDefault(argc, argv, regUriArg, "silkit://localhost:8501");
    const std::string ethernetNetworkName = getArgDefault(argc, argv, networkArg, "tap_demo");
    const bool useVlanTag = (findArg(argc, argv, vlanTagArg, argv) != nullptr);

    const std::string ethernetControllerName = participantName + "_Eth1";
    const std::string participantConfigurationString =
        R"({ "Logging": { "Sinks": [ { "Type": "Stdout", "Level": ")" + loglevel + R"("} ] } })";

    try
    {
        throwInvalidCliIf(thereAreUnknownArguments(
            argc, argv, {&networkArg, &regUriArg, &logLevelArg, &participantNameArg}, {&helpArg, &vlanTagArg}));

        auto participantConfiguration =
            SilKit::Config::ParticipantConfigurationFromString(participantConfigurationString);
        auto participant = SilKit::CreateParticipant(participantConfiguration, participantName, registryURI);

        auto logger = participant->GetLogger();

        logger->Info("Creating ethernet controller '" + ethernetControllerName + "'");
        auto* ethController = participant->CreateEthernetController(ethernetControllerName, ethernetNetworkName);

        static constexpr auto ethernetAddress = demo::EthernetAddress{0x52, 0x54, 0x56, 0x53, 0x4B, 0x55};
        static constexpr auto ip4Address = demo::Ip4Address{192, 168, 7, 35};
        auto demoDevice = demo::Device{ethernetAddress, ip4Address, logger,
                                       [&logger, ethController, useVlanTag](std::vector<std::uint8_t> data) {
            const auto frameSize = data.size();
            std::optional<std::uint16_t> vid;
            if (useVlanTag)
            {
                vid = adapters::vlan::ExtractVlanId(data);
            }
            static intptr_t transmitId = 0;
            ethController->SendFrame(EthernetFrame{std::move(data)}, reinterpret_cast<void*>(++transmitId));

            std::ostringstream SILKitDebugMessage;
            SILKitDebugMessage << "Demo >> SIL Kit: Ethernet frame (" << frameSize << " bytes, txId=" << transmitId;
            if (vid.has_value())
            {
                SILKitDebugMessage << ", VLAN ID " << *vid;
            }
            SILKitDebugMessage << ")";
            logger->Debug(SILKitDebugMessage.str());
        }};

        auto onReceivedEthernetMessageFromSILKit =
            [&logger, &demoDevice, useVlanTag](IEthernetController* /*controller*/, const EthernetFrameEvent& msg) {
            auto rawFrame = msg.frame.raw;
            std::ostringstream SILKitDebugMessage;
            SILKitDebugMessage << "SIL Kit >> Demo: Ethernet frame (" << rawFrame.size() << " bytes";
            if (useVlanTag)
            {
                const auto vid = adapters::vlan::ExtractVlanId(rawFrame);
                if (vid.has_value())
                {
                    SILKitDebugMessage << ", VLAN ID " << *vid;
                }
            }
            SILKitDebugMessage << ")";
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
        return CodeErrorConfiguration;
    }
    catch (const InvalidCli&)
    {
        print_demo_help(false);
        std::cerr << std::endl << "Invalid command line arguments." << std::endl;
        return CodeErrorCli;
    }
    catch (const std::exception& error)
    {
        std::cerr << "Something went wrong: " << error.what() << std::endl;
        return CodeErrorOther;
    }

    return CodeSuccess;
}
