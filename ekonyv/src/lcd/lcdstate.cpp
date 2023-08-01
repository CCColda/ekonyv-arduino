#include "lcdstate.h"

#include "../global/global.h"

namespace LCDState {
void update()
{
#if EK_LCD
	if (global::db.reg_req.active) {
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
} // namespace LCDState