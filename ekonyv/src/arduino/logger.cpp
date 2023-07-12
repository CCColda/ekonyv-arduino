#include "logger.h"

void Logger::printLogHead(const char *severity) const
{
	Serial.print(severity);
	Serial.print(" [");
	Serial.print(m_label);
	Serial.print("]: ");
}

Logger::Logger(const char *label) : m_label(label) {}