#ifndef EKONYV_STRING_H
#define EKONYV_STRING_H

#include <Arduino.h>

namespace Str {

extern const size_t NOT_FOUND;

extern const char WHITESPACE[6];
extern const size_t WHITESPACE_LEN;

size_t find(const char *str, size_t len, char chr, size_t start = 0);
size_t findString(const char *str, size_t len, const char *search, size_t search_len, size_t start = 0);
size_t findLast(const char *str, size_t len, char chr);
size_t findFirstNotOf(const char *str, size_t len, const char *charset, size_t charsetlen, size_t start = 0);
String fromBuffer(const char *buf, size_t begin, size_t end);
bool compare(const char *str1, const char *str2, size_t len);
size_t compareToMap(const char *str, size_t strlen, const char *map[], size_t maplen);

template <typename I>
I fixedAtoi(const char *str, size_t len)
{
	I result = I(0);

	static const char numbers[] = {
	    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};

	const auto last_non_digit = Str::findFirstNotOf(str, len, numbers, sizeof(numbers));

	if (last_non_digit == 0)
		return 0;

	I exponent = I(1);

	for (size_t i = last_non_digit;
	     i > 0;
	     i -= 1, exponent *= 10)
		result += exponent * I(str[i - 1] - '0');

	return result;
}

} // namespace Str

#endif // !defined(EKONYV_STRING_H)