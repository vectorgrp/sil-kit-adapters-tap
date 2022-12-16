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
        //ReadFromStream();
    }

    template<class container>
    void SendEthernetFrameToTapDevice(const container& data)
    {
        auto sizeSent = _tapDeviceStream.write_some(asio::buffer(data.data(), data.size()));
        if (data.size() != sizeSent )
        {
                
            throw demo::InvalidBufferSize{};
                                     
        } 

        std::cout << "SizeSent: " << sizeSent << " data.size(): " << data.size() << std::endl;

        std::cout << "Data from SilKit to TapDevice: " << std::endl;
        for (const auto byte:data)
        {
            std::cout << std::hex << std::setw(3) << (unsigned)byte;
        }
        std::cout << std::endl;
      
    }

    private:
    void DoReceiveFrameFromTapDevice()
    {
            _tapDeviceStream.async_read_some(asio::buffer(mReadBuffer.payload.data(), mReadBuffer.payload.size()),
                             
                                 [this](const std::error_code ec, const std::size_t bytes_received) {
                                     if (ec)
                                     {
                                         throw demo::IncompleteReadError{};
                                     }

                                     auto frame_data = std::vector<std::uint8_t>(bytes_received);
                                     asio::buffer_copy(
                                         asio::buffer(frame_data),
                                         asio::buffer(mReadBuffer.payload.data(), mReadBuffer.payload.size()),
                                         bytes_received);

                                        for (const auto byte:frame_data)
                                        {
                                            std::cout << std::hex << std::setw(3) << (unsigned)byte;
                                        }
                                        std::cout << std::endl;
                                     
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
        
        std::cout << "tap decvice succesfully opened" << std::endl;
        return tapFileDescriptor;
    }


private:
    struct EthernetFrameBuffer
    {
        std::array<uint8_t, 1600> payload;
        uint16_t size;
    };

    //EthernetFrameBuffer mReceiveBuffer;
    EthernetFrameBuffer mReadBuffer;
    uint8_t mSendBufferWithLength[1602];

    asio::posix::stream_descriptor _tapDeviceStream;    
    
    //std::array<uint8_t, 4> _frame_size_buffer = {};
    //std::array<uint8_t, 8192> _frame_data_buffer = {};

    std::function<void(std::vector<uint8_t>)> _onNewFrameHandler;

// tapServer ported tapStream-read functions (deactivated)
#if 0
private:
    void ReadFromStream()
    {   
    async_read(
        _tapDeviceStream,
        asio::buffer(mReadBuffer.payload.data(), mReadBuffer.payload.size()),
        asio::transfer_at_least(14),
        [this](const std::error_code& ec, std::size_t bytesRead)
        {
        if (ec /* && !IsErrorToTryAgain(ec) */)
        {
            //Restart();
        }
        else
        {
            mReadBuffer.size = static_cast<uint16_t>(bytesRead);
            SendToSilKit();
        }
        });
    }

private:
    void SendToSilKit()
    {     
        std::memcpy(mSendBufferWithLength, &mReadBuffer.size, sizeof(mReadBuffer.size));
        std::memcpy(mSendBufferWithLength + sizeof(mReadBuffer.size), mReadBuffer.payload.data(), mReadBuffer.size);

        SendFromBuffer(
            mSendBufferWithLength, 
            sizeof(mReadBuffer.size) + mReadBuffer.size,
            0,
            [this]()
            {                
                ReadFromStream();
                //_onNewFrameHandler(std::move(mSendBufferWithLength));
            });     
    }



private:
  void SendFromBuffer(uint8_t* data, std::size_t toTransfer, std::size_t transferred, std::function<void(void)> onSuccess)
  {
    // Adapt to SilKit

    auto frame_data = std::vector<std::uint8_t>(toTransfer - transferred);
        asio::buffer_copy(
            asio::buffer(frame_data),
            asio::buffer(data + transferred, toTransfer - transferred),
            toTransfer - transferred);

    _onNewFrameHandler(std::move(frame_data));

    transferred += toTransfer - transferred;
    if (transferred < toTransfer)
    {
        SendFromBuffer(data, toTransfer, transferred, onSuccess);
    }
    else
    {
        
        onSuccess();
    }

    // mSocket.async_write_some(
    //   buffer(data + transferred, toTransfer - transferred),
    //   [=](const boost::system::error_code& ec, std::size_t bytesTransferred) mutable
    //   {
    //     if (ec && !IsErrorToTryAgain(ec))
    //     {
    //       Restart();
    //       return;
    //     }

    //     transferred += bytesTransferred;

    //     if (transferred < toTransfer)
    //     {
    //       SendFromBuffer(data, toTransfer, transferred, onSuccess);
    //     }
    //     else
    //     {
    //       onSuccess();
    //     }
    // });
  }
#endif


// tapServer ported tapStream-write functions (deactivated)
#if 0
    private:
    void ReceiveIntoBuffer(uint8_t* data, std::size_t toReceive, std::size_t received, std::function<void(void)> onSuccess)
    {
        //TODO: needs to be adapted to SIL Kit

        // mSocket.async_read_some(
        // buffer(data + received, toReceive - received),
        // [=](const boost::system::error_code& ec, std::size_t bytesReceived) mutable
        // {
        //     if (ec && !IsErrorToTryAgain(ec))
        //     {            
        //     return;
        //     }

        //     received += bytesReceived;

        //     if (received < toReceive)
        //     {
        //     ReceiveIntoBuffer(data, toReceive, received, onSuccess);
        //     }
        //     else
        //     {
        //     onSuccess();
        //     }
        // });
    }

private:
  void ReceiveFromSilKit()
  {
    ReceiveIntoBuffer(
      reinterpret_cast<uint8_t*>(&mReceiveBuffer.size),
      sizeof(mReceiveBuffer.size), 
      0,
      [this]()
      {
        if (mReceiveBuffer.size > mReceiveBuffer.payload.max_size())
        {
          //Restart();
        }
        else
        {
          ReceiveIntoBuffer(
            mReceiveBuffer.payload.data(),
            mReceiveBuffer.size,
            0,
            [this]()
            {
              WriteToStream();
            });
        }
      });
  }
  
private:
  void WriteToStream()
  {
    async_write(
      _tapDeviceStream,
      asio::buffer(mReceiveBuffer.payload.data(), mReceiveBuffer.size),
      [this](const std::error_code& ec, std::size_t /*bytesWritten*/ )
      {
        // if (ec && !IsErrorToTryAgain(ec)) 
        // {
        //    Restart();
        // }
        // else
        {
          ReceiveFromSilKit();
        }
      });
  }

  #endif

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
        return "tap10";
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
            std::cout << "QEMU >> SIL Kit: Ethernet frame (" << frameSize << " bytes, txId=" << transmitId << ")"
                      << std::endl;
        };

        std::cout << "Creating TAP device ethernet connector for '" << tapDevName << "'" << std::endl;
        TapConnection tapConnection{ioContext, tapDevName, onReceiveEthernetFrameFromTapDevice};

        const auto onReceiveEthernetMessageFromSilKit = [&tapConnection](IEthernetController* /*controller*/,
                                                              const EthernetFrameEvent& msg) {
            auto rawFrame = msg.frame.raw;
            tapConnection.SendEthernetFrameToTapDevice(rawFrame);

            std::cout << "SIL Kit >> QEMU: Ethernet frame (" << rawFrame.size() << " bytes)" << std::endl;
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
