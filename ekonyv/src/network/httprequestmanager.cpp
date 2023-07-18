#include "httprequestmanager.h"

#include "httpserverparser.h"
#include <CircularBuffer.h>

HTTPRequestManager::HTTPRequestManager() : m_request_buf(), m_requests()
{
	m_requests.setStorage<EK_HTTP_REQUEST_STORAGE>(m_request_buf);
}

HTTPRequestManager::HTTPRequestManager(HTTPRequestManager &&other) : m_request_buf(), m_requests()
{
	for (uint32_t i = 0; i < other.m_requests.size(); ++i)
		m_requests.push_back(other.m_requests[i]);
}

HTTPRequestManager::~HTTPRequestManager()
{
}

bool HTTPRequestManager::tryAdd(const HTTPRequest &request)
{
	if (m_requests.full())
		return false;

	m_requests.push_back(request);
}

void HTTPRequestManager::update()
{
	CircularBuffer<uint32_t, EK_HTTP_REQUEST_STORAGE> indices_to_remove;

	for (uint32_t i = 0; i < m_requests.size(); ++i) {
		auto &request = m_requests.at(i);
		HTTPServerParser parser;
		parser.body_parse_cb = request.body_parse_cb;
		parser.header_parse_cb = request.header_response_cb;

		while (request.client.available()) {
			bool should_break = false;
			switch (parser.parseBlock(request.client)) {
				case HTTP::SUCCESS: {
					should_break = true;
					break;
				}
				case HTTP::FAIL: {
					should_break = true;
					break;
				}
				default: {
					break;
				}
			}

			if (should_break) {
				indices_to_remove.push(i);
				break;
			}
		}
	}

	while (!indices_to_remove.isEmpty()) {
		const auto i = indices_to_remove.pop();

		if (m_requests[i].client.connected())
			m_requests[i].client.stop();

		m_requests[i].end_cb.call();

		m_requests.remove(i);
	}
}
