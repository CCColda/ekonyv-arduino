#ifndef EKONYV_STRING_HASH_H
#define EKONYV_STRING_HASH_H

#include "../types/fixedbuffer.h"

namespace Str {
FixedBuffer<32> hashAndSaltString(const String &str);
}

#endif // !defined(EKONYV_STRING_HASH_H)