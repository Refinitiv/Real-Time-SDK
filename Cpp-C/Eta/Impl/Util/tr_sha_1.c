/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2018. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>

#include "rtr/tr_sha_1.h"

/* code */
#define SHA1_K0  0x5a827999
#define SHA1_K20 0x6ed9eba1
#define SHA1_K40 0x8f1bbcdc
#define SHA1_K60 0xca62c1d6

void sha1_init(sha1nfo *s) {
#ifdef REAL_SHA1_INIT
	s->state[0] = 0x67452301;
	s->state[1] = 0xefcdab89;
	s->state[2] = 0x98badcfe;
	s->state[3] = 0x10325476;
	s->state[4] = 0xc3d2e1f0;
#else
	/* Used random.org @ 2014-11-18 15:55:56 UTC */
	s->state[0] = 0xeaf6ac5f;
	s->state[1] = 0xc4d74635;
	s->state[2] = 0xa4ac2afd;
	s->state[3] = 0x677d8f6f;
	s->state[4] = 0xcf4b7f9b;
#endif
	s->byteCount = 0;
	s->bufferOffset = 0;
}

/* buffer must have 4*5 = 20 bytes */
/* returns -1 if failure, 0 if success */
int sha1_init_from_buffer(sha1nfo *s, RwfBuffer *sha1InitValue)
{
	if ((sha1InitValue->length <20) || (!sha1InitValue->data))
		return -1;

	memcpy(s->state, sha1InitValue->data, 20);
	s->byteCount = 0;
	s->bufferOffset = 0;
	return 0;

}

static rtrUInt32 sha1_rol32(rtrUInt32 number, rtrUInt8 bits) {
	return ((number << bits) | (number >> (32-bits)));
}

static void sha1_hashBlock(sha1nfo *s) {
	rtrUInt8 i;
	rtrUInt32 a,b,c,d,e,t;

	a=s->state[0];
	b=s->state[1];
	c=s->state[2];
	d=s->state[3];
	e=s->state[4];
	for (i=0; i<80; i++) {
		if (i>=16) {
			t = s->buffer[(i+13)&15] ^ s->buffer[(i+8)&15] ^ s->buffer[(i+2)&15] ^ s->buffer[i&15];
			s->buffer[i&15] = sha1_rol32(t,1);
		}
		if (i<20) {
			t = (d ^ (b & (c ^ d))) + SHA1_K0;
		} else if (i<40) {
			t = (b ^ c ^ d) + SHA1_K20;
		} else if (i<60) {
			t = ((b & c) | (d & (b | c))) + SHA1_K40;
		} else {
			t = (b ^ c ^ d) + SHA1_K60;
		}
		t+=sha1_rol32(a,5) + e + s->buffer[i&15];
		e=d;
		d=c;
		c=sha1_rol32(b,30);
		b=a;
		a=t;
	}
	s->state[0] += a;
	s->state[1] += b;
	s->state[2] += c;
	s->state[3] += d;
	s->state[4] += e;
}

static void sha1_addUncounted(sha1nfo *s, rtrUInt8 data) {
	rtrUInt8 * const b = (rtrUInt8*) s->buffer;
#ifdef SHA_BIG_ENDIAN
	b[s->bufferOffset] = data;
#else
	b[s->bufferOffset ^ 3] = data;
#endif
	s->bufferOffset++;
	if (s->bufferOffset == BLOCK_LENGTH) {
		sha1_hashBlock(s);
		s->bufferOffset = 0;
	}
}

void sha1_writebyte(sha1nfo *s, rtrUInt8 data) {
	++s->byteCount;
	sha1_addUncounted(s, data);
}

void sha1_write(sha1nfo *s, const char *data, size_t len) {
	for (;len--;) 
		sha1_writebyte(s, (rtrUInt8) *data++);
}

static void sha1_pad(sha1nfo *s) {
	// Implement SHA-1 padding (fips180-2 Ȧ5.1.1)

	// Pad with 0x80 followed by 0x00 until the end of the block
	sha1_addUncounted(s, 0x80);
	while (s->bufferOffset != 56) sha1_addUncounted(s, 0x00);

	// Append length in the last 8 bytes
	sha1_addUncounted(s, 0); // We're only using 32 bit lengths
	sha1_addUncounted(s, 0); // But SHA-1 supports 64 bit lengths
	sha1_addUncounted(s, 0); // So zero pad the top bits
	sha1_addUncounted(s, s->byteCount >> 29); // Shifting to multiply by 8
	sha1_addUncounted(s, s->byteCount >> 21); // as SHA-1 supports bitstreams as well as
	sha1_addUncounted(s, s->byteCount >> 13); // byte.
	sha1_addUncounted(s, s->byteCount >> 5);
	sha1_addUncounted(s, s->byteCount << 3);
}

rtrUInt8* sha1_result(sha1nfo *s) {
	int i = 0;

	// Pad to complete the last block
	sha1_pad(s);

#ifndef SHA_BIG_ENDIAN
	// Swap byte order back
	for (i=0; i<5; i++) {
		s->state[i]=
			  (((s->state[i])<<24)& 0xff000000)
			| (((s->state[i])<<8) & 0x00ff0000)
			| (((s->state[i])>>8) & 0x0000ff00)
			| (((s->state[i])>>24)& 0x000000ff);
	}
#endif

	// Return pointer to hash (20 characters)
	return (rtrUInt8*) s->state;
}

static void printHash(rtrUInt8* hash) {
	int i;
	printf("TR SHA-1:");
	for (i=0; i<20; i++) {
		printf("%02x", hash[i]);
	}
	putchar('\n');
}

void SHA_1_test(const char *pStringToDigest) {
	sha1nfo s;

	sha1_init(&s);
	sha1_write(&s, pStringToDigest, strlen(pStringToDigest));
	printHash(sha1_result(&s));
}
