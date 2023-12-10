/**
 * @file
 * @brief Implementation of template string conversion functions.
 */

#ifndef EKONYV_TO_STRING_IMPL_H
#define EKONYV_TO_STRING_IMPL_H

#include <Arduino.h>

#include "../types/fixedbuffer.h"

template <typename... Args>
//! @brief Concatanates @c args into a string, from left to right.
String str(Args... args)
{
	String result;

	(result.operator+=(args), ...);

	return result;
}

template <typename IntegerType>
//! @brief Converts the number @c v to a string, pads it with '0' characters from the left until the length is @c pad_chars .
String number_to_padded_string(IntegerType v, uint8_t pad_chars)
{
	String result = String(v, 10);

	for (uint32_t i = result.length(); i < pad_chars; ++i)
		result = '0' + result;

	return result;
}

template <size_t N>
//! @brief Converts a fixed buffer of @c N bytes to a string with byte formatting.
String fixed_buffer_to_string(const FixedBuffer<N> &buf)
{
	String result;
	result.reserve(N * 2);

	for (size_t i = 0; i < N; ++i)
		result += byte_to_string(buf.data[i]);

	return result;
}

template <size_t N>
//! @brief Converts a byte-formatted string to a fixed buffer of length @c N .
//! @returns true if the length of the string is correct and data has been written to @c output .
bool string_to_fixed_buffer(const SizedString &str, FixedBuffer<N> &output)
{
	if (str.len < (N * 2) || (str.len % 2) != 0)
		return false;

	for (size_t i = 0; i < N; i++)
		output.data[i] = string_to_byte(SizedString{str.ptr + i * 2, 2});

	return true;
}

#endif // !defined(EKONYV_TO_STRING_IMPL_H)