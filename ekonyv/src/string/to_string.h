#ifndef EKONYV_TO_STRING_H
#define EKONYV_TO_STRING_H

#include <Arduino.h>
#include <type_traits>

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

#endif // !defined(EKONYV_TO_STRING_H)