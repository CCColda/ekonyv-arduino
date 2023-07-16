#ifndef EKONYV_PARAMETER_MW_H
#define EKONYV_PARAMETER_MW_H

#include "../network/httpserver.h"

class ParameterMiddleware {
public:
	bool valid;

	const char *name;
	String value;

	constexpr static uint32_t NO_PREPARATION = ~(uint32_t)0;

public:
	ParameterMiddleware(
	    const char *parameter, size_t len,
	    const String &path,
	    uint32_t preparation = NO_PREPARATION);

	int sendMissingResponse(EthernetClient &client) const;

	inline operator bool() const { return valid; }
	inline bool operator!() const { return !valid; }

	static uint32_t preparePath(const String &path);
};

#endif // !defined(EKONYV_PARAMETER_MW_H)