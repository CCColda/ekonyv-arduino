#include "book.db.h"

#include "../config.h"

#include "../global/global.h"

/* extern */ const char *BOOK_HEADERS[] = {
    "id",
    "in",
    "title",
    "authors",
    "published",
    "attributes",
    "created",
    "storage_id",
    "user_id",
    "flags"};

/* extern */ uint8_t BOOK_HEADER_LENGTHS[bh_size] = {
    2,
    2,
    5,
    7,
    9,
    10,
    7,
    10,
    7,
    5};

/* extern */ BookHeaderType BOOK_HEADER_TYPES[bh_size] = {
    I32,
    I64,
    S128,
    S64,
    S32,
    S32,
    I32,
    I16,
    I16,
    FLAG};

/* extern */ const char BOOK_FLAGS[8] = {
    'w', 'r', 't', 'b', 'p', '\0', '\0', '\0'};

BookDatabase::BookDatabase()
    : db(EK_BOOKDB_PATH)
{
}

void BookDatabase::load()
{
	db.tryLoad();
}

void BookDatabase::save()
{
	db.trySave();
}

uint32_t BookDatabase::getLastID()
{
	if (db.size() == 0)
		return 0;

	return db.at(db.size() - 1).value.id;
}

uint32_t BookDatabase::add(Book partial_book)
{
	if (partial_book.title_len == 0 ||
	    partial_book.author_len == 0)
		return 0;

	const auto id = getLastID() + 1;

	Book book_to_add = partial_book;

	book_to_add.id = id;
	book_to_add.created = global::time();

	db.append(book_to_add);

	return id;
}