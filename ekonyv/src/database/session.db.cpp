#include "session.db.h"

#include "../global/global.h"

#if EK_ETHERNET
#define EK_SDB_GET_TIME global::ntp.getEpochTime
#else
#define EK_SDB_GET_TIME millis
#endif

namespace {
byte getRandomByte()
{
	return analogRead(EK_HANGING_ANALOG_PIN) % 0xFF;
}

bool searchForValidToken(
    uint32_t i, const Session &s,
    FixedBuffer<16> token)
{
	return s.token == token;
}

bool searchForValidRefreshToken(
    uint32_t i, const Session &s,
    FixedBuffer<16> token)
{
	return s.refresh_token == token;
}

bool searchExpiredRenewToken(
    uint32_t i, const Session &s,
    unsigned long time)
{
	return (s.expire + EK_SESSION_RENEW_TIMEOUT_MS) < time;
}
} // namespace

SessionDatabase::SessionDatabase()
    : db(EK_SESSIONDB_PATH)
{
}

void SessionDatabase::load()
{
	db.tryLoad();
}

void SessionDatabase::save()
{
	db.trySave();
}

Session SessionDatabase::start(uint16_t user_id)
{
	FixedBuffer<16> new_session_token;
	FixedBuffer<16> new_refresh_token;
	for (uint8_t i = 0; i < 16; ++i) {
		new_session_token.data[i] = getRandomByte();
		new_refresh_token.data[i] = getRandomByte();
	}

	const auto result = Session{
	    new_session_token,
	    new_refresh_token,
	    user_id,
	    EK_SDB_GET_TIME() + EK_SESSION_TIMEOUT_MS};

	db.append(result);

	return result;
}

Session SessionDatabase::extend(uint32_t id, uint16_t user_id)
{
	FixedBuffer<16> new_session_token;
	FixedBuffer<16> new_refresh_token;
	for (uint8_t i = 0; i < 16; ++i) {
		new_session_token.data[i] = getRandomByte();
		new_refresh_token.data[i] = getRandomByte();
	}

	const auto result = Session{
	    new_session_token,
	    new_refresh_token,
	    user_id,
	    EK_SDB_GET_TIME() + EK_SESSION_TIMEOUT_MS};

	db.modify(id, result);

	return result;
}

void SessionDatabase::discard(uint32_t id)
{
	db.remove(id);
}

void SessionDatabase::discardAllForUser(uint16_t user_id)
{
	// todo implement
}

void SessionDatabase::update()
{
	const auto time = EK_SDB_GET_TIME();

	db.remove_if(0, db.size() - 1, searchExpiredRenewToken, time);
}

SessionDatabase::SessionInfo SessionDatabase::check(const FixedBuffer<16> &token)
{
	const auto search_result = db.search(0, true, searchForValidToken, token);

	if (search_result.state == QueryState::ERROR)
		return SessionInfo{
		    false,
		    true,
		    0,
		    0};

	return SessionInfo{
	    true,
	    search_result.value.expire < EK_SDB_GET_TIME(),
	    search_result.index,
	    search_result.value.user_id};
}

SessionDatabase::SessionInfo SessionDatabase::checkRefresh(const FixedBuffer<16> &token)
{
	const auto search_result = db.search(0, true, searchForValidRefreshToken, token);

	if (search_result.state == QueryState::ERROR)
		return SessionInfo{
		    false,
		    false,
		    0,
		    0};

	return SessionInfo{
	    true,
	    (search_result.value.expire + EK_SESSION_RENEW_TIMEOUT_MS) < EK_SDB_GET_TIME(),
	    search_result.index,
	    search_result.value.user_id};
}
