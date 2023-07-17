#ifndef EKONYV_SERVER_H
#define EKONYV_SERVER_H

#include <Ethernet.h>
#include <Vector.h>

#include "../config.h"

#include "http.h"

#include "../arduino/logger.h"
#include "http_response.h"

class HTTPServer {
public:
	using clock_t = decltype(millis());

	struct HandlerBehavior {
		static const uint8_t NONE = 0x00;
		static const uint8_t ALLOW_SUBPATHS = 0x01;
		static const uint8_t ALLOW_PARAMETERS = 0x02;
	};

	typedef int (*HTTPRequestHandlerPtr)(const String &path, const Vector<HTTP::ClientHeaderPair> &headers, EthernetClient &client);

	struct Handler {
		const char *path;
		size_t pathlen;
		uint8_t behavior;
		HTTP::Method method;
		HTTPRequestHandlerPtr handler;
	};

	struct DisconnectPromise {
		uint8_t clientSocket;
		clock_t terminate_after;
	};

private:
	static Logger logger;

	//! @brief When set to true, disconnect events will ignore the timer
	static bool is_overloaded;

private:
	EthernetServer m_server;

	Handler m_handlerStorage[EK_HTTP_HANDLER_STORAGE];
	Vector<Handler> m_handlers;

private:
	void parseRequest(
	    const HTTP::ClientRequestProps &props,
	    const Vector<HTTP::ClientHeaderPair> &headers,
	    EthernetClient &client);

public:
	HTTPServer();

	void on(HTTP::Method method, const char *path, uint8_t behavior, HTTPRequestHandlerPtr handler);

	void start();
	void update();

	static void writeHTTPHeaders(uint16_t status_code, const char *status_message, const char *content_type, EthernetClient &client);
	static void writeStaticHTMLResponse(const HTTPResponse::StaticHTMLResponse &response, EthernetClient &client);
};

#endif // !defined(EKONYV_SERVER_H)