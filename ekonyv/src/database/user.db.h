#ifndef EKONYV_USER_DB_H
#define EKONYV_USER_DB_H

#include "../storage/database.h"
#include "../types/fixedbuffer.h"

#include "search.h"

#include "../types/callback.h"

struct User {
	constexpr static const uint8_t CAN_WRITE = 0b00000001;
	constexpr static const uint8_t IS_ADMIN = 0b00000010;

	uint16_t id;
	uint8_t flags;
	uint8_t username_len;
	char username[64];
	FixedBuffer<32> password_hash;

	constexpr static uint16_t INVALID_ID = 0;
};

enum UserHeader : uint8_t {
	UH_ID,
	UH_FLAGS,
	UH_USERNAME,
	uh_size
};

extern const char *USER_HEADERS[uh_size];
extern uint8_t USER_HEADER_LENGTHS[uh_size];

class UserDatabase {
public:
	Database<User, 4> db;

	struct UserResult {
		bool success;
		User user;
	};

	using SearchCallback = Callback<void, uint32_t, const User &>;

public:
	UserDatabase();
	void load();
	void save();

	uint16_t getLastID();

	bool tryRegister(
	    const char *username, size_t len,
	    const FixedBuffer<32> &passwordHash,
	    uint8_t flags);

	UserResult tryLogin(
	    const char *username, size_t len,
	    const FixedBuffer<32> &passwordHash);

	decltype(db)::QueryResult getByID(uint16_t id);

	void match(const Vector<Search::SearchTerm> &search, SearchCallback callback);
};

#endif // !defined(EKONYV_USER_DB_H)