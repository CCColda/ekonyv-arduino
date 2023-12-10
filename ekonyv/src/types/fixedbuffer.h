/**
 * @file
 * @brief FixedBuffer definition.
 */

#ifndef EKONYV_TYPES_FIXEDBUFFER_H
#define EKONYV_TYPES_FIXEDBUFFER_H

#include <Arduino.h>

template <size_t N>
//! @brief Structure representing a byte array of @c N bytes.
struct FixedBuffer {
	byte data[N];
};

template <size_t N>
inline bool operator==(const FixedBuffer<N> &l, const FixedBuffer<N> &r) { return memcmp(l.data, r.data, N) == 0; }

template <size_t N>
inline bool operator!=(const FixedBuffer<N> &l, const FixedBuffer<N> &r) { return memcmp(l.data, r.data, N) != 0; }

#endif // !defined(EKONYV_TYPES_FIXEDBUFFER_H)