#include "registrationrequest.mem.h"

#include "../config.h"

#include "../global/global.h"

#if EK_ETHERNET
#define EK_REGREQ_GET_TIME global::ntp.getEpochTime
#else
#define EK_REGREQ_GET_TIME millis
#endif

namespace {
const char VALID_CODE_LETTERS[] = {
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p',
    'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', 'A', 'B', 'C', 'D', 'E', 'F',
    'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V',
    'W', 'X', 'Y', 'Z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};

char getRandomLetter()
{
	auto value = analogRead(EK_HANGING_ANALOG_PIN);
	return VALID_CODE_LETTERS[value % sizeof(VALID_CODE_LETTERS)];
}
} // namespace

RegistrationRequest::RegistrationRequest()
    : code(), for_ip(), expire(0), active()
{
}

void RegistrationRequest::update()
{
	if (expire <= EK_REGREQ_GET_TIME())
		active = false;
}

bool RegistrationRequest::tryInitiate(const FixedBuffer<4> &ip)
{
	if (active)
		return false;

	for (uint8_t i = 0; i < 4; ++i)
		code.data[i] = getRandomLetter();

	for_ip = ip;
	active = true;
	expire = EK_REGREQ_GET_TIME() + EK_REGISTRATIONREQUEST_TIMEOUT_MS;
	return true;
}

bool RegistrationRequest::checkCode(const FixedBuffer<4> &ip, const FixedBuffer<4> &icode)
{
	if (!active)
		return false;

	return for_ip == ip && code == icode;
}

void RegistrationRequest::invalidate()
{
	active = false;
}