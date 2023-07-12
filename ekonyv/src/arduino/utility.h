#ifndef EKONYV_UTILITY_H
#define EKONYV_UTILITY_H

#define EK_UTIL_LOG_TAG "UTIL"

#include "logger.h"

namespace Utility {
void halt(const char *reason = "<no reason provided>")
{
	Logger(EK_UTIL_LOG_TAG).warning(String("Halting: ") + String(reason));

	while (true)
		delay(1);
}
} // namespace Utility

#endif // !defined(EKONYV_UTILITY_H)