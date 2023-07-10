#define HOST_NAME "ELib"

#include "httpserver.h"
#include "logger.h"
#include "network.h"
#include "utility.h"

auto network = Network();
auto server = HTTPServer();

auto logger = Logger("MAIN");

void setup()
{
	Serial.begin(9600);
	while (!Serial)
		;

	if (!network.tryConnectUsingDHCP()) {
		logger.warning("DHCP setup failed; falling back to static IP");

		if (!network.connect()) {
			Utility::halt("Failed connecting using static IP.");
		}
	}

	server.start();
}

void loop()
{
	network.maintain();
	server.update();
}