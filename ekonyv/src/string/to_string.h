/**
 * @file
 * @brief Functions related to string conversion.
 */

#ifndef EKONYV_TO_STRING_H
#define EKONYV_TO_STRING_H

#include <Arduino.h>
#include <type_traits>

#include "../types/fixedbuffer.h"
#include "../types/sized_string.h"

//! @brief Converts an IPAddress to a 'xxx.xxx.xxx.xxx' formatted string.
String ip_to_string(const IPAddress &address);

//! @brief Converts a byte to a hexadecimal string.
String byte_to_string(const byte &b);

//! @brief Converts a hexadecimal byte string (format XX) into a byte. The string may be upper or lowercase.
byte string_to_byte(const SizedString &str);

//! @brief Converts a 6-byte mac address to a \c XX:XX:XX:XX formatted string.
String mac_to_string(const byte *mac);

//! @brief Escapes all non-ascii characters in a string.
[[maybe_unused]] String string_to_escaped_string(const String &str);

//! @brief Escapes all \c ["<>] characters in a string to their HTML equivalents.
String string_to_html_escaped_string(const String &str);

template <typename... Args>
//! @brief Concatanates @c args into a string, from left to right.
String str(Args... args);

template <typename IntegerType>
//! @brief Converts the number @c v to a string, pads it with '0' characters from the left until the length is @c pad_chars .
String number_to_padded_string(IntegerType v, uint8_t pad_chars);

template <size_t N>
//! @brief Converts a fixed buffer of @c N bytes to a string with byte formatting.
String fixed_buffer_to_string(const FixedBuffer<N> &buf);

template <size_t N>
//! @brief Converts a byte-formatted string to a fixed buffer of length @c N .
//! @returns true if the length of the string is correct and data has been written to @c output .
bool string_to_fixed_buffer(const SizedString &str, FixedBuffer<N> &output);

#include "to_string_impl.h"

#endif // !defined(EKONYV_TO_STRING_H)