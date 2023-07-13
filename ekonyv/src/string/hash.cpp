#include "hash.h"

#include "../config.h"

#include <SHA256.h>

namespace Str {
FixedBuffer<32> hashAndSaltString(const String &str)
{
	const auto length = str.length();

	byte *salted_buffer = new byte[length + EK_PASSWORD_SALT_LEN];
	memcpy(salted_buffer, str.c_str(), length);
	memcpy(salted_buffer + length, EK_PASSWORD_SALT, EK_PASSWORD_SALT_LEN);

	FixedBuffer<32> hash;

	SHA256 hasher;
	hasher.update(salted_buffer, length + EK_PASSWORD_SALT_LEN);
	hasher.finalize(hash.data, 32);

	delete[] salted_buffer;
}
}