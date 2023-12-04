#include "databases.h"

#include "../arduino/logger.h"

namespace {
Logger logger = Logger("DBS");
}

Databases::Databases()
    : user(), session(), book(), storage(),
      reg_req(),
      m_lastSave(0), m_lastSessionUpdate(0)
{
}

bool Databases::load()
{
	user.load();
	session.load();
	book.load();
	storage.load();
}

bool Databases::update(unsigned long time)
{
	reg_req.update();

	if (time > m_lastSessionUpdate + EK_SESSION_UPDATE_INTERVAL_S) {
		VERBOSE_LOG(logger, "Updating sessions...");

		m_lastSessionUpdate = time;
		session.update();
	}

	if (time > m_lastSave + EK_DB_UPDATE_INTERVAL_S) {
		return save(time);
	}

	return true;
}

bool Databases::save(unsigned long time)
{
	logger.log("Saving databases...");
	m_lastSave = time;
	return session.db.flush() &&
	       user.db.flush() &&
	       book.db.flush() &&
	       storage.db.flush();
}