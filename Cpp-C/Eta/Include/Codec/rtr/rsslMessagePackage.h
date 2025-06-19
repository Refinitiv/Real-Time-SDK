/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015,2018-2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef __RSSL_MESSAGE_PACKAGE_H
#define __RSSL_MESSAGE_PACKAGE_H



#ifdef __cplusplus
extern "C"
{
#endif

/* Include all header files necessary to use the RSSL Message Package */
#include "rtr/rsslMsg.h"
#include "rtr/rsslRequestMsg.h"
#include "rtr/rsslRefreshMsg.h"
#include "rtr/rsslStatusMsg.h"
#include "rtr/rsslUpdateMsg.h"
#include "rtr/rsslCloseMsg.h"
#include "rtr/rsslAckMsg.h"
#include "rtr/rsslGenericMsg.h"
#include "rtr/rsslPostMsg.h"

#include "rtr/rsslMsgEncoders.h"
#include "rtr/rsslMsgDecoders.h"

/** 
 *	@addtogroup MsgPkgVersion Message Package Library Version Helper
 *	@{
 */

/** 
 * @brief Programmatically extracts library and product version information that is compiled into this library
 *
 * User can call this function to programmatically extract version information, or <BR>
 * query version information externally (via 'strings' command or something similar<BR>
 * and grep for the following:<BR>
 * 'VERSION' - contains internal library version information (e.g. eta3.6.0.1)<BR>
 * 'PACKAGE' - contains product information for load/package naming<BR>
 * @param pVerInfo RsslLibraryVersionInfo structure to populate with library version information
 * @see RsslLibraryVersionInfo
 */
RSSL_API void rsslQueryMessagesLibraryVersion(RsslLibraryVersionInfo *pVerInfo);

/**
 * @}
 */



#ifdef __cplusplus
}
#endif


#endif /* __RSSL_MESSAGES_H */



