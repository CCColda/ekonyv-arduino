#ifndef EKONYV_GLOBAL_H
#define EKONYV_GLOBAL_H

#include "../config.h"

#include <NTPClient.h>

#include "../eventqueue/eventqueue.h"
#include "../network/httprequestmanager.h"
#include "../network/httpserver.h"
#include "../network/network.h"
#include "../storage/storage.h"

#include "../lcd/lcd.h"

#include "./databases.h"

namespace global {
#if EK_ETHERNET
extern Network network;
extern HTTPServer server;
extern NTPClient ntp;
extern HTTPRequestManager requests;

#else
#warning "Ethernet is disabled"
#endif

extern Storage sd;
extern Databases db;
extern EventQueue<32> eventqueue;

#if EK_LCD
extern LCD lcd;
#endif

unsigned long time();
} // namespace global

#endif // !defined(EKONYV_GLOBAL_H)