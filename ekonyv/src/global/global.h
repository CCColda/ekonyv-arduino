#ifndef EKONYV_GLOBAL_H
#define EKONYV_GLOBAL_H

#include "../config.h"

#include "../eventqueue/eventqueue.h"
#include "../network/httpserver.h"
#include "../network/network.h"
#include "../storage/storage.h"

namespace global {
#if EK_ETHERNET
extern Network network;
extern HTTPServer server;
#else
#warning "Ethernet is disabled"
#endif

extern Storage sd;
extern EventQueue<32> eventqueue;
} // namespace global

#endif // !defined(EKONYV_GLOBAL_H)