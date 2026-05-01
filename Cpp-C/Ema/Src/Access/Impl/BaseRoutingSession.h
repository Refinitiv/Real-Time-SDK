/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

 
#include "rtr/rsslTransport.h"
#include "rtr/rwfNet.h"
#include "rtr/rsslReactor.h" 

#include "EmaConfigImpl.h"
#include "ActiveConfig.h"
#include "LoginCallbackClient.h"
 
#ifndef __refinitiv_ema_access_RoutingSession_h
#define __refinitiv_ema_access_RoutingSession_h

namespace refinitiv {

namespace ema {

namespace access {

	typedef enum
	{
		NONE = 0,
		CONSUMER = 1,
		NI_PROVIDER = 2
	}RoutingSessionType;

	class BaseRoutingSessionChannel;
	// This class contains the full session, including the structures for each channel 
	class BaseRoutingSession
	{		
	public:
		BaseRoutingSession(OmmBaseImpl&);
		virtual ~BaseRoutingSession();

		OmmBaseImpl& baseImpl;
		ActiveConfig& activeConfig;

		RoutingSessionType sessionType;

		int activeChannelCount;

		EmaVector <BaseRoutingSessionChannel*> routingChannelList;

		bool initialLoginRefreshReceived;
		bool sentInitialLoginRefresh;
		LoginInfo	aggregatedLoginInfo;
		StatusMsg _statusMsg;

		bool aggregateLoginRefreshInfo(RsslRDMLoginRefresh*);

		void processChannelEvent(BaseRoutingSessionChannel*, RsslReactorChannelEvent*);

		// Assumption for destructor and clear: All channels have already been closed and cleaned up 
		virtual void clear();

		void closeChannel(RsslReactorChannel* pRsslReactorChannel);

		void closeReactorChannels();

		int getReconnectingCount();

	};

}

}

}

#endif