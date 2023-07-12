#include "url.h"

#include "../string/string.h"

namespace Url {
String getParameter(const char *path, size_t len, const char *parameter, size_t paramlen)
{
	const auto queryStart = Str::find(path, len, '?');

	if (queryStart == Str::NOT_FOUND)
		return String();

	char *adjustedParameter = new char[paramlen + 1];
	memcpy(adjustedParameter, parameter, paramlen);
	adjustedParameter[paramlen] = '=';

	const auto paramStart = Str::findString(path, len, adjustedParameter, paramlen + 1, queryStart + 1);

	delete[] adjustedParameter;

	if (paramStart == Str::NOT_FOUND)
		return String();

	const auto valueStart = paramStart + paramlen + 1;

	const auto nextParamStart = Str::find(path, len, '&', valueStart);

	if (nextParamStart == Str::NOT_FOUND)
		return String(path + valueStart, len - valueStart);

	return String(path + valueStart, nextParamStart - valueStart);
}
} // namespace Url
