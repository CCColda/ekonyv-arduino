#include "session.mw.h"

#include "../global/global.h"
#include "../string/to_string.h"

SessionMiddleware::SessionMiddleware(
    const String &path, bool get_user,
    uint32_t parameter_preparation)
    : parameter("token", 5, path, parameter_preparation),
      valid(false), expired(true),
      user_id(User::INVALID_ID), session_index(0), user()
{
	if (!parameter.valid)
		return;

	if (parameter.value.length() != 32)
		return;

	FixedBuffer<16> token_value;

	string_to_fixed_buffer<16>(parameter.value.c_str(), parameter.value.length(), token_value);

	const auto session_info = global::db.session.check(token_value);

	user_id = session_info.user_id;
	valid = session_info.valid;
	session_index = session_info.index;
	expired = session_info.expired;

	if (get_user) {
		const auto user_result = global::db.user.getByID(user_id);

		if (user_result.success)
			user = user_result.value;
	}
}

int SessionMiddleware::sendInvalidResponse(EthernetClient &client) const
{
	HTTPServer::writeStaticHTMLResponse(
	    HTTPResponse::StaticHTMLResponse{
	        401,
	        "Unauthorized",
	        "401 - Unauthorized",
	        "Invalid session."},
	    client);

	return 0;
}

int SessionMiddleware::sendUnauthorizedResponse(EthernetClient &client) const
{
	HTTPServer::writeStaticHTMLResponse(
	    HTTPResponse::StaticHTMLResponse{
	        401,
	        "Unauthorized",
	        "401 - Unauthorized",
	        "Insufficient permissions."},
	    client);

	return 0;
}