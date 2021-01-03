#include "machineid.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void
test_error_string_encoding()
{
    assert(strcmp("MACHINEID_ERROR_NONE",
        machineid_error_to_string(MACHINEID_ERROR_NONE)) == 0);
    assert(strcmp("MACHINEID_ERROR_RNG",
        machineid_error_to_string(MACHINEID_ERROR_RNG)) == 0);
    assert(strcmp("MACHINEID_ERROR_NULL_OUTPUT_BUFFER",
        machineid_error_to_string(MACHINEID_ERROR_NULL_OUTPUT_BUFFER)) == 0);
    assert(strcmp("MACHINEID_ERROR_FALLBACK",
        machineid_error_to_string(MACHINEID_ERROR_FALLBACK)) == 0);
    assert(strcmp("MACHINEID_ERROR_HASH_FAILURE",
        machineid_error_to_string(MACHINEID_ERROR_HASH_FAILURE)) == 0);
    assert(machineid_error_to_string(52) == NULL);
}

static void
test_null_output_buffer()
{
    assert(MACHINEID_ERROR_NULL_OUTPUT_BUFFER == machineid_generate(
        NULL, MACHINEID_FLAG_DEFAULT));
}

static void
test_null_terminate_hash()
{
    unsigned char buffer[MACHINEID_HASH_SIZE + 1];

    buffer[MACHINEID_HASH_SIZE + 1] = 52;

    machineid_generate(buffer, MACHINEID_FLAG_NULL_TERMINATE);

    assert(buffer[MACHINEID_HASH_SIZE + 1] == '\0');
}

static void
test_null_terminate_uuid()
{
    unsigned char buffer[MACHINEID_UUID_SIZE + 1];

    buffer[MACHINEID_UUID_SIZE + 1] = 52;

    machineid_generate(buffer, MACHINEID_FLAG_NULL_TERMINATE
        | MACHINEID_FLAG_AS_UUID);

    assert(buffer[MACHINEID_UUID_SIZE + 1] == '\0');
}

static void
test_uuid_not_terminated()
{
    unsigned char buffer[MACHINEID_UUID_SIZE];

    memset(buffer, '\0', MACHINEID_UUID_SIZE);

    machineid_generate(buffer, MACHINEID_FLAG_AS_UUID);

    assert(buffer[MACHINEID_UUID_SIZE] != '\0');
}

int
main()
{
    test_error_string_encoding();
    test_null_output_buffer();
    test_null_terminate_hash();
    test_null_terminate_uuid();
    test_uuid_not_terminated();

    unsigned char buffer[MACHINEID_UUID_SIZE + 1];

    machineid_generate(buffer, MACHINEID_FLAG_AS_UUID
        | MACHINEID_FLAG_NULL_TERMINATE);

    printf("machine id: %s\n", buffer);

    return 0;
}
