#include "global.h"

namespace {
#if EK_ETHERNET
EthernetUDP ethUDP;
#endif
} // namespace

namespace global {
#if EK_ETHERNET
Network network = Network(5);
HTTPServer server = HTTPServer();
NTPClient ntp = NTPClient(ethUDP, EK_NTP_IP, EK_NTP_OFFSET_S);
#endif

Storage sd = Storage(SDCARD_SS_PIN);
Databases db = Databases{};
EventQueue<32> eventqueue = EventQueue<32>();
} // namespace global
