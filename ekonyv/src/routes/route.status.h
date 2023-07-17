#ifndef EKONYV_STATUS_R_H
#define EKONYV_STATUS_R_H

#include "../network/httpserver.h"

namespace StatusRoute {

int handler(const String &path, const Vector<HTTP::ClientHeaderPair> &headers, EthernetClient &client);
void registerRoute(HTTPServer &server);

} // namespace StatusRoute

#endif // !defined(EKONYV_STATUS_R_H)