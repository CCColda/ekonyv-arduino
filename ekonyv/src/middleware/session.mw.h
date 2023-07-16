#ifndef EKONYV_SESSION_MW_H
#define EKONYV_SESSION_MW_H

#include "../database/session.db.h"
#include "../database/user.db.h"

#include "parameter.mw.h"

class SessionMiddleware {
public:
	SessionMiddleware(
	    const String &path, bool get_user,
	    uint32_t parameter_preparation = ParameterMiddleware::NO_PREPARATION);

	inline operator bool() const { return valid && !expired; }
	inline bool operator!() const { return !valid || expired; }

public:
	ParameterMiddleware parameter;
	bool valid;
	bool expired;
	uint32_t session_index;
	uint16_t user_id;
	User user;
};

#endif // !defined(EKONYV_SESSION_MW_H)