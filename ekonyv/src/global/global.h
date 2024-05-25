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
//! @brief Manages the DHCP network connection.
extern Network network;

//! @brief Handler for HTTP server functionality.
extern HTTPServer server;

//! @brief Manages the NTP client for querying the current time.
extern NTPClient ntp;

//! @brief Manages outgoing HTTP requests.
//! @code
//! [[deprecated]] extern HTTPRequestManager requests;
//! @endcode

#else
#warning "Ethernet is disabled"
#endif

//! @brief Stores whether the program is currently running.
//! In the case of a halt or a pause, the value is false.
//! The starting value is @c EK_DEFAULT_RUNNING
extern bool is_running;

//! @brief Queries and stores information about the SD (or MockSD) card
extern Storage sd;

//! @brief Stores the buffered databases
extern Databases db;

//! @brief Manages an event queue for out-of-order execution.
//! @code
//! [[deprecated]] extern EventQueue<32> eventqueue;
//! @endcode

#if EK_LCD
//! @brief Manages the LCD display and its state.
extern LCD lcd;
#endif

//! @brief Returns the local time if NTP is not connected, or UNIX time otherwise.
unsigned long time();
} // namespace global

#endif // !defined(EKONYV_GLOBAL_H)