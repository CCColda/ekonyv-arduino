#include "storage.db.h"

#include "../string/string.h"

/* extern */ const char *STORAGE_HEADERS[sh_size] = {
    "id",
    "name",
    "user"};

/* extern */ uint8_t STORAGE_HEADER_LENGTHS[sh_size] = {
    2,
    4,
    4};

namespace {

Search::SearchField STORAGE_FIELD_TYPES[sh_size] = {
    Search::U16,
    Search::S64,
    Search::U16};

Search::offset_t STORAGE_FIELD_OFFSETS[sh_size] = {
    offsetof(BookStorage, BookStorage::id),
    offsetof(BookStorage, BookStorage::name),
    offsetof(BookStorage, BookStorage::user_id)};

struct SearchIteratorData {
	const Vector<Search::SearchTerm> &terms;
	StorageDatabase::SearchCallback &cb;
};

void match_iterator(uint32_t index, const BookStorage &storage, SearchIteratorData *data)
{
	const uint8_t header_sizes[sh_size] = {
	    sizeof(storage.id),
	    storage.name_len};

	if (Search::match(
	        data->terms,
	        (void *)&storage,
	        STORAGE_FIELD_OFFSETS,
	        STORAGE_FIELD_TYPES,
	        header_sizes,
	        sh_size))
		data->cb.call(index, storage);
}

bool matchID(
    uint32_t i, const BookStorage &storage,
    uint16_t id)
{
	return storage.id == id;
}

bool matchSimilar(
    uint32_t i, const BookStorage &storage,
    const BookStorage *partial_storage)
{
	if (storage.id == partial_storage->id)
		return true;

	if (storage.name_len == partial_storage->name_len)
		return Str::compare(storage.name, partial_storage->name, storage.name_len);

	return false;
}
} // namespace

StorageDatabase::StorageDatabase()
    : db(EK_STORAGEDB_PATH)
{
}

void StorageDatabase::load()
{
	db.tryLoad();
}

void StorageDatabase::save()
{
	db.trySave();
}

uint16_t StorageDatabase::getLastID()
{
	if (db.size() == 0) {
		return 0;
	}

	return db.at(db.size() - 1).value.id;
}

uint16_t StorageDatabase::add(const BookStorage &partial_storage)
{
	BookStorage storage_to_add = partial_storage;
	storage_to_add.id = getLastID() + 1;

	return db.append(storage_to_add) ? storage_to_add.id : BookStorage::INVALID_ID;
}

decltype(StorageDatabase::db)::QueryResult StorageDatabase::getByID(uint16_t id)
{
	const auto searchResult = db.search(
	    0, false,
	    matchID,
	    id);

	return searchResult;
}

decltype(StorageDatabase::db)::QueryResult StorageDatabase::searchSimilarStorage(const BookStorage &partial_storage)
{
	const auto searchResult = db.search(
	    0, false,
	    matchSimilar,
	    &partial_storage);

	return searchResult;
}

void StorageDatabase::match(const Vector<Search::SearchTerm> &search, SearchCallback callback)
{
	auto data = SearchIteratorData{
	    search,
	    callback};

	db.iterate(false, 0, db.size(), false, match_iterator, &data);
}
