#include "src/config.h"

#include "src/arduino/logger.h"
#include "src/string/to_string.h"

#include "src/arduino/utility.h"

#include "src/routes/route.book.h"
#include "src/routes/route.hello.h"
#include "src/routes/route.login.h"
#include "src/routes/route.register.h"
#include "src/routes/route.status.h"
#include "src/routes/route.user.hello.h"

#include "src/global/global.h"

auto logger = Logger("MAIN");

void setup()
{
	pinMode(EK_HANGING_ANALOG_PIN, INPUT);

	Serial.begin(9600);
	while (!Serial)
		delay(5);

	Serial.println("Type \"go\" to start");
	String go_string = "";
	do {
		while (Serial.available() == 0)
			delay(1);
		go_string = Serial.readString();
		go_string.trim();
		go_string.toLowerCase();
	} while (go_string != "go");

	if (!global::sd.init()) {
		logger.error("Failed initializing SD card");
	}
	else {
		logger.log(String("SD Card connected; ") + Storage::infoToString(global::sd.getInfo()));
	}

#if EK_ETHERNET
	if (!global::network.tryConnectUsingDHCP()) {
		logger.warning("DHCP setup failed; falling back to static IP");

		if (!global::network.connect()) {
			Utility::halt("Failed connecting using static IP.");
		}
	}

	global::server.start();

	global::db.user.load();
	global::db.session.load();
	global::db.book.load();

	global::ntp.begin();

	if (!global::ntp.forceUpdate()) {
		logger.warning("NTP update failed");
	}
	else {
		logger.log("Time is: ", global::ntp.getFormattedTime(), " (from server " EK_NTP_IP " with " __EK_MACRO_STRING(EK_NTP_OFFSET_S) "s offset; no correction for DST)");
	}

	HelloRoute::registerRoute(global::server);
	StatusRoute::registerRoute(global::server);
	RegisterRoute::registerRoute(global::server);
	LoginRoute::registerRoute(global::server);
	UserHelloRoute::registerRoute(global::server);
	BookRoute::registerRoute(global::server);
#endif
}

void loop()
{
#if EK_ETHERNET
	global::network.maintain();
	global::ntp.update();
	global::server.update();
	global::requests.update();
#endif

	global::eventqueue.execute(5);
	delayMicroseconds(500000);
}