#include "user.db.h"

#include "../config.h"
#include "../string/string.h"

namespace {
bool matchUsernamePassword(
    uint32_t i, const User &user,
    const char *username, size_t len, FixedBuffer<32> passwordHash)
{
	return Str::compare(user.username, username, len) && passwordHash == user.password_hash;
}

bool matchUsername(
    uint32_t i, const User &user,
    const char *username, size_t len)
{
	return Str::compare(user.username, username, len);
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

	memcpy(user.password_hash.data, passwordHash.data, sizeof(User::password_hash));
	memcpy(user.username, username, len);
	if (len < sizeof(User::username)) {
		user.username[len] = '\0';
	}

	db.append(user);

	return true;
}

UserDatabase::LoginResult UserDatabase::tryLogin(
    const char *username, size_t len,
    const FixedBuffer<32> &passwordHash)
{
	const auto searchResult = db.search(
	    0, false,
	    matchUsernamePassword,
	    username, len, passwordHash);

	if (searchResult.state == QueryState::SUCCESS)
		return LoginResult{false, User()};

	return LoginResult{true, searchResult.value};
}
