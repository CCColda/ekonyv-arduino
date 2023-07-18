#include "route.register.h"

#include "../global/global.h"

#include "../string/hash.h"

#include "../middleware/parameter.mw.h"

int RegisterRoute::requestCodeHandler(const String &path, const Vector<HTTP::ClientHeaderPair> &headers, EthernetClient &client)
{
#if EK_ETHERNET
	HTTPServer::writeHTTPHeaders(200, "OK", "text/csv", client);

	const auto remoteIP = uint32_t(client.remoteIP());
	FixedBuffer<4> ip;

	memcpy(ip.data, (const void *)&remoteIP, 4);

	global::db.reg_req.update();
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

int RegisterRoute::registerHandler(const String &path, const Vector<HTTP::ClientHeaderPair> &headers, EthernetClient &client)
{
#if EK_ETHERNET
	const auto prep = ParameterMiddleware::preparePath(path);

	const auto code_param = ParameterMiddleware("code", 4, path, prep);
	const auto username = ParameterMiddleware("username", 8, path, prep);
	const auto password = ParameterMiddleware("password", 8, path, prep);

	if (!code_param)
		return code_param.sendMissingResponse(client);

	if (!username)
		return username.sendMissingResponse(client);

	if (!password)
		return password.sendMissingResponse(client);

	if (code_param.value.length() != 4 || username.value.length() == 0 ||
	    username.value.length() > 64 || password.value.length() == 0 ||
	    password.value.length() > 32) {
		HTTPServer::writeStaticHTMLResponse(HTTPResponse::HTML_BAD_REQUEST, client);
		return 0;
	}

	// todo verify username for valid alphanumerical characters + some symbols only

	const auto remoteIP = uint32_t(client.remoteIP());
	FixedBuffer<4> ip;

	memcpy(ip.data, (const void *)&remoteIP, 4);

	FixedBuffer<4> code;
	memcpy(code.data, code_param.value.c_str(), 4);

	const auto code_valid = global::db.reg_req.checkCode(ip, code);

	HTTPServer::writeHTTPHeaders(200, "OK", "text/csv", client);
	client.println("key,value");
	client.print("state,");

	if (code_valid) {
		FixedBuffer<32> password_hash;
		Str::hashAndSaltString(password.value, password_hash);

		const auto reg_success = global::db.user.tryRegister(username.value.c_str(), username.value.length(), password_hash, User::Flags{1, 1});

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
	server.on(HTTP::POST, "/api/user/req_code", HTTPServer::HandlerBehavior::NONE, requestCodeHandler);
	server.on(HTTP::POST, "/api/user/register", HTTPServer::HandlerBehavior::ALLOW_PARAMETERS, registerHandler);
}