#ifndef MACHINEID_H
#define MACHINEID_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

#define MACHINEID_HASH_SIZE 32
#define MACHINEID_UUID_SIZE 36

enum machineid_flags {
    MACHINEID_FLAG_DEFAULT        = 0,
    MACHINEID_FLAG_AS_UUID        = 1,
    MACHINEID_FLAG_NULL_TERMINATE = 2
};

enum machineid_error {
    MACHINEID_ERROR_NONE               = 0,
    MACHINEID_ERROR_RNG                = 1,
    MACHINEID_ERROR_NULL_OUTPUT_BUFFER = 2,
    MACHINEID_ERROR_FALLBACK           = 3,
    MACHINEID_ERROR_HASH_FAILURE       = 4
};

enum machineid_error machineid_generate(char *const outputBuffer,
    enum machineid_flags flags);

#ifdef __cplusplus
}
#endif

#endif