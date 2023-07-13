#include "route.register.h"

#include "../global/global.h"

#include "../string/url.h"

#include "../string/hash.h"

int RegisterRoute::requestCodeHandler(const String &path, const Vector<HTTPServer::HeaderPair> &headers, EthernetClient &client)
{
#if EK_ETHERNET
	HTTPServer::writeHTTPHeaders(200, "OK", "text/csv", client);

	const auto remoteIP = uint32_t(client.remoteIP());
	FixedBuffer<4> ip;

	memcpy(ip.data, (const void *)&remoteIP, 4);

	const auto initiation = global::db.reg_req.tryInitiate(ip);

	client.println("key,value");
	client.print("state,");

	if (initiation) {
		client.println("success");

#if !EK_PRODUCTION
		// todo Display on LCD
		client.print("code,");
		client.write(global::db.reg_req.code.data, 4);
		client.println();
#endif

		client.print("expire,");
		client.println(global::db.reg_req.expire);
	}
	else {
		client.println("failure");
	}
#endif
}

int RegisterRoute::registerHandler(const String &path, const Vector<HTTPServer::HeaderPair> &headers, EthernetClient &client)
{
#if EK_ETHERNET

	const auto remoteIP = uint32_t(client.remoteIP());
	FixedBuffer<4> ip;

	memcpy(ip.data, (const void *)&remoteIP, 4);

	const char *path_c_string = path.c_str();
	const auto code_param = Url::getParameter(path_c_string, path.length(), "code", 4);
	const auto username_param = Url::getParameter(path_c_string, path.length(), "username", 8);
	const auto password_param = Url::getParameter(path_c_string, path.length(), "password", 8);

	if (code_param.length() != 4 || username_param.length() == 0 ||
	    username_param.length() > 64 || password_param.length() == 0 ||
	    password_param.length() > 32) {
		HTTPServer::writeStaticHTMLResponse(HTTPResponse::HTML_BAD_REQUEST, client);
		return 0;
	}

	FixedBuffer<4> code;
	memcpy(code.data, code_param.c_str(), 4);

	const auto code_valid = global::db.reg_req.checkCode(ip, code);

	HTTPServer::writeHTTPHeaders(200, "OK", "text/csv", client);
	client.println("key,value");
	client.print("state,");

	if (code_valid) {
		const auto password_hash = Str::hashAndSaltString(password_param);
		const auto reg_success = global::db.user.tryRegister(username_param.c_str(), username_param.length(), password_hash, User::Flags{1, 1});

		if (reg_success) {
			client.println("success");

			global::db.reg_req.invalidate();
		}
		else {
			client.println("failure");
			client.println("reason,registration_failed");
		}
	}
	else {
		client.println("failure");
		client.println("reason,code_invalid");
	}

#endif
}

void RegisterRoute::registerRoute(HTTPServer &server)
{
	server.on(HTTPServer::POST, "/api/user/req_code", HTTPServer::HandlerBehavior::NONE, requestCodeHandler);
	server.on(HTTPServer::POST, "/api/user/register", HTTPServer::HandlerBehavior::ALLOW_PARAMETERS, registerHandler);
}