/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef __SSL_JSONTORSSLMSGDECODER
#define __SSL_JSONTORSSLMSGDECODER
#include "jsonToRwfSimple.h"

class jsonToRsslMsgDecoder : public jsonToRwfSimple
{
 public:
	jsonToRsslMsgDecoder(int bufSize, unsigned int flags, int numTokens, int incSize);
	~jsonToRsslMsgDecoder() {}

 protected:
	bool encodeMsgPayload(RsslMsg *rsslMsgPtr, jsmntok_t *dataTokPtr);
};
#endif
