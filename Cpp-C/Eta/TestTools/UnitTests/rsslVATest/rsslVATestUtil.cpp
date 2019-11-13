/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "rsslVATestUtil.h"
#include "testFramework.h"

void writeRDMMsg(
		RsslRDMMsg *pRDMMsgToWrite, TestWriteAction action, 
		RsslMsg *pRsslMsgToDecodeTo, RsslRDMMsg *pRDMMsgToDecodeTo,
		TypedMessageStats *pStats)
{
	RsslEncodeIterator eIter;
	RsslDecodeIterator dIter;

	RsslBuffer msgBuffer;
	RsslBuffer memoryBuffer, origMemoryBuffer;
	RsslErrorInfo rsslErrorInfo;

	switch(action)
	{
		case TEST_EACTION_ENCODE:
		{
			setupEncodeIterator(&eIter, &msgBuffer);

			ASSERT_TRUE(rsslEncodeRDMMsg(&eIter, pRDMMsgToWrite, &msgBuffer.length, &rsslErrorInfo) == RSSL_RET_SUCCESS);

			updateTypedMessageMsgSize(pStats, msgBuffer.length);

			setupDecodeIterator(&dIter, &msgBuffer, &memoryBuffer);
			origMemoryBuffer = memoryBuffer;

			rsslClearRDMMsg(pRDMMsgToDecodeTo);
			ASSERT_TRUE(rsslDecodeRDMMsg(&dIter, pRsslMsgToDecodeTo, pRDMMsgToDecodeTo, &memoryBuffer, &rsslErrorInfo) == RSSL_RET_SUCCESS);

			updateTypedMessageMemoryBufferSize(pStats, origMemoryBuffer.length - memoryBuffer.length);
			incrTestedEncodes(pStats);
			break;
		}
		case TEST_EACTION_COPY:
		{
			setupMemoryBuffer(&memoryBuffer);
			origMemoryBuffer = memoryBuffer;
			ASSERT_TRUE(rsslCopyRDMMsg(pRDMMsgToDecodeTo, pRDMMsgToWrite, &memoryBuffer) == RSSL_RET_SUCCESS);

			updateTypedMessageCopyMemoryBufferSize(pStats, origMemoryBuffer.length - memoryBuffer.length);
			incrTestedCopies(pStats);
			break;
		}
		default:
			abort();
			break;
	}
}
