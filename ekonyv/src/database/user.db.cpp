#include "user.db.h"

#include "../config.h"
#include "../string/string.h"

/* extern */ const char *USER_HEADERS[uh_size] = {
    "id",
    "flags",
    "username"};

/* extern */ uint8_t USER_HEADER_LENGTHS[uh_size] = {
    2,
    5,
    8};

namespace {
Search::SearchField USER_FIELD_TYPES[uh_size] = {
    Search::U16,
    Search::U8,
    Search::S64};

Search::offset_t USER_FIELD_OFFSETS[uh_size] = {
    offsetof(User, User::id),
    offsetof(User, User::flags),
    offsetof(User, User::username)};

struct SearchIteratorData {
	const Vector<Search::SearchTerm> &terms;
	UserDatabase::SearchCallback &cb;
};

void match_iterator(uint32_t index, const User &user, SearchIteratorData *data)
{
	const uint8_t header_sizes[uh_size] = {
	    sizeof(user.id),
	    sizeof(user.flags),
	    user.username_len};

	if (Search::match(
	        data->terms,
	        (void *)&user,
	        USER_FIELD_OFFSETS,
	        USER_FIELD_TYPES,
	        header_sizes,
	        uh_size))
		data->cb.call(index, user);
}

bool matchUsernamePassword(
    uint32_t i, const User &user,
    const char *username, size_t len, FixedBuffer<32> passwordHash)
{
	if (len != user.username_len)
		return false;

	return Str::compare(user.username, username, len) && passwordHash == user.password_hash;
}

bool matchUsername(
    uint32_t i, const User &user,
    const char *username, size_t len)
{
	if (len != user.username_len)
		return false;

	return Str::compare(user.username, username, len);
}

bool matchID(
    uint32_t i, const User &user,
    uint16_t id)
{
	return user.id == id;
}
} // namespace

UserDatabase::UserDatabase()
    : db(EK_USERDB_PATH)
{
}

void UserDatabase::load()
{
	db.initialize();
}

void UserDatabase::save()
{
	db.flush();
}

uint16_t UserDatabase::getLastID()
{
	if (db.size() == 0) {
		return 0;
	}

	return db.at(db.size() - 1).value.id;
}

bool UserDatabase::tryRegister(
    const char *username, size_t len,
    const FixedBuffer<32> &passwordHash,
    uint8_t flags)
{
	if (len > sizeof(User::username))
		return false;

	const auto searchResult = db.search(
	    0, false,
	    matchUsername,
	    username, len);

	if (!searchResult.success) {
		return false;
	}

	User user;
	user.id = getLastID() + 1;
	user.flags = flags;
	user.username_len = len;
	memcpy(user.username, username, len);
	memset(user.username + len, 0, sizeof(user.username) - len);
	memcpy(user.password_hash.data, passwordHash.data, sizeof(User::password_hash));

	db.append(user);

	return true;
}

UserDatabase::UserResult UserDatabase::tryLogin(
    const char *username, size_t len,
    const FixedBuffer<32> &passwordHash)
{
	const auto searchResult = db.search(
	    0, false,
	    matchUsernamePassword,
	    username, len, passwordHash);

	if (!searchResult.success)
		return UserResult{false, User()};

	return UserResult{true, searchResult.value};
}

decltype(UserDatabase::db)::QueryResult UserDatabase::getByID(uint16_t id)
{
	const auto searchResult = db.search(
	    0, false,
	    matchID,
	    id);

	return searchResult;
}

void UserDatabase::match(const Vector<Search::SearchTerm> &search, SearchCallback callback)
{
	auto data = SearchIteratorData{
	    search,
	    callback};

	db.iterate(0, db.size(), false, match_iterator, &data);
}