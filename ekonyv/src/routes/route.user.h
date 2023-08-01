#ifndef EKONYV_USER_R_H
#define EKONYV_USER_R_H

#include "../network/httpserver.h"

#include <Arduino.h>

namespace UserRoute {

int getAllUsersHandler(const String &path, const Vector<HTTP::ClientHeaderPair> &headers, EthernetClient &client);
int getUserCountHandler(const String &path, const Vector<HTTP::ClientHeaderPair> &headers, EthernetClient &client);
int postUserHandler(const String &path, const Vector<HTTP::ClientHeaderPair> &headers, EthernetClient &client);
int deleteUserHandler(const String &path, const Vector<HTTP::ClientHeaderPair> &headers, EthernetClient &client);
int getUserHandler(const String &path, const Vector<HTTP::ClientHeaderPair> &headers, EthernetClient &client);

void registerRoute(HTTPServer &server);

} // namespace BookRoute

#endif // !defined(EKONYV_USER_R_H)