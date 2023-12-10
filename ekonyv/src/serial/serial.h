/**
 * @file
 * @brief Serial commands.
 */

#ifndef EKONYV_SERIAL_H
#define EKONYV_SERIAL_H

namespace SerialCommands {
/**
 * @brief Handles textual commands coming from the serial connection.
 * Available commands are:
 * - rmf - removes a file
 * - dumpf - dumps a file in ascii to serial
 * - dumpfh - dumps a file in hexadecimal to serial
 * - halt - saves the databases and stops
 * @remark Does not block if the serial buffer is empty.
 */
void update();
} // namespace SerialCommands

#endif // !defined(EKONYV_SERIAL_H)