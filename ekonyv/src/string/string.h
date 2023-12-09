#ifndef EKONYV_STRING_H
#define EKONYV_STRING_H

#include <Arduino.h>

#include "../types/sized_string.h"

namespace Str {

static constexpr const size_t NOT_FOUND = ~(size_t)0;

static constexpr const char WHITESPACE_CHARS[6] = "\x20\t\v\n\r";
static constexpr const size_t WHITESPACE_LEN = sizeof(WHITESPACE_CHARS) / sizeof(WHITESPACE_CHARS[0]);

static constexpr const char DIGIT_CHARS[11] = "0123456789";
static constexpr const size_t DIGIT_LEN = sizeof(DIGIT_CHARS) / sizeof(DIGIT_CHARS[0]);

static constexpr const SizedString WHITESPACE = {WHITESPACE_CHARS, WHITESPACE_LEN};
static constexpr const SizedString DIGIT = {DIGIT_CHARS, DIGIT_LEN};

//! @brief Finds the first occurrence of @c chr in @c str , starting from the index @c start .
//! If @c str does not contain the character, returns @c NOT_FOUND .
size_t find(const SizedString &str, char chr, size_t start = 0);

//! @brief Finds the string @c search in @c str , starting from the index @c start .
//! If @c search is not found, returns @c NOT_FOUND .
size_t findString(const SizedString &str, const SizedString &search, size_t start = 0);

//! @brief Finds the last occurrence of @c chr in @c str , starting from the index @c start from the end.
//! If @c chr is not found, returns @c NOT_FOUND .
size_t findLast(const SizedString &str, char chr);

//! @brief Finds the first character not contained in @c charset .
//! If @c str is made of characters from @c charset , returns @c len .
size_t findFirstNotOf(const SizedString &str, const SizedString &charset, size_t start = 0);

//! @brief Makes an Arduino built-in string from a span of characters between @c buffer+begin and @c buffer+end .
String fromBuffer(const char *buf, size_t begin, size_t end);

//! @brief Checks if two strings are equal.
bool compare(const char *str1, const char *str2, size_t len);

//! @brief Checks if @c str is equal to any of the strings in @c map .
//! If @c str is not found in the map, returns @c NOT_FOUND .
size_t compareToMap(const SizedString &str, const char *map[], size_t maplen);

template <typename IntegerType>
//! @brief Constructs an (unsigned) integer from the decimal number in @c str .
//! @tparam IntegerType The type of output integer.
IntegerType fixedAtoi(const SizedString &str);

} // namespace Str

#include "string_impl.h"

#endif // !defined(EKONYV_STRING_H)