#include "storage.h"

/* private static */ Logger Storage::logger = Logger("SDCR");

/* static */ String Storage::infoToString(const Storage::Info &info)
{
	const char *type = "<unknown>";

	switch (info.type) {
		case Info::SD1:
			type = "SD1";
			break;
		case Info::SD2:
			type = "SD2";
			break;
		case Info::SDHC:
			type = "SDHC";
			break;
		default:
			break;
	}

	return String("type = ") + String(type) +
	       String(", size (bytes) = ") + String((unsigned long)info.size_bytes, 10) +
	       String(", size (megabytes) = " + String((double)info.size_bytes / (1024.0 * 1024.0), 2)) +
	       String(", size (gigabytes) = " + String((double)info.size_bytes / (1024.0 * 1024.0 * 1024.0), 2));
}

/* static */ String Storage::infoToCSV(const Storage::Info &info)
{
	const char *type = "<unknown>";

	switch (info.type) {
		case Info::SD1:
			type = "SD1";
			break;
		case Info::SD2:
			type = "SD2";
			break;
		case Info::SDHC:
			type = "SDHC";
			break;
		default:
			break;
	}

	return String("SD_type,") + String(type) +
	       String("\nSD_size_b,") + String((unsigned long)info.size_bytes, 10) +
	       String("\nSD_size_mb," + String((double)info.size_bytes / (1024.0 * 1024.0), 2)) +
	       String("\nSD_size_gb," + String((double)info.size_bytes / (1024.0 * 1024.0 * 1024.0), 2));
}

Storage::Storage(uint8_t pin)
    : m_pin(pin), m_connected(false), m_card(), m_volume(), root()
{
}

bool Storage::init()
{
	if (!Serial) {
		logger.error("Failed initializing SD card: Serial is not initialized.");
		return false;
	}

	if (!m_card.init(SPI_QUARTER_SPEED, m_pin)) {
		logger.error("Failed initializing SD card; card not found. Make sure to insert it in the MKRZero slot instead of the ETH shield.");
		return false;
	}

	if (!m_volume.init(m_card)) {
		logger.error("Failed opening the volume on the card; make sure it is formatted.");
		return false;
	}

	if (!root.openRoot(m_volume)) {
		logger.error("Failed opening root folder.");
		return false;
	}

	if (!SD.begin(m_pin)) {
		logger.error("Failed initializing SD library.");
		return false;
	}

	m_connected = true;

	return true;
}

bool Storage::close()
{
	m_connected = false;
	root.close();
}

bool Storage::connected() const
{
	return m_connected;
}

Storage::Info Storage::getInfo() const
{
	return Info{
	    Info::Type(m_card.type()),
	    (uint64_t)m_volume.blocksPerCluster() * (uint64_t)m_volume.clusterCount() * (uint64_t)512u};
}
