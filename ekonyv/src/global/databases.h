#ifndef EKONYV_DATABASES_H
#define EKONYV_DATABASES_H

#include "../database/book.db.h"
#include "../database/registrationrequest.mem.h"
#include "../database/session.db.h"
#include "../database/storage.db.h"
#include "../database/user.db.h"

class Databases {
public:
	RegistrationRequest reg_req;
	UserDatabase user;
	SessionDatabase session;
	BookDatabase book;
	StorageDatabase storage;

private:
	unsigned long m_lastSave;
	unsigned long m_lastSessionUpdate;

public:
	Databases();

	bool load();
	bool update(unsigned long time);
	bool save(unsigned long time);
};

#endif // !defined(EKONYV_DATABASES_H)