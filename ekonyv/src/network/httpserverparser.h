#ifndef EKONYV_HTTPSERVERPARSER_H
#define EKONYV_HTTPSERVERPARSER_H

#include "../arduino/logger.h"
#include "../config.h"
#include "http.h"

#include "../types/callback.h"

#include "../types/sized_string.h"

#include <Ethernet.h>
#include <Vector.h>

class HTTPServerParser {
public:
	using index_t = uint16_t;

	Callback<HTTP::ParseResult, uint16_t, const Vector<HTTP::ServerHeaderPair> &> header_parse_cb;
	Callback<HTTP::ParseResult, const char *, size_t> body_parse_cb;

	struct Flags {
		uint8_t props_parsed : 1;
		uint8_t headers_parsed : 1;
	};

private:
	static Logger logger;

	char m_buffer[EK_HTTP_BUFFER_SIZE];
	index_t m_buffer_saturation;

	HTTP::ServerHeaderPair m_header_buf[HTTP::ServerHeader::sh_size];

private:
	static HTTP::ServerResponseProps extractResponseProps(const SizedString &request_line);
	static HTTP::ServerHeaderPair extractHeader(const SizedString &request_line);

public:
	HTTP::ServerResponseProps props;
	Vector<HTTP::ServerHeaderPair> headers;
	Flags flags;

public:
	HTTPServerParser();
	~HTTPServerParser();

	inline operator bool() const { return flags.props_parsed && flags.headers_parsed; }
	inline bool operator!() const { return !flags.props_parsed || !flags.headers_parsed; }

	HTTP::ParseResult parseBlock(EthernetClient &client);
};

#endif // !defined(EKONYV_HTTPSERVERPARSER_H)