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

#ifndef MACHINEID_H
#define MACHINEID_H

#ifdef __cplusplus
extern "C" {
#endif

#define MACHINEID_VERSION "1.0.0"
#define MACHINEID_VERSION_MAJOR 1
#define MACHINEID_VERSION_MINOR 0
#define MACHINEID_VERSION_PATCH 0

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

const char *machineid_error_to_string(enum machineid_error err);

enum machineid_error machineid_generate(unsigned char *const outputBuffer,
    enum machineid_flags flags);

#ifdef __cplusplus
}
#endif

#endif
