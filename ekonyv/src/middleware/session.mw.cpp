#include "session.mw.h"

#include "../global/global.h"
#include "../string/to_string.h"
#include "../string/url.h"

SessionMiddleware::SessionMiddleware(const String &path, bool get_user)
{
	Serial.println("x");
	const char *path_c_string = path.c_str();
	const auto token_param = Url::getParameter(path_c_string, path.length(), "token", 5);

	if (token_param.length() != 32) {
		valid = false;
		expired = true;
		user_id = 0;
		user = {};

		return;
	}

	Serial.println("y");

	FixedBuffer<16> token_value;

	string_to_fixed_buffer<16>(token_param.c_str(), token_param.length(), token_value);
	Serial.println("z");

	const auto session_info = global::db.session.check(token_value);

	user_id = session_info.user_id;
	valid = session_info.valid;
	expired = session_info.expired;

	Serial.println("a");

	if (get_user) {
		const auto user_result = global::db.user.getByID(user_id);
		Serial.println("b");

		if (user_result.success)
			user = user_result.user;
	}
}