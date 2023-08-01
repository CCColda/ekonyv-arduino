#include "route.register.h"

#include "../global/global.h"

#include "../string/hash.h"
#include "../string/to_string.h"
#include "../string/url.h"

#include "../middleware/parameter.mw.h"

namespace {
Logger logger = Logger("RREG");
}

namespace RegisterRoute {

int requestCodeHandler(const String &path, const Vector<HTTP::ClientHeaderPair> &headers, EthernetClient &client)
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
		VERBOSE_LOG(logger, "Sent code ", String(global::db.reg_req.code.data, 4), " to ", ip_to_string(client.remoteIP()));

		client.println("success");

#if !EK_LCD
#warning "Registration codes will be sent along with the response as long as LCD is not enabled."
		client.print("code,");
		client.write(global::db.reg_req.code.data, 4);
		client.println();
#endif

		client.print("expire,");
		client.println(global::db.reg_req.expire);
	}
	else {
		VERBOSE_LOG(logger, "Failed generating code for ", ip_to_string(client.remoteIP()), ": code in use");
		client.println("failure");
	}
#endif
}

int registerHandler(const String &path, const Vector<HTTP::ClientHeaderPair> &headers, EthernetClient &client)
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

	const auto username_decoded = Str::urlDecode(username.value.c_str(), username.value.length());
	const auto password_decoded = Str::urlDecode(password.value.c_str(), password.value.length());

	if (code_param.value.length() != 4 || username_decoded.length() == 0 ||
	    username_decoded.length() > 64 || password_decoded.length() == 0 ||
	    password_decoded.length() > 32) {
		VERBOSE_LOG(logger, "Sending bad request to ", ip_to_string(client.remoteIP()), ": invalid parameters");

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
		Str::hashAndSaltString(password_decoded, password_hash);

		const auto reg_success = global::db.user.tryRegister(username_decoded.c_str(), username_decoded.length(), password_hash, User::CAN_WRITE);

		if (reg_success) {
			logger.log("Successfully registered ", username_decoded);
			client.println("success");

			global::db.reg_req.invalidate();
		}
		else {
			logger.warning("Failed to register ", username_decoded);
			client.println("failure");
			client.println("reason,registration_failed");
		}
	}
	else {
		VERBOSE_LOG(logger, "Failed to register ", username_decoded, " due to mismatching code.");
		client.println("failure");
		client.println("reason,code_invalid");
	}

#endif
}

void registerRoute(HTTPServer &server)
{
	server.on(HTTP::POST, "/api/user/req_code", HTTPServer::HandlerBehavior::NONE, requestCodeHandler);
	server.on(HTTP::POST, "/api/user/register", HTTPServer::HandlerBehavior::ALLOW_PARAMETERS, registerHandler);
}
} // namespace RegisterRoute
