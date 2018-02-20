/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2015. All rights reserved.
*/

#ifndef _RSSL_REACTOR_UTIL_H
#define _RSSL_REACTOR_UTIL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "rtr/rsslTransport.h"
#include "rtr/rsslMsg.h"

#ifdef WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif
#include <stdlib.h>

#define verify_malloc(__statement, __pErrorInfo, __returnVal) \
	{ if (!(__statement)) { rsslSetErrorInfo(__pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Memory allocation failed."); return  __returnVal; }}

/* Compares two sequence numbers.
 * Returns a negative value if the first sequence number is considered to be "before" the second.
 * Returns 0 if they are equal.
 * Returns a positive if the first sequence number is considered to be "after" the second.  */
RTR_C_INLINE RsslInt32 rsslSeqNumCompare(RsslUInt32 seqNum1, RsslUInt32 seqNum2)
{
	return (RsslInt32)(seqNum1 - seqNum2);
}


/* Gets the current time. ticksPerMsec is used only on windows. */
RTR_C_INLINE RsslInt64 getCurrentTimeMs(RsslInt64 ticksPerMsec)
{
	RsslInt64 timeMs;
#ifdef WIN32
	LARGE_INTEGER	queryTime;


	QueryPerformanceCounter(&queryTime);
	timeMs = (RsslInt64)((double)queryTime.QuadPart / ticksPerMsec);
	return timeMs;
#else
	struct timeval currentTime;

	gettimeofday(&currentTime, NULL);

	timeMs = currentTime.tv_sec;
	timeMs *= 1000;
	timeMs += currentTime.tv_usec / 1000;
#endif
	return timeMs;
}

/* Estimates the encoded length of an RsslMsg.  */
RTR_C_INLINE RsslUInt32 rsslGetEstimatedEncodedLength(RsslMsg *pRsslMsg)
{
	const RsslMsgKey *pKey;
	const RsslBuffer *pExtHeader;

	RsslUInt32 msgSize = 128;

	if (pRsslMsg->msgBase.containerType != RSSL_DT_NO_DATA)
		msgSize += pRsslMsg->msgBase.encDataBody.length;

	if ((pKey = rsslGetMsgKey(pRsslMsg)))
	{
		if (pKey->flags & RSSL_MKF_HAS_NAME)
			msgSize += pKey->name.length;

		if (pKey->flags & RSSL_MKF_HAS_ATTRIB)
			msgSize += pKey->encAttrib.length;
	}

	if ((pExtHeader = rsslGetExtendedHeader(pRsslMsg)))
		msgSize += pExtHeader->length;

	return msgSize;
}




#ifdef __cplusplus
};
#endif


#endif
