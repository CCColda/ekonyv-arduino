#include "lcdstate.h"

#include "../global/global.h"

namespace LCDState {
void update()
{
#if EK_LCD
	if (global::lcd.flag == LCD::STATE_ERROR) {
		// do nothing.
	}
	else if (!global::is_running) {
		global::lcd.flag = LCD::STATE_HALTING;
	}
	else if (!global::sd.connected()) {
		global::lcd.flag = LCD::STATE_STARTING_SD;
	}
	else if (global::network.getMode() == Network::Mode::DISCONNECTED) {
		global::lcd.flag = LCD::STATE_STARTING_ETH;
	}
	else if (global::db.reg_req.active) {
		global::lcd.flag = LCD::STATE_DISPLAY_CODE;
		const auto code = global::db.reg_req.code;
		memcpy(global::lcd.state.code.data, &code, sizeof(global::lcd.state.code.data));
	}
	else {
#if EK_ETHERNET
		if (global::network.getMode() != Network::DISCONNECTED) {
			global::lcd.flag = LCD::STATE_SERVER_RUNNING;
			global::lcd.state.ipv4 = global::network.getIP();
		}
		else {
			global::lcd.flag = LCD::STATE_RUNNING;
		}
#else
		global::lcd.flag = LCD::STATE_RUNNING;
#endif
	}

	global::lcd.update();
#endif
}

void setError(const char* error) {
#if EK_LCD
	global::lcd.flag = LCD::STATE_ERROR;
	global::lcd.state.error = FixedBuffer<17>();

	const auto error_len = strlen(error);

	uint8_t i = 0;
	for (i = 0; i < min(16, error_len); i++) {
		global::lcd.state.error.data[i] = error[i];
	}

	global::lcd.state.error.data[i] = '\0';
#endif
}
} // namespace LCDState