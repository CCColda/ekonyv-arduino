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
	char username[64];
	FixedBuffer<32> password_hash;
	Flags flags;
};

class UserDatabase {
public:
	Database<User, 4> db;

private:
	static bool matchUsername(
	    uint32_t i, const User &user,
	    const char *username, size_t len);

	uint16_t findNextID();

public:
	UserDatabase();
	void load();
	void save();

	bool tryRegister(
	    const char *username, size_t len,
	    const FixedBuffer<32> &passwordHash,
	    User::Flags flags);
};

#endif // !defined(EKONYV_USER_DB_H)