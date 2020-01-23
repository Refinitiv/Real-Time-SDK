/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __ripchttp_h
#define __ripchttp_h

#include "rtr/rsslSocketTransportImpl.h"

RsslInt32 iseof(char *data, RsslInt32 offset, RsslInt32 datalen);
RsslInt32 ripcHttpHdrToUpper(char* data, RsslInt32 datalen, RsslInt32 startOffset);
RsslInt32 ipcHttpHdrComplete(char*, RsslInt32, RsslInt32);

#endif /* __ripchttp_h */
