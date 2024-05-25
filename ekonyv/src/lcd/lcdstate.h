#ifndef EKONYV_LCD_STATE_H
#define EKONYV_LCD_STATE_H

namespace LCDState {
//! @brief Updates the LCD display state based on the global state.
void update();

//! @brief Sets the state of the LCD display to an error.
void setError(const char* error);
}

#endif // !defined(EKONYV_LCD_STATE_H)