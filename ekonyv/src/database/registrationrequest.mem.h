#ifndef EKONYV_REGISTRATIONREQUEST_M_H
#define EKONYV_REGISTRATIONREQUEST_M_H

#include "../types/fixedbuffer.h"

class RegistrationRequest {
public:
	FixedBuffer<4> code;
	FixedBuffer<4> for_ip;
	unsigned long expire;
	bool active;

public:
	RegistrationRequest();

	void update();
	bool tryInitiate(const FixedBuffer<4> &ip);
	bool checkCode(const FixedBuffer<4> &ip, const FixedBuffer<4> &code);
	void invalidate();
};

#endif // !defined(EKONYV_REGISTRATIONREQUEST_M_H)