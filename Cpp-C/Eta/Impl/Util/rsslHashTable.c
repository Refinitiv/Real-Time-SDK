/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019 LSEG. All rights reserved.                 --
 *|-----------------------------------------------------------------------------
 */

#include "rtr/rsslHashTable.h"

/* U16 */

RSSL_API RsslUInt32 rsslHashU16Sum(void *pKey)
{ return (RsslUInt32)(*((RsslUInt16*)pKey)); }

RSSL_API RsslBool rsslHashU16Compare(void *pKey1, void *pKey2)
{ return (*(RsslUInt16*)pKey1) == (*(RsslUInt16*)pKey2); }

/* U32 */

RSSL_API RsslUInt32 rsslHashU32Sum(void *pKey)
{ return (RsslUInt32)(*((RsslUInt32*)pKey)); }

RSSL_API RsslBool rsslHashU32Compare(void *pKey1, void *pKey2)
{ return (*(RsslUInt32*)pKey1) == (*(RsslUInt32*)pKey2); }

/* U64 */

RSSL_API RsslUInt32 rsslHashU64Sum(void *pKey)
{ return (RsslUInt32)(*((RsslUInt64*)pKey)); }

RSSL_API RsslBool rsslHashU64Compare(void *pKey1, void *pKey2)
{ return (*(RsslUInt64*)pKey1) == (*(RsslUInt64*)pKey2); }

/* RsslBuffer */

RSSL_API RsslUInt32 rsslHashBufferSum(void *pKey)
{
	RsslUInt32 i;
	RsslUInt32 hashSum = 0;
	RsslBuffer *pBuffer = (RsslBuffer*)pKey;

	for(i = 0; i < pBuffer->length; ++i)
	{
		hashSum = (hashSum << 4) + (RsslUInt32)pBuffer->data[i];
		hashSum ^= (hashSum >> 12);
	}

	return hashSum;
}

RSSL_API RsslBool rsslHashBufferCompare(void *pKey1, void *pKey2)
{ return rsslBufferIsEqual((RsslBuffer*)pKey1, (RsslBuffer*)pKey2); }
