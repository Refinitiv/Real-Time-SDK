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
#include "ConsumerRoutingSession.h"
#include "ChannelCallbackClient.h"
#include "DirectoryCallbackClient.h"
#include "LoginCallbackClient.h"

#ifndef __refinitiv_ema_access_ConsumerSessionChannel_h
#define __refinitiv_ema_access_ConsumerSessionChannel_h

namespace refinitiv {

	namespace ema {

		namespace access {
			typedef const EmaString* EmaStringPtr;
			class ConsumerRoutingSessionChannel;

			// This class defines the configuration of a single Reactor Channel in the Consumer Session
			// This class lifetime will be managed by the ActiveConfig, and all and all cleanup of ConsumerRoutingSessionChannelConfig objects will be through there.
			class ConsumerRoutingSessionChannelConfig : public BaseRoutingSessionChannelConfig
			{
				public:

				ConsumerRoutingSessionChannelConfig(const EmaString&, ActiveConfig&);
				virtual ~ConsumerRoutingSessionChannelConfig();


				void clear();

				// Preferred host
				bool            enablePreferredHostOptions;				// This defaults to false in all cases
				EmaString		phDetectionTimeSchedule;
				UInt32          phDetectionTimeInterval;
				EmaString       preferredChannelName;
				EmaString       preferredWSBChannelName;
				bool			phFallBackWithInWSBGroup;

				// The following are used to configure any WSB configuration for a consumer channel, and this is what's used to pass into RsslReactorConnect.
				EmaVector< WarmStandbyChannelConfig* >  configWarmStandbySet;
				EmaVector< ChannelConfig* >		configChannelSetForWSB;

			protected:
				void clearWSBChannelSet();
				void clearChannelSetForWSB();
			};

			// This class defines an instance of a single Reactor Channel in the Consumer Session
			// This is a rough equivalent of the Channel object
			// This class lifetime will be managed by the consumerRoutingSession class, and all cleanup of ConsumerRoutingSessionChannel objects will be through there.
			// Note for timing: We're 
			class ConsumerRoutingSessionChannel : public BaseRoutingSessionChannel
			{
			protected:
				class UInt16rHasher
				{
				public:
					size_t operator()(const UInt16&) const;
				};

				class UInt16Equal_To
				{
				public:
					bool operator()(const UInt16&, const UInt16&) const;
				};

				class EmaStringPtrHasher
				{
				public:
					size_t operator()(const EmaStringPtr&) const;
				};

				class EmaStringPtrEqual_To
				{
				public:
					bool operator()(const EmaStringPtr&, const EmaStringPtr&) const;
				};

			public:
				ConsumerRoutingSessionChannel(OmmBaseImpl&, const EmaString&, ConsumerRoutingSessionChannelConfig&);
				virtual ~ConsumerRoutingSessionChannel();

				void clear();

				// Hash tables and a direct list of services 
				HashTable<UInt16, DirectoryPtr, UInt16rHasher, UInt16Equal_To> serviceById;			// keyed by the concrete service for this channel
																									// Note: hashTable.find returns a **Directory pointer.
				HashTable<EmaStringPtr, DirectoryPtr, EmaStringPtrHasher, EmaStringPtrEqual_To> serviceByName;	// keyed by the concrete service for this channel

				EmaList< Directory* > serviceList;

				ItemList routedRequestList;			// ItemList of items routed to this channel.		
			};

		}

	}

}

#endif