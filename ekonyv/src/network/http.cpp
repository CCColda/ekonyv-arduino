#include "http.h"

namespace HTTP {
/* extern */ const char *METHOD_MAP[Method::m_size] = {
    "GET",
    "PUT",
    "POST",
    "DELETE"};

/* extern */ const char *CLIENT_HEADERS[ClientHeader::ch_size] = {
    "Authorization",
    "Accept"};

/* extern */ const char *SERVER_HEADERS[ServerHeader::sh_size] = {
    "Content-Type",
    "Connection",
    "Content-Length"};
} // namespace HTTP
