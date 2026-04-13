/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

 
 // These classes represent a the information for and config of a single Reactor Channel instance within the Consumer Session for the request routing concept.
 // The SessionChannel has containers for the Channels, ChannelConfig for the ChannelSet, WarmStandbyChannelConfig, and ChannelConfig for the various Warm Standby channels.
 // A Consumer Session will contain multiple instances of these classes.

#include "EmaConfigImpl.h"
#include "ActiveConfig.h"

#include "rtr/rsslTransport.h"
#include "rtr/rwfNet.h"
#include "rtr/rsslReactor.h"
#include "BaseRoutingSession.h"
#include "BaseRoutingChannel.h"
#include "NiProviderRoutingSession.h"
#include "ChannelCallbackClient.h"
#include "DirectoryCallbackClient.h"
#include "LoginCallbackClient.h"

#ifndef __refinitiv_ema_access_NiProviderSessionChannel_h
#define __refinitiv_ema_access_NiProviderSessionChannel_h

namespace refinitiv {

	namespace ema {

		namespace access {
			typedef const EmaString* EmaStringPtr;
			class NiProviderRoutingSessionChannel;

			// This class defines the configuration of a single Reactor Channel in the Consumer Session
			// This class lifetime will be managed by the ActiveConfig, and all and all cleanup of NiProviderRoutingSessionChannelConfig objects will be through there.
			class NiProviderRoutingSessionChannelConfig : public BaseRoutingSessionChannelConfig
			{
				public:

				NiProviderRoutingSessionChannelConfig(const EmaString&, ActiveConfig&);
				virtual ~NiProviderRoutingSessionChannelConfig();


				void clear();

			};

			// This class defines an instance of a single Reactor Channel in the Consumer Session
			// This is a rough equivalent of the Channel object
			// This class lifetime will be managed by the NiProviderRoutingSession class, and all cleanup of NiProviderRoutingSessionChannel objects will be through there.
			// Note for timing: We're 
			class NiProviderRoutingSessionChannel : public BaseRoutingSessionChannel
			{

			public:
				NiProviderRoutingSessionChannel(OmmBaseImpl&, const EmaString&, NiProviderRoutingSessionChannelConfig&);
				virtual ~NiProviderRoutingSessionChannel();

				void clear();

				RsslBuffer*		_transportBuffer;		// Used during submit message fanout and directory fanout.

				NiProviderRoutingSessionChannelConfig& routingChannelConfig;

				NiProviderRoutingSession* pRoutingSession;
			};

		}

	}

}

#endif