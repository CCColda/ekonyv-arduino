#ifndef EKONYV_BOOK_DB_H
#define EKONYV_BOOK_DB_H

#include "../storage/database.h"
#include "../types/callback.h"

struct Book {
	constexpr static uint8_t PUBLICLY_WRITABLE = 0b00000001;
	constexpr static uint8_t PUBLICLY_READABLE = 0b00000010;
	constexpr static uint8_t TEMPORARY = 0b00000100;
	constexpr static uint8_t BOOK = 0b00001000;
	constexpr static uint8_t PUBLICATION = 0b00010000;

	uint32_t id;
	uint64_t in;

	uint8_t title_len;
	uint8_t author_len;
	uint8_t published_len;
	uint8_t attrib_len;

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
	ID,
	IN,
	TITLE,
	AUTHORS,
	PUBLISHED,
	ATTRIBUTES,
	CREATED,
	STORAGE_ID,
	USER_ID,
	FLAGS,
	bh_size
};

enum BookSearchType : uint8_t {
	VALUE,
	BETWEEN,
	LIKE,
	bst_size,

	ANY
};

enum BookSearchRelation : uint8_t {
	AND,
	OR,
	AND_NOT,
	OR_NOT,
	bsr_size
};

struct BookSearchTerm {
	BookHeader header;
	BookSearchType search_type;
	BookSearchRelation relation;

	String statement;
};

extern const char *BOOK_HEADERS[bh_size];
extern uint8_t BOOK_HEADER_LENGTHS[bh_size];

extern const char *BOOK_SEARCH_TYPES[bst_size];
extern const char *BOOK_RELATION_TYPES[bsr_size];

extern const char BOOK_FLAGS[8];

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
	uint32_t add(Book partial_book);
	void match(const Vector<BookSearchTerm> &search, SearchCallback callback);
};

#endif // !defined(EKONYV_BOOK_DB_H)