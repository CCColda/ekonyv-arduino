#ifndef EKONYV_REGISTER_R_H
#define EKONYV_REGISTER_R_H

#include "../network/httpserver.h"

#include <Arduino.h>

namespace RegisterRoute {

int requestCodeHandler(const String &path, const Vector<HTTPServer::HeaderPair> &headers, EthernetClient &client);
int registerHandler(const String &path, const Vector<HTTPServer::HeaderPair> &headers, EthernetClient &client);
void registerRoute(HTTPServer &server);

} // namespace RegisterRoute

#endif // !defined(EKONYV_REGISTER_R_H)