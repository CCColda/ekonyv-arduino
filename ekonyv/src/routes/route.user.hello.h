#ifndef EKONYV_USER_HELLO_R_H
#define EKONYV_USER_HELLO_R_H

#include "../network/httpserver.h"
#include "../string/to_string.h"

namespace UserHelloRoute {

int handler(const String &path, const Vector<HTTPServer::HeaderPair> &headers, EthernetClient &client)
{
#if EK_ETHERNET
	HTTPServer::writeStaticHTMLResponse(
	    HTTPResponse::StaticHTMLResponse{
	        200,
	        "OK",
	        "Szia!",
	        ":)"},
	    client);
#endif
}

void registerRoute(HTTPServer &server)
{
	server.on(HTTPServer::GET, "/hello", HTTPServer::HandlerBehavior::ALLOW_PARAMETERS, handler);
}

} // namespace UserHelloRoute

#endif // !defined(EKONYV_USER_HELLO_R_H)