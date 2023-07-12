#ifndef EKONYV_LOGGER_H
#define EKONYV_LOGGER_H

#include <Arduino.h>

#include "../config.h"

class Logger {
private:
	const char *const m_label;

private:
	void printLogHead(const char *severity) const;

public:
	Logger(const char *label);

	template <typename... T>
	void log(T... text) const
	{
		printLogHead("LOG");
		(Serial.print(text), ...);
		Serial.println();
	}

	template <typename... T>
	void warning(T... text) const
	{
		printLogHead("WRN");
		(Serial.print(text), ...);
		Serial.println();
	}

	template <typename... T>
	void error(T... text) const
	{
		printLogHead("ERR");
		(Serial.print(text), ...);
		Serial.println();
	}

#if EK_VERBOSE_LOGGING
	template <typename... T>
	void verbose(bool start, bool continuous, T... text) const
	{
		if (start)
			printLogHead("VBS");
		(Serial.print(text), ...);
		if (!continuous)
			Serial.println();
	}
#endif
};

#if EK_VERBOSE_LOGGING
#define VERBOSE_LOG(logger, args...) \
	logger.verbose(true, false, args)

//! @brief Begin a continuous verbose log message
#define VERBOSE_LOG_B(logger, args...) \
	logger.verbose(true, true, args)

//! @brief Continue continuous verbose log message
#define VERBOSE_LOG_C(logger, args...) \
	logger.verbose(false, true, args)

//! @brief End a continuous verbose log message
#define VERBOSE_LOG_E(logger) \
	logger.verbose(false, false, "")
#else
#define VERBOSE_LOG(logger, ...)
#define VERBOSE_LOG_B(logger, ...)
#define VERBOSE_LOG_C(logger, ...)
#define VERBOSE_LOG_E(logger)
#endif

#endif // !defined(EKONYV_LOGGER_H)