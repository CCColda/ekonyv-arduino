#ifndef EKONYV_URL_H
#define EKONYV_URL_H

#include <Arduino.h>

#include "../types/sized_string.h"

namespace Str {
//! @brief Converts @c str into an url encoded string.
String urlEncode(const SizedString &str);

//! @brief Decodes @c str from an url encoded string.
String urlDecode(const SizedString &str);
} // namespace Str

#endif // !defined(EKONYV_URL_H)