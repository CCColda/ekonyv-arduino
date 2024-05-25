#include "utility.h"
#include "../global/global.h"
#include "../lcd/lcdstate.h"

namespace Utility {
void halt(const char *reason, const char* reason_code)
{
	Logger(EK_UTIL_LOG_TAG).warning(String("Halting: ") + String(reason));

	digitalWrite(EK_LED_RUNNING_PIN, LOW);
	digitalWrite(EK_LED_ERROR_PIN, HIGH);

	#if EK_LCD
	LCDState::setError(reason_code);
	#endif

	while (true) {
		#if EK_LCD
		LCDState::update();
		#endif

		delay(1000);
	}
}
} // namespace Utility