#define HOST_NAME "ELib"

#include "httpserver.h"
#include "logger.h"
#include "network.h"
#include "storage.h"
#include "utility.h"

#include "to_string.h"

auto network = Network(5);
auto sd = Storage(SDCARD_SS_PIN);
auto server = HTTPServer();

auto logger = Logger("MAIN");

int testHandler(const String &path, const Vector<HTTPServer::HeaderPair> &headers, EthernetClient &client)
{
	HTTPServer::sendStaticHTMLResponse(
	    HTTPResponse::StaticHTMLResponse{
	        200,
	        "OK",
	        "Szia!",
	        ("Végre működik a HTTP... <code>" + string_to_html_escaped_string(path) + "</code>").c_str()},
	    client);
}

void setup()
{
	Serial.begin(9600);
	while (!Serial)
		delay(1);

	/*
	if (!sd.init()) {
	    Utility::halt("Failed initializing SD card");
	}

	logger.log(String("SD Card connected; ") + Storage::infoToString(sd.getInfo()));
	*/

	if (!network.tryConnectUsingDHCP()) {
		logger.warning("DHCP setup failed; falling back to static IP");

		if (!network.connect()) {
			Utility::halt("Failed connecting using static IP.");
		}
	}

	server.start();

	server.on(HTTPServer::GET, "/hello", HTTPServer::HandlerBehavior::ALLOW_PARAMETERS | HTTPServer::HandlerBehavior::ALLOW_SUBPATHS, testHandler);
}

void loop()
{
	network.maintain();
	server.update();
	delayMicroseconds(500000);
}