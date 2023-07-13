#ifndef EKONYV_CONFIG_H
#define EKONYV_CONFIG_H

#include "salt.h"

#ifndef EK_PASSWORD_SALT
#define EK_PASSWORD_SALT ""
#define EK_PASSWORD_SALT_LEN 0
#warning "Password salt was not defined, using empty string. Make sure to create salt.h."
#endif

#pragma region helper macros

#define __EK_LITERAL_STRING(s) #s
#define __EK_MACRO_STRING(s) __EK_LITERAL_STRING(s)

// helper macros
#pragma endregion

#define EK_VERSION_MAJOR 0
#define EK_VERSION_MINOR 1
#define EK_VERSION_RELEASE 0
#define EK_VERSION __EK_MACRO_STRING(EK_VERSION_MAJOR) "." __EK_MACRO_STRING(EK_VERSION_MINOR) "." __EK_MACRO_STRING(EK_VERSION_RELEASE)

#define EK_NAME "EKonyv"

#define EK_PRODUCTION 0
#define EK_SD 0
#define EK_ETHERNET 1
#define EK_VERBOSE_LOGGING 1

#define EK_SERVER_PORT 80
#define EK_HTTP_HANDLER_STORAGE 8

#define EK_HANGING_ANALOG_PIN A5

#define EK_MOCK_FILE_BLOCKS 2
#define EK_MOCK_FILE_COUNT 8

#define EK_NTP_IP "europe.pool.ntp.org"
#define EK_NTP_OFFSET_S 3600

#define EK_REGISTRATIONREQUEST_TIMEOUT_MS 5 * 60 * 1000
#define EK_SESSION_TIMEOUT_MS 15 * 60 * 1000
#define EK_SESSION_RENEW_TIMEOUT_MS 15 * 60 * 1000

#define EK_USERDB_PATH "ekonyv/db/user.txt"
#define EK_SESSIONDB_PATH "ekonyv/db/session.txt"

//! @warning Do not change
#define EK_MOCK_FILE_SIZE EK_MOCK_FILE_BLOCKS * 512

#endif // !defined(EKONYV_CONFIG_H)