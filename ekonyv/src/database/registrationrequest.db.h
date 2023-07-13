#ifndef EKONYV_REG_REQ_DB_H
#define EKONYV_REG_REQ_DB_H

#include "../config.h"

#include "../storage/database.h"

#include "../types/fixedbuffer.h"

struct RegistrationRequest {
	FixedBuffer<4> code;
	FixedBuffer<4> ip;

	// todo implement NTP to keep data across shutdowns
	unsigned long expireMillis;
};

struct RegistrationInitiation {
	bool success;
	RegistrationRequest req;
};

class RegistrationRequestDatabase {
public:
	Database<RegistrationRequest, 2> db;

private:
	static bool matchCredentials(
	    uint32_t i, const RegistrationRequest &req,
	    FixedBuffer<4> ip);

	static bool matchCode(
	    uint32_t i, const RegistrationRequest &req,
	    FixedBuffer<4> ip, FixedBuffer<4> code,
	    unsigned long timestamp);

	char getRandomLetter() const;

public:
	RegistrationRequestDatabase();
	void load();
	void save();

	uint32_t cleanExpired(uint32_t n);

	RegistrationInitiation tryInitiate(FixedBuffer<4> ip);

	bool checkCode(FixedBuffer<4> ip, FixedBuffer<4> code);
};

#endif // !defined(EKONYV_REG_REQ_DB_H)