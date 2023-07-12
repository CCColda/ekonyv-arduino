#ifndef EKONYV_SERVER_H
#define EKONYV_SERVER_H

#include <Ethernet.h>
#include <Vector.h>

#include "../arduino/logger.h"
#include "../eventqueue/eventqueue.h"
#include "http_response.h"

#define EK_SERVER_PORT 80
#define EK_HANDLER_STORAGE 8

class HTTPServer {
public:
	using clock_t = decltype(millis());

	enum Header : uint8_t {
		AUTHORIZATION,
		ACCEPT,
		h_size,

		h_unknown
	};

	struct HandlerBehavior {
		static const uint8_t NONE = 0x00;
		static const uint8_t ALLOW_SUBPATHS = 0x01;
		static const uint8_t ALLOW_PARAMETERS = 0x02;
	};

	enum Method : uint8_t {
		GET,
		POST,
		UPDATE,
		m_size,

		m_unknown
	};

	struct HeaderPair {
		Header name;
		String value;
	};

	struct RequestProps {
		Method method;
		String path;
	};

	typedef int (*HTTPRequestHandlerPtr)(const String &path, const Vector<HeaderPair> &headers, EthernetClient &client);

	struct Handler {
		const char *path;
		size_t pathlen;
		uint8_t behavior;
		Method method;
		HTTPRequestHandlerPtr handler;
	};

	struct DisconnectPromise {
		uint8_t clientSocket;
		clock_t terminate_after;
	};

private:
	static const char *ACCEPTED_HEADERS[Header::h_size];
	static const char *ACCEPTED_METHODS[Method::m_size];
	static Logger logger;

	//! @brief When set to true, disconnect events will ignore the timer
	static bool is_overloaded;

private:
	EthernetServer m_server;

	Handler m_handlerStorage[8];
	Vector<Handler> m_handlers;

	EventQueue<8> m_serverQueue;

private:
	void parseRequest(const RequestProps &props, const Vector<HeaderPair> &headers, EthernetClient &client);

	static Event disconnectHandler(DisconnectPromise &data);
	static RequestProps extractRequestProps(const char *requestLine, size_t len);
	static HeaderPair extractHeader(const char *requestLine, size_t len);

public:
	HTTPServer();

	void on(Method method, const char *path, uint8_t behavior, HTTPRequestHandlerPtr handler);

	void start();
	void update();

	static void writeHTTPHeaders(uint16_t status_code, const char *status_message, const char *content_type, EthernetClient &client);
	static void writeStaticHTMLResponse(const HTTPResponse::StaticHTMLResponse &response, EthernetClient &client);
};

#endif // !defined(EKONYV_SERVER_H)