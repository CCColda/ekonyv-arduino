#ifndef EKONYV_URL_H
#define EKONYV_URL_H

#include <Arduino.h>

namespace Url {
String getParameter(const char *path, size_t len, const char *parameter, size_t paramlen);
}

#endif // !defined(EKONYV_URL_H)