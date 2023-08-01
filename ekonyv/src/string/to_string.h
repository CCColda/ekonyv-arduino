#ifndef EKONYV_TO_STRING_H
#define EKONYV_TO_STRING_H

#include <Arduino.h>
#include <type_traits>

#include "../types/fixedbuffer.h"

template <typename... Args>
String str(Args... args)
{
	String result;

	(result.operator+=(args), ...);

	return result;
}

template <typename T>
String number_to_padded_string(T v, uint8_t pad_chars)
{
	String result = String(v, 10);

	for (uint32_t i = result.length(); i < pad_chars; ++i)
		result = '0' + result;

	return result;
}

String ip_to_string(const IPAddress &address);
String byte_to_string(const byte &b);
byte string_to_byte(const char *str, size_t size);
String mac_to_string(const byte *mac);
[[maybe_unused]] String string_to_escaped_string(const String &str);
String string_to_html_escaped_string(const String &str);

template <size_t N>
String fixed_buffer_to_string(const FixedBuffer<N> &buf)
{
	String result;
	result.reserve(N * 2);

	for (size_t i = 0; i < N; ++i)
		result += byte_to_string(buf.data[i]);

	return result;
}

template <size_t N>
bool string_to_fixed_buffer(const char *str, size_t len, FixedBuffer<N> &output)
{
	if (len < (N * 2) || (len % 2) != 0)
		return false;

	for (size_t i = 0; i < N; i++)
		output.data[i] = string_to_byte(str + i * 2, 2);

	return true;
}

#endif // !defined(EKONYV_TO_STRING_H)