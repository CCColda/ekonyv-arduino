#include "to_string.h"

String ip_to_string(const IPAddress &address)
{
	String result = String((unsigned int)address[0], 10);
	for (int i = 1; i < 4; ++i) {
		result += String('.');
		result += String((unsigned int)address[i], 10);
	}
	return result;
}

String byte_to_string(const byte &b)
{
	return b < 0x10 ? String('0') + String((unsigned int)b, 0x10)
	                : String((unsigned int)b, 0x10);
}

String mac_to_string(const byte *mac)
{
	String result = byte_to_string(mac[0]);
	for (int i = 1; i < 6; ++i) {
		result += String(':');

		result += byte_to_string(mac[i]);
	}

	return result;
}

[[maybe_unused]] String string_to_escaped_string(const String &str)
{
	String result;
	result.reserve(str.length());

	for (size_t i = 0; i < str.length(); ++i) {
		const unsigned char letter = str[i];
		if ((letter >= (unsigned char)' ') && (letter <= (unsigned char)'~')) {
			result += (char)letter;
		}
		else {
			result += "\\x";
			result += byte_to_string((byte)letter);
		}
	}

	return result;
}

String string_to_html_escaped_string(const String &str)
{
	String result;
	result.reserve(str.length());

	for (size_t i = 0; i < str.length(); ++i) {
		const unsigned char letter = str[i];
		if (letter == '<')
			result += "&lt;";
		else if (letter == '>')
			result += "&gt;";
		else if (letter == '"')
			result += "&quot";
		else
			result += (char)letter;
	}

	return result;
}