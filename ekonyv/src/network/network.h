/**
 * @file
 * @brief Definition of the class responsible for network availability.
 */

#ifndef EKONYV_NETWORK_H
#define EKONYV_NETWORK_H

#include "../arduino/logger.h"

//! @brief Class handling network connections.
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
	Mode m_mode;

	static Logger logger;

public:
	//! @brief The hardcoded MAC address of the device.
	//! @c 02:E1:1B:00:00:01
	static const byte MAC_ADDRESS[6];

	//! @brief The fallback IP address for the device, in case the DHCP connection fails.
	//! @c 192.168.0.78
	static const byte STATIC_IP_ADDRESS[4];

public:
	//! @brief Initializes the network interface
	//! @param pin The CS pin of the ETH shield.
	Network(uint8_t pin);

	//! @brief Tries to connect to the network using DHCP.
	//! @warning Blocking call.
	//! @returns true if the connection was successful.
	bool tryConnectUsingDHCP();

	//! @brief Connects to the network using @c STATIC_IP_ADDRESS .
	//! @returns true if the connection was successful.
	bool connect();

	//! @brief Maintains a DHCP connection.
	void maintain();

	//! @brief Returns the connection mode.
	Mode getMode() const;

	//! @brief Returns the current IP address.
	IPAddress getIP() const;
};

#endif // !defined(EKONYV_NETWORK_H)