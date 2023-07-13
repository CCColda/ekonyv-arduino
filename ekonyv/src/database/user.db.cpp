#include "user.db.h"

#include "../config.h"
#include "../string/string.h"

namespace {
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

/* private */ uint16_t UserDatabase::findNextID()
{
	if (db.size() == 0) {
		return 1;
	}

	return db.at(db.size() - 1).value.id + 1;
}

UserDatabase::UserDatabase()
    : db(EK_USERDB_PATH)
{
}

void UserDatabase::load()
{
	db.tryLoad();
}

void UserDatabase::save()
{
	db.trySave();
}

bool UserDatabase::tryRegister(
    const char *username, size_t len,
    const FixedBuffer<32> &passwordHash,
    User::Flags flags)
{
	if (len > sizeof(User::username))
		return false;

	const auto searchResult = db.search(
	    0, false,
	    matchUsername,
	    username, len);

	if (searchResult.state == QueryState::SUCCESS) {
		return false;
	}

	User user;
	user.id = findNextID();
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

	if (searchResult.state != QueryState::SUCCESS)
		return UserResult{false, User()};

	return UserResult{true, searchResult.value};
}

UserDatabase::UserResult UserDatabase::getByID(uint16_t id)
{
	const auto searchResult = db.search(
	    0, false,
	    matchID,
	    id);

	if (searchResult.state != QueryState::SUCCESS)
		return UserResult{false, User()};

	return UserResult{true, searchResult.value};
}
