#include "book.db.h"

#include "../config.h"

#include "../global/global.h"
#include "../string/string.h"

#include <Regexp.h>

namespace {
enum BookFieldType : uint8_t {
	S128,
	S64,
	S32,
	FLAG,
	U16,
	U32,
	U64
};

BookFieldType BOOK_FIELD_TYPES[bh_size] = {
    U32,
    U64,
    S128,
    S64,
    S32,
    S32,
    U32,
    U16,
    U16,
    FLAG};

uint16_t BOOK_FIELD_OFFSETS[bh_size] = {
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
	const Vector<BookSearchTerm> &terms;
	BookDatabase::SearchCallback &cb;
};

struct Between {
	uint64_t low;
	uint64_t high;
};

Between parseBetween(const String &str)
{
	const auto dash = str.indexOf('-');
	if (dash == -1)
		return Between{0, 0};

	return Between{
	    Str::fixedAtoi<uint64_t>(str.c_str(), dash),
	    Str::fixedAtoi<uint64_t>(str.c_str() + dash + 1, str.length() - dash - 1)};
}

uint8_t getFieldSize(const Book &book, BookHeader header)
{
	switch (BOOK_FIELD_TYPES[header]) {
		case U16:
			return sizeof(uint16_t);
		case U32:
			return sizeof(uint32_t);
		case U64:
			return sizeof(uint64_t);
		case FLAG:
			return sizeof(uint8_t);
		case S128: {
			switch (header) {
				case TITLE:
					return book.title_len;
				default:
					return 0;
			}
		}
		case S64: {
			switch (header) {
				case AUTHORS:
					return book.author_len;
				default:
					return 0;
			}
		}
		case S32: {
			switch (header) {
				case PUBLISHED:
					return book.published_len;
				case ATTRIBUTES:
					return book.attrib_len;
				default:
					return 0;
			}
		}
		default:
			return 0;
	}
}

void search_iterator(uint32_t index, const Book &book, SearchIteratorData *data)
{
	bool match = true;

	for (uint8_t i = 0; i < data->terms.size(); ++i) {
		const auto &term = data->terms.at(i);
		bool term_match = false;

		auto *const header_address = ((byte *)&book) + BOOK_FIELD_OFFSETS[term.header];
		const auto field_type = BOOK_FIELD_TYPES[term.header];

		switch (field_type) {
			case S128:
			case S64:
			case S32: {
				const auto length = getFieldSize(book, term.header);

				if (term.search_type == VALUE) {
					if (term.statement.length() == length) {
						term_match = Str::compare((char *)header_address, term.statement.c_str(), length);
					}
				}
				else if (term.search_type == LIKE) {
					Serial.println("like");
					Serial.println(term.statement);
					auto ms = MatchState();
					ms.Target((char *)header_address, length);

					term_match = ms.Match(term.statement.c_str(), 0) == REGEXP_MATCHED;
				}

				break;
			}
			case U64:
			case U32:
			case U16: {
				const uint64_t field_value =
				    field_type == U16
				        ? *(uint16_t *)header_address
				        : (field_type == U32
				               ? *(uint32_t *)header_address
				               : *(uint64_t *)header_address);

				if (term.search_type == VALUE) {
					const uint64_t value = Str::fixedAtoi<uint64_t>(term.statement.c_str(), term.statement.length());
					term_match = value == field_value;
				}
				else if (term.search_type == BETWEEN) {
					const Between btw = parseBetween(term.statement);
					term_match = btw.low <= field_value && field_value <= btw.high;
				}

				break;
			}
			case FLAG: {
				bool flag_match = true;
				for (uint8_t i = 0; i < min(term.statement.length(), 8); ++i) {
					const auto flag_idx = Str::find(BOOK_FLAGS, sizeof(BOOK_FLAGS), term.statement[i]);
					if (flag_idx == Str::NOT_FOUND) {
						flag_match = false;
						break;
					}

					if ((book.flags & (1 << flag_idx)) == 0) {
						flag_match = false;
						break;
					}
				}

				term_match = flag_match;

				break;
			}
		}

		switch (term.relation) {
			case AND: {
				match = match && term_match;
				break;
			}
			case AND_NOT: {
				match = match && !term_match;
				break;
			}
			case OR: {
				match = match || term_match;
				break;
			}
			case OR_NOT: {
				match = match || !term_match;
				break;
			}
		}
	}

	if (match)
		data->cb.call(index, book);
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

/* extern */ const char *BOOK_SEARCH_TYPES[bst_size] = {
    "v",
    "btw",
    "like"};

/* extern */ const char *BOOK_RELATION_TYPES[bsr_size] = {
    "and",
    "or",
    "and_not",
    "or_not"};

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

void BookDatabase::match(const Vector<BookSearchTerm> &search, SearchCallback callback)
{
	auto data = SearchIteratorData{
	    search,
	    callback};

	db.iterate(false, 0, db.size(), false, search_iterator, &data);
}