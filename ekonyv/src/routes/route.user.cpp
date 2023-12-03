#include "route.user.h"
#include "../global/global.h"
#include "../middleware/parameter.mw.h"
#include "../middleware/session.mw.h"

#include "../string/string.h"
#include "../string/to_string.h"
#include "../string/url.h"

namespace {
User getUserFromParameters(const String &path, uint32_t prep)
{
	auto result = User();

	const auto id = ParameterMiddleware(USER_HEADERS[UH_ID], BOOK_HEADER_LENGTHS[UH_ID], path, prep);
	const auto flags = ParameterMiddleware(USER_HEADERS[UH_FLAGS], BOOK_HEADER_LENGTHS[UH_FLAGS], path, prep);
	const auto username = ParameterMiddleware(USER_HEADERS[UH_USERNAME], BOOK_HEADER_LENGTHS[UH_USERNAME], path, prep);

	result.id = id ? Str::fixedAtoi<uint32_t>(id.value.c_str(), id.value.length()) : 0;
	result.flags = flags ? Str::fixedAtoi<uint8_t>(flags.value.c_str(), flags.value.length()) : 0;

	if (username) {
		const auto decoded = Str::urlDecode(username.value.c_str(), username.value.length());

		result.username_len = min(decoded.length(), sizeof(Book::title));
		memcpy(result.username, decoded.c_str(), result.username_len);
	}

	return result;
}

void sendUser(uint32_t i, const User &user, EthernetClient *client)
{
	client->print(user.id);
	client->print(',');
	client->print(user.flags);
	client->print(",\"");
	client->write(user.username, user.username_len);
	client->println("\"");
}

} // namespace

namespace UserRoute {

int postUserHandler(const String &path, const Vector<HTTP::ClientHeaderPair> &headers, EthernetClient &client)
{
	const auto prep = ParameterMiddleware::preparePath(path);
	const auto session = SessionMiddleware(path, true, prep);

	if (!session)
		return session.sendInvalidResponse(client);

	User user = getUserFromParameters(path, prep);
	if (user.id != session.user_id && (session.user.flags & User::IS_ADMIN) == 0) {
		HTTPServer::writeStaticHTMLResponse(HTTPResponse::HTML_UNAUTHORIZED, client);
		return 0;
	}

	if (user.id == 0) {
		HTTPServer::writeStaticHTMLResponse(HTTPResponse::HTML_BAD_REQUEST, client);
		return 0;
	}

	const auto user_info = global::db.user.getByID(user.id);

	if (!user_info.success) {
		HTTPServer::writeStaticHTMLResponse(HTTPResponse::HTML_INVALID_USER, client);
		return 0;
	}

	auto result_user = user_info.value;

	if ((session.user.flags & User::IS_ADMIN) != 0)
		result_user.flags = user.flags;

	if (user.username_len != 0) {
		result_user.username_len = user.username_len;
		memcpy(result_user.username, user.username, result_user.username_len);
	}

	global::db.user.db.modify(user_info.index, result_user);

	HTTPServer::writeHTTPHeaders(200, "OK", "text/csv", client);
	client.println("key,value");
	client.println("state,success");
	client.print("id,");
	client.println(result_user.id);

	return 0;
}

int deleteUserHandler(const String &path, const Vector<HTTP::ClientHeaderPair> &headers, EthernetClient &client)
{
	const auto prep = ParameterMiddleware::preparePath(path);
	const auto session = SessionMiddleware(path, true, prep);

	if (!session)
		return session.sendInvalidResponse(client);

	const auto id = ParameterMiddleware("id", 2, path, prep);

	if (!id)
		return id.sendMissingResponse(client);

	const auto stored_user = global::db.user.getByID(Str::fixedAtoi<uint16_t>(id.value.c_str(), id.value.length()));

	if (!stored_user.success) {
		HTTPServer::writeStaticHTMLResponse(HTTPResponse::HTML_INVALID_USER, client);
		return 0;
	}

	if (stored_user.value.id != session.user_id && (session.user.flags & User::IS_ADMIN) == 0) {
		HTTPServer::writeStaticHTMLResponse(HTTPResponse::HTML_UNAUTHORIZED, client);
		return 0;
	}

	global::db.book.removeAllOfUser(stored_user.value.id);
	global::db.user.db.remove(stored_user.index);

	HTTPServer::writeHTTPHeaders(200, "OK", "text/csv", client);
	client.println("key,value");
	client.println("state,success");

	return 0;
}

int getUserCountHandler(const String &path, const Vector<HTTP::ClientHeaderPair> &headers, EthernetClient &client)
{
	const auto prep = ParameterMiddleware::preparePath(path);
	const auto session = SessionMiddleware(path, false, prep);

	if (!session)
		return session.sendInvalidResponse(client);

	HTTPServer::writeHTTPHeaders(200, "OK", "text/csv", client);
	client.println("key,value");
	client.print("count,");
	client.println(global::db.user.db.size());
	client.print("last,");
	client.println(global::db.user.getLastID());

	return 0;
}

int getAllUsersHandler(const String &path, const Vector<HTTP::ClientHeaderPair> &headers, EthernetClient &client)
{
	const auto prep = ParameterMiddleware::preparePath(path);
	const auto session = SessionMiddleware(path, false, prep);

	if (!session)
		return session.sendInvalidResponse(client);

	HTTPServer::writeHTTPHeaders(200, "OK", "text/csv", client);

	for (uint8_t i = 0; i < uh_size; ++i) {
		client.print(USER_HEADERS[i]);

		if (i < uh_size - 1)
			client.print(',');
	}
	client.println();

	global::db.user.db.iterate(0, global::db.book.db.size(), false, sendUser, &client);

	return 0;
}

int getUserHandler(const String &path, const Vector<HTTP::ClientHeaderPair> &headers, EthernetClient &client)
{
	const auto prep = ParameterMiddleware::preparePath(path);
	const auto session = SessionMiddleware(path, false, prep);

	if (!session)
		return session.sendInvalidResponse(client);

	Search::SearchTerm terms_buf[EK_MAX_SEARCH_TERMS] = {};
	Vector<Search::SearchTerm> terms;

	terms.setStorage<EK_MAX_SEARCH_TERMS>(terms_buf);

	const auto num_search_terms = Search::parseSearchTermsFromURL(path, prep, USER_HEADERS, uh_size, terms);

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

	for (uint8_t i = 0; i < uh_size; ++i) {
		client.print(USER_HEADERS[i]);

		if (i < uh_size - 1)
			client.print(',');
	}
	client.println();

	UserDatabase::SearchCallback cb;
	cb.set(sendUser, &client);

	global::db.user.match(terms, cb);

	return 0;
}

void registerRoute(HTTPServer &server)
{
	server.on(HTTP::GET, "/api/user/count", HTTPServer::HandlerBehavior::ALLOW_PARAMETERS, getUserCountHandler);
	server.on(HTTP::GET, "/api/user/all", HTTPServer::HandlerBehavior::ALLOW_PARAMETERS, getAllUsersHandler);

	server.on(HTTP::GET, "/api/user", HTTPServer::HandlerBehavior::ALLOW_PARAMETERS, getUserHandler);
	server.on(HTTP::POST, "/api/user", HTTPServer::HandlerBehavior::ALLOW_PARAMETERS, postUserHandler);

	server.on(HTTP::DELETE, "/api/user", HTTPServer::HandlerBehavior::ALLOW_PARAMETERS, deleteUserHandler);
}

} // namespace UserRoute