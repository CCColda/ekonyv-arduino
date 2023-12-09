#ifndef EKONYV_URL_H
#define EKONYV_URL_H

#include <Arduino.h>

namespace Str {
//! @brief Converts str into an url encoded string.
String urlEncode(const char *str, size_t len);

//! @brief Decodes str from an url encoded string.
String urlDecode(const char *str, size_t len);
} // namespace Str

#endif // !defined(EKONYV_URL_H)