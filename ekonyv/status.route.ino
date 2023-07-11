#include "status.route.h"

#include "global.h"

static const char *NETWORK_MODES[Network::Mode::m_size] = {
    "disconnected",
    "dhcp",
    "static"};

int StatusRoute::handler(const String &path, const Vector<HTTPServer::HeaderPair> &headers, EthernetClient &client)
{
	HTTPServer::writeHTTPHeaders(200, "OK", "text/csv", client);

	client.println("key,value");
	client.print("IP,");
	client.println(ip_to_string(global::network.getIP()));
	client.print("MAC,");
	client.println(mac_to_string(Network::MAC_ADDRESS));
	client.print("DHCP,");
	client.println(NETWORK_MODES[global::network.getMode()]);

	if (global::sd.connected()) {
		client.println("SD,connected");
		client.println(Storage::infoToCSV(global::sd.getInfo()));
	}
	else {
		client.println("SD,disconnected");
	}
}

void StatusRoute::registerRoute(HTTPServer &server)
{
	server.on(HTTPServer::GET, "/status", HTTPServer::HandlerBehavior::NONE, handler);
}