#include "network.h"

#include "logger.h"

#include <Ethernet.h>

/* private static */ Logger Network::logger = Logger("NETW");

/* static */ const byte Network::MAC_ADDRESS[6] = {
    0b00000010, 0xE1, 0x1B, 0x00, 0x00, 0x01};

/* static */ const byte Network::STATIC_IP_ADDRESS[4] = {
    192, 168, 0, 78};

/* private static */ String Network::ipToString(const IPAddress &address)
{
	String result = String((unsigned int)address[0], 10);
	for (int i = 1; i < 4; ++i) {
		result += String('.');
		result += String((unsigned int)address[i], 10);
	}
	return result;
}

/* private static */ String Network::byteToString(byte b)
{
	return b < 0x10 ? String('0') + String((unsigned int)b, 0x10)
	                : String((unsigned int)b, 0x10);
}

/* private static */ String Network::macToString(const byte *mac)
{
	String result = byteToString(mac[0]);
	for (int i = 1; i < 6; ++i) {
		result += String(':');

		result += byteToString(mac[i]);
	}

	return result;
}

/* private static */ bool Network::checkAndLogHardwareErrors()
{
	if (Ethernet.hardwareStatus() == EthernetNoHardware) {
		logger.error("Failed initializing ethernet: MKR shield hardware was not found.");
		return false;
	}
	else if (Ethernet.linkStatus() == LinkOFF) {
		logger.error("Failed initializing ethernet: Not connected to ethernet via cable.");
		return false;
	}

	return true;
}

/* private static */ void Network::logNetworkInfo()
{
	logger.log(String("IP Address: ") + ipToString(Ethernet.localIP()));

	byte mac[6] = {};
	Ethernet.MACAddress(mac);

	logger.log(String("MAC address: ") + macToString(mac));
}

Network::Network() : m_mode(DISCONNECTED)
{
	Ethernet.init(EK_ETHERNET_COMM_PIN);
}

bool Network::tryConnectUsingDHCP()
{
	if (!Ethernet.begin((byte *)Network::MAC_ADDRESS)) {
		if (checkAndLogHardwareErrors()) { // if the error wasn't a hardware one
			logger.error("Failed initializing ethernet: Failed DHCP request.");
		}

		return false;
	}

	m_mode = USING_DHCP;

	logNetworkInfo();

	return true;
}

bool Network::connect()
{
	Ethernet.begin((byte *)Network::MAC_ADDRESS, IPAddress(Network::STATIC_IP_ADDRESS));

	if (!checkAndLogHardwareErrors()) {
		return false;
	}

	m_mode = USING_STATIC;

	logNetworkInfo();

	return true;
}

int Network::maintain()
{
	if (m_mode != USING_DHCP)
		return 0;

	switch (Ethernet.maintain()) {
		case 1: {
			logger.warning("Failed to renew IP.");
			break;
		}

		case 2: {
			logger.log("Renewed IP.");
			logNetworkInfo();

			break;
		}

		case 3: {
			logger.error("Failed rebinding IP.");

			break;
		}

		case 4: {
			logger.log("Successfully rebound IP");
			logNetworkInfo();

			break;
		}

		default:
			break;
	}
}