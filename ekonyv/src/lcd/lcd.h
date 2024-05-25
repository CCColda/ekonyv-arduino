#ifndef EKONYV_LCD_H
#define EKONYV_LCD_H

#include "../types/fixedbuffer.h"
#include <LiquidCrystal.h>

//! @brief Manages the LCD display and its state.
class LCD {
public:
	LiquidCrystal lcd;

	enum StateFlag : uint8_t {
		STATE_HALTING,
		STATE_STARTING_SD,
		STATE_STARTING_ETH,
		STATE_RUNNING,
		STATE_SERVER_RUNNING,
		STATE_DISPLAY_CODE,
		STATE_ERROR
	};

	union State {
		FixedBuffer<4> code;
		uint32_t ipv4;
		FixedBuffer<17> error;
	};

	using pin_t = uint8_t;

	State state;
	StateFlag flag;

private:
	StateFlag old_flag;

public:
	LCD(
	    pin_t rs, pin_t enable,
	    pin_t d4, pin_t d5, pin_t d6, pin_t d7);

	void update();
};

#endif // !defined(EKONYV_LCD_H)