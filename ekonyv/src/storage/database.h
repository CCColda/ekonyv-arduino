#ifndef EKONYV_DATABASE_H
#define EKONYV_DATABASE_H

#include "blockfile.h"

#include <CircularBuffer.h>
#include <type_traits>

#include "../arduino/logger.h"

enum QueryState : uint8_t {
	SUCCESS,
	ERROR
};

template <typename Record, size_t CacheSize = 8>
class Database {
	static_assert(std::is_trivial<Record>::value, "Record type must be trivial.");

private:
	static Logger logger;

public:
	static constexpr size_t CACHE_SIZE = CacheSize;

	template <typename... Args>
	using SearchFnPtr = bool (*)(uint32_t, const Record &, Args...);

	template <typename... Args>
	using IterationFnPtr = void (*)(uint32_t, const Record &, Args...);

	struct QueryResult {
		QueryState state;
		uint32_t index;
		Record value;
	};

private:
	enum UpdateType : uint8_t {
		ADD,
		REMOVE,
		MODIFY
	};

	struct Update {
		UpdateType type;
		uint32_t n;
		Record data;
	};

private:
	BlockFile<sizeof(Record)> m_file;
	CircularBuffer<Update, CacheSize> m_updateCache;
	uint32_t m_file_num_records;
	uint32_t m_num_records;

private:
	struct IndexInfo {
		enum IndexState : uint8_t {
			REMOVED,
			FOUND,
			IN_FILE
		};

		IndexState state;
		uint32_t file_index;

		Record data;
	};

public:
	Database(const char *path) : m_file(path), m_file_num_records(0), m_num_records(0)
	{
	}

	Database(Database &&other)
	    : m_updateCache(), m_file(static_cast<BlockFile<sizeof(Record)> &&>(other.m_file)), m_file_num_records(other.m_file_num_records), m_num_records(other.m_num_records)
	{
		while (!other.m_updateCache.isEmpty())
			m_updateCache.push(other.m_updateCache.shift());
	}

	bool tryLoad()
	{
		if (!m_file.open())
			return false;

		m_num_records = m_file_num_records = m_file.getRecordCount();

		m_file.close();

		VERBOSE_LOG(logger, "Got record count: ", m_file_num_records);

		return true;
	}

	bool trySave()
	{
		if (m_updateCache.isEmpty()) {
			VERBOSE_LOG(logger, "Trying to save ", m_file.getPath(), ", but cache is empty");
			return true;
		}

		if (!m_file.open())
			return false;

		VERBOSE_LOG(logger, "Trying to save ", m_file.getPath());

		while (!m_updateCache.isEmpty()) {
			const auto update = m_updateCache.shift();
			switch (update.type) {
				case UpdateType::ADD: {
					VERBOSE_LOG(logger, "Processing add command");
					m_file.append((void *)&update.data); // todo const cast
					++m_file_num_records;
					break;
				}
				case UpdateType::REMOVE: {
					VERBOSE_LOG(logger, "Processing remove command at ", update.n);

					if (update.n >= m_file_num_records) {
						VERBOSE_LOG(logger, "Invalid remove; removing beyond ", m_file_num_records);
						break;
					}

					m_file.erase(update.n);

					--m_file_num_records;
					break;
				}
				case UpdateType::MODIFY: {
					size_t correctedN = update.n;

					VERBOSE_LOG(logger, "Processing modify command at ", update.n);

					m_file.modify(correctedN, (void *)&update.data); // todo const cast
					break;
				}
			}
		}

		m_file.updateHeader();

		m_file.close();

		return true;
	}

	bool append(const Record &data)
	{
		m_updateCache.push(Update{
		    UpdateType::ADD,
		    0,
		    data});

		++m_num_records;

		if (m_updateCache.isFull())
			if (!trySave())
				return false;

		return true;
	}

	bool modify(uint32_t n, const Record &data)
	{
		m_updateCache.push(Update{
		    UpdateType::MODIFY,
		    n,
		    data});

		if (m_updateCache.isFull())
			if (!trySave())
				return false;

		return true;
	}

	bool remove(uint32_t n)
	{
		m_updateCache.push(Update{
		    UpdateType::REMOVE,
		    n,
		    Record()});

		--m_num_records;

		if (m_updateCache.isFull())
			if (!trySave())
				return false;

		return true;
	}

	QueryResult at(uint32_t n)
	{
		if (n >= m_num_records)
			return QueryResult{QueryState::ERROR, n, Record()};

		if (!trySave())
			return QueryResult{QueryState::ERROR, n, Record()};

		m_file.open();

		auto result = Record();
		m_file.readNth(n, &result, sizeof(result));

		m_file.close();

		return QueryResult{
		    QueryState::SUCCESS, n, result};
	}

	uint32_t size() const
	{
		return m_num_records;
	}

	template <typename... SearchArgs>
	QueryResult search(uint32_t start, bool reversed, SearchFnPtr<SearchArgs...> searchFn, SearchArgs... searchArgs)
	{
		if (!trySave())
			return QueryResult{QueryState::ERROR, 0, Record()};

		QueryResult result = {
		    QueryState::ERROR,
		    0,
		    Record()};

		m_file.open();

		for (uint32_t i = reversed ? m_num_records - start : 0;
		     reversed ? i > 0 : i < m_num_records;
		     reversed ? --i : ++i) {

			const auto corrected_i = i - 1;

			auto record = Record();
			m_file.readNth(corrected_i, &record, sizeof(record));

			if (searchFn(corrected_i, record, searchArgs...)) {
				result = QueryResult{QueryState::SUCCESS, corrected_i, record};
				break;
			}
		}

		m_file.close();

		return result;
	}

	template <typename... IterArgs>
	bool iterate(bool unlockFile, uint32_t start, uint32_t end, bool reversed, IterationFnPtr<IterArgs...> iterFn, IterArgs... iterArgs)
	{
		if (!trySave())
			return false;

		for (uint32_t i = reversed ? end : start;
		     reversed ? i > start : i < end;
		     reversed ? --i : ++i) {
			if (!m_file.is_open())
				m_file.open();

			const auto corrected_i = i - 1;

			auto result = Record();
			m_file.readNth(corrected_i, &result, sizeof(result));

			if (unlockFile)
				m_file.close();

			iterFn(corrected_i, result, iterArgs...);
		}

		if (m_file.is_open())
			m_file.close();

		return true;
	}

	template <typename... SearchArgs>
	uint32_t remove_if(uint32_t start, uint32_t end, SearchFnPtr<SearchArgs...> searchFn, SearchArgs... searchArgs)
	{
		if (!trySave())
			return 0;

		uint32_t result = 0;

		m_file.open();

		for (uint32_t i = start; i > 0; --i) {
			const auto corrected_i = i - 1;

			auto record = Record();
			m_file.readNth(corrected_i, &record, sizeof(record));

			if (searchFn(corrected_i, record, searchArgs...)) {
				++result;
				--m_num_records;

				m_file.erase(corrected_i);
			}
		}

		m_file.close();

		return result;
	}
};

template <typename Record, size_t CacheSize>
Logger Database<Record, CacheSize>::logger = Logger("DBMG");

#endif // !defined(EKONYV_DATABASE_H)