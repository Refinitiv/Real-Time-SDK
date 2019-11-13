/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef TEST_TUNNEL_STREAM_PROVIDER_H
#define TEST_TUNNEL_STREAM_PROVIDER_H

#include "rtr/rsslReactor.h"
#include "rtr/rsslTunnelStream.h"
#include "TestReactorEvent.h"
#include "TestReactorComponent.h"
#include "Provider.h"
#include "rtr/rsslClassOfService.h"
#include "gtest/gtest.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DEFAULT_MAX_MSG_SIZE 614400
#define DEFAULT_MAX_FRAG_SIZE 6144

/* This provider always accepts tunnel streams. */
class TunnelStreamProvider : public Provider
{
public:
	static RsslInt _maxMsgSize;
	static RsslInt _maxFragmentSize;

	TunnelStreamProvider(TestReactor* pTestReactor);

	static void maxMsgSize(RsslInt maxMsgSize);

	static void maxFragmentSize(RsslInt maxFragmentSize);

	static RsslReactorCallbackRet tunnelStreamListenerCallback(RsslTunnelStreamRequestEvent* pEvent, RsslErrorInfo* pErrorInfo);
};

#ifdef __cplusplus
};
#endif

#endif
