#ifndef EKONYV_BLOCKFILE_H
#define EKONYV_BLOCKFILE_H

#include "storage.h"

/**
 * @brief Represents a file containing a header and blocks of @c RecordSize bytes.
 * @tparam RecordSize the size of the records, in bytes
 */
template <uint16_t RecordSize>
class BlockFile {
public:
	using file_index_t = uint32_t;

private:
	static Logger logger;

private:
	const char *m_path;
	File m_file;
	uint32_t m_num_records;
	bool m_headerChecked;

	//! Tracks the transactions for this blockfile.
	//! A transaction can be started with @c beginTransaction() and ended with @c endTransaction()
	//! @c beginTransaction ensures that the file is open.
	//! The file will be closed automatically after the last @c endTransaction() was called.
	uint8_t m_transactionCount;

	struct Header {
		char magic[2];
		uint16_t recordSize;
		uint32_t numRecords;
	};

private:
	//! @brief Reads the header from the file, and checks the record size to @c RecordSize
	//! If there was an error reading or the sizes mismatch, @c out_num_records is unchanged;
	//! otherwise it holds the number of records in the file.
	//! @returns true if checking the header was successful.
	bool checkAndReadHeader(uint32_t *out_num_records)
	{
		auto header = Header{};

		m_file.seek(0);
		const size_t bytes_read = m_file.readBytes((byte *)&header, sizeof(header));

		if (bytes_read == sizeof(header) &&
		    header.magic[0] == 'D' && header.magic[1] == 'B' &&
		    header.recordSize == RecordSize) {

			VERBOSE_LOG(logger, "Checked header and read ", header.numRecords, " records");
			*out_num_records = header.numRecords;
			return true;
		}

		VERBOSE_LOG(logger, "Failed checking header");
		return false;
	}

	//! @brief Overrides the header in the file with the currently stored number of records and record size.
	void updateHeader()
	{
		VERBOSE_LOG(logger, "Updating header in ", m_path, " to ", m_num_records, " records");

		const auto header = Header{
		    {'D', 'B'},
		    RecordSize,
		    m_num_records};

		m_file.seek(0);
		m_file.write((const byte *)&header, sizeof(header)) == sizeof(header);
	}

	constexpr static uint32_t recordPosition(uint32_t n) { return sizeof(Header) + (RecordSize * n); }

public:
	//! @brief Initializes a file to a path, but does not open it.
	BlockFile(const char *path)
	    : m_path(path), m_file(), m_num_records(0), m_headerChecked(false) {}

	//! @brief Moves the state of a blockfile to another object.
	BlockFile(BlockFile &&other)
	    : m_path(other.m_path), m_file(other.m_file),
	      m_num_records(other.m_num_records), m_headerChecked(other.m_headerChecked) {}

	//! @brief Destructs the data and closes the file.
	~BlockFile() { close(); }

	//! @brief Returns the path of the file.
	constexpr const char *getPath() const { return m_path; }

	//! @brief Returns the number of records in the file.
	//! After the file is closed, the record count is kept.
	constexpr file_index_t getRecordCount() const { return m_num_records; }

	bool isOpen() { return bool(m_file); }

	/**
	 * @brief Tries to open the file provided on construction for reading and writing.
	 * @returns true if opening the file was successful and the header, if found, is correct.
	 */
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

				return checkAndReadHeader(&m_num_records);
			}

			return true;
		}
		else {
			logger.error("Failed opening blockfile ", m_path);
			return false;
		}
	}

	//! @brief Closes the file, if it is open
	bool close()
	{
		if (!isOpen())
			return false;

		m_file.close();

		return true;
	}

	//! @brief Adds a record to the end of the file, and updates the header.
	//! @c data must be a pointer to a @c RecordSize sized buffer in memory.
	//! @note Assumes that the file is open.
	void append(const void *data)
	{
		m_file.seek(recordPosition(m_num_records));
		m_file.write((const byte *)data, RecordSize);

		++m_num_records;
		updateHeader();
	}

	//! @brief Removes a record from a file, shifts all other records back to its place.
	//! Updates the header accordingly.
	//! @note Assumes that the file is open.
	void erase(file_index_t n)
	{
		byte recordBuffer[RecordSize] = {};

		for (file_index_t i = n + 1; i < m_num_records; ++i) {
			m_file.seek(recordPosition(i));
			m_file.readBytes(recordBuffer, sizeof(recordBuffer));

			m_file.seek(recordPosition(i - 1));
			m_file.write((const byte *)&recordBuffer, sizeof(recordBuffer));
		}

		--m_num_records;
		updateHeader();
	}

	//! @brief Overrides the nth record in the file with @c data , assuming that the
	//! nth record exists.
	//! @note Assumes that the file is open and has more than @c n records.
	void modify(file_index_t n, const void *data)
	{
		m_file.seek(recordPosition(n));
		m_file.write((const byte *)data, RecordSize);
	}

	//! @brief Writes the nth record to @c out_data .
	//! @param out_size The size of the output buffer. Should be at least @c RecordSize bytes.
	void readNth(file_index_t n, void *out_data, size_t out_size)
	{
		m_file.seek(recordPosition(n));
		m_file.readBytes((byte *)out_data, min(out_size, RecordSize));
	}

	//! @brief Begins a transaction; if this is the first transaction in the file, then the file is opened.
	bool beginTransaction()
	{
		if (m_transactionCount == decltype(m_transactionCount)(~0))
			return false;

		if (!m_file) {
			if (!open())
				return false;
		}

		m_transactionCount++;
		return true;
	}

	//! @brief Ends a transaction. If it is the last, the file is closed.
	void endTransaction()
	{
		if (m_transactionCount == 1) {
			close();
		}

		m_transactionCount--;
	}
};

template <uint16_t RecordSize>
Logger BlockFile<RecordSize>::logger = Logger("BFMG");

#endif // !defined(EKONYV_BLOCKFILE_H)