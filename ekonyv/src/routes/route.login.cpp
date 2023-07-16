#include "route.login.h"

#include "../global/global.h"

#include "../string/hash.h"
#include "../string/to_string.h"

#include "../middleware/parameter.mw.h"

int LoginRoute::loginHandler(const String &path, const Vector<HTTPServer::HeaderPair> &headers, EthernetClient &client)
{
#if EK_ETHERNET
	const auto prep = ParameterMiddleware::preparePath(path);
	const auto username = ParameterMiddleware("username", 8, path, prep);
	const auto password = ParameterMiddleware("password", 8, path, prep);

	if (!username)
		return username.sendMissingResponse(client);

	if (!password)
		return password.sendMissingResponse(client);

	if (username.value.length() == 0 || username.value.length() > 64 ||
	    password.value.length() == 0 || password.value.length() > 32) {
		HTTPServer::writeStaticHTMLResponse(HTTPResponse::HTML_BAD_REQUEST, client);
		return 0;
	}

	FixedBuffer<32> password_hash;
	Str::hashAndSaltString(password.value, password_hash);

	const auto loginResult = global::db.user.tryLogin(
	    username.value.c_str(), username.value.length(),
	    password_hash);

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
// todo implement
#endif
}

int LoginRoute::logoutHandler(const String &path, const Vector<HTTPServer::HeaderPair> &headers, EthernetClient &client)
{
#if EK_ETHERNET
// todo implement
#endif
}

void LoginRoute::registerRoute(HTTPServer &server)
{
	server.on(HTTPServer::POST, "/api/user/login", HTTPServer::HandlerBehavior::ALLOW_PARAMETERS, loginHandler);
	server.on(HTTPServer::POST, "/api/user/renew", HTTPServer::HandlerBehavior::ALLOW_PARAMETERS, renewHandler);
	server.on(HTTPServer::POST, "/api/user/logout", HTTPServer::HandlerBehavior::ALLOW_PARAMETERS, logoutHandler);
}