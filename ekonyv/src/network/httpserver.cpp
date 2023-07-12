#include "httpserver.h"

#include "../string/string.h"
#include "../string/to_string.h"

/* private static */ Logger HTTPServer::logger = Logger("HTTP");

/* private static */ const char *HTTPServer::ACCEPTED_HEADERS[HTTPServer::Header::h_size] = {
    "Authorization",
    "Accept"};

/* private static */ const char *HTTPServer::ACCEPTED_METHODS[HTTPServer::Method::m_size] = {
    "GET",
    "POST",
    "UPDATE"};

/* private static */ bool HTTPServer::is_overloaded = false;

/* private */ void HTTPServer::parseRequest(const HTTPServer::RequestProps &props, const Vector<HeaderPair> &headers, EthernetClient &client)
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

/* private static event */ Event HTTPServer::disconnectHandler(DisconnectPromise &data)
{
	logger.log("DisconnectHandler attempted");

	logger.log(millis());
	logger.log(data.terminate_after);

	if (millis() >= data.terminate_after || is_overloaded) {
		auto client = EthernetClient(data.clientSocket);

		logger.log("Disconnecting from " + ip_to_string(client.remoteIP()));
		client.stop();

		return Event::REMOVE;
	}

	return Event::RERUN;
}

/* private static */ HTTPServer::RequestProps HTTPServer::extractRequestProps(const char *requestLine, size_t len)
{
	const auto firstSpace = Str::find(requestLine, len, ' ');
	const auto secondSpace = Str::find(requestLine, len, ' ', firstSpace + 1);

	if (firstSpace == Str::NOT_FOUND || secondSpace == Str::NOT_FOUND)
		return RequestProps{Method::m_unknown, String()};

	const size_t method_index = Str::compareToMap(requestLine, firstSpace, ACCEPTED_METHODS, Method::m_size);

	if (method_index == Str::NOT_FOUND)
		return RequestProps{Method::m_unknown, String()};

	return RequestProps{
	    (Method)(uint8_t)method_index,
	    Str::fromBuffer(requestLine, firstSpace + 1, secondSpace)};
}

/* private static */ HTTPServer::HeaderPair HTTPServer::extractHeader(const char *requestLine, size_t len)
{
	const auto colon = Str::find(requestLine, len, ':');
	if (colon == Str::NOT_FOUND)
		return HeaderPair{Header::h_unknown, String()};

	const size_t map_index = Str::compareToMap(requestLine, colon, ACCEPTED_HEADERS, Header::h_size);

	if (map_index == Str::NOT_FOUND)
		return HeaderPair{Header::h_unknown, String()};

	const size_t first_non_whitespace = Str::findFirstNotOf(requestLine, len, Str::WHITESPACE, Str::WHITESPACE_LEN, colon + 1);
	if (first_non_whitespace == Str::NOT_FOUND)
		return HeaderPair{Header::h_unknown, String()};

	return HeaderPair{
	    (Header)(uint8_t)map_index,
	    Str::fromBuffer(requestLine, first_non_whitespace, len)};
}

HTTPServer::HTTPServer() : m_server(EK_SERVER_PORT), m_handlers(), m_handlerStorage{}
{
	m_handlers.setStorage<EK_HANDLER_STORAGE>(m_handlerStorage);
}

void HTTPServer::on(Method method, const char *path, uint8_t behavior, HTTPRequestHandlerPtr handler)
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
	m_serverQueue.execute(4);

	// listen for incoming clients
	EthernetClient client = m_server.available();

	if (client) {
		char buffer[512];
		size_t buffer_saturation = 0;
		bool props_parsed = false;
		auto props = RequestProps{};

		HeaderPair headerStorage[Header::h_size] = {};
		auto headers = Vector<HeaderPair>();
		headers.setStorage<Header::h_size>(headerStorage);

		logger.log("Incoming connection from " + ip_to_string(client.remoteIP()));

		while (client.connected()) {
			if (client.available()) {
				size_t bytes_read = client.readBytes(buffer + buffer_saturation, sizeof(buffer) - buffer_saturation);

				if (!props_parsed) {
					props_parsed = true;

					const auto headerEnd = Str::find(buffer, bytes_read, '\n');
					if (headerEnd == Str::NOT_FOUND) {
						logger.error("Invalid request from " + ip_to_string(client.remoteIP()) + "; URI is too long");
						writeStaticHTMLResponse(HTTPResponse::HTML_BAD_REQUEST, client);
						break;
					}

					props = extractRequestProps(buffer, headerEnd);
					memmove(buffer, buffer + headerEnd + 1, bytes_read - headerEnd - 1);

					bytes_read -= headerEnd + 1;
				}

				size_t offset = 0;
				size_t new_line_char = Str::find(buffer, bytes_read, '\n');

				while (new_line_char != Str::NOT_FOUND && new_line_char != 0) {
					const size_t extraction_length = buffer[new_line_char - 1] == '\r' ? new_line_char - 1 : new_line_char;
					const HeaderPair header = extractHeader(buffer + offset, extraction_length - offset);

					if (header.name != Header::h_unknown)
						headers.push_back(header);

					offset = new_line_char + 1;
					new_line_char = Str::find(buffer, bytes_read, '\n', offset);
				}

				if (bytes_read < 512) {
					parseRequest(props, headers, client);
					break;
				}
				else {
					const auto last_newline_in_buffer = Str::findLast(buffer, bytes_read, '\n');

					if (last_newline_in_buffer == 0 || last_newline_in_buffer == Str::NOT_FOUND) {
						logger.error("Invalid request from " + ip_to_string(client.remoteIP()) + "; line over 512 chars");
						writeStaticHTMLResponse(HTTPResponse::HTML_BAD_REQUEST, client);
						break;
					}

					buffer_saturation = bytes_read - last_newline_in_buffer;

					memmove(buffer, buffer + last_newline_in_buffer, buffer_saturation);
				}
			}
		}

		const auto disconnect_at = millis() + 1000;

		if (!m_serverQueue.tryEnqueue(DisconnectPromise{
		                                  client.getSocketNumber(),
		                                  disconnect_at},
		                              disconnectHandler)) {
			logger.warning("Server overloaded");

			is_overloaded = true;

			m_serverQueue.forceEnqueue(
			    DisconnectPromise{client.getSocketNumber(), disconnect_at},
			    disconnectHandler);

			is_overloaded = false;
		}
	}
}
/* static */ void HTTPServer::writeHTTPHeaders(uint16_t status_code, const char *status_message, const char *content_type, EthernetClient &client)
{
	client.print("HTTP/1.1 ");
	client.print(status_code);
	client.print(' ');
	client.println(status_message);

	client.print("Content-Type: ");
	client.println(content_type);

	client.println("Connection: close");
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
