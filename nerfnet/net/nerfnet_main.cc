
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

// Sets flags for a given interface. Quits and logs the error on failure.
void SetInterfaceFlags(const std::string_view &device_name, int flags)
{
  int fd = socket(AF_INET, SOCK_DGRAM, 0);
  CHECK(fd >= 0, "Failed to open socket: %s (%d)", strerror(errno), errno);

  struct ifreq ifr = {};
  ifr.ifr_flags = flags;
  strncpy(ifr.ifr_name, std::string(device_name).c_str(), IFNAMSIZ);
  int status = ioctl(fd, SIOCSIFFLAGS, &ifr);
  CHECK(status >= 0, "Failed to set tunnel interface: %s (%d)",
        strerror(errno), errno);
  close(fd);
}

void SetIPAddress(const std::string_view &device_name,
                  const std::string_view &ip, const std::string &ip_mask)
{
  int fd = socket(AF_INET, SOCK_DGRAM, 0);
  CHECK(fd >= 0, "Failed to open socket: %s (%d)", strerror(errno), errno);

  struct ifreq ifr = {};
  strncpy(ifr.ifr_name, std::string(device_name).c_str(), IFNAMSIZ);

  ifr.ifr_addr.sa_family = AF_INET;
  CHECK(inet_pton(AF_INET, std::string(ip).c_str(),
                  &reinterpret_cast<struct sockaddr_in *>(&ifr.ifr_addr)->sin_addr) == 1,
        "Failed to assign IP address: %s (%d)", strerror(errno), errno);
  int status = ioctl(fd, SIOCSIFADDR, &ifr);
  CHECK(status >= 0, "Failed to set tunnel interface ip: %s (%d)",
        strerror(errno), errno);

  ifr.ifr_netmask.sa_family = AF_INET;
  CHECK(inet_pton(AF_INET, std::string(ip_mask).c_str(),
                  &reinterpret_cast<struct sockaddr_in *>(&ifr.ifr_netmask)->sin_addr) == 1,
        "Failed to assign IP mask: %s (%d)", strerror(errno), errno);
  status = ioctl(fd, SIOCSIFNETMASK, &ifr);
  CHECK(status >= 0, "Failed to set tunnel interface mask: %s (%d)",
        strerror(errno), errno);
  close(fd);
}

// Opens the tunnel interface to listen on. Always returns a valid file
// descriptor or quits and logs the error.
int OpenTunnel(const std::string_view &device_name)
{
  int fd = open("/dev/net/tun", O_RDWR);
  CHECK(fd >= 0, "Failed to open tunnel file: %s (%d)", strerror(errno), errno);

  struct ifreq ifr = {};
  ifr.ifr_flags = IFF_TUN | IFF_NO_PI;
  strncpy(ifr.ifr_name, std::string(device_name).c_str(), IFNAMSIZ);

  int status = ioctl(fd, TUNSETIFF, &ifr);
  CHECK(status >= 0, "Failed to set tunnel interface: %s (%d)",
        strerror(errno), errno);
  return fd;
}

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

  int tunnel_fd = OpenTunnel("nerf0");
  LOGI("tunnel '%s' opened", "nerf0");
  SetInterfaceFlags("nerf0", IFF_UP);
  LOGI("tunnel '%s' up", "nerf0");
  SetIPAddress("nerf0", tunnel_ip, "255.255.255.0");
  LOGI("tunnel '%s' configured with '%s' mask '%s'", "nerf0", "192.168.10.1", "255.255.255.0");

  // TODO(aarossig): Start the network manager and block until quit.
  while (1)
  {
    nerfnet::SleepUs(1000000);
    LOGI("heartbeat");
  }

  return 0;
}
