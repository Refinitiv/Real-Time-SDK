/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

 
#include "rtr/rsslTransport.h"
#include "rtr/rwfNet.h"
#include "rtr/rsslReactor.h" 

#include "EmaConfigImpl.h"
#include "ActiveConfig.h"
#include "LoginCallbackClient.h"
#include "BaseRoutingSession.h"
 
#ifndef __refinitiv_ema_access_NiProviderSession_h
#define __refinitiv_ema_access_NiProviderSession_h

namespace refinitiv {

namespace ema {

namespace access {

	class NiProviderRoutingSessionChannel;
	// This class contains the full session, including the structures for each channel 
	class NiProviderRoutingSession : public BaseRoutingSession
	{		
	public:
		NiProviderRoutingSession(OmmBaseImpl&);
		virtual ~NiProviderRoutingSession();

		// Assumption for destructor and clear: All channels have already been closed and cleaned up 
		void clear();
	};

}

}

}

#endif