#ifndef EKONYV_BLOCKFILE_DEF_H
#define EKONYV_BLOCKFILE_DEF_H

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
	bool checkAndReadHeader(uint32_t *out_num_records);

	//! @brief Overrides the header in the file with the currently stored number of records and record size.
	void updateHeader();

	constexpr static uint32_t recordPosition(uint32_t n) { return sizeof(Header) + (RecordSize * n); }

public:
	//! @brief Initializes a file to a path, but does not open it.
	BlockFile(const char *path);

	//! @brief Moves the state of a blockfile to another object.
	BlockFile(BlockFile &&other);

	//! @brief Destructs the data and closes the file.
	~BlockFile();

	//! @brief Returns the path of the file.
	constexpr const char *getPath() const { return m_path; }

	//! @brief Returns the number of records in the file.
	//! After the file is closed, the record count is kept.
	constexpr file_index_t getRecordCount() const { return m_num_records; }

	bool isOpen();

	/**
	 * @brief Tries to open the file provided on construction for reading and writing.
	 * @returns true if opening the file was successful and the header, if found, is correct.
	 */
	bool open();

	//! @brief Closes the file, if it is open
	bool close();

	//! @brief Adds a record to the end of the file, and updates the header.
	//! @c data must be a pointer to a @c RecordSize sized buffer in memory.
	//! @note Assumes that the file is open.
	void append(const void *data);

	//! @brief Removes a record from a file, shifts all other records back to its place.
	//! Updates the header accordingly.
	//! @note Assumes that the file is open.
	void erase(file_index_t n);

	//! @brief Overrides the nth record in the file with @c data , assuming that the
	//! nth record exists.
	//! @note Assumes that the file is open and has more than @c n records.
	void modify(file_index_t n, const void *data);

	//! @brief Writes the nth record to @c out_data .
	//! @param out_size The size of the output buffer. Should be at least @c RecordSize bytes.
	void readNth(file_index_t n, void *out_data, size_t out_size);

	//! @brief Begins a transaction; if this is the first transaction in the file, then the file is opened.
	bool beginTransaction();

	//! @brief Ends a transaction. If it is the last, the file is closed.
	void endTransaction();
};

#include "blockfile_impl.h"

#endif // !defined(EKONYV_BLOCKFILE_DEF_H)