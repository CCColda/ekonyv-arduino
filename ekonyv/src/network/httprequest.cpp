#include "httprequest.h"
#include "../config.h"

HTTPRequest::HTTPRequest()
    : client(), body_parse_cb(), header_response_cb() {}

HTTPRequest::HTTPRequest(const HTTPRequest &other)
    : body_parse_cb(other.body_parse_cb), header_response_cb(other.header_response_cb)
{
}

HTTPRequest::~HTTPRequest()
{
	if (client)
		client.stop();
}

void HTTPRequest::connect(const char *ip, uint16_t port)
{
	client.connect(ip, port);
}

void HTTPRequest::request(HTTP::Method method, const char *path, size_t len)
{
	if (!client.connected())
		return;

	client.print(HTTP::METHOD_MAP[method]);
	client.print(' ');
	client.write(path, len);
	client.print(' ');
	client.println("HTTP/1.1");
}

void HTTPRequest::addHeader(const char *name, size_t namelen, const char *value, size_t valuelen)
{
	if (!client.connected())
		return;

	client.write(name, namelen);
	client.print(':');
	client.write(value, valuelen);
	client.println();
}

void HTTPRequest::end()
{
	client.println();
}

/* static */ HTTPRequest HTTPRequest::make(HTTP::Method method, const String &host, const String &path)
{
	HTTPRequest result;

	result.connect(host.c_str(), 80);
	result.request(method, path.c_str(), path.length());
	result.addHeader("Host", 4, host.c_str(), host.length());
	result.addHeader("User-Agent", 10, (EK_NAME "/" EK_VERSION), sizeof(EK_NAME "/" EK_VERSION));
	result.addHeader("Accept", 6, "text/html;charset=UTF-8", 23);
	result.addHeader("Accept-Encoding", 15, "identity", 8);
	result.end();

	return result;
}