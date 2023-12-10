/**
 * @file
 * @brief SizedString definition.
 */

#ifndef EKONYV_SIZED_STRING_H
#define EKONYV_SIZED_STRING_H

#include <Arduino.h>

//! @brief Structure representing a string with a pointer and a length.
struct SizedString {
	const char *ptr;
	size_t len;

	inline static SizedString fromString(const String &str) { return SizedString{str.c_str(), str.length()}; }
};

#endif // !defined(EKONYV_SIZED_STRING_H)