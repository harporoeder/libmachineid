#include <stdio.h>

#include "machineid.h"

#ifndef MIN
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#endif

#ifdef __linux__
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