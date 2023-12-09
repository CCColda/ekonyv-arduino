#include "session.db.h"

#include "../global/global.h"

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

bool searchForUser(uint32_t i, const Session &s, uint16_t user_id)
{
	return s.user_id == user_id;
}

} // namespace

SessionDatabase::SessionDatabase()
    : db(EK_SESSIONDB_PATH)
{
}

void SessionDatabase::load()
{
	db.initialize();
}

void SessionDatabase::save()
{
	db.flush();
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
	    global::time() + EK_SESSION_TIMEOUT_MS};

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
	    global::time() + EK_SESSION_TIMEOUT_MS};

	db.modify(id, result);

	return result;
}

void SessionDatabase::discard(uint32_t id)
{
	db.remove(id);
}

void SessionDatabase::discardAllForUser(uint16_t user_id)
{
	db.remove_if(0, db.size(), searchForUser, user_id);
}

void SessionDatabase::update()
{
	const auto time = global::time();

	db.remove_if(0, db.size() - 1, searchExpiredRenewToken, time);
}

SessionDatabase::SessionInfo SessionDatabase::check(const FixedBuffer<16> &token)
{
	const auto search_result = db.search_reverse(0, searchForValidToken, token);

	if (!search_result.success)
		return SessionInfo{
		    false,
		    true,
		    0,
		    User::INVALID_ID};

	return SessionInfo{
	    true,
	    search_result.value.expire < global::time(),
	    search_result.index,
	    search_result.value.user_id};
}

SessionDatabase::SessionInfo SessionDatabase::checkRefresh(const FixedBuffer<16> &token)
{
	const auto search_result = db.search_reverse(0, searchForValidRefreshToken, token);

	if (!search_result.success)
		return SessionInfo{
		    false,
		    false,
		    0,
		    0};

	return SessionInfo{
	    true,
	    (search_result.value.expire + EK_SESSION_RENEW_TIMEOUT_MS) < global::time(),
	    search_result.index,
	    search_result.value.user_id};
}
