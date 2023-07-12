#include "mocksd.h"

#include "../string/to_string.h"

#pragma region MockFile

MockFile::MockFile()
    : m_buf{}, m_size(0), m_path("<invalid>"), m_pos(0) {}

/* private */ MockFile::MockFile(const char *path)
    : m_buf{}, m_size(0), m_path(path), m_pos(0) {}

void MockFile::seek(uint32_t ipos)
{
	if (SD.m_open_file != m_path)
		return;

	m_pos = min(m_size, ipos);
}
uint32_t MockFile::size()
{
	if (SD.m_open_file != m_path)
		return 0;

	return m_size;
}
uint32_t MockFile::position()
{
	if (SD.m_open_file != m_path)
		return 0;

	return m_pos;
}

void MockFile::write(const byte *data, size_t size)
{
	if (SD.m_open_file != m_path)
		return;

#if EK_VERBOSE_LOGGING
	if (m_pos + size >= EK_MOCK_FILE_SIZE)
		VERBOSE_LOG(mockLogger, "Buffer overrun by ", (m_pos + size) - m_size);
#endif

	VERBOSE_LOG_B(mockLogger, "Writing ", size, " bytes to ", m_path, ": ");

	for (int i = 0; i < size; ++i) {
		VERBOSE_LOG_C(mockLogger, byte_to_string(data[i]));
	}

	VERBOSE_LOG_E(mockLogger);

	memcpy(m_buf + m_pos, data, min(size, EK_MOCK_FILE_SIZE - m_pos));
	m_size = min(max(m_size, m_pos + size), EK_MOCK_FILE_SIZE);
	m_pos = min(m_pos + size, EK_MOCK_FILE_SIZE);

	VERBOSE_LOG(mockLogger, "Size: ", m_size, " pos: ", m_pos);
}

void MockFile::readBytes(byte *data, size_t size)
{
	if (m_pos == m_size)
		return;

	memcpy(data, m_buf + m_pos, min(size, m_size - m_pos));
}

void MockFile::close()
{
	SD.m_open_file = nullptr;
}

MockFile::operator bool()
{
	return SD.m_open_file == m_path;
}

// MockFile
#pragma endregion

#pragma region MockFileReference

MockFileReference::MockFileReference() : m_file(nullptr)
{
}

MockFileReference::MockFileReference(MockFile &file) : m_file(&file)
{
}

void MockFileReference::seek(uint32_t ipos)
{
	if (m_file)
		m_file->seek(ipos);
}

uint32_t MockFileReference::size()
{
	if (m_file)
		return m_file->size();
	else
		return 0;
}
uint32_t MockFileReference::position()
{
	if (m_file)
		return m_file->position();
	else
		return 0;
}

void MockFileReference::write(const byte *data, size_t size)
{
	if (m_file)
		m_file->write(data, size);
}

void MockFileReference::readBytes(byte *data, size_t size)
{
	if (m_file)
		m_file->readBytes(data, size);
}

void MockFileReference::close()
{
	if (m_file)
		m_file->close();
}

MockFileReference::operator bool()
{
	if (m_file)
		return (*m_file);
	else
		return false;
}

// MockFileReference
#pragma endregion

#pragma region MockSD

MockSD::MockSD()
    : m_files(), m_file_buf(),
      m_invalid_file(nullptr),
      m_initialized(false)
{
	m_files.setStorage<EK_MOCK_FILE_COUNT>(m_file_buf);
}

bool MockSD::begin(uint8_t)
{
	m_initialized = true;
	return true;
}

MockFileReference MockSD::open(const char *path, uint8_t write_mode)
{
	if (!m_initialized || m_open_file != nullptr)
		return m_invalid_file;

	for (size_t i = 0; i < m_files.size(); ++i) {
		auto &file = m_files[i];
		if (file.m_path == path) {
			file.m_pos = write_mode == FILE_WRITE ? file.m_size : 0;
			m_open_file = file.m_path;
			return file;
		}
	}

	auto new_file = MockFile(path);
	m_files.push_back(new_file);

	VERBOSE_LOG(mockLogger, "Fulfilling file open at \"", path, "\", size: ", m_files[m_files.size() - 1].size());

	return m_files[m_files.size() - 1];
}

bool MockSD::exists(const char *path)
{
	if (m_open_file == path)
		return true;

	for (size_t i = 0; i < m_files.size(); ++i)
		if (m_files.at(i).m_path == path)
			return true;

	return false;
}

void MockSD::remove(const char *path)
{
	if (m_open_file == path)
		return;

	for (size_t i = 0; i < m_files.size(); ++i) {
		if (m_files.at(i).m_path == path) {
			m_files.remove(i);
			return;
		}
	}
}

// MockSD
#pragma endregion

#pragma region MockSd2Card

MockSd2Card::MockSd2Card() : m_initialized(false)
{
}

bool MockSd2Card::init(uint8_t, uint8_t)
{
	m_initialized = true;
	return true;
}

uint8_t MockSd2Card::type() const
{
	return SD_CARD_TYPE_MOCK;
}

// MockSd2Card
#pragma endregion

#pragma region MockSdVolume

MockSdVolume::MockSdVolume() : m_initialized(false)
{
}

bool MockSdVolume::init(MockSd2Card &card)
{
	if (!card.m_initialized)
		return false;
	m_initialized = true;
	return true;
}

uint32_t MockSdVolume::blocksPerCluster() const
{
	return EK_MOCK_FILE_BLOCKS;
}

uint32_t MockSdVolume::clusterCount() const
{
	return EK_MOCK_FILE_COUNT;
}

// MockSdVolume
#pragma endregion

#pragma region MockSdFile

MockSdFile::MockSdFile() : m_initialized(false)
{
}

bool MockSdFile::openRoot(MockSdVolume &volume)
{
	if (!volume.m_initialized)
		return false;
	m_initialized = true;
	return true;
}

void MockSdFile::close()
{
}

// MockSdFile
#pragma endregion

MockSD SD = MockSD();
Logger mockLogger = Logger("MKSD");