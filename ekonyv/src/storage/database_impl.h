#ifndef EKONYV_DATABASE_IMPL_H
#define EKONYV_DATABASE_IMPL_H

#include "database_def.h"

template <typename Record, size_t BufferSize>
Database<Record, BufferSize>::Database(const char *path)
    : m_file(path) {}

template <typename Record, size_t BufferSize>
Database<Record, BufferSize>::Database(Database &&other)
    : m_file(static_cast<BufferedBlockfile<sizeof(Record), BufferSize> &&>(other.m_file)) {}

template <typename Record, size_t BufferSize>
bool Database<Record, BufferSize>::initialize() { return m_file.initialize(); }

template <typename Record, size_t BufferSize>
bool Database<Record, BufferSize>::flush() { return m_file.flush(); }

template <typename Record, size_t BufferSize>
bool Database<Record, BufferSize>::append(const Record &data)
{
	bool result = m_file.append(&data);

	if (m_file.isFull())
		return result && m_file.flush();

	return result;
}

template <typename Record, size_t BufferSize>
bool Database<Record, BufferSize>::modify(uint32_t n, const Record &data)
{
	bool result = m_file.modify(n, &data);

	if (m_file.isFull())
		return result && m_file.flush();

	return result;
}

template <typename Record, size_t BufferSize>
bool Database<Record, BufferSize>::remove(uint32_t n)
{
	bool result = m_file.erase(n);

	if (m_file.isFull())
		return result && m_file.flush();

	return result;
}

template <typename Record, size_t BufferSize>
typename Database<Record, BufferSize>::QueryResult Database<Record, BufferSize>::at(uint32_t n)
{
	QueryResult result = {
	    false,
	    0,
	    Record()};

	result.success = m_file.at(n, &result.value);

	return result;
}

template <typename Record, size_t BufferSize>
template <typename... SearchArgs>
typename Database<Record, BufferSize>::QueryResult Database<Record, BufferSize>::search(uint32_t start, SearchFnPtr<SearchArgs...> searchFn, SearchArgs... searchArgs)
{
	if (!m_file.beginTransaction())
		return QueryResult{false, 0, Record()};

	QueryResult result = {
	    false,
	    0,
	    Record()};

	for (uint32_t i = 0; i < m_file.getRecordCount(); ++i) {
		const auto [success, n, nthRecord] = at(i);
		if (!success)
			continue;

		if (searchFn(i, nthRecord, searchArgs...)) {
			result = QueryResult{true, i, nthRecord};
			break;
		}
	}

	m_file.endTransaction();

	return result;
}

template <typename Record, size_t BufferSize>
template <typename... SearchArgs>
typename Database<Record, BufferSize>::QueryResult Database<Record, BufferSize>::search_reverse(uint32_t start, SearchFnPtr<SearchArgs...> searchFn, SearchArgs... searchArgs)
{
	if (!m_file.beginTransaction())
		return QueryResult{false, 0, Record()};

	QueryResult result = {
	    false,
	    0,
	    Record()};

	for (uint32_t i = m_file.getRecordCount() - start; i > 0; --i) {
		const auto corrected_i = i - 1;

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

template <typename Record, size_t BufferSize>
template <typename... IterArgs>
bool Database<Record, BufferSize>::iterate(uint32_t start, uint32_t end, IterationFnPtr<IterArgs...> iterFn, IterArgs... iterArgs)
{
	if (!m_file.beginTransaction())
		return false;

	for (uint32_t i = start; i < end; ++i) {
		auto [success, n, nthRecord] = at(i);
		if (!success)
			continue;

		iterFn(i, nthRecord, iterArgs...);
	}

	m_file.endTransaction();

	return true;
}

template <typename Record, size_t BufferSize>
template <typename... IterArgs>
bool Database<Record, BufferSize>::iterate_reverse(uint32_t start, uint32_t end, IterationFnPtr<IterArgs...> iterFn, IterArgs... iterArgs)
{
	if (!m_file.beginTransaction())
		return false;

	for (uint32_t i = end; i > start; --i) {
		const auto corrected_i = i - 1;

		auto [success, n, nthRecord] = at(corrected_i);
		if (!success)
			continue;

		iterFn(corrected_i, nthRecord, iterArgs...);
	}

	m_file.endTransaction();

	return true;
}

template <typename Record, size_t BufferSize>
template <typename... SearchArgs>
uint32_t Database<Record, BufferSize>::remove_if(uint32_t start, uint32_t end, SearchFnPtr<SearchArgs...> searchFn, SearchArgs... searchArgs)
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

#endif // !defined(EKONYV_DATABASE_IMPL_H)