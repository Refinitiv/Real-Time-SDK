/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "testFramework.h"
#include "gtest/gtest.h"

static char msgBufferBlock[16384];
static char memoryBufferBlock[16384];
RsslInt64 totalTestCount = 0;
const char *funcName = NULL;

RsslUInt32 _createFlagCombinations(RsslUInt32** destFlags, const RsslUInt32* baseFlags, RsslUInt32 baseFlagsSize, RsslBool skipZero)
{
	RsslUInt32 skip = skipZero? 1 : 0;
	RsslUInt32 destFlagsSize = (RsslUInt32)pow(2.0, (int)baseFlagsSize) - skip;
	RsslUInt32 baseFlagsIter, destFlagsIter;

	*destFlags = (RsslUInt32*)malloc(destFlagsSize * sizeof(RsslUInt32));

	for (destFlagsIter = skip; destFlagsIter < destFlagsSize + skip; ++destFlagsIter)
	{
		(*destFlags)[destFlagsIter-skip] = 0;
		for (baseFlagsIter = 0; baseFlagsIter < baseFlagsSize; ++baseFlagsIter)
		{
			if ((destFlagsIter >> baseFlagsIter) & 0x1) 
				(*destFlags)[destFlagsIter-skip] |= baseFlags[baseFlagsIter];			
		}
	}

	return destFlagsSize;

}

void setupEncodeIterator(RsslEncodeIterator *pIter, RsslBuffer *pBuf)
{
	/* Setup msg buffer */
	pBuf->length = sizeof(msgBufferBlock);
	pBuf->data = msgBufferBlock;

	/* Setup iterator */
	rsslClearEncodeIterator(pIter);
	rsslSetEncodeIteratorRWFVersion(pIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION);
	rsslSetEncodeIteratorBuffer(pIter, pBuf);
}

void setupDecodeIterator(RsslDecodeIterator *pIter, RsslBuffer *pBuf, RsslBuffer *pMemBuf)
{
	/* Setup iterator */
	rsslClearDecodeIterator(pIter);
	rsslSetDecodeIteratorRWFVersion(pIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION);
	rsslSetDecodeIteratorBuffer(pIter, pBuf);

	pMemBuf->data = memoryBufferBlock;
	pMemBuf->length = sizeof(memoryBufferBlock);
}

void setupMemoryBuffer(RsslBuffer *pMemBuf)
{
	pMemBuf->data = memoryBufferBlock;
	pMemBuf->length = sizeof(memoryBufferBlock);
}

void createFileFromString(const char *pFilename, const char *pFileData, RsslInt32 length)
{
	FILE * fp = fopen(pFilename, "w");
	ASSERT_TRUE(fp);
	fwrite(pFileData, 1, length, fp);
	fclose(fp);
}

void deleteFile(const char *pFilename)
{
	remove(pFilename);
}
