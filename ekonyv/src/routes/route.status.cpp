#include "route.status.h"

#include "../global/global.h"

#include "../string/to_string.h"

static const char *NETWORK_MODES[Network::Mode::m_size] = {
    "disconnected",
    "dhcp",
    "static"};

int StatusRoute::handler(const String &path, const Vector<HTTP::ClientHeaderPair> &headers, EthernetClient &client)
{
#if EK_ETHERNET
	HTTPServer::writeHTTPHeaders(200, "OK", "text/csv", client);

	client.println("key,value");
	client.println("NAME," EK_NAME);
	client.println("VERSION," EK_VERSION);
	client.print("IP,");
	client.println(ip_to_string(global::network.getIP()));
	client.print("MAC,");
	client.println(mac_to_string(Network::MAC_ADDRESS));
	client.print("DHCP,");
	client.println(NETWORK_MODES[global::network.getMode()]);
	client.print("TIME_STRING,");
	client.println(global::ntp.getFormattedTime());
	client.print("TIME_SINCE_EPOCH,");
	client.println(global::ntp.getEpochTime());
	client.print("TIME_SINCE_DEVICE_EPOCH,");
	client.println(millis() / 1000);

	if (global::sd.connected()) {
		client.println("SD,connected");
		client.println(Storage::infoToCSV(global::sd.getInfo()));
	}
	else {
		client.println("SD,disconnected");
	}
#endif
}

void StatusRoute::registerRoute(HTTPServer &server)
{
	server.on(HTTP::GET, "/status", HTTPServer::HandlerBehavior::NONE, handler);
}