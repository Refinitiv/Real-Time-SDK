/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019 LSEG. All rights reserved.                 --
 *|-----------------------------------------------------------------------------
 */

#include "TunnelStreamProvider.h"
#include <time.h>

using namespace std;

RsslInt TunnelStreamProvider::_maxMsgSize = DEFAULT_MAX_MSG_SIZE;
RsslInt TunnelStreamProvider::_maxFragmentSize = DEFAULT_MAX_FRAG_SIZE;

RsslBool TunnelStreamProvider::_delayAfterAccepting = RSSL_FALSE;


void time_sleep(int millisec)
{
#ifdef WIN32
	Sleep(millisec);
#else
	if (millisec)
	{
		struct timespec ts;
		ts.tv_sec = millisec / 1000;
		ts.tv_nsec = (millisec % 1000) * 1000000;
		nanosleep(&ts, NULL);
	}
#endif
}

TunnelStreamProvider::TunnelStreamProvider(TestReactor* pTestReactor, RsslBool acceptingDelay) : Provider(pTestReactor)
{
	_delayAfterAccepting = acceptingDelay;
}

RsslReactorCallbackRet TunnelStreamProvider::tunnelStreamListenerCallback(RsslTunnelStreamRequestEvent *pEvent, RsslErrorInfo *pErrorInfo)
{
	RsslReactorAcceptTunnelStreamOptions acceptOpts;
	RsslErrorInfo errorInfo;

	Provider::tunnelStreamListenerCallback(pEvent, pErrorInfo);
		
	/* Accept the tunnel stream request. */
	rsslClearReactorAcceptTunnelStreamOptions(&acceptOpts);
	acceptOpts.statusEventCallback = tunnelStreamStatusEventCallback;
	acceptOpts.defaultMsgCallback = tunnelStreamDefaultMsgCallback;
	acceptOpts.classOfService.common.maxFragmentSize = _maxFragmentSize;
	acceptOpts.classOfService.common.maxMsgSize = _maxMsgSize;
	acceptOpts.classOfService.dataIntegrity.type = RDM_COS_DI_RELIABLE;
	acceptOpts.classOfService.flowControl.type = RDM_COS_FC_BIDIRECTIONAL;
	if (pEvent->classOfServiceFilter & RDM_COS_AUTHENTICATION_FLAG)
	{
		acceptOpts.classOfService.authentication.type = RDM_COS_AU_OMM_LOGIN;
	}
	if (rsslReactorAcceptTunnelStream(pEvent, &acceptOpts, &errorInfo) < RSSL_RET_SUCCESS)
	{
		cout << "rsslReactorAcceptTunnelStream() failed with return code " << errorInfo.rsslError.rsslErrorId << " and error text " << errorInfo.rsslError.text << endl;
	}

	if (_delayAfterAccepting)
	{
		time_sleep(3000);
	}
		
	return RSSL_RC_CRET_SUCCESS;
}
	
void TunnelStreamProvider::maxMsgSize(RsslInt maxMsgSize)
{
	TunnelStreamProvider::_maxMsgSize = maxMsgSize;
}

void TunnelStreamProvider::maxFragmentSize(RsslInt maxFragmentSize)
{
	TunnelStreamProvider::_maxFragmentSize = maxFragmentSize;
}
