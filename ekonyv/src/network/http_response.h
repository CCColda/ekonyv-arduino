#ifndef EKONYV_HTTP_RESPONSE_H
#define EKONYV_HTTP_RESPONSE_H

namespace HTTPResponse {
struct StaticHTMLResponse {
	uint16_t statusCode;
	const char *statusMessage;

	const char *heading;
	const char *description;
};

const StaticHTMLResponse HTML_BAD_REQUEST = {
    400,
    "Bad Request",
    "400 - Bad Request",
    "The request is malformed."};

const StaticHTMLResponse HTML_NOT_FOUND = {
    404,
    "Not Found",
    "404 - Not Found",
    "The resource you requested does not seem to exist."};

const StaticHTMLResponse HTML_UNAUTHORIZED = {
    401,
    "Unauthorized",
    "401 - Unauthorized",
    "Invalid credentials."};

const StaticHTMLResponse HTMP_NOT_IMPLEMENTED = {
    501,
    "Unimplemented",
    "501 - Unimplemented",
    "The endpoint you provided was registered but not implemented."};
} // namespace HTTPResponse

#endif // !defined(EKONYV_HTTP_RESPONSE_H)