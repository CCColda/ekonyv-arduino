#ifndef EKONYV_GLOBAL_H
#define EKONYV_GLOBAL_H

#include "eventqueue.h"
#include "httpserver.h"
#include "network.h"
#include "storage.h"

namespace global {
extern Network network;
extern Storage sd;
extern HTTPServer server;
extern EventQueue<32> eventqueue;
} // namespace global

#endif // !defined(EKONYV_GLOBAL_H)