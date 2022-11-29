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
#include "WriteUintBe.hpp"

#include <asio/ts/buffer.hpp>
#include <asio/ts/io_context.hpp>
#include <asio/ts/net.hpp>

using namespace SilKit::Services::Ethernet;

using namespace std::chrono_literals;

class QemuSocketClient
{
public:
    QemuSocketClient(asio::io_context& io_context, const std::string& host, const std::string& service,
                     std::function<void(std::vector<uint8_t>)> onNewFrameHandler)
        : _socket{io_context}
        , _resolver{io_context}
        , _onNewFrameHandler(std::move(onNewFrameHandler))
    {
        asio::connect(_socket, _resolver.resolve(host, service));
        std::cout << "connect success" << std::endl;
        DoReceiveFrameFromQemu();
    }

    template<class container>
    void SendEthernetFrameToQemu(const container& data)
    {
        std::array<std::uint8_t, 4> frameSizeBytes = {};
        demo::WriteUintBe(asio::buffer(frameSizeBytes), std::uint32_t(data.size()));

        asio::write(_socket, asio::buffer(frameSizeBytes));
        asio::write(_socket, asio::buffer(data.data(),data.size()));
    }

private:
    void DoReceiveFrameFromQemu()
    {
        asio::async_read(_socket, asio::buffer(_frame_size_buffer.data(), _frame_size_buffer.size()),
                         [this](const std::error_code ec, const std::size_t bytes_received) {
                             if (ec || bytes_received != _frame_size_buffer.size())
                             {
                                 throw demo::IncompleteReadError{};
                             }

                             std::uint32_t frame_size = 0;
                             for (unsigned byte_index = 0; byte_index < 4; ++byte_index)
                             {
                                 frame_size |= static_cast<std::uint32_t>(_frame_size_buffer[4 - 1 - byte_index])
                                               << (byte_index * 8u);
                             }

                             if (frame_size > _frame_data_buffer.size())
                             {
                                 throw demo::InvalidFrameSizeError{};
                             }

                             asio::async_read(
                                 _socket, asio::buffer(_frame_data_buffer.data(), frame_size),
                                 [this, frame_size](const std::error_code ec, const std::size_t bytes_received) {
                                     if (ec || bytes_received != frame_size)
                                     {
                                         throw demo::IncompleteReadError{};
                                     }

                                     auto frame_data = std::vector<std::uint8_t>(frame_size);
                                     asio::buffer_copy(
                                         asio::buffer(frame_data),
                                         asio::buffer(_frame_data_buffer.data(), _frame_data_buffer.size()),
                                         frame_size);

                                     _onNewFrameHandler(std::move(frame_data));

                                     DoReceiveFrameFromQemu();
                                 });
                         });
    }

private:
    asio::ip::tcp::socket _socket;
    asio::ip::tcp::resolver _resolver;

    std::array<uint8_t, 4> _frame_size_buffer = {};
    std::array<uint8_t, 4096> _frame_data_buffer = {};

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

    const std::string participantName = "EthernetQemu";
    const std::string registryURI = "silkit://localhost:8501";

    const std::string ethernetControllerName = participantName + "_Eth1";
    const std::string ethernetNetworkName = "qemu_demo";

    const std::string qemuHostname = [argc, argv]() -> std::string {
        if (argc >= 2)
        {
            return argv[1];
        }
        return "localhost";
    }();
    const std::string qemuService = [argc, argv]() -> std::string {
        if (argc >= 3)
        {
            return argv[2];
        }
        return "12345";
    }();

    asio::io_context ioContext;

    try
    {
        auto participantConfiguration = SilKit::Config::ParticipantConfigurationFromString(participantConfigurationString);

        std::cout << "Creating participant '" << participantName << "' at " << registryURI << std::endl;
        auto participant = SilKit::CreateParticipant(participantConfiguration, participantName, registryURI);

        std::cout << "Creating ethernet controller '" << ethernetControllerName << "'" << std::endl;
        auto* ethController = participant->CreateEthernetController(ethernetControllerName,ethernetNetworkName);

        const auto onReceiveEthernetFrameFromQemu = [ethController](std::vector<std::uint8_t> data) {
            if (data.size() < 60)
            {
                data.resize(60, 0);
            }
            const auto frameSize = data.size();
            static intptr_t transmitId = 0;
            ethController->SendFrame(EthernetFrame{std::move(data)}, reinterpret_cast < void * >(++transmitId));
            std::cout << "QEMU >> SIL Kit: Ethernet frame (" << frameSize << " bytes, txId=" << transmitId << ")"
                      << std::endl;
        };

        std::cout << "Creating QEMU ethernet connector for '" << qemuHostname << ":" << qemuService << "'" << std::endl;
        QemuSocketClient client{ioContext, qemuHostname, qemuService, onReceiveEthernetFrameFromQemu};

        const auto onReceiveEthernetMessageFromIb = [&client](IEthernetController* /*controller*/,
                                                              const EthernetFrameEvent& msg) {
            auto rawFrame = msg.frame.raw;
            client.SendEthernetFrameToQemu(rawFrame);

            std::cout << "SIL Kit >> QEMU: Ethernet frame (" << rawFrame.size() << " bytes)" << std::endl;
        };

        ethController->AddFrameHandler(onReceiveEthernetMessageFromIb);
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
