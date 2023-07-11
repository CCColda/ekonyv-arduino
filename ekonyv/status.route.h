#ifndef EKONYV_STATUS_R_H
#define EKONYV_STATUS_R_H

#include "httpserver.h"

namespace StatusRoute {

int handler(const String &path, const Vector<HTTPServer::HeaderPair> &headers, EthernetClient &client);
void registerRoute(HTTPServer &server);

} // namespace StatusRoute

#endif // !defined(EKONYV_STATUS_R_H)