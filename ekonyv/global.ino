#include "global.h"

namespace global {
Network network = Network(5);
Storage sd = Storage(SDCARD_SS_PIN);

HTTPServer server = HTTPServer();
EventQueue<32> eventqueue = EventQueue<32>();
} // namespace global
