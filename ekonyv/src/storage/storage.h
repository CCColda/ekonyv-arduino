#ifndef EKONYV_STORAGE_H
#define EKONYV_STORAGE_H

#if EK_SD
#include <SD.h>
#else
#include "../storage/mocksd.h"
#warning "Using mock SD library"
#endif

#include "../arduino/logger.h"

class Storage {
public:
	struct Info {
		enum Type : uint8_t {
			UNKNOWN = 0,
			SD1 = SD_CARD_TYPE_SD1,
			SD2 = SD_CARD_TYPE_SD2,
			SDHC = SD_CARD_TYPE_SDHC,
			MOCK = SD_CARD_TYPE_MOCK
		};

		Type type;
		uint64_t size_bytes;
	};

	static const char *typeToString(Info::Type type);
	static String infoToString(const Info &info);
	static String infoToCSV(const Info &info);

private:
	static Logger logger;

private:
	uint8_t m_pin;
	bool m_connected;

	Sd2Card m_card;
	SdVolume m_volume;

public:
	SdFile root;

public:
	Storage(uint8_t pin);

	bool init();
	bool close();

	bool connected() const;

	Info getInfo() const;
};

#endif // !defined(EKONYV_STORAGE_H)