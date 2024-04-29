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

#include "nerfnet/net/secondary_radio_interface.h"

#include <unistd.h>
#include <vector>

#include "nerfnet/util/log.h"
#include "nerfnet/util/time.h"

namespace nerfnet
{

  SecondaryRadioInterface::SecondaryRadioInterface(
      uint16_t ce_pin, int tunnel_fd,
      uint32_t primary_addr, uint32_t secondary_addr, uint8_t channel)
      : RadioInterface(ce_pin, tunnel_fd, primary_addr, secondary_addr, channel),
        payload_in_flight_(false)
  {
    uint8_t writing_addr[5] = {
        static_cast<uint8_t>(secondary_addr),
        static_cast<uint8_t>(secondary_addr >> 8),
        static_cast<uint8_t>(secondary_addr >> 16),
        static_cast<uint8_t>(secondary_addr >> 24),
        0,
    };

    uint8_t reading_addr[5] = {
        static_cast<uint8_t>(primary_addr),
        static_cast<uint8_t>(primary_addr >> 8),
        static_cast<uint8_t>(primary_addr >> 16),
        static_cast<uint8_t>(primary_addr >> 24),
        0,
    };
  }

  void SecondaryRadioInterface::Run()
  {
    uint8_t packet[kMaxPacketSize];

    while (1)
    {
      std::vector<uint8_t> request(kMaxPacketSize, 0x00);
      auto result = Receive(request);
    }
  }

  void SecondaryRadioInterface::HandleRequest(
      const std::vector<uint8_t> &request)
  {
    if (request.size() != kMaxPacketSize)
    {
      LOGE("Received short packet");
    }
    else if (request[0] == 0x00)
    {
      HandleNetworkTunnelReset();
    }
    else
    {
      HandleNetworkTunnelTxRx(request);
    }
  }

  void SecondaryRadioInterface::HandleNetworkTunnelReset()
  {
  }

  void SecondaryRadioInterface::HandleNetworkTunnelTxRx(
      const std::vector<uint8_t> &request)
  {
  }

}
