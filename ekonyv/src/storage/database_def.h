#ifndef EKONYV_DATABASE_DEF_H
#define EKONYV_DATABASE_DEF_H

#include "buffered_blockfile.h"

#include <type_traits>

/**
 * @brief Defines a fixed-size record database.
 *
 * @tparam Record the type of the record
 */
template <typename Record, size_t BufferSize>
class Database {
	static_assert(std::is_trivial<Record>::value, "Record type must be trivial.");

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
	BufferedBlockfile<sizeof(Record), BufferSize> m_file;

public:
	Database(const char *path);
	Database(Database &&other);
	~Database() = default;

	//! @brief Returns the number of records stored in the database.
	constexpr uint32_t size() const { return m_file.getRecordCount(); }

	//! @brief Tries to load the blockfile at the path given on initialization;
	//!        loads the record count data if it is available, otherwise creates the file.
	//! @remark wrapper for @c BufferedBlockfile::initialize
	bool initialize();

	//! @brief Tries to save all pending changes to the path given on initialization.
	//! Processes the cache item by item and executes the corresponding insert/modify/erase commands.
	//! Failed commands are lost from the cache.
	//! @returns true if opening the file was successful.
	//! @remark wrapper for @c BufferedBlockfile::flush
	bool flush();

	//! @brief Enqueues an append command into the cache.
	//! @note To save the data on the disk, @c flush must be called.
	//! @returns true if more items can be inserted to the cache. When the cache is full,
	//!          @c flush must be called to empty it.
	bool append(const Record &data);

	//! @brief Enqueues a modify command into the cache; modifies the nth record to @c data .
	//! @note To save the data on the disk, @c flush must be called.
	//! @returns true if more items can be inserted to the cache. When the cache is full,
	//!          @c flush must be called to empty it.
	bool modify(uint32_t n, const Record &data);

	//! @brief Enqueues a remove command into the cache; erases the nth record.
	//! @note To save the data on the disk, @c flush must be called.
	//! @returns true if more items can be inserted to the cache. When the cache is full,
	//!          @c flush must be called to empty it.
	bool remove(uint32_t n);

	//! @brief Tries to look up the n-th record in the buffer. On failure, it reads the file from the storage.
	QueryResult at(uint32_t n);

	template <typename... SearchArgs>
	//! @brief Returns the first record where @c searchFn returns true.
	QueryResult search(uint32_t start, SearchFnPtr<SearchArgs...> searchFn, SearchArgs... searchArgs);

	template <typename... SearchArgs>
	//! @brief Returns the last record where @c searchFn returns true.
	QueryResult search_reverse(uint32_t start, SearchFnPtr<SearchArgs...> searchFn, SearchArgs... searchArgs);

	template <typename... IterArgs>
	//! @brief Runs @c iterFn for every record between @c start and @c end .
	bool iterate(uint32_t start, uint32_t end, IterationFnPtr<IterArgs...> iterFn, IterArgs... iterArgs);

	template <typename... IterArgs>
	//! @brief Runs @c iterFn for every record between @c start and @c end in reverse order.
	bool iterate_reverse(uint32_t start, uint32_t end, IterationFnPtr<IterArgs...> iterFn, IterArgs... iterArgs);

	template <typename... SearchArgs>
	//! @brief Removes all records for which @c searchFn returns true.
	uint32_t remove_if(uint32_t start, uint32_t end, SearchFnPtr<SearchArgs...> searchFn, SearchArgs... searchArgs);
};

#endif // !defined(EKONYV_DATABASE_DEF_H)