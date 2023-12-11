/**
 * @file
 * @brief Defines types for parsing HTTP packets.
 */

#ifndef EKONYV_HTTP_H
#define EKONYV_HTTP_H

#include "../config.h"

#include <Arduino.h>

namespace HTTP {
//! @brief Enumerates HTTP request methods.
enum Method : uint8_t {
	GET,
	PUT,
	POST,
	DELETE,
	m_size,

	m_unknown
};

//! @brief Enumerates HTTP client request headers.
enum ClientHeader : uint8_t {
	AUTHORIZATION,
	ACCEPT,
	ch_size,

	ch_unknown
};

//! @brief Enumerates HTTP server response headers.
enum ServerHeader : uint8_t {
	CONTENT_TYPE,
	CONNECTION,
	CONTENT_LENGTH,
	sh_size,

	sh_unknown
};

//! @brief Enumerates HTTP parsing statuses.
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

//! @brief Holds the string representations of HTTP methods
extern const char *METHOD_MAP[Method::m_size];

//! @brief Holds the string representations of HTTP client request headers
extern const char *CLIENT_HEADERS[ClientHeader::ch_size];

//! @brief Holds the string representations of HTTP server response headers
extern const char *SERVER_HEADERS[ServerHeader::sh_size];

} // namespace HTTP

#endif // !defined(EKONYV_HTTP_H)