#ifndef EKONYV_SESSION_MW_H
#define EKONYV_SESSION_MW_H

#include "../database/session.db.h"
#include "../database/user.db.h"

class SessionMiddleware {
public:
	SessionMiddleware(const String &path, bool get_user);

	bool valid;
	Session session;
	User user;
};

#endif // !defined(EKONYV_SESSION_MW_H)