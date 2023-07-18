#ifndef EKONYV_GLOBAL_H
#define EKONYV_GLOBAL_H

#include "../config.h"

#include <NTPClient.h>

#include "../eventqueue/eventqueue.h"
#include "../network/httprequestmanager.h"
#include "../network/httpserver.h"
#include "../network/network.h"
#include "../storage/storage.h"

#include "../database/registrationrequest.mem.h"
#include "../database/session.db.h"
#include "../database/user.db.h"

namespace global {
struct Databases {
	RegistrationRequest reg_req;
	UserDatabase user;
	SessionDatabase session;
};

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

unsigned long time();
} // namespace global

#endif // !defined(EKONYV_GLOBAL_H)