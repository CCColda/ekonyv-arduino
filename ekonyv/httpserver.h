#ifndef EKONYV_SERVER_H
#define EKONYV_SERVER_H

#include <Ethernet.h>

#define EK_SERVER_PORT 80

class HTTPServer {
private:
	EthernetServer m_server;

public:
	HTTPServer();

	void start();
	void update();
};

#endif // !defined(EKONYV_SERVER_H)