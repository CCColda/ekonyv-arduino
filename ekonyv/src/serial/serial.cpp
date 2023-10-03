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
}
} // namespace Serial