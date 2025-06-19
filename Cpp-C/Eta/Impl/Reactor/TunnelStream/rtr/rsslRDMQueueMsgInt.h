/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015,2019,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef RSSL_RDM_QUEUE_MSG_INT_H
#define RSSL_RDM_QUEUE_MSG_INT_H

#include "rtr/rsslRDMQueueMsg.h"

#ifdef __cplusplus
extern "C" {
#endif

RSSL_VA_API RsslRet rsslDecodeRDMQueueMsg(RsslDecodeIterator *pIter, RsslMsg *pMsg, RsslRDMQueueMsg *pQueueMsg, RsslBuffer *pMemoryBuffer, RsslErrorInfo *pError);

#ifdef __cplusplus
};
#endif

#endif
