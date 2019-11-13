/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef TEST_TUNNEL_STREAM_CORE_PROVIDER_H
#define TEST_TUNNEL_STREAM_CORE_PROVIDER_H

#include "rtr/msgQueueHeader.h"
#include "CoreComponent.h"

#define MSG_BUF_LEN 512

#ifdef __cplusplus
extern "C" {
#endif

class TunnelStreamCoreProvider : public CoreComponent
{
private:
	/* Needed for RsslClassOfService decode */
	RsslErrorInfo _errorInfo;

public:
	void acceptTunnelStreamRequest(RsslRequestMsg* pRequestMsg, RsslClassOfService* pProvClassOfService);
	
	void submitTunnelStreamAck(TunnelStreamAck* pTunnelStreamAck);
	
	void submitTunnelStreamAck(TunnelStreamAck* pTunnelStreamAck, AckRangeList* pAckRangeList, AckRangeList* pNakRangeList, RsslInt actionOpCode);

	/* Submit a Codec RsslMsg in a TunnelStream. */
	void submitTunnelStreamMsg(TunnelStreamData* pTunnelStreamData, RsslMsg* pMsg);
};

#ifdef __cplusplus
};
#endif

#endif
