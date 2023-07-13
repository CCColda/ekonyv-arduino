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

String ip_to_string(const IPAddress &address);
String byte_to_string(const byte &b);
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

#endif // !defined(EKONYV_TO_STRING_H)