#ifndef EKONYV_UTILITY_H
#define EKONYV_UTILITY_H

#define EK_UTIL_LOG_TAG "UTIL"

#include "logger.h"

namespace Utility {
void halt(const char *reason = "<no reason provided>");
} // namespace Utility

#endif // !defined(EKONYV_UTILITY_H)