/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2024 LSEG. All rights reserved.
*/

#ifndef _RSSL_RDM_MSG_BASE_H
#define _RSSL_RDM_MSG_BASE_H

#include "rtr/rsslVAExports.h"
#include "rtr/rsslTransport.h"
#include "rtr/rsslMessagePackage.h"
#include "rtr/rsslRDM.h"
#include "rtr/rsslMemoryBuffer.h"
#include "rtr/rsslErrorInfo.h"


#ifdef __cplusplus
extern "C" {
#endif

/**
 *	@addtogroup VARDMCommonBase
 *	@{
 */

/**
 *	@brief This message structure contains the information that is common across all RDM Message formats.  It is included in all Value Added RDM Components.
 */
typedef struct
{
	RsslInt32		streamId;  		/*!< The Stream Id for the given item. */
	RsslUInt8		domainType;		/*!< The Domain type of this message.  */
	RsslInt32		rdmMsgType;		/*!< Class of the message.  These are defined on a per-domain basis.  
										@see RsslRDMLoginMsgType, RsslRDMDictionaryMsgType, RsslRDMDirectoryMsgType, RsslRDMQueueMsgType   */
} RsslRDMMsgBase;

/**
 *	@brief This is used to clear the data contained in the RsslRDMMsgBase structure. 
 */
RTR_C_INLINE void rsslClearRDMMsgBase(RsslRDMMsgBase *pBase)
{
	pBase->streamId = 0;
	pBase->domainType = 0;
	pBase->rdmMsgType = 0;
}

/**
 *	@}
 */

#ifdef __cplusplus
}
#endif

#endif
