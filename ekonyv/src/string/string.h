#ifndef EKONYV_STRING_H
#define EKONYV_STRING_H

#include <Arduino.h>

namespace Str {

static constexpr const size_t NOT_FOUND = ~(size_t)0;

static constexpr const char WHITESPACE[6] = "\x20\t\v\n\r";
static constexpr const char NUMBER[11] = "0123456789";

static constexpr const size_t WHITESPACE_LEN = sizeof(WHITESPACE) / sizeof(WHITESPACE[0]);

//! @brief Finds the first occurrence of @c chr in @c str , starting from the index @c start .
//! If @c str does not contain the character, returns @c NOT_FOUND .
size_t find(const char *str, size_t len, char chr, size_t start = 0);

//! @brief Finds the string @c search in @c str , starting from the index @c start .
//! If @c search is not found, returns @c NOT_FOUND .
size_t findString(const char *str, size_t len, const char *search, size_t search_len, size_t start = 0);

//! @brief Finds the last occurrence of @c chr in @c str , starting from the index @c start from the end.
//! If @c chr is not found, returns @c NOT_FOUND .
size_t findLast(const char *str, size_t len, char chr);

//! @brief Finds the first character not contained in @c charset .
//! If @c str is made of characters from @c charset , returns @c len .
size_t findFirstNotOf(const char *str, size_t len, const char *charset, size_t charsetlen, size_t start = 0);

//! @brief Makes an Arduino built-in string from a span of characters between @c buffer+begin and @c buffer+end .
String fromBuffer(const char *buf, size_t begin, size_t end);

//! @brief Checks if two strings are equal.
bool compare(const char *str1, const char *str2, size_t len);

//! @brief Checks if @c str is equal to any of the strings in @c map .
//! If @c str is not found in the map, returns @c NOT_FOUND .
size_t compareToMap(const char *str, size_t strlen, const char *map[], size_t maplen);

template <typename IntegerType>
//! @brief Constructs an (unsigned) integer from the decimal number in @c str .
//! @tparam IntegerType The type of output integer.
IntegerType fixedAtoi(const char *str, size_t len);

} // namespace Str

#include "string_impl.h"

#endif // !defined(EKONYV_STRING_H)