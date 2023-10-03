#include "global.h"

namespace {
#if EK_ETHERNET
EthernetUDP ethUDP;
#endif
} // namespace

namespace global {
#if EK_ETHERNET
Network network = Network(EK_ETH_SHIELD_PIN);
HTTPServer server = HTTPServer();
NTPClient ntp = NTPClient(ethUDP, EK_NTP_IP, EK_NTP_OFFSET_S);
HTTPRequestManager requests = HTTPRequestManager();
#endif

Storage sd = Storage(EK_SDCARD_PIN);
Databases db = Databases();
EventQueue<32> eventqueue = EventQueue<32>();

#if EK_LCD
LCD lcd = LCD(
    EK_LCD_RS_PIN, EK_LCD_ENABLE_PIN,
    EK_LCD_D4_PIN, EK_LCD_D5_PIN, EK_LCD_D6_PIN, EK_LCD_D7_PIN);
#endif

unsigned long time()
{
#if EK_ETHERNET
	return ntp.getEpochTime();
#else
	return millis() / 1000;
#endif
}
} // namespace global
