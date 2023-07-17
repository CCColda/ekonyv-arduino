#include "httpclientparser.h"

#include "../string/string.h"
#include "../string/to_string.h"

/* private */ Logger HTTPClientParser::logger = Logger("HTCP");

/* private static */ HTTP::ClientRequestProps HTTPClientParser::extractRequestProps(const char *requestLine, index_t len)
{
	const auto firstSpace = Str::find(requestLine, len, ' ');
	const auto secondSpace = Str::find(requestLine, len, ' ', firstSpace + 1);

	if (firstSpace == Str::NOT_FOUND || secondSpace == Str::NOT_FOUND)
		return HTTP::ClientRequestProps{HTTP::Method::m_unknown, String()};

	const size_t method_index = Str::compareToMap(requestLine, firstSpace, HTTP::METHOD_MAP, HTTP::Method::m_size);

	if (method_index == Str::NOT_FOUND)
		return HTTP::ClientRequestProps{HTTP::Method::m_unknown, String()};

	return HTTP::ClientRequestProps{
	    (HTTP::Method)(uint8_t)method_index,
	    Str::fromBuffer(requestLine, firstSpace + 1, secondSpace)};
}

/* private static */ HTTP::ClientHeaderPair HTTPClientParser::extractHeader(const char *requestLine, index_t len)
{
	const auto colon = Str::find(requestLine, len, ':');
	if (colon == Str::NOT_FOUND)
		return HTTP::ClientHeaderPair{HTTP::ClientHeader::ch_unknown, String()};

	const size_t map_index = Str::compareToMap(requestLine, colon, HTTP::CLIENT_HEADERS, HTTP::ClientHeader::ch_size);

	if (map_index == Str::NOT_FOUND)
		return HTTP::ClientHeaderPair{HTTP::ClientHeader::ch_unknown, String()};

	const size_t first_non_whitespace = Str::findFirstNotOf(requestLine, len, Str::WHITESPACE, Str::WHITESPACE_LEN, colon + 1);
	if (first_non_whitespace == Str::NOT_FOUND)
		return HTTP::ClientHeaderPair{HTTP::ClientHeader::ch_unknown, String()};

	return HTTP::ClientHeaderPair{
	    (HTTP::ClientHeader)(uint8_t)map_index,
	    Str::fromBuffer(requestLine, first_non_whitespace, len)};
}

HTTPClientParser::HTTPClientParser()
    : m_buffer(), m_buffer_saturation(0),
      m_props_parsed(false), props(), m_header_buf(), headers()
{
	headers.setStorage<HTTP::ClientHeader::ch_size>(m_header_buf);
}

HTTP::ParseResult HTTPClientParser::parseBlock(EthernetClient &client)
{
	const index_t bytes_read = client.readBytes(
	    m_buffer + m_buffer_saturation,
	    EK_HTTP_BUFFER_SIZE - m_buffer_saturation);

	index_t bytes_in_buffer = bytes_read;

	if (!m_props_parsed) {
		m_props_parsed = true;

		const auto props_end = Str::find(m_buffer, bytes_read, '\n');
		if (props_end == Str::NOT_FOUND) {
			logger.error("Invalid request from ", ip_to_string(client.remoteIP()), "; URI is too long");
			return HTTP::FAIL;
		}

		props = extractRequestProps(m_buffer, props_end);
		memmove(m_buffer, m_buffer + props_end + 1, bytes_read - props_end - 1);

		bytes_in_buffer -= props_end + 1;
	}

	size_t new_line_char = Str::find(m_buffer, bytes_in_buffer, '\n');

	index_t offset = 0;
	while (new_line_char != Str::NOT_FOUND && new_line_char != 0) {
		const index_t extraction_length = m_buffer[new_line_char - 1] == '\r' ? new_line_char - 1 : new_line_char;
		const auto header = extractHeader(m_buffer + offset, extraction_length - offset);

		if (header.name != HTTP::ClientHeader::ch_unknown)
			headers.push_back(header);

		offset = new_line_char + 1;
		new_line_char = Str::find(m_buffer, bytes_in_buffer, '\n', offset);
	}

	if (bytes_read < EK_HTTP_BUFFER_SIZE)
		return HTTP::SUCCESS;

	const size_t last_newline_in_buffer = Str::findLast(m_buffer, bytes_in_buffer, '\n');

	if (last_newline_in_buffer == 0 || last_newline_in_buffer == Str::NOT_FOUND) {
		logger.error("Invalid request from ", ip_to_string(client.remoteIP()), "; line over " __EK_MACRO_STRING(EK_HTTP_BUFFER_SIZE) " chars");
		return HTTP::FAIL;
	}

	m_buffer_saturation = bytes_read - last_newline_in_buffer;

	memmove(m_buffer, m_buffer + last_newline_in_buffer, m_buffer_saturation);

	return HTTP::CONTINUE;
}
