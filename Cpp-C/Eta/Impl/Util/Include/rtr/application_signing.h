/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2018. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */
 
#ifndef _APPLICATION_SIGNING_H_
#define _APPLICATION_SIGNING_H_


#include "rtr/os.h"

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <time.h>
#ifdef _WIN32
#include <windows.h>
#include <wincrypt.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


	/* Static encryption type definitions */

	static const rtrUInt8 TR_SL_1 = 1;




/**********************************************************************
* The following 3 functions are used for the Diffie Hellman key 
* exchange without having to include any openSource or 3rd party
* libraries.
**********************************************************************/

/* Simple random number generator that is resonably random. */
/* generates 64 bit unsigned random value */
RTR_C_ALWAYS_INLINE rtrUInt64 randull() {
#ifdef _WIN32
	rtrUInt64	li = 0;
	HCRYPTPROV prov;

	/* Since dwFlags is usually zero and has been working for most applications, a second
	attempt is be added with the flag _VERIFYCONTEXT for a rare case found randull returned 0 */
	if (!CryptAcquireContext(&prov, NULL, NULL, PROV_RSA_FULL, 0)) {
		if (!CryptAcquireContext(&prov, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
			if (!CryptAcquireContext(&prov, NULL, NULL, PROV_RSA_FULL, CRYPT_NEWKEYSET))
				return 0;
		}
	}
	CryptGenRandom(prov, sizeof(li), (BYTE *)&li);
	CryptReleaseContext(prov, 0);
	li &= 0x7fffffffffffffffULL;
	return li;
#else
	rtrUInt8   buffer[sizeof(rtrUInt64)];
    rtrUInt64  iBuffer;
	rtrInt32	fd;

	/* open /dev/random to get random byte stream */
     memset(buffer, 0, sizeof(buffer));
     fd = open("/dev/urandom", O_RDONLY);
	/* Problems opening the device, just use rand() */
     if(fd == -1) {
		rtrUInt64	ullTemp;
deviceFailure:
        srand((int)time((time_t *)NULL));
		ullTemp = 0ULL;
		ullTemp = rand();
		ullTemp += ((rtrUInt64)rand()) << 16; 
		ullTemp += ((rtrUInt64)rand()) << 32; 
		ullTemp += ((rtrUInt64)rand()) << 48; 
                return(ullTemp & 0x7fffffffffffffffULL);
        }
	/* read the random byte stream */
        if(read(fd, buffer, sizeof(buffer)) != sizeof(buffer)) {
                close(fd);
                goto deviceFailure;
        }
        close(fd);
        memcpy(&iBuffer, buffer, sizeof(iBuffer));
        return iBuffer & 0x7fffffffffffffffULL;
#endif
}

/* Simple random number generator that is resonably random. */
/* generates 32 bit unsigned random value */
RTR_C_ALWAYS_INLINE rtrUInt32 randu32() 
{
#ifdef _WIN32
	rtrUInt32 li = 0;
	HCRYPTPROV prov;

	if (!CryptAcquireContext(&prov, NULL, NULL, PROV_RSA_FULL, 0)) {
		if (!CryptAcquireContext(&prov, NULL, NULL, PROV_RSA_FULL, CRYPT_NEWKEYSET)) {
			return (rtrUInt32)time((time_t *)NULL);
		}
	}
	CryptGenRandom(prov, sizeof(li), (BYTE *)&li);
	CryptReleaseContext(prov, 0);
	return li;
#else
        rtrUInt8   buffer[sizeof(rtrUInt32)];
        rtrUInt32  iBuffer;
		rtrInt32	fd;

	/* open /dev/random to get random byte stream */
        memset(buffer, 0, sizeof(buffer));
        fd = open("/dev/urandom", O_RDONLY);
	/* Problems opening the device, just use rand() */
        if(fd == -1) {
		rtrUInt32	ulTemp;
deviceFailure:
        srand((rtrInt32)time((time_t *)NULL));
		ulTemp = 0UL;
		ulTemp = rand();
		ulTemp += ((rtrUInt64)rand()) << 16; 
                return(ulTemp);
        }
	/* read the random byte stream */
        if(read(fd, buffer, sizeof(buffer)) != sizeof(buffer)) {
                close(fd);
                goto deviceFailure;
        }
        close(fd);
        memcpy(&iBuffer, buffer, sizeof(iBuffer));
        return iBuffer;
#endif
}


/* This function will mutiply 2 numbers without overflow with mod */
RTR_C_ALWAYS_INLINE rtrUInt64 MulMod(rtrUInt64 a, rtrUInt64 b, rtrUInt64 m) {
	rtrUInt64	x = 0,y=a%m;
	while(b > 0){
		if(b%2 == 1) {
			x = (x+y)%m;
		}
		y = (y*2)%m;
		b /= 2;
	}
	return x%m;
}

/* This function will compute (base^exp)%mod */
RTR_C_ALWAYS_INLINE rtrUInt64 modPowFast(rtrUInt64 base, rtrUInt64 exp, rtrUInt64 mod) {
	rtrUInt64 a=base, b=exp, c=1;
	while (b) {
		while( !(b & 1) ) {
			b>>=1;
			a= MulMod(a, a, mod);
		}
		b--;
		c=MulMod(a, c, mod);
	}
	return c;
}

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
