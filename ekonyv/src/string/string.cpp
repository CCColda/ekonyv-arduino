/**
 * @file
 * @brief Implementation of string functions.
 */

#include "./string.h"

namespace Str {
size_t find(const SizedString &str, char chr, size_t start)
{
	if (start >= str.len)
		return NOT_FOUND;

	const char *ptr = reinterpret_cast<const char *>(memchr(str.ptr + start, str.len - start, chr));
	return (ptr == NULL) ? NOT_FOUND : ptr - str.ptr;
}

size_t findString(const SizedString &str, const SizedString &search, size_t start)
{
	if (start + search.len >= str.len)
		return NOT_FOUND;

	for (size_t i = start; i < str.len - search.len; ++i) {
		bool found = true;

		for (size_t j = 0; j < search.len; ++j) {
			if (search.ptr[j] != str.ptr[i + j]) {
				found = false;
				break;
			}
		}

		if (found)
			return i;
	}

	return NOT_FOUND;
}

size_t findLast(const SizedString &str, char chr)
{
	if (str.len == 0)
		return NOT_FOUND;

	for (size_t i = str.len; i > 0; --i) {
		if (str.ptr[i - 1] == chr)
			return i - 1;
	}

	return NOT_FOUND;
}

size_t findFirstNotOf(const SizedString &str, const SizedString &charset, size_t start)
{
	if (start >= str.len)
		return 0;

	for (size_t i = start; i < str.len; ++i) {
		bool found = false;

		for (size_t j = 0; j < charset.len; ++j) {
			if (str.ptr[i] == charset.ptr[j]) {
				found = true;
				break;
			}
		}

		if (!found)
			return i;
	}

	return str.len;
}

String fromBuffer(const char *buf, size_t begin, size_t end)
{
	if (begin >= end)
		return String();

	return String(buf + begin, end - begin);
}

bool compare(const char *str1, const char *str2, size_t len)
{
	return memcmp(str1, str2, len) == 0;
}

size_t compareToMap(const SizedString &str, const char *map[], size_t maplen)
{
	for (size_t i = 0; i < maplen; ++i) {
		if (Str::compare(map[i], str.ptr, str.len)) {
			return i;
		}
	}

	return NOT_FOUND;
}

} // namespace Str