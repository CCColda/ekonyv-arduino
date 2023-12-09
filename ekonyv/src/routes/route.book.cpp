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

	const auto in = ParameterMiddleware(BOOK_HEADERS[BH_IN], BOOK_HEADER_LENGTHS[BH_IN], path, prep);
	const auto title = ParameterMiddleware(BOOK_HEADERS[BH_TITLE], BOOK_HEADER_LENGTHS[BH_TITLE], path, prep);
	const auto authors = ParameterMiddleware(BOOK_HEADERS[BH_AUTHORS], BOOK_HEADER_LENGTHS[BH_AUTHORS], path, prep);
	const auto published = ParameterMiddleware(BOOK_HEADERS[BH_PUBLISHED], BOOK_HEADER_LENGTHS[BH_PUBLISHED], path, prep);
	const auto attributes = ParameterMiddleware(BOOK_HEADERS[BH_ATTRIBUTES], BOOK_HEADER_LENGTHS[BH_ATTRIBUTES], path, prep);
	const auto storage_id = ParameterMiddleware(BOOK_HEADERS[BH_STORAGE_ID], BOOK_HEADER_LENGTHS[BH_STORAGE_ID], path, prep);
	const auto flags = ParameterMiddleware(BOOK_HEADERS[BH_FLAGS], BOOK_HEADER_LENGTHS[BH_FLAGS], path, prep);
	const auto id = ParameterMiddleware(BOOK_HEADERS[BH_ID], BOOK_HEADER_LENGTHS[BH_ID], path, prep);

	result.id = id ? Str::fixedAtoi<uint32_t>(SizedString::fromString(id.value)) : 0;

	result.in = in ? Str::fixedAtoi<uint64_t>(SizedString::fromString(in.value)) : 0;
	result.storage_id = storage_id ? Str::fixedAtoi<uint16_t>(SizedString::fromString(storage_id.value)) : 0;

	result.flags = flags ? Str::fixedAtoi<uint8_t>(SizedString::fromString(flags.value)) : 0u;

	if (title) {
		const auto decoded = Str::urlDecode(SizedString::fromString(title.value));

		result.title_len = min(decoded.length(), sizeof(Book::title));
		memcpy(result.title, decoded.c_str(), result.title_len);
	}

	if (authors) {
		const auto decoded = Str::urlDecode(SizedString::fromString(authors.value));

		result.authors_len = min(decoded.length(), sizeof(Book::authors));
		memcpy(result.authors, decoded.c_str(), result.authors_len);
	}

	if (published) {
		const auto decoded = Str::urlDecode(SizedString::fromString(published.value));

		result.published_len = min(decoded.length(), sizeof(Book::published));
		memcpy(result.published, decoded.c_str(), result.published_len);
	}

	if (attributes) {
		const auto decoded = Str::urlDecode(SizedString::fromString(attributes.value));

		result.attributes_len = min(decoded.length(), sizeof(Book::attributes));
		memcpy(result.attributes, decoded.c_str(), result.attributes_len);
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
	client->print((uint32_t)book.flags);

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

	if ((session.user.flags & User::CAN_WRITE) == 0) {
		HTTPServer::writeStaticHTMLResponse(HTTPResponse::HTML_UNAUTHORIZED, client);
		return 0;
	}

	Book book = getBookFromParameters(path, prep);

	if (book.id == 0) {
		const auto similar_book = global::db.book.searchSimilarBook(book);
		if (similar_book.success) {
			HTTPServer::writeStaticHTMLResponse(
			    HTTPResponse::StaticHTMLResponse{
			        400, "Bad Request",
			        "400 - Bad Request",
			        "A similar book already exists."},
			    client);
			return 0;
		}

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

		if (!book_info.success) {
			HTTPServer::writeStaticHTMLResponse(HTTPResponse::HTML_INVALID_BOOK, client);
			return 0;
		}

		if (book_info.value.user_id != session.user_id && (book_info.value.flags & Book::PUBLICLY_WRITABLE) != 0) {
			HTTPServer::writeStaticHTMLResponse(HTTPResponse::HTML_BOOK_WRITE_PROTECTED, client);
			return 0;
		}

		auto result_book = book_info.value;

		if (book.user_id == session.user_id)
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
	const auto prep = ParameterMiddleware::preparePath(path);
	const auto session = SessionMiddleware(path, true, prep);

	if (!session)
		return session.sendInvalidResponse(client);

	if ((session.user.flags & User::CAN_WRITE) == 0) {
		HTTPServer::writeStaticHTMLResponse(HTTPResponse::HTML_UNAUTHORIZED, client);
		return 0;
	}

	const auto id = ParameterMiddleware("id", 2, path, prep);

	if (!id)
		return id.sendMissingResponse(client);

	const auto stored_book = global::db.book.getByID(Str::fixedAtoi<uint16_t>(SizedString::fromString(id.value)));

	if (!stored_book.success) {
		HTTPServer::writeStaticHTMLResponse(HTTPResponse::HTML_INVALID_BOOK, client);
		return 0;
	}

	if (stored_book.value.user_id != session.user_id && (stored_book.value.flags & Book::PUBLICLY_WRITABLE) != 0) {
		HTTPServer::writeStaticHTMLResponse(HTTPResponse::HTML_BOOK_WRITE_PROTECTED, client);
		return 0;
	}

	global::db.book.db.remove(stored_book.index);

	HTTPServer::writeHTTPHeaders(200, "OK", "text/csv", client);
	client.println("key,value");
	client.println("state,success");

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
	global::db.book.db.iterate(0, global::db.book.db.size(), sendBookIfPublic, &args);

	return 0;
}

int getBookHandler(const String &path, const Vector<HTTP::ClientHeaderPair> &headers, EthernetClient &client)
{
	const auto prep = ParameterMiddleware::preparePath(path);
	const auto session = SessionMiddleware(path, false, prep);

	if (!session)
		return session.sendInvalidResponse(client);

	Search::SearchTerm terms_buf[EK_MAX_SEARCH_TERMS] = {};
	Vector<Search::SearchTerm> terms;

	terms.setStorage<EK_MAX_SEARCH_TERMS>(terms_buf);

	const auto num_search_terms = Search::parseSearchTermsFromURL(path, prep, BOOK_HEADERS, bh_size, terms);

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

	server.on(HTTP::DELETE, "/api/book", HTTPServer::HandlerBehavior::ALLOW_PARAMETERS, deleteBookHandler);
}

} // namespace BookRoute