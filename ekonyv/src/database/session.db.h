#ifndef EKONYV_SESSION_H
#define EKONYV_SESSION_H

#include "../storage/database.h"
#include "../types/fixedbuffer.h"

struct Session {
	FixedBuffer<16> token;
	FixedBuffer<16> refresh_token;
	uint16_t user_id;
	unsigned long expire;
};

class SessionDatabase {
public:
	Database<Session, 4> db;

	struct SessionInfo {
		bool valid;
		bool expired;
		uint32_t index;
		uint16_t user_id;
	};

public:
	SessionDatabase();
	void load();
	void save();

	Session start(uint16_t user_id);
	Session extend(uint32_t n, uint16_t user_id);
	void discard(uint32_t id);
	void discardAllForUser(uint16_t user_id);

	void update();

	SessionInfo check(const FixedBuffer<16> &token);
	SessionInfo checkRefresh(const FixedBuffer<16> &token);
};

#endif // !defined(EKONYV_SESSION_H)