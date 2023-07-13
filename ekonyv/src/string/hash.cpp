#include "hash.h"

#include "../config.h"

#include <SHA256.h>

namespace Str {
void hashAndSaltString(const String &str, FixedBuffer<32> &output)
{
	const auto length = str.length();

	byte *salted_buffer = new byte[length + EK_PASSWORD_SALT_LEN];
	memcpy(salted_buffer, str.c_str(), length);
	memcpy(salted_buffer + length, EK_PASSWORD_SALT, EK_PASSWORD_SALT_LEN);

	SHA256 hasher;
	hasher.update(salted_buffer, length + EK_PASSWORD_SALT_LEN);
	hasher.finalize(output.data, 32);

	delete[] salted_buffer;
}
} // namespace Str