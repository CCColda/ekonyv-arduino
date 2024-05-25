#ifndef EKONYV_UTILITY_H
#define EKONYV_UTILITY_H

#define EK_UTIL_LOG_TAG "UTIL"

#include "logger.h"

namespace Utility {
/**
 * @brief Applies an infinite waiting loop, sends @c reason to Serial and @c reason_code to the LCD.
 * Halts can only be reverted via restarting.
 */
void halt(const char *reason = "<no reason provided>", const char* reason_code = "unknown");
} // namespace Utility

#endif // !defined(EKONYV_UTILITY_H)