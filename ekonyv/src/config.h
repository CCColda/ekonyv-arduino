#ifndef EKONYV_CONFIG_H
#define EKONYV_CONFIG_H

#pragma region helper macros

#define __EK_STRING(s) #s
#define __EK_C_STRING(s) __EK_STRING(s)

// helper macros
#pragma endregion

#define EK_VERSION_MAJOR 0
#define EK_VERSION_MINOR 1
#define EK_VERSION_RELEASE 0
#define EK_VERSION __EK_C_STRING(EK_VERSION_MAJOR) "." __EK_C_STRING(EK_VERSION_MINOR) "." __EK_C_STRING(EK_VERSION_RELEASE)

#define EK_NAME "EKonyv"

#define EK_PRODUCTION 0

#define EK_HANGING_ANALOG_PIN A5
#define EK_VERBOSE_LOGGING 1
#define EK_SD 0
#define EK_ETHERNET 1
#define EK_MOCK_FILE_BLOCKS 2
#define EK_MOCK_FILE_COUNT 8

#define EK_REGISTRATIONREQUEST_TIMEOUT_MS 600000

#define EK_REGISTRATIONREQUESTDB_PATH "ekonyv/db/regreq.txt"
#define EK_USERDB_PATH "ekonyv/db/user.txt"

//! @warning Do not change
#define EK_MOCK_FILE_SIZE EK_MOCK_FILE_BLOCKS * 512

#endif // !defined(EKONYV_CONFIG_H)