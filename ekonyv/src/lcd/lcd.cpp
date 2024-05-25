#include "lcd.h"

#include "../string/to_string.h"

#include "../global/global.h"

namespace {
String timeToHHMMSS(unsigned long time_seconds)
{
	const auto seconds = time_seconds % 60;
	const auto minutes = (time_seconds / 60) % 60;
	const auto hours = (time_seconds / 3600) % 24;

	return str(
	    number_to_padded_string(hours, 2), ':',
	    number_to_padded_string(minutes, 2), ':',
	    number_to_padded_string(seconds, 2));
}

constexpr uint8_t calculateByteDecimalLength(uint8_t byte)
{
	return byte >= 100 ? 3 : (byte >= 10 ? 2 : 1);
}

constexpr uint8_t calculateIPStringLength(uint32_t address)
{
	return calculateByteDecimalLength(address & 0xFF) +
	       calculateByteDecimalLength((address >> 8) & 0xFF) +
	       calculateByteDecimalLength((address >> 16) & 0xFF) +
	       calculateByteDecimalLength((address >> 24) & 0xFF) + 3;
}
} // namespace

LCD::LCD(
    pin_t rs, pin_t enable,
    pin_t d4, pin_t d5, pin_t d6, pin_t d7)
    : lcd(rs, enable, d4, d5, d6, d7)
{
	lcd.begin(16, 2);
}

void LCD::update()
{
	if (old_flag != flag)
		lcd.clear();

	old_flag = flag;

	bool print_time = true;

	switch (flag) {
		case STATE_HALTING: {
			lcd.setCursor(1, 0);
			lcd.print("EKonyv: " EK_VERSION);
			lcd.setCursor(0, 2);
			lcd.print("Gombnyomasra var");
			print_time = false;
			break;
		}
		case STATE_STARTING_SD: {
			lcd.setCursor(1, 0);
			lcd.print("EKonyv: " EK_VERSION);
			lcd.setCursor(0, 1);
			lcd.print("SD inditasa...");
			print_time = false;
			break;
		}
		case STATE_STARTING_ETH: {
			lcd.setCursor(1, 0);
			lcd.print("EKonyv: " EK_VERSION);
			lcd.setCursor(0, 1);
			lcd.print("Csatlakozas...");
			print_time = false;
			break;
		}
		case STATE_RUNNING: {
			lcd.setCursor(1, 0);
			lcd.print("EKonyv: " EK_VERSION);
			break;
		}
		case STATE_SERVER_RUNNING: {
			lcd.setCursor(0, 0);

			if (calculateIPStringLength(uint32_t(state.ipv4)) <= 13)
				lcd.print("IP:");

			lcd.print(IPAddress(state.ipv4));
			break;
		}
		case STATE_DISPLAY_CODE: {
			lcd.setCursor(3, 0);
			lcd.print("Kod: ");
			for (uint8_t i = 0; i < 4; ++i) {
				lcd.print((char)state.code.data[i]);
			}

			break;
		}
		case STATE_ERROR: {
			lcd.setCursor(0, 0);
			lcd.print("Futasideju hiba!");
			lcd.setCursor(0, 1);
			lcd.print((char*)state.error.data);
			print_time = false;

			break;
		}
		default: {
			break;
		}
	}

	if (print_time) {
		lcd.setCursor(4, 1);
		lcd.print(timeToHHMMSS(global::time()));
	}
}