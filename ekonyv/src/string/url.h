#ifndef EKONYV_URL_H
#define EKONYV_URL_H

#include <Arduino.h>

namespace Str {
String urlEncode(const char *str, size_t len);
String urlDecode(const char *str, size_t len);
} // namespace Str

#endif // !defined(EKONYV_URL_H)