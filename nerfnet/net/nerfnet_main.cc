
#include <tclap/CmdLine.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "nerfnet/net/network_manager.h"
#include "nerfnet/net/nrf_link.h"
#include "nerfnet/net/radio_transport.h"
#include "nerfnet/net/transport_manager.h"
#include "nerfnet/util/log.h"
#include "nerfnet/util/string.h"
#include "nerfnet/util/time.h"

#define RPI5_HOST "raspberrypi5"
#define RPI5_ADD 0xC0A80A01
#define DEF_ADD 0xC0A80A02

constexpr char kDescription[] = "Mesh networking for NRF24L01 radios.";
constexpr char kVersion[] = "0.0.1";

int main(int argc, char **argv)
{
  TCLAP::CmdLine cmd(kDescription, ' ', kVersion);
  TCLAP::ValueArg<uint16_t> channel_arg("", "channel", "The channel to use for transmit/receive.", false, 1, "channel", cmd);
  TCLAP::ValueArg<uint16_t> ce_pin_arg("", "ce_pin", "Set to the index of the NRF24L01 chip-enable pin.", false, 22, "index", cmd);
  cmd.parse(argc, argv);

  // Auto detect host:
  char hostname[256];
  uint32_t addr;
  gethostname(hostname, sizeof(hostname));
  printf("Detected host: %s\n", hostname);
  if (strcmp(RPI5_HOST, hostname) == 0)
  {
    addr = RPI5_ADD;
  }
  else
  {
    addr = DEF_ADD;
  }

  // Register transports.
  std::unique_ptr<nerfnet::Link> link = std::make_unique<nerfnet::NRFLink>(addr, channel_arg.getValue(), ce_pin_arg.getValue());

  // Setup the network.
  nerfnet::NetworkManager network_manager;
  network_manager.RegisterTransport<nerfnet::RadioTransport>(std::move(link));

  // TODO(aarossig): Start the network manager and block until quit.
  while (1)
  {
    nerfnet::SleepUs(1000000);
    LOGI("heartbeat");
  }

  return 0;
}
