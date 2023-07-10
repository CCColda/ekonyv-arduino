#ifndef EKONYV_NETWORK_H
#define EKONYV_NETWORK_H

#define EK_ETHERNET_COMM_PIN 5

namespace NetworkConstants {

} // namespace NetworkConstants

class Network {
private:
	static String ipToString(const IPAddress &address);
	static String byteToString(byte b);
	static String macToString(const byte *mac);
	static bool checkAndLogHardwareErrors();
	static void logNetworkInfo();

	static Logger logger;

private:
	enum Mode : byte {
		DISCONNECTED,
		USING_DHCP,
		USING_STATIC
	};

	Mode m_mode;

public:
	static const byte MAC_ADDRESS[6];
	static const byte STATIC_IP_ADDRESS[4];

public:
	Network();
	bool tryConnectUsingDHCP();
	bool connect();
	int maintain();

	IPAddress getIP() const;
};

#endif // !defined(EKONYV_NETWORK_H)