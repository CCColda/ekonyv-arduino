#include "httpserver.h"

#include "../string/string.h"
#include "../string/to_string.h"

#include "httpclientparser.h"

/* private static */ Logger HTTPServer::logger = Logger("HTTP");

/* private static */ bool HTTPServer::is_overloaded = false;

/* private */ void HTTPServer::parseRequest(
    const HTTP::ClientRequestProps &props,
    const Vector<HTTP::ClientHeaderPair> &headers,
    EthernetClient &client)
{
	bool found = false;

	for (size_t i = 0; i < m_handlers.size(); ++i) {
		const auto &handler = m_handlers.at(i);

		if (Str::compare(handler.path, props.path.c_str(), handler.pathlen) && handler.method == props.method) {
			if (props.path.length() > handler.pathlen) {
				if ((handler.behavior & HandlerBehavior::ALLOW_SUBPATHS) == 0 && props.path[handler.pathlen] == '/')
					continue;

				if ((handler.behavior & HandlerBehavior::ALLOW_PARAMETERS) == 0 && props.path[handler.pathlen] == '?')
					continue;
			}

			logger.log("Found handler for " + props.path);
			handler.handler(props.path, headers, client);
			found = true;
			break;
		}
	}

	if (!found) {
		logger.error("Invalid request for " + props.path + " from " + ip_to_string(client.remoteIP()));
		writeStaticHTMLResponse(HTTPResponse::HTML_NOT_FOUND, client);
	}
}

HTTPServer::HTTPServer() : m_server(EK_SERVER_PORT), m_handlers(), m_handlerStorage{}
{
	m_handlers.setStorage<EK_HTTP_HANDLER_STORAGE>(m_handlerStorage);
}

void HTTPServer::on(HTTP::Method method, const char *path, uint8_t behavior, HTTPRequestHandlerPtr handler)
{
	m_handlers.push_back(Handler{
	    path,
	    strlen(path),
	    behavior,
	    method,
	    handler});
}

void HTTPServer::start()
{
	m_server.begin();
}

void HTTPServer::update()
{
	EthernetClient client = m_server.available();

	if (client) {
		logger.log("Incoming connection from ", ip_to_string(client.remoteIP()));

		HTTPClientParser parser;

		while (client.connected()) {
			if (client.available()) {
				bool should_break = false;

				switch (parser.parseBlock(client)) {
					case HTTP::FAIL: {
						should_break = true;
						writeStaticHTMLResponse(HTTPResponse::HTML_BAD_REQUEST, client);
						break;
					}
					case HTTP::SUCCESS: {
						should_break = true;
						parseRequest(parser.props, parser.headers, client);
						break;
					}
					default: {
						break;
					}
				}

				if (should_break)
					break;
			}
		}

		logger.log("Disconnecting from ", ip_to_string(client.remoteIP()));
		client.stop();
	}
}
/* static */ void HTTPServer::writeHTTPHeaders(uint16_t status_code, const char *status_message, const char *content_type, EthernetClient &client)
{
	client.print("HTTP/1.1 ");
	client.print(status_code);
	client.print(' ');
	client.println(status_message);

	client.print("Server:");
	client.println(EK_NAME);

	client.print("Content-Type:");
	client.println(content_type);

	client.println("Connection:close");
	client.println();
}

/* static */ void HTTPServer::writeStaticHTMLResponse(const HTTPResponse::StaticHTMLResponse &response, EthernetClient &client)
{
	writeHTTPHeaders(
	    response.statusCode, response.statusMessage,
	    "text/html; charset=utf-8",
	    client);

	client.println("<html><body>");
	client.println("<style>body{display:flex;align-items:center;justify-content:center;gap:1rem;}</style>");

	client.print("<h1>");
	client.print(response.heading);
	client.print("</h1><p>");
	client.print(response.description);
	client.println("</p>");

	client.println("</body></html>");
}
