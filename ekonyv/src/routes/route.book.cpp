#include "route.book.h"
#include "../global/global.h"
#include "../middleware/parameter.mw.h"
#include "../middleware/session.mw.h"

#include "../string/string.h"
#include "../string/to_string.h"
#include "../string/url.h"

namespace {

struct SendIfPublicArgs {
	EthernetClient *client;
	uint16_t user_id;
};

Book getBookFromParameters(const String &path, uint32_t prep)
{
	auto result = Book();

	const auto in = ParameterMiddleware(BOOK_HEADERS[BookHeader::IN], BOOK_HEADER_LENGTHS[BookHeader::IN], path, prep);
	const auto title = ParameterMiddleware(BOOK_HEADERS[BookHeader::TITLE], BOOK_HEADER_LENGTHS[BookHeader::TITLE], path, prep);
	const auto authors = ParameterMiddleware(BOOK_HEADERS[BookHeader::AUTHORS], BOOK_HEADER_LENGTHS[BookHeader::AUTHORS], path, prep);
	const auto published = ParameterMiddleware(BOOK_HEADERS[BookHeader::PUBLISHED], BOOK_HEADER_LENGTHS[BookHeader::PUBLISHED], path, prep);
	const auto attributes = ParameterMiddleware(BOOK_HEADERS[BookHeader::ATTRIBUTES], BOOK_HEADER_LENGTHS[BookHeader::ATTRIBUTES], path, prep);
	const auto storage_id = ParameterMiddleware(BOOK_HEADERS[BookHeader::STORAGE_ID], BOOK_HEADER_LENGTHS[BookHeader::STORAGE_ID], path, prep);
	const auto flags = ParameterMiddleware(BOOK_HEADERS[BookHeader::FLAGS], BOOK_HEADER_LENGTHS[BookHeader::FLAGS], path, prep);
	const auto id = ParameterMiddleware(BOOK_HEADERS[BookHeader::ID], BOOK_HEADER_LENGTHS[BookHeader::ID], path, prep);

	result.id = id ? Str::fixedAtoi<uint32_t>(id.value.c_str(), id.value.length()) : 0;

	result.in = in ? Str::fixedAtoi<uint64_t>(in.value.c_str(), in.value.length()) : 0;
	result.storage_id = storage_id ? Str::fixedAtoi<uint16_t>(storage_id.value.c_str(), storage_id.value.length()) : 0;

	if (title) {
		const auto decoded = Str::urlDecode(title.value.c_str(), title.value.length());

		result.title_len = min(decoded.length(), sizeof(Book::title));
		memcpy(result.title, decoded.c_str(), result.title_len);
	}

	if (authors) {
		const auto decoded = Str::urlDecode(authors.value.c_str(), authors.value.length());

		result.authors_len = min(decoded.length(), sizeof(Book::authors));
		memcpy(result.authors, decoded.c_str(), result.authors_len);
	}

	if (published) {
		const auto decoded = Str::urlDecode(published.value.c_str(), published.value.length());

		result.published_len = min(decoded.length(), sizeof(Book::published));
		memcpy(result.published, decoded.c_str(), result.published_len);
	}

	if (attributes) {
		const auto decoded = Str::urlDecode(attributes.value.c_str(), attributes.value.length());

		result.attributes_len = min(decoded.length(), sizeof(Book::attributes));
		memcpy(result.attributes, decoded.c_str(), result.attributes_len);
	}

	if (flags)
		for (uint32_t i = 0; i < flags.value.length(); ++i)
			for (uint8_t j = 0; j < sizeof(BOOK_FLAGS); ++j)
				if (flags.value[i] == BOOK_FLAGS[j])
					result.flags |= (0b1 << j);

	return result;
}

size_t countSearchTerms(const String &path, uint32_t prep)
{
	size_t result = 0;

	auto search_string = str('s', result);
	auto search_param = ParameterMiddleware(search_string.c_str(), search_string.length(), path, prep);

	while (search_param) {
		++result;

		search_string = str('s', result);
		search_param = ParameterMiddleware(search_string.c_str(), search_string.length(), path, prep);
	}

	return result;
}

BookSearchTerm parseSearchTerm(const String &path, uint32_t prep, size_t term_index)
{
	auto result = BookSearchTerm{};

	const auto search_param = ParameterMiddleware(str('s', term_index), path, prep);

	const auto header_index = Str::compareToMap(
	    search_param.value.c_str(), search_param.value.length(),
	    BOOK_HEADERS, sizeof(BOOK_HEADERS) / sizeof(BOOK_HEADERS[0]));

	if (header_index == Str::NOT_FOUND)
		return result;

	result.header = (BookHeader)header_index;
	result.search_type = BookSearchType::ANY;
	result.statement = "";
	result.relation = BookSearchRelation::AND;

	for (uint8_t i = 0; i < bst_size; ++i) {
		const auto search_param = ParameterMiddleware(str(BOOK_SEARCH_TYPES[i], term_index), path, prep);
		if (search_param) {
			result.search_type = (BookSearchType)i;
			result.statement = Str::urlDecode(search_param.value.c_str(), search_param.value.length());
			break;
		}
	}

	const auto relation_param = ParameterMiddleware(str('r', term_index), path, prep);

	if (relation_param) {
		const auto relation_index = Str::compareToMap(
		    relation_param.value.c_str(), relation_param.value.length(),
		    BOOK_RELATION_TYPES, bsr_size);

		if (relation_index != Str::NOT_FOUND)
			result.relation = (BookSearchRelation)relation_index;
	}

	return result;
}

void sendBook(uint32_t i, const Book &book, EthernetClient *client)
{
	client->print(book.id);
	client->print(',');
	client->print(book.in);
	client->print(",\"");
	client->write(book.title, book.title_len);
	client->print("\",\"");
	client->write(book.authors, book.authors_len);
	client->print("\",\"");
	client->write(book.published, book.published_len);
	client->print("\",\"");
	client->write(book.attributes, book.attributes_len);
	client->print("\",");
	client->print(book.created);
	client->print(',');
	client->print(book.storage_id);
	client->print(',');
	client->print(book.user_id);
	client->print(',');

	for (uint8_t i = 0; i < 8; ++i) {
		const auto flag = (book.flags & (0b1 << i)) != 0;

		if (flag)
			client->print((char)BOOK_FLAGS[i]);
	}

	client->println();
}

void sendBookIfPublic(uint32_t i, const Book &book, SendIfPublicArgs *args)
{
	if (!(book.user_id != args->user_id && (book.flags & Book::PUBLICLY_READABLE) != 0))
		sendBook(i, book, args->client);
}

} // namespace

namespace BookRoute {
int postBookHandler(const String &path, const Vector<HTTP::ClientHeaderPair> &headers, EthernetClient &client)
{
	const auto prep = ParameterMiddleware::preparePath(path);
	const auto session = SessionMiddleware(path, true, prep);

	if (!session)
		return session.sendInvalidResponse(client);

	Book book = getBookFromParameters(path, prep);
	const auto similar_book = global::db.book.searchSimilarBook(book);
	if (similar_book.state == QueryState::SUCCESS) {
		HTTPServer::writeStaticHTMLResponse(
		    HTTPResponse::StaticHTMLResponse{
		        400, "Bad Request",
		        "400 - Bad Request",
		        "A similar book already exists."},
		    client);
		return 0;
	}

	if (book.id == 0) {
		book.user_id = session.user_id;

		const auto id = global::db.book.add(book);

		if (id == 0) {
			HTTPServer::writeStaticHTMLResponse(
			    HTTPResponse::StaticHTMLResponse{
			        400, "Bad Request",
			        "400 - Bad Request",
			        "Failed adding book."},
			    client);
			return 0;
		}

		HTTPServer::writeHTTPHeaders(200, "OK", "text/csv", client);
		client.println("key,value");
		client.println("state,success");
		client.print("id,");
		client.println(id);

		return 0;
	}
	else {
		const auto book_info = global::db.book.getByID(book.id);

		if (book_info.state == QueryState::ERROR) {
			HTTPServer::writeStaticHTMLResponse(
			    HTTPResponse::StaticHTMLResponse{
			        400, "Bad Request",
			        "400 - Bad Request",
			        "The book with the given ID doesn't exist."},
			    client);
			return 0;
		}

		if (book_info.value.user_id != session.user_id && (book_info.value.flags & Book::PUBLICLY_WRITABLE) != 0) {
			HTTPServer::writeStaticHTMLResponse(
			    HTTPResponse::StaticHTMLResponse{
			        400, "Bad Request",
			        "400 - Bad Request",
			        "The book must be edited by its uploader."},
			    client);
			return 0;
		}

		auto result_book = book_info.value;

		result_book.flags = book.flags;

#define EK_COPY_STR_IF_VALID(s)                             \
	if (book.s##_len != 0) {                                \
		result_book.s##_len = book.s##_len;                 \
		memcpy(result_book.s, book.s, result_book.s##_len); \
	}

#define EK_COPY_NUM_IF_VALID(n) \
	if (book.n != 0)            \
		result_book.n = book.n;

		EK_COPY_STR_IF_VALID(title);
		EK_COPY_STR_IF_VALID(authors);
		EK_COPY_STR_IF_VALID(published);
		EK_COPY_STR_IF_VALID(attributes);

		EK_COPY_NUM_IF_VALID(storage_id);
		EK_COPY_NUM_IF_VALID(in);

#undef EK_COPY_STR_IF_VALID
#undef EK_COPY_NUM_IF_VALID

		global::db.book.db.modify(book_info.index, result_book);

		HTTPServer::writeHTTPHeaders(200, "OK", "text/csv", client);
		client.println("key,value");
		client.println("state,success");
		client.print("id,");
		client.println(result_book.id);

		return 0;
	}
}

int deleteBookHandler(const String &path, const Vector<HTTP::ClientHeaderPair> &headers, EthernetClient &client)
{
	return 0;
}

int getBookCountHandler(const String &path, const Vector<HTTP::ClientHeaderPair> &headers, EthernetClient &client)
{
	const auto prep = ParameterMiddleware::preparePath(path);
	const auto session = SessionMiddleware(path, false, prep);

	if (!session)
		return session.sendInvalidResponse(client);

	HTTPServer::writeHTTPHeaders(200, "OK", "text/csv", client);
	client.println("key,value");
	client.print("count,");
	client.println(global::db.book.db.size());
	client.print("last,");
	client.println(global::db.book.getLastID());

	return 0;
}

int getAllBooksHandler(const String &path, const Vector<HTTP::ClientHeaderPair> &headers, EthernetClient &client)
{
	const auto prep = ParameterMiddleware::preparePath(path);
	const auto session = SessionMiddleware(path, false, prep);

	if (!session)
		return session.sendInvalidResponse(client);

	HTTPServer::writeHTTPHeaders(200, "OK", "text/csv", client);

	for (uint8_t i = 0; i < bh_size; ++i) {
		client.print(BOOK_HEADERS[i]);

		if (i < bh_size - 1)
			client.print(',');
	}
	client.println();

	SendIfPublicArgs args = {&client, session.user_id};
	global::db.book.db.iterate(false, 0, global::db.book.db.size(), false, sendBookIfPublic, &args);

	return 0;
}

int getBookHandler(const String &path, const Vector<HTTP::ClientHeaderPair> &headers, EthernetClient &client)
{
	const auto prep = ParameterMiddleware::preparePath(path);
	const auto session = SessionMiddleware(path, false, prep);

	if (!session)
		return session.sendInvalidResponse(client);

	const auto num_search_terms = countSearchTerms(path, prep);

	if (num_search_terms == 0) {
		HTTPServer::writeStaticHTMLResponse(
		    HTTPResponse::StaticHTMLResponse{
		        400,
		        "Bad Request",
		        "400 - Bad Request",
		        "No search terms were provided."},
		    client);
		return 0;
	}

	if (num_search_terms > EK_MAX_BOOK_SEARCH_TERMS) {
		HTTPServer::writeStaticHTMLResponse(
		    HTTPResponse::StaticHTMLResponse{
		        400,
		        "Bad Request",
		        "400 - Bad Request",
		        "Only up to " __EK_MACRO_STRING(EK_MAX_BOOK_SEARCH_TERMS) " search terms can be provided."},
		    client);
		return 0;
	}

	BookSearchTerm terms_buf[EK_MAX_BOOK_SEARCH_TERMS] = {};
	Vector<BookSearchTerm> terms;
	terms.setStorage<EK_MAX_BOOK_SEARCH_TERMS>(terms_buf);

	for (uint8_t i = 0; i < num_search_terms; ++i)
		terms.push_back(parseSearchTerm(path, prep, i));

	HTTPServer::writeHTTPHeaders(200, "OK", "text/csv", client);

	for (uint8_t i = 0; i < bh_size; ++i) {
		client.print(BOOK_HEADERS[i]);

		if (i < bh_size - 1)
			client.print(',');
	}
	client.println();

	SendIfPublicArgs args = {&client, session.user_id};

	BookDatabase::SearchCallback cb;
	cb.set(sendBookIfPublic, &args);
	global::db.book.match(terms, cb);

	return 0;
}

void registerRoute(HTTPServer &server)
{
	server.on(HTTP::GET, "/api/book/count", HTTPServer::HandlerBehavior::ALLOW_PARAMETERS, getBookCountHandler);
	server.on(HTTP::GET, "/api/book/all", HTTPServer::HandlerBehavior::ALLOW_PARAMETERS, getAllBooksHandler);

	server.on(HTTP::GET, "/api/book", HTTPServer::HandlerBehavior::ALLOW_PARAMETERS, getBookHandler);
	server.on(HTTP::POST, "/api/book", HTTPServer::HandlerBehavior::ALLOW_PARAMETERS, postBookHandler);

	server.on(HTTP::DELETE, "/api/book", HTTPServer::HandlerBehavior::ALLOW_PARAMETERS, getBookHandler);
}

} // namespace BookRoute