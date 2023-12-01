#ifndef EKONYV_USER_HELLO_R_H
#define EKONYV_USER_HELLO_R_H

#include "../network/httpserver.h"
#include "../string/to_string.h"

#include "../middleware/session.mw.h"

namespace SessionRoute {

int sessionHandler(const String &path, const Vector<HTTP::ClientHeaderPair> &headers, EthernetClient &client)
{
#if EK_ETHERNET
	const auto session = SessionMiddleware(path, true);

	if (!session) {
		HTTPServer::writeStaticHTMLResponse(HTTPResponse::HTML_UNAUTHORIZED, client);
		return 0;
	}

	HTTPServer::writeHTTPHeaders(200, "OK", "text/csv", client);
	client.println("key,value");
	client.println("status,success");
#endif
}

void registerRoute(HTTPServer &server)
{
	server.on(HTTP::GET, "/api/user/session", HTTPServer::HandlerBehavior::ALLOW_PARAMETERS, sessionHandler);
}

} // namespace SessionRoute

#endif // !defined(EKONYV_USER_HELLO_R_H)