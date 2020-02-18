/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */


#include "rtr/jsonToRsslMsgDecoder.h"

jsonToRsslMsgDecoder::jsonToRsslMsgDecoder(int bufSize, unsigned int flags, RsslUInt16 defaultSrvcId, int numTokens, int incSize) :
	jsonToRwfSimple(bufSize, flags, defaultSrvcId, numTokens, incSize)
{ }

bool jsonToRsslMsgDecoder::encodeMsgPayload(RsslMsg *rsslMsgPtr, jsmntok_t *dataTokPtr)
{
	if (rsslMsgPtr->msgBase.domainType == RSSL_DMT_DICTIONARY || rsslMsgPtr->msgBase.domainType == RSSL_DMT_LOGIN ||
		rsslMsgPtr->msgBase.domainType == RSSL_DMT_SOURCE || rsslMsgPtr->msgBase.domainType == RSSL_DMT_SYMBOL_LIST)
	{
		return jsonToRwfSimple::encodeMsgPayload(rsslMsgPtr, dataTokPtr);
	}
	rsslMsgPtr->msgBase.encDataBody.data = 0;
	rsslMsgPtr->msgBase.encDataBody.length = 0;
	return true;
}
