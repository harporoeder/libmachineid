#include <stdio.h>
#include <string.h>

#include <sodium.h>
#include <openssl/rand.h>

#include "machineid.h"

static void machineid_bin_to_hex(char *const outputBuffer,
    const char *const inputBuffer, const size_t inputBufferSize);

static void machineid_bin_to_uuid(char *const outputBuffer,
    const char *const inputBuffer);

static size_t machineid_raw(char *const outputBuffer,
    const size_t outputBufferSize);

static char machineid_random_bytes(char *const outputBuffer,
    const size_t count);

const char *const HEX_ALPHABET = "0123456789ABCDEF";

#ifndef MIN
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#endif

#ifdef MACHINEID_USE_SODIUM
static char
machineid_random_bytes(char *const outputBuffer, const size_t count)
{
    randombytes_buf((void *const)outputBuffer, count);

    return 0;
}
#endif

#ifdef MACHINEID_USE_OPENSSL
static char
machineid_random_bytes(char *const outputBuffer, const size_t count)
{
    return RAND_bytes((unsigned char *const)outputBuffer, (int)count) != 0;
}
#endif

enum machineid_error
machineid_generate(char *const outputBuffer, enum machineid_flags flags)
{
    char fallback;
    char rawBuffer[256], hashBuffer[MACHINEID_HASH_SIZE];
    size_t rawSize;
    int status;

    fallback = 0;

    if (outputBuffer == NULL) {
        return MACHINEID_ERROR_NULL_OUTPUT_BUFFER;
    }

    rawSize = machineid_raw(rawBuffer, sizeof(rawBuffer));

    if (rawSize == 0) {
        if (machineid_random_bytes(rawBuffer, 16)) {
            return MACHINEID_ERROR_RNG;
        } else {
            fallback = 1;
        }
    }

    status = crypto_hash_sha256((unsigned char *const)hashBuffer,
        (unsigned char *const)rawBuffer, rawSize);

    if (status != 0) {
        return MACHINEID_ERROR_HASH_FAILURE;
    }

    if (flags & MACHINEID_FLAG_AS_UUID) {
        machineid_bin_to_uuid(outputBuffer, hashBuffer);
    } else {
        memcpy(outputBuffer, hashBuffer, sizeof(hashBuffer));
    }

    if (flags & MACHINEID_FLAG_NULL_TERMINATE) {
        if (flags & MACHINEID_FLAG_AS_UUID) {
            outputBuffer[MACHINEID_UUID_SIZE+1] = '\0';
        } else {
            outputBuffer[MACHINEID_HASH_SIZE+1] = '\0';
        }
    }

    if (fallback == 1) {
        return MACHINEID_ERROR_FALLBACK;
    } else {
        return MACHINEID_ERROR_NONE;
    }
}

#if defined(__linux__) || defined(__FreeBSD__)
static size_t
posix_read_file(const char *const path, char *const outputBuffer,
    const size_t outputBufferSize)
{
    FILE *handle;
    long fileSize;
    int status;
    size_t expectedResultSize, resultSize;

    handle = fopen(path, "r");

    if (handle == NULL) {
        return 0;
    }

    status = fseek(handle, 0L, SEEK_END);

    if (status != 0) {
        goto err;
    }

    fileSize = ftell(handle);

    if (fileSize == -1L) {
        goto err;
    }

    status = fseek(handle, 0L, SEEK_SET);

    if (status != 0) {
        goto err;
    }

    expectedResultSize = MIN((size_t)fileSize, outputBufferSize);

    resultSize = fread(outputBuffer, sizeof(char),
        expectedResultSize, handle);
    
    if (resultSize != expectedResultSize) {
        goto err;
    }

    fclose(handle);

    return resultSize;

  err:
    fclose(handle);

    return 0;
}
#endif

#ifdef __linux__
size_t
machineid_raw(char *const outputBuffer, const size_t outputBufferSize)
{
    size_t resultSize;

    resultSize = posix_read_file("/etc/machine-id", outputBuffer,
        outputBufferSize);

    if (resultSize != 0) {
        return resultSize;
    }

    return posix_read_file("/var/lib/dbus/machine-id", outputBuffer,
        outputBufferSize);
}
#endif

#ifdef __FreeBSD__
size_t
machineid_raw(char *const outputBuffer, const size_t outputBufferSize)
{
    return posix_read_file("/etc/hostid", outputBuffer,
        outputBufferSize);
}
#endif

static void
machineid_bin_to_hex(char *const outputBuffer, const char *const inputBuffer,
    const size_t inputBufferSize)
{
    size_t i;
    char *outputIter;
    const char *inputIter;

    outputIter = outputBuffer;
    inputIter = inputBuffer;

    for (i = 0; i < inputBufferSize; i += 1) {
        *outputIter++ = HEX_ALPHABET[(*inputIter>>4)&0xF];
        *outputIter++ = HEX_ALPHABET[(*inputIter++)&0xF];
    }
}

static void
machineid_bin_to_uuid(char *const outputBuffer, const char *const inputBuffer)
{
    char *outputIter;
    const char *inputIter;
    unsigned int i;

    outputIter = outputBuffer;
    inputIter = inputBuffer;

    machineid_bin_to_hex(outputIter, inputIter, 4);
    outputIter += 8; inputIter += 4; *outputIter = '-'; outputIter++;
    for (i = 0; i < 4; i++) {
        machineid_bin_to_hex(outputIter, inputIter, 2);
        outputIter += 4; inputIter += 2; *outputIter = '-'; outputIter++;
    }
    machineid_bin_to_hex(outputIter, inputIter, 4);
}