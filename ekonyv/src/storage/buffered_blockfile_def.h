#ifndef EKONYV_BUFFERED_BLOCKFILE_DEF_H
#define EKONYV_BUFFERED_BLOCKFILE_DEF_H

#include "blockfile.h"
#include <Vector.h>

/**
 * @brief A class for handling buffering in blockfiles.
 * Append/modify/erase operations are buffered until @c flush() is called.
 *
 * @tparam RecordSize The size of a record, in bytes
 * @tparam BufferSize The number of operations to buffer.
 */
template <size_t RecordSize, size_t BufferSize>
class BufferedBlockfile {
private:
	enum UpdateType : uint8_t {
		APPEND,
		MODIFY,
		ERASE
	};

	struct Update {
		UpdateType type;
		uint32_t n;
		uint8_t buffer[RecordSize];
	};

private:
	BlockFile<RecordSize> m_file;
	Update m_buffer_data[BufferSize];

	Vector<Update> m_buffer;
	uint32_t m_num_records;

public:
	//! @brief Initializes the buffer, sets the underlying blockfile's path.
	BufferedBlockfile(const char *path);

	//! @brief Moves the buffer and file data from another object.
	BufferedBlockfile(BufferedBlockfile &&filebuffer);

	~BufferedBlockfile() = default;

	//! @brief Returns the number of records stored in the database.
	constexpr uint32_t getRecordCount() const { return m_num_records; }

	//! @brief Returns whether the buffer is full.
	inline bool isFull() const { return m_buffer.full(); }

	inline bool beginTransaction() { return m_file.beginTransaction(); }
	inline void endTransaction() { m_file.endTransaction(); }

	//! @brief Tries to load the blockfile at the path given on initialization;
	//!        loads the record count data if it is available, otherwise creates the file.
	bool initialize();

	//! @brief Tries to save all pending changes to the path given on initialization.
	//! Processes the buffer item by item and executes the corresponding insert/modify/erase commands.
	//! Failed commands are lost from the buffer.
	//! @returns true if opening the file was successful.
	bool flush();

	//! @brief Enqueues an append command into the buffer.
	//! @note To save the data on the disk, @c flush must be called.
	bool append(const void *data);

	//! @brief Enqueues a modify command into the buffer; modifies the nth record to @c data .
	//! @note To save the data on the disk, @c flush must be called.
	//! @returns true if n is correct.
	bool modify(uint32_t n, const void *data);

	//! @brief Enqueues a remove command into the buffer; erases the nth record.
	//! @note To save the data on the disk, @c flush must be called.
	//! @returns true if n is correct.
	bool erase(uint32_t n);

	//! @brief Reads the nth element from the buffer if it is available,
	//!        otherwise reads from the file.
	//! @returns true if out_ptr has the valid data.
	bool at(uint32_t n, void *out_ptr);

private:
	bool readFromFile(uint32_t n, void *out_ptr);
	bool tryEraseFromBuffer(uint32_t n);
};

#endif // !defined(EKONYV_BUFFERED_BLOCKFILE_DEF_H)