#include "registrationrequest.db.h"
#include "../string/string.h"

#include "../global/global.h"

#include <Arduino.h>

namespace {
const char VALID_CODE_LETTERS[] = {
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p',
    'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', 'A', 'B', 'C', 'D', 'E', 'F',
    'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V',
    'W', 'X', 'Y', 'Z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};

void countInvalids(uint32_t i, const RegistrationRequest &req,
                   uint32_t *count, unsigned long time)
{
	if (time > req.expireMillis)
		++(*count);
}

void selectInvalids(uint32_t i, const RegistrationRequest &req,
                    Vector<uint32_t> *vec, unsigned long time)
{
	if (time > req.expireMillis)
		vec->push_back(i);
}
} // namespace

/* private static */ bool RegistrationRequestDatabase::matchCredentials(
    uint32_t i, const RegistrationRequest &req,
    FixedBuffer<4> ip)
{
	return ip == req.ip;
}

/* private static */ bool RegistrationRequestDatabase::matchCode(
    uint32_t i, const RegistrationRequest &req,
    FixedBuffer<4> ip, FixedBuffer<4> code,
    unsigned long timestamp)
{
	if (timestamp > req.expireMillis) {
		return false;
	}

	return code == req.code && ip == req.ip;
}

/* private */ char RegistrationRequestDatabase::getRandomLetter() const
{
	auto value = analogRead(EK_HANGING_ANALOG_PIN);
	return VALID_CODE_LETTERS[value % sizeof(VALID_CODE_LETTERS)];
}

RegistrationRequestDatabase::RegistrationRequestDatabase()
    : db(EK_REGISTRATIONREQUESTDB_PATH)
{
	pinMode(EK_HANGING_ANALOG_PIN, INPUT);
}

void RegistrationRequestDatabase::load()
{
	db.tryLoad();
}

void RegistrationRequestDatabase::save()
{
	db.trySave();
}

uint32_t RegistrationRequestDatabase::cleanExpired(uint32_t n)
{
	const unsigned long time = global::ntp.getEpochTime();

	uint32_t invalid_count = 0;

	db.iterate(false, 0, db.size(), false, countInvalids, &invalid_count, time);

	const auto buffer_size = min(n, invalid_count);

	uint32_t *invalid_buffer = new uint32_t[buffer_size];

	{
		Vector<uint32_t> invalids;
		invalids.setStorage(invalid_buffer, buffer_size, 0);

		db.iterate(false, 0, db.size(), true, selectInvalids, &invalids, time);

		for (uint32_t i = 0; i < invalids.size(); ++i)
			db.remove(invalids.at(i));
	}

	delete[] invalid_buffer;

	return invalid_count > n ? invalid_count - n : 0;
}

RegistrationInitiation RegistrationRequestDatabase::tryInitiate(FixedBuffer<4> ip)
{
	const auto searchResult = db.search(
	    0, false,
	    matchCredentials,
	    ip);

	if (searchResult.state == QueryState::SUCCESS) {
		return RegistrationInitiation{false, {}};
	}

	RegistrationRequest request;

	for (size_t i = 0; i < 4; ++i)
		request.code.data[i] = getRandomLetter();

	request.ip = ip;
	request.expireMillis = global::ntp.getEpochTime() + EK_REGISTRATIONREQUEST_TIMEOUT_MS;

	db.append(request);

	return RegistrationInitiation{true, request};
}

bool RegistrationRequestDatabase::checkCode(FixedBuffer<4> ip, FixedBuffer<4> code)
{
	const auto time = global::ntp.getEpochTime();

	const auto searchResult = db.search(
	    0, false,
	    matchCode,
	    ip, code, time);

	return searchResult.state == QueryState::SUCCESS;
}
