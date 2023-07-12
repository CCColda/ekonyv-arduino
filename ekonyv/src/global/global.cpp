#include "global.h"

namespace global {
#if EK_ETHERNET
Network network = Network(5);
HTTPServer server = HTTPServer();
#endif

Storage sd = Storage(SDCARD_SS_PIN);
Databases db = Databases{};
EventQueue<32> eventqueue = EventQueue<32>();
} // namespace global
