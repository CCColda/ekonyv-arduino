#ifndef EKONYV_CONFIG_H
#define EKONYV_CONFIG_H

#define EK_VERBOSE_LOGGING 1
#define EK_SD 0
#define EK_ETHERNET 0
#define EK_MOCK_FILE_BLOCKS 2
#define EK_MOCK_FILE_COUNT 8

//! @warning Do not change
#define EK_MOCK_FILE_SIZE EK_MOCK_FILE_BLOCKS * 512

#endif // !defined(EKONYV_CONFIG_H)