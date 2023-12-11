/**
 * @file
 * @brief Contains definitions of fixed HTTP response messages.
 */

#ifndef EKONYV_HTTP_RESPONSE_H
#define EKONYV_HTTP_RESPONSE_H

namespace HTTPResponse {
struct StaticHTMLResponse {
	uint16_t statusCode;
	const char *statusMessage;

	const char *heading;
	const char *description;
};

constexpr static const StaticHTMLResponse HTML_BAD_REQUEST = {
    400,
    "Bad Request",
    "400 - Bad Request",
    "The request is malformed."};

constexpr static const StaticHTMLResponse HTML_NOT_FOUND = {
    404,
    "Not Found",
    "404 - Not Found",
    "The resource you requested does not seem to exist."};

constexpr static const StaticHTMLResponse HTML_UNAUTHORIZED = {
    401,
    "Unauthorized",
    "401 - Unauthorized",
    "Invalid credentials."};

constexpr static const StaticHTMLResponse HTML_NOT_IMPLEMENTED = {
    501,
    "Unimplemented",
    "501 - Unimplemented",
    "The endpoint you provided was registered but not implemented."};

constexpr static const StaticHTMLResponse HTML_INVALID_BOOK = HTTPResponse::StaticHTMLResponse{
    400, "Bad Request",
    "400 - Bad Request",
    "The book with the given ID doesn't exist."};

constexpr static const StaticHTMLResponse HTML_BOOK_WRITE_PROTECTED = HTTPResponse::StaticHTMLResponse{
    400, "Bad Request",
    "400 - Bad Request",
    "The book must be edited by its uploader."};

constexpr static const StaticHTMLResponse HTML_INVALID_USER = HTTPResponse::StaticHTMLResponse{
    400, "Bad Request",
    "400 - Bad Request",
    "The user with the given ID doesn't exist."};

constexpr static const StaticHTMLResponse HTML_INVALID_STORAGE = HTTPResponse::StaticHTMLResponse{
    400, "Bad Request",
    "400 - Bad Request",
    "The storage place with the given ID doesn't exist."};
} // namespace HTTPResponse

#endif // !defined(EKONYV_HTTP_RESPONSE_H)