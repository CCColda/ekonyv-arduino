#ifndef EKONYV_HTTPREQUEST_H
#define EKONYV_HTTPREQUEST_H

#include <Ethernet.h>

#include "http.h"
#include "httpserverparser.h"

#include "../types/callback.h"

class HTTPRequest {
	friend class HTTPRequestManager;

public:
	decltype(HTTPServerParser::header_parse_cb) header_response_cb;
	decltype(HTTPServerParser::body_parse_cb) body_parse_cb;
	Callback<void> end_cb;

public:
	EthernetClient client;

	HTTPRequest();
	HTTPRequest(const HTTPRequest &other);
	~HTTPRequest();

	void connect(const char *ip, uint16_t port);
	void request(HTTP::Method method, const char *path, size_t len);
	void addHeader(const char *name, size_t namelen, const char *value, size_t valuelen);
	void end();

	static HTTPRequest make(HTTP::Method method, const String &host, uint16_t port, const String &path);
};

#endif // !defined(EKONYV_HTTPREQUEST_H)