#include "httpserverparser.h"

#include "../string/string.h"
#include "../string/to_string.h"

/* private */ Logger HTTPServerParser::logger = Logger("HTCP");

/* private static */ HTTP::ServerResponseProps HTTPServerParser::extractResponseProps(const SizedString &request_line)
{
	const auto firstSpace = Str::find(request_line, ' ');

	if (firstSpace == Str::NOT_FOUND)
		return HTTP::ServerResponseProps{400};

	const auto secondSpace = Str::find(request_line, ' ', firstSpace + 1);

	if (secondSpace == Str::NOT_FOUND)
		return HTTP::ServerResponseProps{400};

	return HTTP::ServerResponseProps{
	    Str::fixedAtoi<index_t>(SizedString{request_line.ptr + firstSpace + 1, secondSpace - firstSpace - 1})};
}

/* private static */ HTTP::ServerHeaderPair HTTPServerParser::extractHeader(const SizedString &request_line)
{
	const auto colon = Str::find(request_line, ':');

	if (colon == Str::NOT_FOUND)
		return HTTP::ServerHeaderPair{HTTP::ServerHeader::sh_unknown, String()};

	const size_t map_index = Str::compareToMap(SizedString{request_line.ptr, colon}, HTTP::SERVER_HEADERS, HTTP::ServerHeader::sh_size);

	if (map_index == Str::NOT_FOUND)
		return HTTP::ServerHeaderPair{HTTP::ServerHeader::sh_unknown, String()};

	const size_t first_non_whitespace = Str::findFirstNotOf(request_line, Str::WHITESPACE, colon + 1);

	if (first_non_whitespace == Str::NOT_FOUND)
		return HTTP::ServerHeaderPair{HTTP::ServerHeader::sh_unknown, String()};

	return HTTP::ServerHeaderPair{
	    (HTTP::ServerHeader)(uint8_t)map_index,
	    Str::fromBuffer(request_line.ptr, first_non_whitespace, request_line.len)};
}

HTTPServerParser::HTTPServerParser()
    : m_buffer(), m_buffer_saturation(0),
      header_parse_cb(), body_parse_cb(), flags{0, 0},
      props(), m_header_buf(), headers()
{
	headers.setStorage<HTTP::ServerHeader::sh_size>(m_header_buf);
}

HTTPServerParser::~HTTPServerParser()
{
}

HTTP::ParseResult HTTPServerParser::parseBlock(EthernetClient &client)
{
	const index_t bytes_read = client.readBytes(
	    m_buffer + m_buffer_saturation,
	    EK_HTTP_BUFFER_SIZE - m_buffer_saturation);

	m_buffer_saturation = bytes_read;

	const auto buffer_as_string = SizedString{m_buffer, m_buffer_saturation};

	if (!flags.props_parsed) {
		flags.props_parsed = true;

		const auto props_end = Str::find(SizedString{m_buffer, bytes_read}, '\n');
		if (props_end == Str::NOT_FOUND) {
			logger.error("Invalid response from ", ip_to_string(client.remoteIP()), "; URI is too long");
			return HTTP::FAIL;
		}

		props = extractResponseProps(SizedString{m_buffer, props_end});
		memmove(m_buffer, m_buffer + props_end + 1, bytes_read - props_end - 1);

		m_buffer_saturation -= props_end + 1;
	}

	if (!flags.headers_parsed) {
		size_t new_line_char = Str::find(buffer_as_string, '\n');
		index_t offset = 0;
		while (new_line_char != Str::NOT_FOUND && new_line_char != 0) {
			const index_t extraction_length = m_buffer[new_line_char - 1] == '\r' ? new_line_char - 1 : new_line_char;

			if (extraction_length - offset == 0) {
				flags.headers_parsed = true;
				break;
			}

			const auto header = extractHeader(SizedString{m_buffer + offset, extraction_length - offset});

			if (header.name != HTTP::ServerHeader::sh_unknown)
				headers.push_back(header);

			offset = new_line_char + 1;
			new_line_char = Str::find(buffer_as_string, '\n', offset);
		}

		memmove(m_buffer, m_buffer + new_line_char + 1, m_buffer_saturation - new_line_char - 1);
		m_buffer_saturation -= new_line_char + 1;
	}

	if (flags.props_parsed && flags.headers_parsed) {
		if (header_parse_cb && header_parse_cb.call_count == 0) {
			const auto result = header_parse_cb.call(props.status_code, headers);

			if (result != HTTP::CONTINUE)
				return result;
		}

		if (body_parse_cb) {
			const size_t last_newline_in_buffer = Str::findLast(buffer_as_string, '\n');
			if (last_newline_in_buffer == 0 || last_newline_in_buffer == Str::NOT_FOUND || last_newline_in_buffer >= m_buffer_saturation) {
				logger.error("Invalid request from ", ip_to_string(client.remoteIP()), "; line over " __EK_MACRO_STRING(EK_HTTP_BUFFER_SIZE) " chars");
				return HTTP::FAIL;
			}

			const size_t adjusted_last_newline = m_buffer[last_newline_in_buffer - 1] == '\r' ? last_newline_in_buffer - 1 : last_newline_in_buffer;

			const auto result = body_parse_cb.call(m_buffer, adjusted_last_newline);
			if (result != HTTP::CONTINUE)
				return result;

			memmove(m_buffer, m_buffer + last_newline_in_buffer + 1, m_buffer_saturation - last_newline_in_buffer - 1);

			m_buffer_saturation -= last_newline_in_buffer + 1;
		}
	}

	if (bytes_read < EK_HTTP_BUFFER_SIZE)
		return HTTP::SUCCESS;

	return HTTP::CONTINUE;
}
