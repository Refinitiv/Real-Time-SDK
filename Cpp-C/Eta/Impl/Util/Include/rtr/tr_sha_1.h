/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2018. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef _TR_SHA_1_H_
#define _TR_SHA_1_H_

#include "application_signing.h"
#include "rtr/os.h"

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32) || defined(LINUX)
#define __LITTLE_ENDIAN__
#endif


#ifdef __BIG_ENDIAN__
# define SHA_BIG_ENDIAN
#elif defined __LITTLE_ENDIAN__
/* override */
#elif defined __BYTE_ORDER
# if __BYTE_ORDER__ ==  __ORDER_BIG_ENDIAN__
# define SHA_BIG_ENDIAN
# endif
#else // ! defined __LITTLE_ENDIAN__
#ifdef LINUX
# include <endian.h> // machine/endian.h
#endif
# if __BYTE_ORDER__ ==  __ORDER_BIG_ENDIAN__
#  define SHA_BIG_ENDIAN
# endif
#endif

/* Forward reference SHA-1 functions */
#ifndef _WIN32
#include <stdint.h>
#endif

#define HASH_LENGTH 20
#define BLOCK_LENGTH 64

typedef struct sha1nfo {
	rtrUInt32 buffer[BLOCK_LENGTH/4];
	rtrUInt32 state[HASH_LENGTH/4];
	rtrUInt32 byteCount;
	rtrUInt8 bufferOffset;
	rtrUInt8 keyBuffer[BLOCK_LENGTH];
	rtrUInt8 innerHash[HASH_LENGTH];
} sha1nfo;

void sha1_init(sha1nfo *s);
int sha1_init_from_buffer(sha1nfo *s, RwfBuffer *sha1InitValue);
void sha1_writebyte(sha1nfo *s, rtrUInt8 data);
void sha1_write(sha1nfo *s, const char *data, size_t len);
rtrUInt8* sha1_result(sha1nfo *s);
extern void SHA_1_test(const char *pStringToDigest);



#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
