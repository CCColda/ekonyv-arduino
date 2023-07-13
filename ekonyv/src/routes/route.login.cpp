#include "route.login.h"

#include "../global/global.h"

#include "../string/hash.h"
#include "../string/to_string.h"
#include "../string/url.h"

int LoginRoute::loginHandler(const String &path, const Vector<HTTPServer::HeaderPair> &headers, EthernetClient &client)
{
#if EK_ETHERNET
	const char *path_c_string = path.c_str();
	const auto username_param = Url::getParameter(path_c_string, path.length(), "username", 8);
	const auto password_param = Url::getParameter(path_c_string, path.length(), "password", 8);

	if (username_param.length() == 0 || username_param.length() > 64 ||
	    password_param.length() == 0 || password_param.length() > 32) {
		HTTPServer::writeStaticHTMLResponse(HTTPResponse::HTML_BAD_REQUEST, client);
		return 0;
	}

	FixedBuffer<32> password_hash;
	Str::hashAndSaltString(password_param, password_hash);

	Serial.println("Login hash");
	Serial.println(fixed_buffer_to_string(password_hash));

	const auto loginResult = global::db.user.tryLogin(username_param.c_str(), username_param.length(), password_hash);

	if (!loginResult.success) {
		HTTPServer::writeStaticHTMLResponse(HTTPResponse::HTML_UNAUTHORIZED, client);
		return 0;
	}

	const auto token = global::db.session.start(loginResult.user.id);

	HTTPServer::writeHTTPHeaders(200, "OK", "text/csv", client);

	client.println("key,value");
	client.print("token,");
	client.println(fixed_buffer_to_string(token.token));
	client.print("refresh,");
	client.println(fixed_buffer_to_string(token.refresh_token));
	client.print("expire,");
	client.println(token.expire);
#endif
}

int LoginRoute::renewHandler(const String &path, const Vector<HTTPServer::HeaderPair> &headers, EthernetClient &client)
{
#if EK_ETHERNET

#endif
}

int LoginRoute::logoutHandler(const String &path, const Vector<HTTPServer::HeaderPair> &headers, EthernetClient &client)
{
#if EK_ETHERNET

#endif
}

void LoginRoute::registerRoute(HTTPServer &server)
{
	server.on(HTTPServer::POST, "/api/user/login", HTTPServer::HandlerBehavior::ALLOW_PARAMETERS, loginHandler);
	server.on(HTTPServer::POST, "/api/user/renew", HTTPServer::HandlerBehavior::ALLOW_PARAMETERS, renewHandler);
	server.on(HTTPServer::POST, "/api/user/logout", HTTPServer::HandlerBehavior::ALLOW_PARAMETERS, logoutHandler);
}