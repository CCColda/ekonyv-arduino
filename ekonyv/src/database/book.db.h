#ifndef EKONYV_BOOK_DB_H
#define EKONYV_BOOK_DB_H

#include "../storage/database.h"
#include "../types/callback.h"
#include "search.h"

struct Book {
	constexpr static uint8_t PUBLICLY_WRITABLE = 0b00000001;
	constexpr static uint8_t PUBLICLY_READABLE = 0b00000010;
	constexpr static uint8_t TEMPORARY = 0b00000100;
	constexpr static uint8_t BOOK = 0b00001000;
	constexpr static uint8_t PUBLICATION = 0b00010000;

	uint32_t id;
	uint64_t in;

	uint8_t title_len;
	uint8_t authors_len;
	uint8_t published_len;
	uint8_t attributes_len;

	char title[128];
	char authors[64];
	char published[32];
	char attributes[32];

	unsigned long created;
	uint16_t storage_id;
	uint16_t user_id;

	uint8_t flags;
};

enum BookHeader : uint8_t {
	BH_ID,
	BH_IN,
	BH_TITLE,
	BH_AUTHORS,
	BH_PUBLISHED,
	BH_ATTRIBUTES,
	BH_CREATED,
	BH_STORAGE_ID,
	BH_USER_ID,
	BH_FLAGS,
	bh_size
};

extern const char *BOOK_HEADERS[bh_size];
extern uint8_t BOOK_HEADER_LENGTHS[bh_size];

class BookDatabase {
public:
	Database<Book, 4> db;

	struct BookResult {
		bool success;
		Book book;
	};

	using SearchCallback = Callback<void, uint32_t, const Book &>;

public:
	BookDatabase();
	void load();
	void save();

	uint32_t getLastID();
	void match(const Vector<Search::SearchTerm> &search, SearchCallback callback);

	void removeAllOfUser(uint16_t user_id);

	uint32_t add(const Book &partial_book);
	decltype(db)::QueryResult searchSimilarBook(const Book &partial_book);
	decltype(db)::QueryResult getByID(uint32_t id);
};

#endif // !defined(EKONYV_BOOK_DB_H)