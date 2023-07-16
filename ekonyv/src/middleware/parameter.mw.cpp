#include "parameter.mw.h"

#include "../string/string.h"
#include "../string/to_string.h"

ParameterMiddleware::ParameterMiddleware(
    const char *parameter, size_t len,
    const String &path,
    uint32_t preparation)
    : valid(false), name(parameter), value()
{
	const size_t start_index =
	    start_index == NO_PREPARATION
	        ? preparation
	        : preparePath(path);

	if (start_index >= path.length())
		return;

	const char *const adjusted_path = path.c_str() + start_index;
	const size_t adjusted_len = path.length() - start_index;

	char *const parameter_extended = new char[len + 1];
	memcpy(parameter_extended, parameter, len);
	parameter_extended[len] = '=';

	uint32_t param_index = Str::findString(adjusted_path, adjusted_len, parameter_extended, len + 1, 0);

	if (param_index != 0 && param_index != Str::NOT_FOUND) {
		while (adjusted_path[param_index - 1] != '&') {
			param_index = Str::findString(adjusted_path, adjusted_len, parameter_extended, len + 1, param_index + len + 1);

			if (param_index == Str::NOT_FOUND)
				break;
		}
	}

	delete[] parameter_extended;

	if (param_index == Str::NOT_FOUND)
		return;

	const auto param_value_start = param_index + len + 1;

	const auto next_param_or_end = Str::find(adjusted_path, adjusted_len, '&', param_value_start);

	valid = true;
	value = String(
	    adjusted_path + param_value_start,
	    next_param_or_end == Str::NOT_FOUND
	        ? adjusted_len - param_value_start
	        : next_param_or_end - param_value_start);
}

int ParameterMiddleware::sendMissingResponse(EthernetClient &client) const
{
	const auto description_string = str(
	    "The value for query parameter \"", name, "\" could not be found.");

	HTTPServer::writeStaticHTMLResponse(
	    HTTPResponse::StaticHTMLResponse{
	        400,
	        "Bad Request",
	        "400 - Bad Request - Parameter invalid",
	        description_string.c_str()},
	    client);
}

/* static */ uint32_t ParameterMiddleware::preparePath(const String &path)
{
	const auto query_start = path.indexOf('?');
	if (query_start == -1 || query_start == (path.length() - 1))
		return path.length();

	return query_start + 1;
}
