#ifndef EKONYV_USER_HELLO_R_H
#define EKONYV_USER_HELLO_R_H

#include "../network/httpserver.h"
#include "../string/to_string.h"

#include "../middleware/session.mw.h"

namespace UserHelloRoute {

int handler(const String &path, const Vector<HTTP::ClientHeaderPair> &headers, EthernetClient &client)
{
#if EK_ETHERNET
	const auto session = SessionMiddleware(path, true);

	if (!session) {
		HTTPServer::writeStaticHTMLResponse(HTTPResponse::HTML_UNAUTHORIZED, client);
		return 0;
	}

	const auto reply = str(
	    "A session tokened alapján te <code> ",
	    string_to_html_escaped_string(String((char *)session.user.username, strlen(session.user.username))),
	    " </code> vagy.");

	HTTPServer::writeStaticHTMLResponse(
	    HTTPResponse::StaticHTMLResponse{
	        200,
	        "OK",
	        "Szia!",
	        reply.c_str()},
	    client);
#endif
}

void registerRoute(HTTPServer &server)
{
	server.on(HTTP::GET, "/api/user/hello", HTTPServer::HandlerBehavior::ALLOW_PARAMETERS, handler);
}

} // namespace UserHelloRoute

#endif // !defined(EKONYV_USER_HELLO_R_H)