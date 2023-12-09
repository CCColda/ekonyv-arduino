#ifndef EKONYV_STRING_IMPL_H
#define EKONYV_STRING_IMPL_H

#include <Arduino.h>

namespace Str {

template <typename IntegerType>
IntegerType fixedAtoi(const char *str, size_t len)
{
	IntegerType result = IntegerType(0);

	static const char numbers[] = {
	    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};

	const auto last_non_digit = Str::findFirstNotOf(str, len, numbers, sizeof(numbers));

	if (last_non_digit == 0)
		return 0;

	IntegerType exponent = IntegerType(1);

	for (size_t i = last_non_digit;
	     i > 0;
	     i -= 1, exponent *= 10)
		result += exponent * IntegerType(str[i - 1] - '0');

	return result;
}

} // namespace Str

#endif // !defined(EKONYV_STRING_IMPL_H)