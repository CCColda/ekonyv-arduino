#include "serial.h"
#include "../config.h"

#include "../arduino/utility.h"

#include "../global/global.h"

#include <Arduino.h>

namespace SerialCommands {
void update()
{
	if (Serial.available() == 0)
		return;

	String command = Serial.readString();
	command.trim();
	command.toLowerCase();

	if (command == "halt") {
		global::db.save(global::time());
		Utility::halt("Halt command received");
	}
	else if (command.startsWith("dumpfh")) {
		const auto space = command.indexOf(' ') + 1;
		if (space > 0) {
			const auto path = command.substring(space);

			auto file = SD.open(path, FILE_READ);
			if (file) {
				int byte_n = 0;
				while (file.available()) {
					const auto byte = file.read();
					Serial.print((byte < 0x10 ? "0" : "") + String(byte, 0x10u));
					if (byte_n > 16) {
						Serial.println();
						byte_n = 0;
					}
					else {
						byte_n++;
					}
				}
				Serial.println();
				Serial.println("---- dumped \"" + path + "\" ----");
			}
			file.close();
		}
	}
	else if (command.startsWith("dumpf")) {
		const auto space = command.indexOf(' ') + 1;
		if (space > 0) {
			const auto path = command.substring(space);

			auto file = SD.open(path, FILE_READ);
			if (file) {
				while (file.available())
					Serial.write(file.read());
				Serial.println();
				Serial.println("---- dumped \"" + path + "\" ----");
			}
			file.close();
		}
	}
}
} // namespace SerialCommands