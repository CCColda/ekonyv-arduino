#ifndef EKONYV_TYPES_FIXEDBUFFER_H
#define EKONYV_TYPES_FIXEDBUFFER_H

#include <Arduino.h>

template <size_t N>
struct FixedBuffer {
	byte data[N];
};

template <size_t N>
bool operator==(const FixedBuffer<N> &l, const FixedBuffer<N> &r)
{
	return memcmp(l.data, r.data, N) == 0;
}

template <size_t N>
bool operator!=(const FixedBuffer<N> &l, const FixedBuffer<N> &r)
{
	return memcmp(l.data, r.data, N) != 0;
}

template <size_t N>
FixedBuffer<N> &assign(FixedBuffer<N> &l, const FixedBuffer<N> &r)
{
	memcpy(l.data, r.data, N);
	return l;
}

#endif // !defined(EKONYV_TYPES_FIXEDBUFFER_H)