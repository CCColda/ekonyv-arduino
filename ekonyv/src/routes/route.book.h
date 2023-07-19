#ifndef EKONYV_BOOK_R_H
#define EKONYV_BOOK_R_H

#include "../network/httpserver.h"

#include <Arduino.h>

namespace BookRoute {

int getAllBooksHandler(const String &path, const Vector<HTTP::ClientHeaderPair> &headers, EthernetClient &client);
int getBookCountHandler(const String &path, const Vector<HTTP::ClientHeaderPair> &headers, EthernetClient &client);

int postBookHandler(const String &path, const Vector<HTTP::ClientHeaderPair> &headers, EthernetClient &client);
int deleteBookHandler(const String &path, const Vector<HTTP::ClientHeaderPair> &headers, EthernetClient &client);
int getBookHandler(const String &path, const Vector<HTTP::ClientHeaderPair> &headers, EthernetClient &client);
void registerRoute(HTTPServer &server);

} // namespace BookRoute

#endif // !defined(EKONYV_BOOK_R_H)