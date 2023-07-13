#ifndef EKONYV_USER_DB_H
#define EKONYV_USER_DB_H

#include "../storage/database.h"
#include "../types/fixedbuffer.h"

struct User {
	struct Flags {
		uint8_t can_write : 1;
		uint8_t admin : 1;
	};

	uint16_t id;
	Flags flags;
	uint8_t username_len;
	char username[64];
	FixedBuffer<32> password_hash;
};

class UserDatabase {
public:
	Database<User, 4> db;

	struct UserResult {
		bool success;
		User user;
	};

private:
	uint16_t
	findNextID();

public:
	UserDatabase();
	void load();
	void save();

	bool tryRegister(
	    const char *username, size_t len,
	    const FixedBuffer<32> &passwordHash,
	    User::Flags flags);

	UserResult tryLogin(
	    const char *username, size_t len,
	    const FixedBuffer<32> &passwordHash);

	UserResult getByID(uint16_t id);
};

#endif // !defined(EKONYV_USER_DB_H)