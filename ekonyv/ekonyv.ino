#include "src/config.h"

#include "src/arduino/logger.h"
#include "src/string/to_string.h"

#include "src/arduino/utility.h"

#include "src/routes/route.book.h"
#include "src/routes/route.ekonyv.h"
#include "src/routes/route.login.h"
#include "src/routes/route.register.h"
#include "src/routes/route.session.h"
#include "src/routes/route.status.h"
#include "src/routes/route.storage.h"

#include "src/global/global.h"

#include "src/lcd/lcdstate.h"

#include "src/serial/serial.h"

auto logger = Logger("MAIN");
bool startstopbuttondown = false;

void setup()
{
	pinMode(EK_HANGING_ANALOG_PIN, INPUT);
	pinMode(EK_STARTSTOP_BUTTON_PIN, INPUT);

	pinMode(EK_LED_RUNNING_PIN, OUTPUT);
	pinMode(EK_LED_ERROR_PIN, OUTPUT);

#if EK_SERIAL
	Serial.begin(9600);

#if EK_SERIAL_INPUT_FOR_START
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

#endif
#endif

	Serial.println("Starting...");

#if EK_LCD
	Serial.println("Starting LCD...");
	LCDState::update();
#endif
	while(!global::is_running) {
		if (digitalRead(EK_STARTSTOP_BUTTON_PIN) == HIGH) {
			global::is_running = true;
			startstopbuttondown = true;
			logger.log("Button pressed, starting...");
		}
		else {
			Serial.println("Waiting for start/stop...");
			delay(500);
		}

#if EK_LCD
		LCDState::update();
#endif
	}

	digitalWrite(EK_LED_RUNNING_PIN, HIGH);

	if (!global::sd.init()) {
		// logger.error("Failed initializing SD card");
		Utility::halt("Failed initializing SD card", "SD_INIT_FAIL");
	}
	else {
		logger.log("SD Card connected; ", Storage::infoToString(global::sd.getInfo()));
	}

//! Don't run on MockSD
#if EK_SD
	if (!SD.exists(EK_DB_ROOT_PATH)) {
		logger.log("Creating " EK_DB_ROOT_PATH " on SD card...");

		if (!SD.mkdir(EK_DB_ROOT_PATH)) {
			logger.warning("Failed creating directories.");
		}
	}
#endif

#if EK_LCD
	LCDState::update();
#endif

#if EK_ETHERNET
	if (!global::network.tryConnectUsingDHCP()) {
		logger.warning("DHCP setup failed; falling back to static IP");

		if (!global::network.connect()) {
			Utility::halt("Failed connecting using static IP.", "STATIC_IP_FAIL");
		}
	}

	global::server.start();

	global::db.load();

	global::ntp.begin();

	if (!global::ntp.forceUpdate()) {
		logger.warning("NTP update failed");
	}
	else {
		logger.log("Time is: ", global::ntp.getFormattedTime(), " (from server " EK_NTP_IP " with " __EK_MACRO_STRING(EK_NTP_OFFSET_S) "s offset; no correction for DST)");
	}

	EKonyvRoute::registerRoute(global::server);
	StatusRoute::registerRoute(global::server);
	RegisterRoute::registerRoute(global::server);
	LoginRoute::registerRoute(global::server);
	SessionRoute::registerRoute(global::server);
	BookRoute::registerRoute(global::server);
	StorageRoute::registerRoute(global::server);
#endif

	LCDState::update();
}

void loop()
{
	if (global::is_running) {
	#if EK_SERIAL
		SerialCommands::update();
	#endif

	#if EK_ETHERNET
	
		global::network.maintain();
		global::ntp.update();
		global::server.update();
		// global::requests.update();
	
	#endif

		global::db.update(global::time());

		// global::eventqueue.execute(5);
	}

	if (digitalRead(EK_STARTSTOP_BUTTON_PIN) == LOW && startstopbuttondown) {
		startstopbuttondown = false;

		if (global::is_running) {
			global::db.save(global::time());
			global::is_running = false;
			digitalWrite(EK_LED_RUNNING_PIN, LOW);
		}
		else {
			global::is_running = true;
			digitalWrite(EK_LED_RUNNING_PIN, HIGH);
		}
	}
	else if (digitalRead(EK_STARTSTOP_BUTTON_PIN) == HIGH) {
		startstopbuttondown = true;
	}

	#if EK_LCD
		LCDState::update();
	#endif

	delayMicroseconds(500000);
}