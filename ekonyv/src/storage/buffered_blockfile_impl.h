#ifndef EKONYV_BUFFERED_BLOCKFILE_IMPL_H
#define EKONYV_BUFFERED_BLOCKFILE_IMPL_H

#include "buffered_blockfile_def.h"

template <size_t RecordSize, size_t BufferSize>
BufferedBlockfile<RecordSize, BufferSize>::BufferedBlockfile(const char *path)
    : m_file(path), m_buffer(m_buffer_data), m_num_records(0) {}

template <size_t RecordSize, size_t BufferSize>
BufferedBlockfile<RecordSize, BufferSize>::BufferedBlockfile(BufferedBlockfile &&filebuffer)
    : m_file(static_cast<BlockFile<RecordSize> &&>(filebuffer.m_file)),
      m_buffer(m_buffer_data), m_num_records(filebuffer.m_num_records)
{
	m_buffer.assign(filebuffer.m_buffer.size(), filebuffer.m_buffer);
}

template <size_t RecordSize, size_t BufferSize>
bool BufferedBlockfile<RecordSize, BufferSize>::initialize()
{
	if (!m_file.beginTransaction()) {
		VERBOSE_LOG(logger, "LOAD - Failed loading file ", m_file.getPath());
		return false;
	}

	m_num_records = m_file.getRecordCount();

	m_file.endTransaction();

	return true;
}

template <size_t RecordSize, size_t BufferSize>
bool BufferedBlockfile<RecordSize, BufferSize>::flush()
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

template <size_t RecordSize, size_t BufferSize>
bool BufferedBlockfile<RecordSize, BufferSize>::append(const void *data)
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

template <size_t RecordSize, size_t BufferSize>
bool BufferedBlockfile<RecordSize, BufferSize>::modify(uint32_t n, const void *data)
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

template <size_t RecordSize, size_t BufferSize>
bool BufferedBlockfile<RecordSize, BufferSize>::erase(uint32_t n)
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

template <size_t RecordSize, size_t BufferSize>
bool BufferedBlockfile<RecordSize, BufferSize>::at(uint32_t n, void *out_ptr)
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

template <size_t RecordSize, size_t BufferSize>
bool BufferedBlockfile<RecordSize, BufferSize>::readFromFile(uint32_t n, void *out_ptr)
{
	if (!m_file.beginTransaction()) {
		return false;
	}

	m_file.readNth(n, out_ptr, RecordSize);
	m_file.endTransaction();

	return true;
}

template <size_t RecordSize, size_t BufferSize>
bool BufferedBlockfile<RecordSize, BufferSize>::tryEraseFromBuffer(uint32_t n)
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

#endif // !defined(EKONYV_BUFFERED_BLOCKFILE_IMPL_H)