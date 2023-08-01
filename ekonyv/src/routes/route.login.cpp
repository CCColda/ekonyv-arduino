#include "route.login.h"

#include "../global/global.h"

#include "../string/hash.h"
#include "../string/to_string.h"
#include "../string/url.h"

#include "../middleware/parameter.mw.h"
#include "../middleware/session.mw.h"

#include "../arduino/logger.h"

namespace {
Logger logger = Logger("RLGN");
}

int LoginRoute::loginHandler(const String &path, const Vector<HTTP::ClientHeaderPair> &headers, EthernetClient &client)
{
#if EK_ETHERNET
	const auto prep = ParameterMiddleware::preparePath(path);
	const auto username = ParameterMiddleware("username", 8, path, prep);
	const auto password = ParameterMiddleware("password", 8, path, prep);

	if (!username)
		return username.sendMissingResponse(client);

	if (!password)
		return password.sendMissingResponse(client);

	const auto username_decoded = Str::urlDecode(username.value.c_str(), username.value.length());
	const auto password_decoded = Str::urlDecode(password.value.c_str(), password.value.length());

	if (username_decoded.length() == 0 || username_decoded.length() > 64 ||
	    password_decoded.length() == 0 || password_decoded.length() > 32) {
		VERBOSE_LOG(logger, "Failed to log in from ", ip_to_string(client.remoteIP()), ": invalid parameters");

		HTTPServer::writeStaticHTMLResponse(HTTPResponse::HTML_BAD_REQUEST, client);
		return 0;
	}

	FixedBuffer<32> password_hash;
	Str::hashAndSaltString(password_decoded, password_hash);

	const auto loginResult = global::db.user.tryLogin(
	    username_decoded.c_str(), username_decoded.length(),
	    password_hash);

	if (!loginResult.success) {
		VERBOSE_LOG(logger, "Failed to log in from ", ip_to_string(client.remoteIP()), " as \"", username_decoded, '"');

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

int LoginRoute::renewHandler(const String &path, const Vector<HTTP::ClientHeaderPair> &headers, EthernetClient &client)
{
#if EK_ETHERNET
	const auto prep = ParameterMiddleware::preparePath(path);
	const auto renew_token = ParameterMiddleware("refresh", path, prep);

	FixedBuffer<16> renew_token_value;
	string_to_fixed_buffer<16>(renew_token.value.c_str(), renew_token.value.length(), renew_token_value);

	const auto session = global::db.session.checkRefresh(renew_token_value);

	if (!session.valid) {
		HTTPServer::writeStaticHTMLResponse(HTTPResponse::HTML_UNAUTHORIZED, client);
		return 0;
	}
	const auto new_session = global::db.session.extend(session.index, session.user_id);

	HTTPServer::writeHTTPHeaders(200, "OK", "text/csv", client);

	client.println("key,value");
	client.print("token,");
	client.println(fixed_buffer_to_string(new_session.token));
	client.print("refresh,");
	client.println(fixed_buffer_to_string(new_session.refresh_token));
	client.print("expire,");
	client.println(new_session.expire);

	return 0;
#endif
}

int LoginRoute::logoutHandler(const String &path, const Vector<HTTP::ClientHeaderPair> &headers, EthernetClient &client)
{
#if EK_ETHERNET
	const auto session = SessionMiddleware(path, false);

	if (!session)
		return session.sendUnauthorizedResponse(client);

	global::db.session.discard(session.session_index);

	HTTPServer::writeHTTPHeaders(200, "OK", "text/csv", client);
	client.println("key,value");
	client.println("state,success");

	return 0;
#endif
}

int LoginRoute::logoutEverywhereHandler(const String &path, const Vector<HTTP::ClientHeaderPair> &headers, EthernetClient &client)
{
#if EK_ETHERNET
	const auto session = SessionMiddleware(path, true);

	if (!session)
		return session.sendUnauthorizedResponse(client);

	global::db.session.discardAllForUser(session.user_id);

	HTTPServer::writeHTTPHeaders(200, "OK", "text/csv", client);
	client.println("key,value");
	client.println("state,success");

	return 0;
#endif
}

void LoginRoute::registerRoute(HTTPServer &server)
{
	server.on(HTTP::POST, "/api/user/login", HTTPServer::HandlerBehavior::ALLOW_PARAMETERS, loginHandler);
	server.on(HTTP::POST, "/api/user/renew", HTTPServer::HandlerBehavior::ALLOW_PARAMETERS, renewHandler);
	server.on(HTTP::POST, "/api/user/logout", HTTPServer::HandlerBehavior::ALLOW_PARAMETERS, logoutHandler);
	server.on(HTTP::POST, "/api/user/logout_everywhere", HTTPServer::HandlerBehavior::ALLOW_PARAMETERS, logoutEverywhereHandler);
}