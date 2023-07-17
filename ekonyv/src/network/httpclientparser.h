#ifndef EKONYV_HTTP_CLIENT_PARSER_H
#define EKONYV_HTTP_CLIENT_PARSER_H

#include "http.h"

#include "../arduino/logger.h"
#include "../config.h"

#include <Ethernet.h>
#include <Vector.h>

class HTTPClientParser {
public:
	using index_t = uint16_t;

private:
	static Logger logger;

	char m_buffer[EK_HTTP_BUFFER_SIZE];
	index_t m_buffer_saturation;
	bool m_props_parsed;

	HTTP::ClientHeaderPair m_header_buf[HTTP::ClientHeader::ch_size];

private:
	static HTTP::ClientRequestProps extractRequestProps(const char *requestLine, index_t len);
	static HTTP::ClientHeaderPair extractHeader(const char *requestLine, index_t len);

public:
	HTTP::ClientRequestProps props;
	Vector<HTTP::ClientHeaderPair> headers;

public:
	HTTPClientParser();

	HTTP::ParseResult parseBlock(EthernetClient &client);
};

#endif // !defined(EKONYV_HTTP_CLIENT_PARSER_H)