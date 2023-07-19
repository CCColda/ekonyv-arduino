#include "url.h"

#include "to_string.h"

namespace Str {
String urlEncode(const char *str, size_t len)
{
	String result;
	result.reserve(len);

	for (size_t i = 0; i < len; ++i) {
		const char c = str[i];
		if (('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || ('0' <= c && c <= '9') || c == '-' || c == '_' || c == '.' || c == '~')
			result += c;
		else
			result += '%' + byte_to_string(c);
	}

	return result;
}

String urlDecode(const char *str, size_t len)
{
	String result;

	for (size_t i = 0; i < len;) {
		const char c = str[i];
		if (c == '%') {
			if ((i + 3) < len) {
				result += (char)string_to_byte(str + i + 1, 2);
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