#ifndef EKONYV_BLOCKFILE_H
#define EKONYV_BLOCKFILE_H

#include "storage.h"

template <uint16_t RecordSize>
class BlockFile {
public:
	using file_index_t = uint32_t;

private:
	static Logger logger;

private:
	const char *m_path;
	File m_file;
	uint32_t m_numRecords;

	struct Header {
		char magic[2];
		uint16_t recordSize;
		uint32_t numRecords;
	};

private:
	void writeHeader()
	{
		const auto header = Header{
		    {'D', 'B'},
		    RecordSize,
		    m_numRecords};

		m_file.write((const byte *)&header, sizeof(header));
	}

	bool checkAndReadHeader()
	{
		auto header = Header{};

		m_file.seek(0);
		m_file.readBytes((byte *)&header, sizeof(header));

		if (header.magic[0] == 'D' && header.magic[1] == 'B' && header.recordSize == RecordSize) {
			VERBOSE_LOG(logger, "Checked header and read ", header.numRecords, " records");
			m_numRecords = header.numRecords;
			return true;
		}

		VERBOSE_LOG(logger, "Header checking failed");

		return false;
	}

	static uint32_t recordPosition(uint32_t n)
	{
		return sizeof(Header) + (RecordSize * n);
	}

public:
	BlockFile(const char *path) : m_path(path), m_file(), m_numRecords(0)
	{
	}

	BlockFile(BlockFile &&other) : m_path(other.m_path), m_file(other.m_file), m_numRecords(other.m_numRecords)
	{
	}

	~BlockFile()
	{
		close();
	}

	const char *getPath() const
	{
		return m_path;
	}

	bool open()
	{
		m_file = SD.open(m_path, FILE_WRITE);

		if (m_file) {
			if (m_file.size() == 0) {
				VERBOSE_LOG(logger, "Opening blockfile; Writing header for ", m_path);
				writeHeader();
				return true;
			}

			VERBOSE_LOG(logger, "Opening blockfile; Checking header for ", m_path);

			return checkAndReadHeader();
		}
		else {
			logger.error("Failed opening blockfile ", m_path);
			return false;
		}
	}

	bool close()
	{
		if (!is_open())
			return false;

		m_file.close();
		m_numRecords = 0;

		return true;
	}

	bool is_open()
	{
		return m_file;
	}

	file_index_t getRecordCount() const
	{
		return m_numRecords;
	}

	void append(void *data)
	{
		m_file.seek(recordPosition(m_numRecords));
		m_file.write((const byte *)data, RecordSize);
		++m_numRecords;
	}

	void erase(file_index_t n)
	{
		byte recordBuffer[RecordSize] = {};

		VERBOSE_LOG(logger, "Erasing #", n, "; until ", m_numRecords);

		for (file_index_t i = n + 1; i < m_numRecords; ++i) {
			m_file.seek(recordPosition(i));
			m_file.readBytes(recordBuffer, sizeof(recordBuffer));

			m_file.seek(recordPosition(i - 1));
			m_file.write((const byte *)&recordBuffer, sizeof(recordBuffer));
		}
		--m_numRecords;
	}

	void modify(file_index_t n, void *data)
	{
		m_file.seek(recordPosition(n));
		m_file.write((const byte *)data, RecordSize);
	}

	void readNth(file_index_t n, void *out_data, size_t out_size)
	{
		m_file.seek(recordPosition(n));
		m_file.readBytes((byte *)out_data, min(out_size, RecordSize));
	}

	void updateHeader()
	{
		VERBOSE_LOG(logger, "Updating header in ", m_path);
		m_file.seek(0);
		writeHeader();
	}

	/* template <size_t BufferSize>
bool shrink_to_fit()
{
	byte copyBuffer[BufferSize] = {};

	String tempFilePath = String(m_path) + '~';

	const auto numRecords = m_numRecords;
	const auto totalBlocksToCopy = (numRecords * RecordSize + sizeof(Header)) / BufferSize;
	const auto leftover = (numRecords * RecordSize + sizeof(Header)) % BufferSize;

	for (uint32_t i = 0; i < totalBlocksToCopy; ++i) {
	    m_file.seek(i * BufferSize);
	    m_file.readBytes(copyBuffer, sizeof(copyBuffer));

	    m_file.close();

	    File destinationFile = SD.open(tempFilePath, FILE_WRITE);
	    destinationFile.write(copyBuffer, sizeof(copyBuffer));
	    destinationFile.close();

	    m_file.open(m_path, FILE_WRITE);
	}

	if (leftover != 0) {
	    m_file.seek(totalBlocksToCopy * BufferSize);
	    m_file.readBytes(copyBuffer, leftover);

	    m_file.close();

	    File destinationFile = SD.open(tempFilePath, FILE_WRITE);
	    destinationFile.write(copyBuffer, sizeof(copyBuffer));
	    destinationFile.close();
	}

	if (m_file)
	    m_file.close();

	// there is no move???
} */

	// void modify(file_index_t n, )
};

template <uint16_t RecordSize>
Logger BlockFile<RecordSize>::logger = Logger("BFMG");

#endif // !defined(EKONYV_BLOCKFILE_H)