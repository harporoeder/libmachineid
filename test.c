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
    enum machineid_error err;
    unsigned char buffer[MACHINEID_UUID_SIZE + 1];

    test_error_string_encoding();
    test_null_output_buffer();
    test_null_terminate_hash();
    test_null_terminate_uuid();
    test_uuid_not_terminated();

    err = machineid_generate(buffer, MACHINEID_FLAG_AS_UUID
        | MACHINEID_FLAG_NULL_TERMINATE);

    printf("error: %s\n", machineid_error_to_string(err));

    if (err != MACHINEID_ERROR_NONE && err != MACHINEID_ERROR_FALLBACK) {
        return 1;
    }

    printf("machine id: %s\n", buffer);

    return 0;
}
