#include "./string.h"

namespace Str {

const size_t NOT_FOUND = ~(size_t)0;

const char WHITESPACE[6] = "\x20\t\v\n\r";
const size_t WHITESPACE_LEN = sizeof(WHITESPACE);

size_t find(const char *str, size_t len, char chr, size_t start)
{
	if (start >= len)
		return NOT_FOUND;

	for (size_t i = start; i < len; ++i) {
		if (str[i] == chr)
			return i;
	}

	return NOT_FOUND;
}

size_t findString(const char *str, size_t len, const char *search, size_t search_len, size_t start)
{
	if (search_len > len)
		return NOT_FOUND;

	if (start + search_len >= len)
		return NOT_FOUND;

	for (size_t i = start; i < len - search_len; ++i) {
		bool found = true;

		for (size_t j = 0; j < search_len; ++j) {
			if (search[j] != str[i + j]) {
				found = false;
				break;
			}
		}

		if (found)
			return i;
	}

	return NOT_FOUND;
}

size_t findLast(const char *str, size_t len, char chr)
{
	if (len == 0)
		return NOT_FOUND;

	for (size_t i = len; i > 0; --i) {
		if (str[i - 1] == chr)
			return i - 1;
	}

	return NOT_FOUND;
}

size_t findFirstNotOf(const char *str, size_t len, const char *charset, size_t charsetlen, size_t start)
{
	if (start >= len)
		return 0;

	for (size_t i = start; i < len; ++i) {
		bool found = false;

		for (size_t j = 0; j < charsetlen; ++j) {
			if (str[i] == charset[j]) {
				found = true;
				break;
			}
		}

		if (!found)
			return i;
	}

	return len;
}

String fromBuffer(const char *buf, size_t begin, size_t end)
{
	if (begin >= end)
		return String();

	return String(buf + begin, end - begin);
}

bool compare(const char *str1, const char *str2, size_t len)
{
	for (size_t i = 0; i < len; ++i) {
		if (str1[i] != str2[i])
			return false;
		if (str1[i] == '\0' || str2[i] == '\0')
			return false;
	}

	return true;
}

size_t compareToMap(const char *str, size_t strlen, const char *map[], size_t maplen)
{
	for (size_t i = 0; i < maplen; ++i) {
		if (Str::compare(map[i], str, strlen)) {
			return i;
		}
	}

	return NOT_FOUND;
}

} // namespace Str