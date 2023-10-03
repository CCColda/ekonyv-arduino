#include "utility.h"

namespace Utility {
void halt(const char *reason)
{
	Logger(EK_UTIL_LOG_TAG).warning(String("Halting: ") + String(reason));

	while (true)
		delay(1);
}
} // namespace Utility