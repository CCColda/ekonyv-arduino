/**
 * @file
 * @brief Functions for string hashing.
 */

#ifndef EKONYV_STRING_HASH_H
#define EKONYV_STRING_HASH_H

#include "../types/fixedbuffer.h"

namespace Str {
//! @brief Salts a string with @c EK_PASSWORD_SALT and generates a SHA-256 hash into @c output .
void hashAndSaltString(const String &str, FixedBuffer<32> &output);
} // namespace Str

#endif // !defined(EKONYV_STRING_HASH_H)