/*
 * Copyright 2020 Andrew Rossignol andrew.rossignol@gmail.com
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "nerfnet/net/radio_interface.h"

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <wiringPi.h>
#include <wiringSerial.h>

#include "nerfnet/util/log.h"
#include "nerfnet/util/time.h"

namespace nerfnet
{

  RadioInterface::RadioInterface(uint16_t ce_pin, int tunnel_fd,
                                 uint32_t primary_addr, uint32_t secondary_addr,
                                 uint8_t channel)
      : tunnel_fd_(tunnel_fd),
        primary_addr_(primary_addr),
        secondary_addr_(secondary_addr),
        tunnel_thread_(&RadioInterface::TunnelThread, this),
        next_id_(1),
        tunnel_logs_enabled_(false)
  {

    // Auto detect host:
    char hostname[256];
    char serial_dev[256];
    gethostname(hostname, sizeof(hostname));
    printf("Detected host: %s\n", hostname);
    if (strcmp(RPI5_HOST, hostname) == 0)
    {
      strcpy(serial_dev, RPI5_UART);
    }
    else
    {
      strcpy(serial_dev, DEFAULT_UART);
    }

    CHECK((wiringPiSetup() != -1), "Unable to initialize WiringPi");
    CHECK(((fd = serialOpen(serial_dev, SPEED_UART)) >= 0), "Unable to open serial");
    LOGI("Serial Port OK");
  }

  RadioInterface::~RadioInterface()
  {
    running_ = false;
    tunnel_thread_.join();
  }

  RadioInterface::RequestResult RadioInterface::Send(
      const std::vector<uint8_t> &request)
  {
    // printf("TX: ");
    // for (uint8_t val : request)
    // {
    //   serialPutchar(fd, val);
    //   // printf("%d", val);
    // }
    // printf("\n");
    return RequestResult::Success;
  }

  RadioInterface::RequestResult RadioInterface::Receive(
      std::vector<uint8_t> &response, uint64_t timeout_us)
  {
    int i = 0;
    // printf("\n");
    return RequestResult::Success;
  }

  size_t RadioInterface::GetReadBufferSize()
  {
    std::lock_guard<std::mutex> lock(read_buffer_mutex_);
    return read_buffer_.size();
  }

  size_t RadioInterface::GetTransferSize(const std::vector<uint8_t> &frame)
  {
    return std::min(frame.size(), static_cast<size_t>(kMaxPayloadSize));
  }

  void RadioInterface::AdvanceID()
  {
    next_id_++;
    if (next_id_ > kIDMask)
    {
      next_id_ = 1;
    }
  }

  bool RadioInterface::ValidateID(uint8_t id)
  {
    if (!last_ack_id_.has_value() || (last_ack_id_.value() == kIDMask && id == 1) || (id == (last_ack_id_.value() + 1)))
    {
      last_ack_id_ = id;
      return true;
    }

    return false;
  }

  void RadioInterface::TunnelThread()
  {
    // The maximum number of network frames to buffer here.
    constexpr size_t kMaxBufferedFrames = 255;

    running_ = true;
    uint8_t buffer[3200];
    while (running_)
    {
      int bytes_read = read(tunnel_fd_, buffer, sizeof(buffer));
      if (bytes_read < 0)
      {
        LOGE("Failed to read: %s (%d)", strerror(errno), errno);
        continue;
      }
      else
      {
        printf("TX: \n");
        for (int i = 0; i < bytes_read; i++)
        {
          serialPutchar(fd, buffer[i]);
        }
      }

      while (serialDataAvail(fd))
      {
        printf("RX: \n");
        uint8_t data;
        uint8_t data0;
        data0 = 0;
        data = serialGetchar(fd);
        serialFlush(fd);
        write(tunnel_fd_, &data0, 1);
        write(tunnel_fd_, &data0, 1);
        write(tunnel_fd_, &data, 1);
      }
    }
  }

  bool RadioInterface::DecodeTunnelTxRxPacket(
      const std::vector<uint8_t> &request, TunnelTxRxPacket &tunnel)
  {

    tunnel.payload = {request.begin(), request.end()};
  }

  bool RadioInterface::EncodeTunnelTxRxPacket(
      const TunnelTxRxPacket &tunnel, std::vector<uint8_t> &request)
  {
    request.resize(kMaxPacketSize, 0x00);
    if (tunnel.id.has_value())
    {
      request[0] = tunnel.id.value();
    }

    if (tunnel.ack_id.has_value())
    {
      request[0] |= (tunnel.ack_id.value() << 4);
    }

    if (tunnel.payload.size() > kMaxPayloadSize)
    {
      LOGE("TxRx packet payload is too large");
      return false;
    }

    request[1] = tunnel.bytes_left;
    for (size_t i = 0; i < tunnel.payload.size(); i++)
    {
      request[2 + i] = tunnel.payload[i];
    }

    return true;
  }

  void RadioInterface::WriteTunnel()
  {
    int bytes_written = write(tunnel_fd_, frame_buffer_.data(), frame_buffer_.size());
    if (tunnel_logs_enabled_)
    {
      LOGI("Writing %d bytes to the tunnel", frame_buffer_.size());
    }

    frame_buffer_.clear();
    if (bytes_written < 0)
    {
      LOGE("Failed to write to tunnel %s (%d)", strerror(errno), errno);
    }
  }
} // namespace nerfnet
