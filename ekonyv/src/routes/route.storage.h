#ifndef EKONYV_STORAGE_R_H
#define EKONYV_STORAGE_R_H

#include "../network/httpserver.h"

#include <Arduino.h>

namespace StorageRoute {

int getAllStoragesHandler(const String &path, const Vector<HTTP::ClientHeaderPair> &headers, EthernetClient &client);
int getStorageCountHandler(const String &path, const Vector<HTTP::ClientHeaderPair> &headers, EthernetClient &client);
int postStorageHandler(const String &path, const Vector<HTTP::ClientHeaderPair> &headers, EthernetClient &client);
int deleteStorageHandler(const String &path, const Vector<HTTP::ClientHeaderPair> &headers, EthernetClient &client);
int getStorageHandler(const String &path, const Vector<HTTP::ClientHeaderPair> &headers, EthernetClient &client);

void registerRoute(HTTPServer &server);

} // namespace StorageRoute

#endif // !defined(EKONYV_STORAGE_R_H)