/**
 * @file
 * @brief Implementation of URL functions
 */

#include "url.h"

#include "to_string.h"

namespace Str {
String urlEncode(const SizedString &str)
{
	String result;
	result.reserve(str.len);

	for (size_t i = 0; i < str.len; ++i) {
		const char c = str.ptr[i];
		if (('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || ('0' <= c && c <= '9') || c == '-' || c == '_' || c == '.' || c == '~')
			result += c;
		else
			result += '%' + byte_to_string(c);
	}

	return result;
}

String urlDecode(const SizedString &str)
{
	String result;

	size_t i = 0;

	while (i < str.len) {
		const char c = str.ptr[i];
		if (c == '%') {
			if ((i + 3) < str.len) {
				result += (char)string_to_byte(SizedString{str.ptr + i + 1, 2});
			}
			i += 3;
		}
		else {
			result += c;
			i += 1;
		}
	}

	return result;
}

} // namespace Str