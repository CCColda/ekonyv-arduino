#ifndef EKONYV_SESSION_MW_H
#define EKONYV_SESSION_MW_H

#include "../database/session.db.h"
#include "../database/user.db.h"

class SessionMiddleware {
public:
	SessionMiddleware(const String &path, bool get_user);

	bool valid;
	bool expired;
	uint16_t user_id;
	User user;
};

#endif // !defined(EKONYV_SESSION_MW_H)