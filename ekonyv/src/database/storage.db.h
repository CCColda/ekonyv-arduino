#ifndef EKONYV_STORAGE_DB_H
#define EKONYV_STORAGE_DB_H

#include "../storage/database.h"
#include "../types/fixedbuffer.h"

#include "search.h"

#include "../types/callback.h"

struct BookStorage {
	uint16_t id;
	char name[64];
	uint8_t name_len;
	uint16_t user_id;

	constexpr static uint16_t INVALID_ID = 0;
};

enum BookStorageHeader : uint8_t {
	SH_ID,
	SH_NAME,
	SH_USER_ID,
	sh_size
};

extern const char *STORAGE_HEADERS[sh_size];
extern uint8_t STORAGE_HEADER_LENGTHS[sh_size];

class StorageDatabase {
public:
	Database<BookStorage, 4> db;

	using SearchCallback = Callback<void, uint32_t, const BookStorage &>;

public:
	StorageDatabase();
	void load();
	void save();

	uint16_t getLastID();

	uint16_t add(const BookStorage &partial_storage);
	decltype(db)::QueryResult getByID(uint16_t id);
	decltype(db)::QueryResult searchSimilarStorage(const BookStorage &partial_storage);

	void match(const Vector<Search::SearchTerm> &search, SearchCallback callback);
};

#endif // !defined(EKONYV_STORAGE_DB_H)