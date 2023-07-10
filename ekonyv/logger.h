#ifndef EKONYV_LOGGER_H
#define EKONYV_LOGGER_H

class Logger {
private:
	const char *const m_label;

private:
	template <typename T>
	void print(const char *severity, T text) const
	{
		if (!Serial)
			return;

		Serial.print(severity);
		Serial.print(" [");
		Serial.print(m_label);
		Serial.print("]: ");
		Serial.println(text);
	}

public:
	Logger(const char *label) : m_label(label) {}

	template <typename T>
	void log(T text) const
	{
		print("LOG", text);
	}

	template <typename T>
	void warning(T text) const
	{
		print("WRN", text);
	}

	template <typename T>
	void error(T text) const
	{
		print("ERR", text);
	}
};

#endif // !defined(EKONYV_LOGGER_H)