#ifndef EKONYV_HELLO_R_H
#define EKONYV_HELLO_R_H

#include "../network/httpserver.h"
#include "../string/to_string.h"

namespace EKonyvRoute {

int handler(const String &path, const Vector<HTTP::ClientHeaderPair> &headers, EthernetClient &client)
{
#if EK_ETHERNET
	HTTPServer::writeHTTPHeaders(200, "OK", "text/csv", client);
	client.println("key,value");
	client.println("name," EK_NAME);
	client.println("version," EK_VERSION);
#endif
}

void registerRoute(HTTPServer &server)
{
	server.on(HTTP::GET, "/ekonyv", HTTPServer::HandlerBehavior::ALLOW_PARAMETERS, handler);
}

} // namespace EKonyvRoute

#endif // !defined(EKONYV_HELLO_R_H)