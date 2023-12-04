#ifndef EKONYV_BLOCKFILE_IMPL_H
#define EKONYV_BLOCKFILE_IMPL_H

#include "blockfile_def.h"

template <uint16_t RecordSize>
bool BlockFile<RecordSize>::checkAndReadHeader(uint32_t *out_num_records)
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

template <uint16_t RecordSize>
void BlockFile<RecordSize>::updateHeader()
{
	VERBOSE_LOG(logger, "Updating header in ", m_path, " to ", m_num_records, " records");

	const auto header = Header{
	    {'D', 'B'},
	    RecordSize,
	    m_num_records};

	m_file.seek(0);
	m_file.write((const byte *)&header, sizeof(header)) == sizeof(header);
}

template <uint16_t RecordSize>
BlockFile<RecordSize>::BlockFile(const char *path)
    : m_path(path), m_file(), m_num_records(0), m_headerChecked(false) {}

template <uint16_t RecordSize>
BlockFile<RecordSize>::BlockFile(BlockFile &&other)
    : m_path(other.m_path), m_file(other.m_file),
      m_num_records(other.m_num_records), m_headerChecked(other.m_headerChecked) {}

template <uint16_t RecordSize>
BlockFile<RecordSize>::~BlockFile() { close(); }

template <uint16_t RecordSize>
bool BlockFile<RecordSize>::isOpen() { return bool(m_file); }

template <uint16_t RecordSize>
bool BlockFile<RecordSize>::open()
{
	VERBOSE_LOG(logger, "Opening file ", m_path);
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

template <uint16_t RecordSize>
bool BlockFile<RecordSize>::close()
{
	if (!isOpen())
		return false;

	m_file.close();

	return true;
}

template <uint16_t RecordSize>
void BlockFile<RecordSize>::append(const void *data)
{
	m_file.seek(recordPosition(m_num_records));
	m_file.write((const byte *)data, RecordSize);

	++m_num_records;
	updateHeader();
}

template <uint16_t RecordSize>
void BlockFile<RecordSize>::erase(file_index_t n)
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

template <uint16_t RecordSize>
void BlockFile<RecordSize>::modify(file_index_t n, const void *data)
{
	m_file.seek(recordPosition(n));
	m_file.write((const byte *)data, RecordSize);
}

template <uint16_t RecordSize>
void BlockFile<RecordSize>::readNth(file_index_t n, void *out_data, size_t out_size)
{
	m_file.seek(recordPosition(n));
	m_file.readBytes((byte *)out_data, min(out_size, RecordSize));
}

template <uint16_t RecordSize>
bool BlockFile<RecordSize>::beginTransaction()
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

template <uint16_t RecordSize>
void BlockFile<RecordSize>::endTransaction()
{
	if (m_transactionCount == 1) {
		close();
	}

	m_transactionCount--;
}

template <uint16_t RecordSize>
Logger BlockFile<RecordSize>::logger = Logger("BFMG");

#endif // !defined(EKONYV_BLOCKFILE_IMPL_H)
