#define HOST_NAME "ELib"

#include "logger.h"
#include "to_string.h"

#include "utility.h"

#include "hello.route.h"
#include "status.route.h"

#include "global.h"

auto logger = Logger("MAIN");

void setup()
{
	Serial.begin(9600);
	while (!Serial)
		delay(1);

	if (!global::sd.init()) {
		logger.error("Failed initializing SD card");
	}
	else {
		logger.log(String("SD Card connected; ") + Storage::infoToString(global::sd.getInfo()));
	}

	if (!global::network.tryConnectUsingDHCP()) {
		logger.warning("DHCP setup failed; falling back to static IP");

		if (!global::network.connect()) {
			Utility::halt("Failed connecting using static IP.");
		}
	}

	global::server.start();

	HelloRoute::registerRoute(global::server);
	StatusRoute::registerRoute(global::server);
}

void loop()
{
	global::network.maintain();
	global::eventqueue.execute(5);
	global::server.update();
	delayMicroseconds(500000);
}