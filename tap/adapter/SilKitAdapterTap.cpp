// SPDX-FileCopyrightText: Copyright 2025 Vector Informatik GmbH
// SPDX-License-Identifier: MIT

#include "Parsing.hpp"
#include "TapConnection.hpp"
#include "EthernetHeader.hpp"

#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <cstdint>
#include <optional>

#include "common/Parsing.hpp"
#include "common/Cli.hpp"
#include "common/ParticipantCreation.hpp"
#include "common/Exceptions.hpp"

#include "asio/ts/buffer.hpp"
#include "asio/ts/io_context.hpp"

#include "silkit/SilKit.hpp"
#include "silkit/config/all.hpp"
#include "silkit/services/ethernet/all.hpp"
#include "silkit/services/ethernet/string_utils.hpp"
#include "silkit/services/logging/all.hpp"

using namespace SilKit::Services::Ethernet;
using namespace SilKit::Services::Orchestration;
using namespace std::chrono_literals;
using namespace util;
using namespace adapters;

int main(int argc, char** argv)
{
    if (findArg(argc, argv, versionArg, argv) != NULL)
    {
        print_version();
        return CodeSuccess;
    }

    print_version();

    if (findArg(argc, argv, helpArg, argv) != NULL)
    {
        print_help(true);
        return CodeSuccess;
    }

    const std::string tapDevName = getArgDefault(argc, argv, tapNameArg, "silkit_tap");
    const std::string ethernetNetworkName = getArgDefault(argc, argv, networkArg, "Ethernet1");
    const std::string ethernetControllerName = "SilKit_ETH_CTRL_1";

    const std::string vlanTagStr = getArgDefault(argc, argv, vlanTagArg, "");
    std::optional<std::uint16_t> vlanId;
    if (!vlanTagStr.empty())
    {
        try
        {
            unsigned long parsedId = std::stoul(vlanTagStr);
            if (parsedId > 4094)
            {
                std::cerr << "Error: VLAN ID must be in range 0..4094, got " << parsedId << std::endl;
                return CodeErrorCli;
            }
            vlanId = static_cast<std::uint16_t>(parsedId);
        }
        catch (const std::exception&)
        {
            std::cerr << "Error: Invalid VLAN ID '" << vlanTagStr << "', expected a number in range 0..4094"
                      << std::endl;
            return CodeErrorCli;
        }
    }

    asio::io_context ioContext;

    try
    {
        throwInvalidCliIf(thereAreUnknownArguments(
            argc, argv,
            {&tapNameArg, &networkArg, &vlanTagArg, &regUriArg, &logLevelArg, &participantNameArg, &configurationArg},
            {&helpArg, &versionArg}));

        SilKit::Services::Logging::ILogger* logger;
        SilKit::Services::Orchestration::ILifecycleService* lifecycleService;
        std::promise<void> runningStatePromise;

        std::string participantName = "SilKitAdapterTap";
        const auto participant =
            CreateParticipant(argc, argv, logger, &participantName, &lifecycleService, &runningStatePromise);

        const bool debugActivated = logger->GetLogLevel() < SilKit::Services::Logging::Level::Info;

        logger->Info("Creating ethernet controller '" + ethernetControllerName + "'");
        auto* ethController = participant->CreateEthernetController(ethernetControllerName, ethernetNetworkName);

        if (vlanId.has_value())
        {
            logger->Info("VLAN tagging enabled: injecting 802.1Q VLAN ID " + std::to_string(*vlanId));
        }

        const auto onReceiveEthernetFrameFromTapDevice = [&logger, debugActivated, ethController,
                                                          vlanId](std::vector<std::uint8_t> data) {
            if (vlanId.has_value())
            {
                data = vlan::InjectVlanTag(std::move(data), *vlanId);
            }

            if (data.size() < 60)
            {
                data.resize(60, 0);
            }
            const auto frameSize = data.size();
            static intptr_t transmitId = 0;
            ethController->SendFrame(EthernetFrame{std::move(data)}, reinterpret_cast<void*>(++transmitId));

            if (debugActivated)
            {
                std::ostringstream SILKitDebugMessage;
                SILKitDebugMessage << "TAP device >> SIL Kit: Ethernet frame (" << frameSize
                                   << " bytes, txId=" << transmitId;
                if (vlanId.has_value())
                {
                    SILKitDebugMessage << ", VLAN ID " << *vlanId;
                }
                SILKitDebugMessage << ")";
                logger->Debug(SILKitDebugMessage.str());
            }
        };

        logger->Info("Creating TAP device ethernet connector for [" + tapDevName + "]");
        TapConnection tapConnection{ioContext, tapDevName, onReceiveEthernetFrameFromTapDevice, logger};

        const auto onReceiveEthernetMessageFromSilKit = [&logger, debugActivated, &tapConnection, vlanId](
                                                            IEthernetController* /*controller*/,
                                                            const EthernetFrameEvent& msg) {
            auto rawFrame = msg.frame.raw;

            if (vlanId.has_value())
            {
                const auto vid = vlan::ExtractVlanId(rawFrame);
                if (!vid.has_value() || *vid != *vlanId)
                    return; // No 802.1Q tag or VLAN ID mismatch, drop frame

                auto strippedFrame = vlan::RemoveVlanTag(rawFrame);
                tapConnection.SendEthernetFrameToTapDevice(strippedFrame);

                if (debugActivated)
                {
                    std::ostringstream SILKitDebugMessage;
                    SILKitDebugMessage << "SIL Kit >> TAP device: Ethernet frame (" << rawFrame.size()
                                       << " bytes, removed VLAN ID " << *vid << ", sent " << strippedFrame.size()
                                       << " bytes)";
                    logger->Debug(SILKitDebugMessage.str());
                }
            }
            else
            {
                tapConnection.SendEthernetFrameToTapDevice(rawFrame);

                if (debugActivated)
                {
                    std::ostringstream SILKitDebugMessage;
                    SILKitDebugMessage << "SIL Kit >> TAP device: Ethernet frame (" << rawFrame.size() << " bytes)";
                    logger->Debug(SILKitDebugMessage.str());
                }
            }
        };

        ethController->AddFrameHandler(onReceiveEthernetMessageFromSilKit);

        if (debugActivated)
        {
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

            ethController->AddFrameTransmitHandler(onEthAckCallback);
        }

        // Called during startup
        lifecycleService->SetCommunicationReadyHandler([&ethController]() { ethController->Activate(); });

        auto finalStateFuture = lifecycleService->StartLifecycle();

        std::thread t([&]() -> void { ioContext.run(); });

        promptForExit();

        Stop(ioContext, t, *logger, &runningStatePromise, lifecycleService, &finalStateFuture);
    }
    catch (const SilKit::ConfigurationError& error)
    {
        std::cerr << "Invalid configuration: " << error.what() << std::endl;
        return CodeErrorConfiguration;
    }
    catch (const InvalidCli&)
    {
        adapters::print_help();
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
