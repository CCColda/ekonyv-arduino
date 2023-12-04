#ifndef EKONYV_DATABASE_H
#define EKONYV_DATABASE_H

#include "buffered_blockfile.h"

#include "../arduino/logger.h"

#include <type_traits>

/**
 * @brief Defines a fixed-size record database.
 *
 * @tparam Record the type of the record
 */
template <typename Record, size_t BufferSize>
class Database {
	static_assert(std::is_trivial<Record>::value, "Record type must be trivial.");

private:
	static Logger logger;

public:
	//! @brief Describes a search function that receives the number of the record,
	//!        the record data itself, and custom arguments. The function should return
	//!        whether the record matches a given criteria.
	template <typename... Args>
	using SearchFnPtr = bool (*)(uint32_t, const Record &, Args...);

	//! @brief Describes an iteration function that receives the number of the record,
	//!        the record data itself, and custom arguments.
	template <typename... Args>
	using IterationFnPtr = void (*)(uint32_t, const Record &, Args...);

	struct QueryResult {
		bool success; //!< Describes whether the query was successful.
		uint32_t index;
		Record value; //!< The value of the query. Invalid if state is @c ERROR .
	};

private:
	BlockFileBuffer<sizeof(Record), BufferSize> m_file;

public:
	Database(const char *path)
	    : m_file(path) {}

	Database(Database &&other)
	    : m_file(static_cast<BlockFileBuffer<sizeof(Record), BufferSize> &&>(other.m_file)) {}

	//! @brief Returns the number of records stored in the database.
	constexpr uint32_t size() const { return m_file.getRecordCount(); }

	//! @brief Tries to load the blockfile at the path given on initialization;
	//!        loads the record count data if it is available, otherwise creates the file.
	bool initialize() { return m_file.initialize(); }

	//! @brief Tries to save all pending changes to the path given on initialization.
	//! Processes the cache item by item and executes the corresponding insert/modify/erase commands.
	//! Failed commands are lost from the cache.
	//! @returns true if opening the file was successful.
	bool flush() { return m_file.flush(); }

	//! @brief Enqueues an append command into the cache.
	//! @note To save the data on the disk, @c flush must be called.
	//! @returns true if more items can be inserted to the cache. When the cache is full,
	//!          @c flush must be called to empty it.
	bool append(const Record &data)
	{
		bool result = m_file.append(&data);

		if (m_file.isFull())
			return result && m_file.flush();

		return result;
	}

	//! @brief Enqueues a modify command into the cache; modifies the nth record to @c data .
	//! @note To save the data on the disk, @c flush must be called.
	//! @returns true if more items can be inserted to the cache. When the cache is full,
	//!          @c flush must be called to empty it.
	bool modify(uint32_t n, const Record &data)
	{
		bool result = m_file.modify(n, &data);

		if (m_file.isFull())
			return result && m_file.flush();

		return result;
	}

	//! @brief Enqueues a remove command into the cache; erases the nth record.
	//! @note To save the data on the disk, @c flush must be called.
	//! @returns true if more items can be inserted to the cache. When the cache is full,
	//!          @c flush must be called to empty it.
	bool remove(uint32_t n)
	{
		bool result = m_file.erase(n);

		if (m_file.isFull())
			return result && m_file.flush();

		return result;
	}

	//! @brief Tries to look up the n-th record in the buffer. On failure, it reads the file from the storage.
	QueryResult at(uint32_t n)
	{
		QueryResult result = {
		    false,
		    0,
		    Record()};

		result.success = m_file.at(n, &result.value);

		return result;
	}

	template <typename... SearchArgs>
	//! @brief Returns the first record where @c searchFn returns true.
	QueryResult search(uint32_t start, bool reversed, SearchFnPtr<SearchArgs...> searchFn, SearchArgs... searchArgs)
	{
		if (!m_file.beginTransaction())
			return QueryResult{false, 0, Record()};

		QueryResult result = {
		    false,
		    0,
		    Record()};

		for (uint32_t i = reversed ? m_file.getRecordCount() - start : 0;
		     reversed ? i > 0 : i < m_file.getRecordCount();
		     reversed ? --i : ++i) {

			const auto corrected_i = reversed ? i - 1 : i;

			const auto [success, n, nthRecord] = at(corrected_i);
			if (!success)
				continue;

			if (searchFn(corrected_i, nthRecord, searchArgs...)) {
				result = QueryResult{true, corrected_i, nthRecord};
				break;
			}
		}

		m_file.endTransaction();

		return result;
	}

	template <typename... IterArgs>
	//! @brief Runs @c iterFn for every record between @c start and @c end .
	bool iterate(uint32_t start, uint32_t end, bool reversed, IterationFnPtr<IterArgs...> iterFn, IterArgs... iterArgs)
	{
		if (!m_file.beginTransaction())
			return false;

		for (uint32_t i = reversed ? end : start;
		     reversed ? i > start : i < end;
		     reversed ? --i : ++i) {

			const auto corrected_i = reversed ? i - 1 : i;

			auto [success, n, nthRecord] = at(corrected_i);
			if (!success)
				continue;

			iterFn(corrected_i, nthRecord, iterArgs...);
		}

		m_file.endTransaction();

		return true;
	}

	template <typename... SearchArgs>
	//! @brief Removes all records for which @c searchFn returns true.
	uint32_t remove_if(uint32_t start, uint32_t end, SearchFnPtr<SearchArgs...> searchFn, SearchArgs... searchArgs)
	{
		if (!m_file.beginTransaction())
			return false;

		uint32_t result = 0;

		for (uint32_t i = start; i > 0; --i) {
			const auto corrected_i = i - 1;

			auto [success, n, nthRecord] = at(corrected_i);
			if (!success)
				continue;

			if (searchFn(corrected_i, nthRecord, searchArgs...)) {
				++result;

				if (!remove(corrected_i))
					if (!flush())
						break;
			}
		}

		m_file.endTransaction();

		return result;
	}
};

template <typename Record, size_t BufferSize>
Logger Database<Record, BufferSize>::logger = Logger("DB");

#endif // !defined(EKONYV_DATABASE_H)