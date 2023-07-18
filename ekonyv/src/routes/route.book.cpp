#include "route.book.h"
#include "../global/global.h"
#include "../middleware/parameter.mw.h"
#include "../middleware/session.mw.h"

#include "../string/string.h"
#include "../string/to_string.h"

namespace {

struct BookFromParametersOptions {
	uint8_t ignore_id : 1;
	uint8_t ignore_user_id : 1;
};

Book getBookFromParameters(const String &path, uint32_t prep, BookFromParametersOptions opts = {0, 0})
{
	auto result = Book();

	const auto in = ParameterMiddleware(BOOK_HEADERS[BookHeader::IN], BOOK_HEADER_LENGTHS[BookHeader::IN], path, prep);
	const auto title = ParameterMiddleware(BOOK_HEADERS[BookHeader::TITLE], BOOK_HEADER_LENGTHS[BookHeader::TITLE], path, prep);
	const auto authors = ParameterMiddleware(BOOK_HEADERS[BookHeader::AUTHORS], BOOK_HEADER_LENGTHS[BookHeader::AUTHORS], path, prep);
	const auto published = ParameterMiddleware(BOOK_HEADERS[BookHeader::PUBLISHED], BOOK_HEADER_LENGTHS[BookHeader::PUBLISHED], path, prep);
	const auto attributes = ParameterMiddleware(BOOK_HEADERS[BookHeader::ATTRIBUTES], BOOK_HEADER_LENGTHS[BookHeader::ATTRIBUTES], path, prep);
	const auto storage_id = ParameterMiddleware(BOOK_HEADERS[BookHeader::STORAGE_ID], BOOK_HEADER_LENGTHS[BookHeader::STORAGE_ID], path, prep);
	const auto flags = ParameterMiddleware(BOOK_HEADERS[BookHeader::FLAGS], BOOK_HEADER_LENGTHS[BookHeader::FLAGS], path, prep);

	if (!opts.ignore_id) {
		const auto id = ParameterMiddleware(BOOK_HEADERS[BookHeader::ID], BOOK_HEADER_LENGTHS[BookHeader::ID], path, prep);
		result.id = id ? Str::fixedAtoi<uint32_t>(id.value.c_str(), id.value.length()) : 0;
	}

	if (!opts.ignore_user_id) {
		const auto user_id = ParameterMiddleware(BOOK_HEADERS[BookHeader::USER_ID], BOOK_HEADER_LENGTHS[BookHeader::USER_ID], path, prep);
		result.user_id = user_id ? Str::fixedAtoi<uint16_t>(user_id.value.c_str(), user_id.value.length()) : 0;
	}

	result.in = in ? Str::fixedAtoi<uint64_t>(in.value.c_str(), in.value.length()) : 0;
	result.storage_id = storage_id ? Str::fixedAtoi<uint16_t>(storage_id.value.c_str(), storage_id.value.length()) : 0;

	if (title) {
		result.title_len = min(title.value.length(), sizeof(Book::title));
		memcpy(result.title, title.value.c_str(), result.title_len);
	}

	if (authors) {
		result.author_len = min(authors.value.length(), sizeof(Book::authors));
		memcpy(result.authors, authors.value.c_str(), result.author_len);
	}

	if (published) {
		result.published_len = min(published.value.length(), sizeof(Book::published));
		memcpy(result.published, published.value.c_str(), result.published_len);
	}

	if (attributes) {
		result.attrib_len = min(attributes.value.length(), sizeof(Book::attributes));
		memcpy(result.attributes, attributes.value.c_str(), result.attrib_len);
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

void sendBook(uint32_t i, const Book &book, EthernetClient *client)
{
	client->print(book.id);
	client->print(',');
	client->print(book.in);
	client->print(",\"");
	client->write(book.title, book.title_len);
	client->print("\",\"");
	client->write(book.authors, book.author_len);
	client->print("\",\"");
	client->write(book.published, book.published_len);
	client->print("\",\"");
	client->write(book.attributes, book.attrib_len);
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

} // namespace

namespace BookRoute {
int putBookHandler(const String &path, const Vector<HTTP::ClientHeaderPair> &headers, EthernetClient &client)
{
	const auto prep = ParameterMiddleware::preparePath(path);
	const auto session = SessionMiddleware(path, true, prep);

	if (!session)
		return session.sendInvalidResponse(client);

	BookFromParametersOptions opts;
	opts.ignore_id = 1;
	opts.ignore_user_id = 1;

	Book book = getBookFromParameters(path, prep, opts);

	book.user_id = session.user_id;

	const auto id = global::db.book.add(book);

	if (id == 0) {
		HTTPServer::writeHTTPHeaders(400, "Bad Request", "text/csv", client);
	}
	else {
		HTTPServer::writeHTTPHeaders(200, "OK", "text/csv", client);
		client.println("key,value");
		client.print("id,");
		client.println(id);
	}

	return 0;
}

int postBookHandler(const String &path, const Vector<HTTP::ClientHeaderPair> &headers, EthernetClient &client)
{
	return 0;
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

	global::db.book.db.iterate(false, 0, global::db.book.db.size(), false, sendBook, &client);

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
	}

	return 0;
}

void registerRoute(HTTPServer &server)
{
	server.on(HTTP::GET, "/api/book/count", HTTPServer::HandlerBehavior::ALLOW_PARAMETERS, getBookCountHandler);
	server.on(HTTP::GET, "/api/book/all", HTTPServer::HandlerBehavior::ALLOW_PARAMETERS, getAllBooksHandler);

	/* server.on(HTTP::GET, "/api/book", HTTPServer::HandlerBehavior::ALLOW_PARAMETERS, getBookHandler);
	server.on(HTTP::GET, "/api/book", HTTPServer::HandlerBehavior::ALLOW_PARAMETERS, getBookHandler); */

	server.on(HTTP::PUT, "/api/book", HTTPServer::HandlerBehavior::ALLOW_PARAMETERS, putBookHandler);

	/* server.on(HTTP::POST, "/api/book", HTTPServer::HandlerBehavior::ALLOW_PARAMETERS, getBookHandler);
	server.on(HTTP::DELETE, "/api/book", HTTPServer::HandlerBehavior::ALLOW_PARAMETERS, getBookHandler); */
}

} // namespace BookRoute