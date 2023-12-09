#include "book.db.h"

#include "../config.h"

#include "../global/global.h"
#include "../string/string.h"

#include <Regexp.h>

namespace {
Search::SearchField BOOK_FIELD_TYPES[bh_size] = {
    Search::U32,
    Search::U64,
    Search::S128,
    Search::S64,
    Search::S32,
    Search::S32,
    Search::U32,
    Search::U16,
    Search::U16,
    Search::U8};

Search::offset_t BOOK_FIELD_OFFSETS[bh_size] = {
    offsetof(Book, Book::id),
    offsetof(Book, Book::in),
    offsetof(Book, Book::title),
    offsetof(Book, Book::authors),
    offsetof(Book, Book::published),
    offsetof(Book, Book::attributes),
    offsetof(Book, Book::created),
    offsetof(Book, Book::storage_id),
    offsetof(Book, Book::user_id),
    offsetof(Book, Book::flags)};

struct SearchIteratorData {
	const Vector<Search::SearchTerm> &terms;
	BookDatabase::SearchCallback &cb;
};

void match_iterator(uint32_t index, const Book &book, SearchIteratorData *data)
{
	const uint8_t header_sizes[bh_size] = {
	    sizeof(book.id),
	    sizeof(book.in),
	    book.title_len,
	    book.authors_len,
	    book.published_len,
	    book.attributes_len,
	    sizeof(book.created),
	    sizeof(book.storage_id),
	    sizeof(book.user_id),
	    sizeof(book.flags)};

	if (Search::match(
	        data->terms,
	        (void *)&book,
	        BOOK_FIELD_OFFSETS,
	        BOOK_FIELD_TYPES,
	        header_sizes,
	        bh_size))
		data->cb.call(index, book);
}

bool similar_search(uint32_t index, const Book &book, const Book *compare_book)
{
	if (book.in == compare_book->in)
		return true;

	if (book.title_len == compare_book->title_len && book.authors_len == compare_book->authors_len)
		if (Str::compare(book.title, compare_book->title, book.title_len) &&
		    Str::compare(book.authors, compare_book->authors, book.authors_len))
			return true;

	return false;
}

bool id_search(uint32_t idx, const Book &book, uint32_t id)
{
	return book.id == id;
}

bool userid_search(uint32_t idx, const Book &book, uint16_t user_id)
{
	return book.user_id == user_id;
}

bool storageid_search(uint32_t idx, const Book &book, uint16_t storage_id)
{
	return book.storage_id == storage_id;
}
} // namespace

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

/* extern */ uint8_t BOOK_HEADER_LENGTHS[] = {
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

BookDatabase::BookDatabase()
    : db(EK_BOOKDB_PATH)
{
}

void BookDatabase::load()
{
	db.initialize();
}

void BookDatabase::save()
{
	db.flush();
}

uint32_t BookDatabase::getLastID()
{
	if (db.size() == 0)
		return 0;

	return db.at(db.size() - 1).value.id;
}

void BookDatabase::match(const Vector<Search::SearchTerm> &search, SearchCallback callback)
{
	auto data = SearchIteratorData{
	    search,
	    callback};

	db.iterate(0, db.size(), match_iterator, &data);
}

void BookDatabase::removeAllOfUser(uint16_t user_id)
{
	db.remove_if(0, db.size(), userid_search, user_id);
}

void BookDatabase::removeAllOfStorage(uint16_t storage_id)
{
	db.remove_if(0, db.size(), storageid_search, storage_id);
}

uint32_t BookDatabase::add(const Book &partial_book)
{
	if (partial_book.title_len == 0 ||
	    partial_book.authors_len == 0)
		return 0;

	const auto id = getLastID() + 1;

	Book book_to_add = partial_book;

	book_to_add.id = id;
	book_to_add.created = global::time();

	db.append(book_to_add);

	return id;
}

decltype(BookDatabase::db)::QueryResult BookDatabase::searchSimilarBook(const Book &partial_book)
{
	return db.search_reverse(0, similar_search, &partial_book);
}

decltype(BookDatabase::db)::QueryResult BookDatabase::getByID(uint32_t id)
{
	return db.search_reverse(0, id_search, id);
}
