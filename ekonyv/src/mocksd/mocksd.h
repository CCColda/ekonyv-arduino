#ifndef MOCKFILE_H
#define MOCKFILE_H

#include "../config.h"

#include "../arduino/logger.h"
#include <Vector.h>

#define FILE_WRITE 1
#define FILE_READ 0

#define SD_CARD_TYPE_SD1 4
#define SD_CARD_TYPE_SD2 5
#define SD_CARD_TYPE_SDHC 6
#define SD_CARD_TYPE_MOCK 7

#define SPI_QUARTER_SPEED 1
#define SPI_HALF_SPEED 2
#define SPI_FULL_SPEED 3

class MockFile {
	friend class MockSD;

public:
	byte m_buf[EK_MOCK_FILE_SIZE];
	uint32_t m_size;
	const char *m_path;

	uint32_t m_pos;

private:
	MockFile(const char *path);

public:
	MockFile();
	void seek(uint32_t ipos);
	uint32_t size();
	uint32_t position();

	void write(const byte *data, size_t size);

	void readBytes(byte *data, size_t size);

	void close();
	operator bool();
};

class MockFileReference {
	MockFile *m_file;

public:
	MockFileReference();
	MockFileReference(MockFile &file);

	void seek(uint32_t ipos);
	uint32_t size();
	uint32_t position();

	void write(const byte *data, size_t size);

	void readBytes(byte *data, size_t size);

	void close();
	operator bool();
};

class MockSD {
	friend class MockFile;

public:
	Vector<MockFile> m_files;
	MockFile m_file_buf[EK_MOCK_FILE_COUNT];
	MockFile m_invalid_file;

	const char *m_open_file;

	bool m_initialized;

public:
	MockSD();

	bool begin(uint8_t);
	MockFileReference open(const char *path, uint8_t write_mode);
	bool exists(const char *path);
	void remove(const char *path);
};

class MockSd2Card {
	friend class MockSdVolume;

private:
	bool m_initialized;

public:
	MockSd2Card();
	bool init(uint8_t, uint8_t);
	uint8_t type() const;
};

class MockSdVolume {
	friend class MockSdFile;

private:
	bool m_initialized;

public:
	MockSdVolume();
	bool init(MockSd2Card &card);
	uint32_t blocksPerCluster() const;
	uint32_t clusterCount() const;
};

class MockSdFile {
private:
	bool m_initialized;

public:
	MockSdFile();
	bool openRoot(MockSdVolume &volume);
	void close();
};

extern MockSD SD;
extern Logger mockLogger;

using File = MockFileReference;
using Sd2Card = MockSd2Card;
using SdVolume = MockSdVolume;
using SdFile = MockSdFile;

#endif // !defined(MOCKFILE_H)