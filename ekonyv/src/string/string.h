#ifndef EKONYV_STRING_H
#define EKONYV_STRING_H

#include <Arduino.h>

namespace Str {

extern const size_t NOT_FOUND;

extern const char WHITESPACE[6];
extern const size_t WHITESPACE_LEN;

size_t find(const char *str, size_t len, char chr, size_t start = 0);
size_t findLast(const char *str, size_t len, char chr);
size_t findFirstNotOf(const char *str, size_t len, const char *charset, size_t charsetlen, size_t start = 0);
String fromBuffer(const char *buf, size_t begin, size_t end);
bool compare(const char *str1, const char *str2, size_t len);
size_t compareToMap(const char *str, size_t strlen, const char *map[], size_t maplen);

} // namespace Str

#endif // !defined(EKONYV_STRING_H)