// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "SilKitAdapterTap.hpp"

#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <cstdint>

#include "Exceptions.hpp"
#include "Parsing.hpp"
#include "SignalHandler.hpp"
#include "TapConnection.hpp"

#include "asio/ts/buffer.hpp"
#include "asio/ts/io_context.hpp"

#include "silkit/SilKit.hpp"
#include "silkit/config/all.hpp"
#include "silkit/services/ethernet/all.hpp"
#include "silkit/services/ethernet/string_utils.hpp"
#include "silkit/services/logging/all.hpp"

using namespace SilKit::Services::Ethernet;
using namespace exceptions;
using namespace SilKit::Services::Orchestration;
using namespace std::chrono_literals;
using namespace adapters;

void promptForExit()
{    
    std::promise<int> signalPromise;
    auto signalValue = signalPromise.get_future();
    RegisterSignalHandler([&signalPromise](auto sigNum) {
        signalPromise.set_value(sigNum);
    });
        
    std::cout << "Press CTRL + C to stop the process..." << std::endl;

    signalValue.wait();

    std::cout << "\nSignal " << signalValue.get() << " received!" << std::endl;
    std::cout << "Exiting..." << std::endl;
}

int main(int argc, char** argv)
{
    if (findArg(argc, argv, helpArg, argv) != NULL)
    {
        print_help(true);
        return NO_ERROR;
    }

    const std::string configurationFile = getArgDefault(argc, argv, configurationArg, "");
    const std::string registryURI = getArgDefault(argc, argv, regUriArg, "silkit://localhost:8501");

    const std::string tapDevName = getArgDefault(argc, argv, tapNameArg, "silkit_tap");
    const std::string participantName = getArgDefault(argc, argv, participantNameArg, "EthernetTapDevice");
    const std::string ethernetNetworkName = getArgDefault(argc, argv, networkArg, "tap_demo");
    const std::string ethernetControllerName = participantName + "_Eth1";

    asio::io_context ioContext;

    try
    {
        throwInvalidCliIf(thereAreUnknownArguments(argc, argv));

        std::shared_ptr<SilKit::Config::IParticipantConfiguration> participantConfiguration;
        if (!configurationFile.empty())
        {
            participantConfiguration = SilKit::Config::ParticipantConfigurationFromFile(configurationFile);
            static const auto conflictualArguments = {
                &logLevelArg,
                /* &participantNameArg, &regUriArg are correctly handled by SilKit if one is overwritten.*/};
            for (const auto* conflictualArgument : conflictualArguments)
            {
                if (findArg(argc, argv, *conflictualArgument, argv) != NULL)
                {
                    auto configFileName = configurationFile;
                    if (configurationFile.find_last_of("/\\") != std::string::npos)
                    {
                        configFileName = configurationFile.substr(configurationFile.find_last_of("/\\") + 1);
                    }
                    std::cout << "[info] Be aware that argument given with " << *conflictualArgument 
                              << " can be overwritten by a different value defined in the given configuration file "
                              << configFileName << std::endl;
                }
            }
        }
        else
        {
            const std::string loglevel = getArgDefault(argc, argv, logLevelArg, "Info");
            const std::string participantConfigurationString =
                R"({ "Logging": { "Sinks": [ { "Type": "Stdout", "Level": ")" + loglevel + R"("} ] } })";
            participantConfiguration =
                SilKit::Config::ParticipantConfigurationFromString(participantConfigurationString);
        }

        auto participant = SilKit::CreateParticipant(participantConfiguration, participantName, registryURI);
        auto logger = participant->GetLogger();

        auto* lifecycleService = participant->CreateLifecycleService({OperationMode::Autonomous});
        auto* systemMonitor = participant->CreateSystemMonitor();
        std::promise<void> runningStatePromise;

        systemMonitor->AddParticipantStatusHandler(
            [&runningStatePromise, participantName](const SilKit::Services::Orchestration::ParticipantStatus& status) {
                if (participantName == status.participantName)
                {
                    if (status.state == SilKit::Services::Orchestration::ParticipantState::Running)
                    {
                        runningStatePromise.set_value();
                    }
                }
            });

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
        SILKitInfoMessage << "Creating TAP device ethernet connector for [" << tapDevName << "] on network [" << ethernetNetworkName << "]";
        logger->Info(SILKitInfoMessage.str());
        TapConnection tapConnection{ioContext, tapDevName, onReceiveEthernetFrameFromTapDevice, logger};

        const auto onReceiveEthernetMessageFromSilKit = [&logger, &tapConnection](IEthernetController* /*controller*/,
                                                                                  const EthernetFrameEvent& msg) {
            auto rawFrame = msg.frame.raw;
            tapConnection.SendEthernetFrameToTapDevice(rawFrame);

            std::ostringstream SILKitDebugMessage;
            SILKitDebugMessage << "SIL Kit >> TAP device: Ethernet frame (" << rawFrame.size() << " bytes)";
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

        auto finalStateFuture = lifecycleService->StartLifecycle();

        std::thread t([&]() -> void {
            ioContext.run();
        });
        
        promptForExit();

        ioContext.stop();

        if (t.joinable())
        {
            t.join();
        }
            
        auto runningStateFuture = runningStatePromise.get_future();
        auto futureStatus = runningStateFuture.wait_for(15s);
        if (futureStatus != std::future_status::ready)
        {
            std::ostringstream SILKitDebugMessage;
            SILKitDebugMessage << "Lifecycle Service Stopping: timed out while checking if the participant is currently running.";
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
        adapters::print_help();
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
