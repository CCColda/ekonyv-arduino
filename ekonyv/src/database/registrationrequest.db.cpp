#include "registrationrequest.db.h"
#include "../string/string.h"

#include <Arduino.h>

namespace {
const char VALID_CODE_LETTERS[] = {
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p',
    'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', 'A', 'B', 'C', 'D', 'E', 'F',
    'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V',
    'W', 'X', 'Y', 'Z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
}

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
	request.expireMillis = millis() + EK_REGISTRATIONREQUEST_TIMEOUT_MS;

	db.append(request);

	return RegistrationInitiation{true, request};
}

bool RegistrationRequestDatabase::checkCode(FixedBuffer<4> ip, FixedBuffer<4> code)
{
	const auto searchResult = db.search(
	    0, false,
	    matchCode,
	    ip, code, millis());

	return searchResult.state == QueryState::SUCCESS;
}
