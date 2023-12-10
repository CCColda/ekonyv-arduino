/**
 * @file
 * @brief Implementation of template string functions.
 */

#ifndef EKONYV_STRING_IMPL_H
#define EKONYV_STRING_IMPL_H

#include <Arduino.h>

namespace Str {

template <typename IntegerType>
IntegerType fixedAtoi(const SizedString &str)
{
	IntegerType result = IntegerType(0);

	const auto last_non_digit = Str::findFirstNotOf(str, Str::DIGIT);

	if (last_non_digit == 0)
		return 0;

	IntegerType exponent = IntegerType(1);

	for (size_t i = last_non_digit;
	     i > 0;
	     i -= 1, exponent *= 10)
		result += exponent * IntegerType(str.ptr[i - 1] - '0');

	return result;
}

} // namespace Str

#endif // !defined(EKONYV_STRING_IMPL_H)