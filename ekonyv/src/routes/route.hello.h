#ifndef EKONYV_HELLO_R_H
#define EKONYV_HELLO_R_H

#include "../network/httpserver.h"
#include "../string/to_string.h"

namespace HelloRoute {

int handler(const String &path, const Vector<HTTPServer::HeaderPair> &headers, EthernetClient &client)
{
#if EK_ETHERNET
	const String body = "Végre működik a HTTP... <code>" + string_to_html_escaped_string(path) + "</code>";
	HTTPServer::writeStaticHTMLResponse(
	    HTTPResponse::StaticHTMLResponse{
	        200,
	        "OK",
	        "Szia!",
	        body.c_str()},
	    client);
#endif
}

void registerRoute(HTTPServer &server)
{
	server.on(HTTPServer::GET, "/hello", HTTPServer::HandlerBehavior::ALLOW_PARAMETERS, handler);
}

} // namespace HelloRoute

#endif // !defined(EKONYV_HELLO_R_H)