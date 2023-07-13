#ifndef EKONYV_STRING_HASH_H
#define EKONYV_STRING_HASH_H

#include "../types/fixedbuffer.h"

namespace Str {
void hashAndSaltString(const String &str, FixedBuffer<32> &output);
}

#endif // !defined(EKONYV_STRING_HASH_H)