/**
 * @file
 * @brief Implementation of serial commands.
 */

#include "serial.h"

#include "../arduino/utility.h"
#include "../config.h"
#include "../global/global.h"
#include "../string/to_string.h"

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
	else if (command.startsWith("rmf")) {
		const auto space = command.indexOf(' ') + 1;
		if (space > 0) {
			const auto path = command.substring(space);

			if (SD.remove(path))
				Serial.println("Removed \"" + path + "\"");
			else
				Serial.println("Failed to remove \"" + path + "\"");
		}
	}
	else if (command.startsWith("dumpfh")) {
		const auto space = command.indexOf(' ') + 1;
		if (space > 0) {
			const auto path = command.substring(space);

			auto file = SD.open(path, FILE_READ);
			if (file) {
				while (file.available()) {
					byte line_buffer[16];

					const auto bytes_read = file.readBytes(line_buffer, 16);

					for (size_t i = 0; i < bytes_read; i++) {
						Serial.print(byte_to_string(line_buffer[i]));
						Serial.write(' ');
					}

					Serial.println();
				}
				Serial.println("---- dumped \"" + path + "\" ----");
			}
			else {
				Serial.println("Failed to open file.\n");
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
			else {
				Serial.println("Failed to open file.\n");
			}

			file.close();
		}
	}
	else {
		Serial.println("Available commands:\n"
		               "halt            - save and stop execution\n"
		               "rmf <path>      - remove file\n"
		               "dumpf[h] <path> - dump a file to serial (hexadecimal format if 'h' is specified)\n");
	}
}
} // namespace SerialCommands