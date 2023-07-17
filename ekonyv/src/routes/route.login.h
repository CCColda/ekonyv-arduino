#ifndef EKONYV_LOGIN_R_H
#define EKONYV_LOGIN_R_H

#include "../network/httpserver.h"

#include <Arduino.h>

namespace LoginRoute {

int loginHandler(const String &path, const Vector<HTTP::ClientHeaderPair> &headers, EthernetClient &client);
int renewHandler(const String &path, const Vector<HTTP::ClientHeaderPair> &headers, EthernetClient &client);
int logoutHandler(const String &path, const Vector<HTTP::ClientHeaderPair> &headers, EthernetClient &client);
void registerRoute(HTTPServer &server);

} // namespace LoginRoute

#endif // !defined(EKONYV_LOGIN_R_H)