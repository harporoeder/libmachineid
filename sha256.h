/*********************************************************************
* Filename:   sha256.h
* Author:     Brad Conte (brad AT bradconte.com)
* Copyright:
* Disclaimer: This code is presented "as is" without any guarantees.
* Details:    Defines the API for the corresponding SHA1 implementation.

This code is released into the public domain free of any restrictions.
The author requests acknowledgement if the code is used, but does not require
it. This code is provided free of any liability and without any quality claims
by the author.
*********************************************************************/

#ifndef SHA256_H
#define SHA256_H

/*************************** HEADER FILES ***************************/
#include <stddef.h>
#include <stdint.h>

/****************************** MACROS ******************************/
#define SHA256_BLOCK_SIZE 32

/**************************** DATA TYPES ****************************/
typedef uint8_t LIBSHA256_BYTE;
typedef uint32_t LIBSHA256_WORD;

typedef struct {
	LIBSHA256_BYTE data[64];
	LIBSHA256_WORD datalen;
	uint64_t bitlen;
	LIBSHA256_WORD state[8];
} SHA256_CTX;

/*********************** FUNCTION DECLARATIONS **********************/
void sha256_init(SHA256_CTX *ctx);
void sha256_update(SHA256_CTX *ctx, const LIBSHA256_BYTE data[], size_t len);
void sha256_final(SHA256_CTX *ctx, LIBSHA256_BYTE hash[]);

#endif
