#ifndef EKONYV_NETWORK_H
#define EKONYV_NETWORK_H

#include "logger.h"

class Network {
public:
	enum Mode : uint8_t {
		DISCONNECTED,
		USING_DHCP,
		USING_STATIC,
		m_size
	};

private:
	static bool checkAndLogHardwareErrors();
	static void logNetworkInfo();

private:
	static Logger logger;

private:
	Mode m_mode;

public:
	static const byte MAC_ADDRESS[6];
	static const byte STATIC_IP_ADDRESS[4];

public:
	Network(uint8_t pin);
	bool tryConnectUsingDHCP();
	bool connect();
	int maintain();

	Mode getMode() const;
	IPAddress getIP() const;
};

#endif // !defined(EKONYV_NETWORK_H)