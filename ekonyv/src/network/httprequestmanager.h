#ifndef EKONYV_HTTPREQUEST_MANAGER_H
#define EKONYV_HTTPREQUEST_MANAGER_H

#include "../config.h"

#include "httprequest.h"
#include <Vector.h>

class HTTPRequestManager {
private:
	HTTPRequest m_request_buf[EK_HTTP_REQUEST_STORAGE];
	Vector<HTTPRequest> m_requests;

public:
	HTTPRequestManager();
	HTTPRequestManager(HTTPRequestManager &&other);
	~HTTPRequestManager();

	bool tryAdd(const HTTPRequest &request);
	void update();
};

#endif // !defined(EKONYV_HTTPREQUEST_MANAGER_H)