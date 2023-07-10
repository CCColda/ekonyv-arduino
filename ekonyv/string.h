#ifndef EKONYV_STRING_H
#define EKONYV_STRING_H

namespace Str {
const size_t NOT_FOUND = ~(size_t)0;

const char WHITESPACE[] = "\x20\t\v\n\r";
const size_t WHITESPACE_LEN = sizeof(WHITESPACE);

size_t find(const char *str, size_t len, char chr, size_t start = 0)
{
	if (start >= len)
		return NOT_FOUND;

	for (size_t i = start; i < len; ++i) {
		if (str[i] == chr)
			return i;
	}

	return NOT_FOUND;
}

size_t findLast(const char *str, size_t len, char chr)
{
	if (len == 0)
		return NOT_FOUND;

	for (size_t i = len - 1; i >= 0; --i) {
		if (str[i] == chr)
			return i;
	}

	return NOT_FOUND;
}

size_t findFirstNotOf(const char *str, size_t len, const char *charset, size_t charsetlen, size_t start = 0)
{
	if (start >= len)
		return NOT_FOUND;

	for (size_t i = start; i < len; ++i)
		for (size_t j = 0; j < charsetlen; ++j)
			if (str[i] != charset[j])
				return i;

	return NOT_FOUND;
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

#endif // !defined(EKONYV_STRING_H)