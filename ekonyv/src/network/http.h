#ifndef EKONYV_HTTP_H
#define EKONYV_HTTP_H

#include "../config.h"

#include <Arduino.h>
#include <Ethernet.h>

namespace HTTP {
enum Method : uint8_t {
	GET,
	PUT,
	POST,
	DELETE,
	m_size,

	m_unknown
};

enum ClientHeader : uint8_t {
	AUTHORIZATION,
	ACCEPT,
	ch_size,

	ch_unknown
};

enum ServerHeader : uint8_t {
	CONTENT_TYPE,
	CONNECTION,
	CONTENT_LENGTH,
	sh_size,

	sh_unknown
};

enum ParseResult : uint8_t {
	FAIL,
	SUCCESS,
	CONTINUE
};

struct ClientRequestProps {
	Method method;
	String path;
};

struct ServerResponseProps {
	uint16_t status_code;
};

template <typename T>
struct HeaderPair {
	T name;
	String value;
};

using ClientHeaderPair = HeaderPair<ClientHeader>;
using ServerHeaderPair = HeaderPair<ServerHeader>;

extern const char *METHOD_MAP[Method::m_size];
extern const char *CLIENT_HEADERS[ClientHeader::ch_size];
extern const char *SERVER_HEADERS[ServerHeader::sh_size];

} // namespace HTTP

#endif // !defined(EKONYV_HTTP_H)