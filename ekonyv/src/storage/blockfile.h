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
	bool m_headerChecked;

	struct Header {
		char magic[2];
		uint16_t recordSize;
		uint32_t numRecords;
	};

	struct CheckHeaderResult {
		bool success;
		uint32_t numRecords;
	};

private:
	CheckHeaderResult checkAndReadHeader()
	{
		auto header = Header{};

		(void)m_file.seek(0);
		const size_t bytes_read = m_file.readBytes((byte *)&header, sizeof(header));

		if (bytes_read == sizeof(header) &&
		    header.magic[0] == 'D' && header.magic[1] == 'B' &&
		    header.recordSize == RecordSize) {

			VERBOSE_LOG(logger, "Checked header and read ", header.numRecords, " records");
			return {true, header.numRecords};
		}

		VERBOSE_LOG(logger, "Failed checking header");

		return {false, 0};
	}

	bool updateHeader()
	{
		VERBOSE_LOG(logger, "Updating header in ", m_path, " to ", m_numRecords, " records");

		const auto header = Header{
		    {'D', 'B'},
		    RecordSize,
		    m_numRecords};

		(void)m_file.seek(0);
		return m_file.write((const byte *)&header, sizeof(header)) == sizeof(header);
	}

	constexpr static uint32_t recordPosition(uint32_t n)
	{
		return sizeof(Header) + (RecordSize * n);
	}

public:
	BlockFile(const char *path) : m_path(path), m_file(), m_numRecords(0), m_headerChecked(false)
	{
	}

	BlockFile(BlockFile &&other) : m_path(other.m_path), m_file(other.m_file), m_numRecords(other.m_numRecords), m_headerChecked(other.m_headerChecked)
	{
	}

	~BlockFile()
	{
		(void)close();
	}

	constexpr const char *getPath() const
	{
		return m_path;
	}

	bool open()
	{
		logger.log("Opening file ", m_path);
		m_file = SD.open(m_path, O_CREAT | O_RDWR | O_SYNC);

		if (m_file) {
			if (m_file.size() == 0) {
				m_headerChecked = true;
				VERBOSE_LOG(logger, "Opening blockfile; Writing header for ", m_path);
				updateHeader();
				return true;
			}

			if (!m_headerChecked) {
				m_headerChecked = true;
				VERBOSE_LOG(logger, "Opening blockfile; Checking header for ", m_path);

				const auto [success, numRecords] = checkAndReadHeader();

				if (success)
					m_numRecords = numRecords;

				return success;
			}

			return true;
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

		return true;
	}

	bool is_open()
	{
		return m_file;
	}

	constexpr file_index_t getRecordCount() const
	{
		return m_numRecords;
	}

	bool append(void *data)
	{
		(void)m_file.seek(recordPosition(m_numRecords));
		if (!m_file.write((const byte *)data, RecordSize)) {
			logger.error("Failed appending record to ", m_path);
			return false;
		}

		++m_numRecords;
		return updateHeader();
	}

	void erase(file_index_t n)
	{
		byte recordBuffer[RecordSize] = {};

		for (file_index_t i = n + 1; i < m_numRecords; ++i) {
			m_file.seek(recordPosition(i));
			m_file.readBytes(recordBuffer, sizeof(recordBuffer));

			m_file.seek(recordPosition(i - 1));
			m_file.write((const byte *)&recordBuffer, sizeof(recordBuffer));
		}

		--m_numRecords;
		updateHeader();
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
};

template <uint16_t RecordSize>
Logger BlockFile<RecordSize>::logger = Logger("BFMG");

#endif // !defined(EKONYV_BLOCKFILE_H)