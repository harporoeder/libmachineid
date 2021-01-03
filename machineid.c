/*
BSD 3-Clause License

Copyright (c) 2021, Harpo Roeder
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <stdio.h>
#include <string.h>
#include <stddef.h>

#ifdef _WIN32
#define _CRT_RAND_S
#include <windows.h>
#endif

#include <stdlib.h>

#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#endif

#ifdef MACHINEID_USE_SODIUM
#include <sodium.h>
#include <openssl/rand.h>
#elif MACHINEID_USE_OPENSSL
#include <openssl/sha.h>
#else
#include "sha256.h"
#endif

#ifdef __OpenBSD__
#include <sys/param.h>
#include <sys/sysctl.h>
#endif

#include "machineid.h"

static void machineid_bin_to_hex(unsigned char *const outputBuffer,
    const unsigned char *const inputBuffer, const size_t inputBufferSize);

static void machineid_bin_to_uuid(unsigned char *const outputBuffer,
    const unsigned char *const inputBuffer);

static size_t machineid_raw(unsigned char *const outputBuffer,
    const size_t outputBufferSize);

static char machineid_random_bytes(unsigned char *const outputBuffer,
    const size_t count);

static char machineid_sha256(unsigned char *const outputBuffer,
    const unsigned char *const inputBuffer, const size_t inputBufferSize);

const char *const HEX_ALPHABET = "0123456789ABCDEF";

#ifndef MIN
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#endif

static char
machineid_random_bytes(unsigned char *const outputBuffer, const size_t count)
{
#ifdef MACHINEID_USE_SODIUM
    randombytes_buf((void *const)outputBuffer, count);

    return 0;
#elif MACHINEID_USE_OPENSSL
    return RAND_bytes(outputBuffer, (int)count) != 0;
#elif defined(__OpenBSD__) || defined(__FreeBSD__)
    arc4random_buf((void *const)outputBuffer, count);
#elif _WIN32
    size_t i;
    errno_t err;

    for (i = 0; i < count; i++) {
        unsigned int number;

        err = rand_s(&number);

        if (err != 0) {
            return 1;
        }

        outputBuffer[i] = number;
    }

    return 0;
#else
    size_t i;

    for (i = 0; i < count; i++) {
        outputBuffer[i] = rand();
    }

    return 0;
#endif
}

static char
machineid_sha256(unsigned char *const outputBuffer,
    unsigned const char *const inputBuffer,
    const size_t inputBufferSize)
{
#ifdef MACHINEID_USE_SODIUM
    return crypto_hash_sha256(outputBuffer, inputBuffer, inputBufferSize);
#elif MACHINEID_USE_OPENSSL
    SHA256(inputBuffer, inputBufferSize, outputBuffer);

    return 0;
#else
    SHA256_CTX context;

    sha256_init(&context);
    sha256_update(&context, (const LIBSHA256_BYTE *const)inputBuffer,
        inputBufferSize);
    sha256_final(&context, (LIBSHA256_BYTE *const)outputBuffer);

    return 0;
#endif
}

enum machineid_error
machineid_generate(unsigned char *const outputBuffer,
    enum machineid_flags flags)
{
    char fallback;
    unsigned char rawBuffer[256], hashBuffer[MACHINEID_HASH_SIZE];
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

    status = machineid_sha256(hashBuffer, rawBuffer, rawSize);

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

#if defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__)
static size_t
posix_read_file(const char *const path, unsigned char *const outputBuffer,
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

    resultSize = fread(outputBuffer, sizeof(unsigned char),
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
machineid_raw(unsigned char *const outputBuffer, const size_t outputBufferSize)
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
machineid_raw(unsigned char *const outputBuffer, const size_t outputBufferSize)
{
    return posix_read_file("/etc/hostid", outputBuffer,
        outputBufferSize);
}
#endif

#ifdef _WIN32
size_t
machineid_raw(unsigned char *const outputBuffer, const size_t outputBufferSize)
{
    LSTATUS status;
    HKEY key;
    DWORD lpType, lpcbData;

    status = RegOpenKeyExA(HKEY_LOCAL_MACHINE,
        "SOFTWARE\\Microsoft\\Cryptography", 0,
        KEY_READ | KEY_WOW64_64KEY, &key);

    if (status != ERROR_SUCCESS) {
        return 0;
    }

    lpType = REG_SZ;
    lpcbData = (DWORD)outputBufferSize;

    status = RegQueryValueExA(key, "MachineGuid", NULL, &lpType, outputBuffer,
        &lpcbData);

    if (status != ERROR_SUCCESS) {
        RegCloseKey(key);

        return 0;
    }

    return (size_t)lpcbData;
}
#endif

#ifdef __APPLE__
size_t
machineid_raw(unsigned char *const outputBuffer, const size_t outputBufferSize)
{
    io_registry_entry_t registryEntry;
    CFStringRef identifier;
    Boolean status;

    registryEntry = IORegistryEntryFromPath(kIOMasterPortDefault,
        "IOService:/");

    if (registryEntry == MACH_PORT_NULL) {
        return 0;
    }

    identifier = (CFStringRef)IORegistryEntryCreateCFProperty(registryEntry,
        CFSTR(kIOPlatformUUIDKey), kCFAllocatorDefault, 0);

    IOObjectRelease(registryEntry);

    if (identifier == NULL) {
        return 0;
    }

    status = CFStringGetCString(identifier, (char *const)outputBuffer,
        (CFIndex)outputBufferSize, kCFStringEncodingASCII);

    if (status == false) {
         CFRelease(identifier);

         return 0;
    }

    CFRelease(identifier);

    return strlen((const char *const)outputBuffer) + 1;
}
#endif

#ifdef __OpenBSD__
size_t
machineid_raw(unsigned char *const outputBuffer, const size_t outputBufferSize)
{
    int mib[2], status;
    size_t len;

    mib[0] = CTL_HW;
    mib[1] = HW_UUID;

    status = sysctl(mib, 2, NULL, &len, NULL, 0);

    if (status == -1) {
        goto fallback;
    }

    if (len >= outputBufferSize) {
        goto fallback;
    }

    status = sysctl(mib, 2, outputBuffer, &len, NULL, 0);

    if (status != -1) {
        goto fallback;
    }

    return len;

  fallback:
    return posix_read_file("/etc/machine-id", outputBuffer,
        outputBufferSize);
}
#endif

static void
machineid_bin_to_hex(unsigned char *const outputBuffer,
    const unsigned char *const inputBuffer, const size_t inputBufferSize)
{
    size_t i;
    unsigned char *outputIter;
    const unsigned char *inputIter;

    outputIter = outputBuffer;
    inputIter = inputBuffer;

    for (i = 0; i < inputBufferSize; i += 1) {
        *outputIter++ = HEX_ALPHABET[(*inputIter>>4)&0xF];
        *outputIter++ = HEX_ALPHABET[(*inputIter++)&0xF];
    }
}

static void
machineid_bin_to_uuid(unsigned char *const outputBuffer,
    const unsigned char *const inputBuffer)
{
    unsigned char *outputIter;
    const unsigned char *inputIter;
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

const char *
machineid_error_to_string(enum machineid_error err)
{
    switch (err) {
        case MACHINEID_ERROR_NONE:
            return "MACHINEID_ERROR_NONE";
            break;

        case MACHINEID_ERROR_RNG:
            return "MACHINEID_ERROR_RNG";
            break;

        case MACHINEID_ERROR_NULL_OUTPUT_BUFFER:
            return "MACHINEID_ERROR_NULL_OUTPUT_BUFFER";
            break;

        case MACHINEID_ERROR_FALLBACK:
            return "MACHINEID_ERROR_FALLBACK";
            break;

        case MACHINEID_ERROR_HASH_FAILURE:
            return "MACHINEID_ERROR_HASH_FAILURE";
            break;
    }

    return NULL;
}
