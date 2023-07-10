#ifndef EKONYV_STORAGE_H
#define EKONYV_STORAGE_H

#include <SD.h>

#include "logger.h"

class Storage {
public:
	struct Info {
		enum Type : uint8_t {
			UNKNOWN = 0,
			SD1 = SD_CARD_TYPE_SD1,
			SD2 = SD_CARD_TYPE_SD2,
			SDHC = SD_CARD_TYPE_SDHC,
		};

		Type type;
		uint64_t size_bytes;
	};

	static String infoToString(const Info &info);

private:
	static Logger logger;

private:
	uint8_t m_pin;

	Sd2Card m_card;
	SdVolume m_volume;

public:
	SdFile root;

public:
	Storage(uint8_t pin);

	bool init();
	bool close();

	Info getInfo() const;
};

#endif // !defined(EKONYV_STORAGE_H)