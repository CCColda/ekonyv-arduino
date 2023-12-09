#include "route.storage.h"
#include "../global/global.h"
#include "../middleware/parameter.mw.h"
#include "../middleware/session.mw.h"

#include "../string/string.h"
#include "../string/to_string.h"
#include "../string/url.h"

namespace {

BookStorage getStorageFromParameters(const String &path, uint32_t prep)
{
	auto result = BookStorage();

	const auto id = ParameterMiddleware(STORAGE_HEADERS[SH_ID], STORAGE_HEADER_LENGTHS[SH_ID], path, prep);
	const auto name = ParameterMiddleware(STORAGE_HEADERS[SH_NAME], STORAGE_HEADER_LENGTHS[SH_NAME], path, prep);

	result.id = id ? Str::fixedAtoi<uint32_t>(SizedString::fromString(id.value)) : 0;

	if (name) {
		const auto decoded = Str::urlDecode(SizedString::fromString(name.value));

		result.name_len = min(decoded.length(), sizeof(BookStorage::name));
		memcpy(result.name, decoded.c_str(), result.name_len);
	}

	return result;
}

void sendBookStorage(uint32_t i, const BookStorage &storage, EthernetClient *client)
{
	client->print(storage.id);
	client->print(",\"");
	client->write(storage.name, storage.name_len);
	client->print("\",");
	client->println(storage.user_id);
}

} // namespace

namespace StorageRoute {
int postStorageHandler(const String &path, const Vector<HTTP::ClientHeaderPair> &headers, EthernetClient &client)
{
	const auto prep = ParameterMiddleware::preparePath(path);
	const auto session = SessionMiddleware(path, true, prep);

	if (!session)
		return session.sendInvalidResponse(client);

	if ((session.user.flags & User::CAN_WRITE) == 0) {
		HTTPServer::writeStaticHTMLResponse(HTTPResponse::HTML_UNAUTHORIZED, client);
		return 0;
	}

	BookStorage storage = getStorageFromParameters(path, prep);

	if (storage.id == 0) {
		const auto similar_book = global::db.storage.searchSimilarStorage(storage);
		if (similar_book.success) {
			HTTPServer::writeStaticHTMLResponse(
			    HTTPResponse::StaticHTMLResponse{
			        400, "Bad Request",
			        "400 - Bad Request",
			        "A similar storage place already exists."},
			    client);
			return 0;
		}

		storage.user_id = session.user_id;

		const auto id = global::db.storage.add(storage);

		if (id == 0) {
			HTTPServer::writeStaticHTMLResponse(
			    HTTPResponse::StaticHTMLResponse{
			        400, "Bad Request",
			        "400 - Bad Request",
			        "Failed adding storage."},
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
		const auto storage_info = global::db.storage.getByID(storage.id);

		if (!storage_info.success) {
			HTTPServer::writeStaticHTMLResponse(HTTPResponse::HTML_INVALID_STORAGE, client);
			return 0;
		}

		auto result_storage = storage_info.value;

		if (storage.name_len != 0) {
			result_storage.name_len = storage.name_len;
			memcpy(result_storage.name, storage.name, result_storage.name_len);
		}

		global::db.storage.db.modify(storage_info.index, result_storage);

		HTTPServer::writeHTTPHeaders(200, "OK", "text/csv", client);
		client.println("key,value");
		client.println("state,success");
		client.print("id,");
		client.println(result_storage.id);

		return 0;
	}
}

int deleteStorageHandler(const String &path, const Vector<HTTP::ClientHeaderPair> &headers, EthernetClient &client)
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

	const auto stored_storage = global::db.storage.getByID(Str::fixedAtoi<uint16_t>(SizedString::fromString(id.value)));

	if (!stored_storage.success) {
		HTTPServer::writeStaticHTMLResponse(HTTPResponse::HTML_INVALID_STORAGE, client);
		return 0;
	}

	global::db.book.removeAllOfStorage(stored_storage.value.id);
	global::db.storage.db.remove(stored_storage.index);

	HTTPServer::writeHTTPHeaders(200, "OK", "text/csv", client);
	client.println("key,value");
	client.println("state,success");

	return 0;
}

int getStorageCountHandler(const String &path, const Vector<HTTP::ClientHeaderPair> &headers, EthernetClient &client)
{
	const auto prep = ParameterMiddleware::preparePath(path);
	const auto session = SessionMiddleware(path, false, prep);

	if (!session)
		return session.sendInvalidResponse(client);

	HTTPServer::writeHTTPHeaders(200, "OK", "text/csv", client);
	client.println("key,value");
	client.print("count,");
	client.println(global::db.storage.db.size());
	client.print("last,");
	client.println(global::db.storage.getLastID());

	return 0;
}

int getAllStoragesHandler(const String &path, const Vector<HTTP::ClientHeaderPair> &headers, EthernetClient &client)
{
	const auto prep = ParameterMiddleware::preparePath(path);
	const auto session = SessionMiddleware(path, false, prep);

	if (!session)
		return session.sendInvalidResponse(client);

	HTTPServer::writeHTTPHeaders(200, "OK", "text/csv", client);

	for (uint8_t i = 0; i < sh_size; ++i) {
		client.print(STORAGE_HEADERS[i]);

		if (i < sh_size - 1)
			client.print(',');
	}
	client.println();

	global::db.storage.db.iterate(0, global::db.book.db.size(), sendBookStorage, &client);

	return 0;
}

int getStorageHandler(const String &path, const Vector<HTTP::ClientHeaderPair> &headers, EthernetClient &client)
{
	const auto prep = ParameterMiddleware::preparePath(path);
	const auto session = SessionMiddleware(path, false, prep);

	if (!session)
		return session.sendInvalidResponse(client);

	Search::SearchTerm terms_buf[EK_MAX_SEARCH_TERMS] = {};
	Vector<Search::SearchTerm> terms;

	terms.setStorage<EK_MAX_SEARCH_TERMS>(terms_buf);

	const auto num_search_terms = Search::parseSearchTermsFromURL(path, prep, STORAGE_HEADERS, sh_size, terms);

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

	StorageDatabase::SearchCallback cb;
	cb.set(sendBookStorage, &client);
	global::db.storage.match(terms, cb);

	return 0;
}

void registerRoute(HTTPServer &server)
{
	server.on(HTTP::GET, "/api/storage/count", HTTPServer::HandlerBehavior::ALLOW_PARAMETERS, getStorageCountHandler);
	server.on(HTTP::GET, "/api/storage/all", HTTPServer::HandlerBehavior::ALLOW_PARAMETERS, getAllStoragesHandler);

	server.on(HTTP::GET, "/api/storage", HTTPServer::HandlerBehavior::ALLOW_PARAMETERS, getStorageHandler);
	server.on(HTTP::POST, "/api/storage", HTTPServer::HandlerBehavior::ALLOW_PARAMETERS, postStorageHandler);

	server.on(HTTP::DELETE, "/api/storage", HTTPServer::HandlerBehavior::ALLOW_PARAMETERS, deleteStorageHandler);
}

} // namespace StorageRoute