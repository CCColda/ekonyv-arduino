#ifndef EKONYV_BUFFERED_BLOCKFILE_H
#define EKONYV_BUFFERED_BLOCKFILE_H

#include "blockfile.h"
#include <Vector.h>

template <size_t RecordSize, size_t BufferSize>
class BlockFileBuffer {
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
	constexpr static uint32_t BUFFER_SIZE = BufferSize;
	constexpr static uint32_t RECORD_SIZE = RecordSize;

public:
	BlockFileBuffer(const char *path)
	    : m_file(path), m_buffer(m_buffer_data), m_num_records(0) {}

	BlockFileBuffer(BlockFileBuffer &&filebuffer)
	    : m_file(static_cast<BlockFile<RecordSize> &&>(filebuffer.m_file)),
	      m_buffer(m_buffer_data), m_num_records(filebuffer.m_num_records)
	{
		m_buffer.assign(filebuffer.m_buffer.size(), filebuffer.m_buffer);
	}

	//! @brief Returns the number of records stored in the database.
	constexpr uint32_t getRecordCount() const { return m_num_records; }

	//! @brief Returns whether the buffer is full.
	bool isFull() const { return m_buffer.full(); }

	bool beginTransaction() { return m_file.beginTransaction(); }
	void endTransaction() { m_file.endTransaction(); }

	//! @brief Tries to load the blockfile at the path given on initialization;
	//!        loads the record count data if it is available, otherwise creates the file.
	bool initialize()
	{
		if (!m_file.beginTransaction()) {
			VERBOSE_LOG(logger, "LOAD - Failed loading file ", m_file.getPath());
			return false;
		}

		m_num_records = m_file.getRecordCount();

		m_file.endTransaction();

		return true;
	}

	//! @brief Tries to save all pending changes to the path given on initialization.
	//! Processes the cache item by item and executes the corresponding insert/modify/erase commands.
	//! Failed commands are lost from the cache.
	//! @returns true if opening the file was successful.
	bool flush()
	{
		if (!m_file.beginTransaction()) {
			return false;
		}

		for (const Update &update : m_buffer) {
			switch (update.type) {
				case UpdateType::APPEND: {
					m_file.append(update.buffer);
					break;
				}
				case UpdateType::MODIFY: {
					m_file.modify(update.n, reinterpret_cast<const void *>(update.buffer));
					break;
				}
				case UpdateType::ERASE: {
					m_file.erase(update.n);
					break;
				}
			}
		}

		m_file.endTransaction();

		m_buffer.clear();
		return true;
	}

	//! @brief Enqueues an append command into the cache.
	//! @note To save the data on the disk, @c flush must be called.
	bool append(const void *data)
	{
		auto update = Update{
		    UpdateType::APPEND,
		    0,
		    {}};

		memcpy(update.buffer, data, RecordSize);

		m_buffer.push_back(update);

		m_num_records++;

		return true;
	}

	//! @brief Enqueues a modify command into the cache; modifies the nth record to @c data .
	//! @note To save the data on the disk, @c flush must be called.
	//! @returns true if n is correct.
	bool modify(uint32_t n, const void *data)
	{
		if (n >= m_num_records)
			return false;

		auto update = Update{
		    UpdateType::MODIFY,
		    n,
		    {}};

		memcpy(update.buffer, data, RecordSize);

		m_buffer.push_back(update);

		return true;
	}

	//! @brief Enqueues a remove command into the cache; erases the nth record.
	//! @note To save the data on the disk, @c flush must be called.
	//! @returns true if n is correct.
	bool erase(uint32_t n)
	{
		if (n >= m_num_records)
			return false;

		if (!tryEraseFromBuffer(n)) {
			Update update = Update{
			    UpdateType::ERASE,
			    n,
			    {}};

			m_buffer.push_back(update);
		}

		m_num_records--;
		return true;
	}

	//! @brief Reads the nth element from the buffer if it is available,
	//!        otherwise reads from the file.
	//! @returns true if out_ptr has the valid data.
	bool at(uint32_t n, void *out_ptr)
	{
		if (n >= m_num_records)
			return false;

		uint32_t idx_file = n,
		         file_num_records = m_file.getRecordCount();

		const void *data_ptr = nullptr;

		for (const Update &update : m_buffer) {
			switch (update.type) {
				case UpdateType::APPEND: {
					if (file_num_records == n) {
						data_ptr = reinterpret_cast<const void *>(update.buffer);
					}

					file_num_records++;
					break;
				}
				case UpdateType::MODIFY: {
					if (update.n == n) {
						data_ptr = reinterpret_cast<const void *>(update.buffer);
					}

					break;
				}
				case UpdateType::ERASE: {
					if (update.n <= n) {
						idx_file++;
						data_ptr = nullptr;
					}

					file_num_records--;
					break;
				}
			}
		}

		if (data_ptr != nullptr) {
			memcpy(out_ptr, data_ptr, RecordSize);
			return true;
		}

		return readFromFile(idx_file, out_ptr);
	}

private:
	bool readFromFile(uint32_t n, void *out_ptr)
	{
		if (!m_file.beginTransaction()) {
			return false;
		}

		m_file.readNth(n, out_ptr, RecordSize);
		m_file.endTransaction();

		return true;
	}

	bool tryEraseFromBuffer(uint32_t n)
	{
		uint32_t file_num_records = m_file.getRecordCount();
		bool append_removed = false;

		uint32_t num_updates = m_buffer.size();
		uint32_t idx_update = 0;

		while (idx_update < num_updates) {
			Update &update = m_buffer[idx_update];

			switch (update.type) {
				case UpdateType::APPEND: {
					if (file_num_records == n) {
						//! @remark @c update is invalid from here
						m_buffer.remove(idx_update);

						append_removed = true;

						num_updates--;

						file_num_records++;
						break;
					}

					file_num_records++;
					break;
				}

				case UpdateType::MODIFY: {
					if (append_removed) {
						if (update.n == n) {
							m_buffer.remove(idx_update);

							num_updates--;
							break;
						}
						else if (update.n > n) {
							update.n--;
						}
					}

					idx_update++;
					break;
				}

				case UpdateType::ERASE: {
					file_num_records--;
					break;
				}
			}
		}

		return append_removed;
	}
};

#endif // !defined(EKONYV_BUFFERED_BLOCKFILE_H)