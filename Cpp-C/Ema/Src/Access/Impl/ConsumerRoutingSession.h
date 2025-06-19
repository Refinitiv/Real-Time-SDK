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
#include "ConsumerRoutingService.h"
 
#ifndef __refinitiv_ema_access_ConsumerSession_h
#define __refinitiv_ema_access_ConsumerSession_h

namespace refinitiv {

namespace ema {

namespace access {

	class ConsumerRoutingSessionChannel;
	class ConsumerRoutingService;
	// This class contains the full session, including the structures for each channel 
	class ConsumerRoutingSession
	{		
	public:
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

		ConsumerRoutingSession(OmmBaseImpl&);
		virtual ~ConsumerRoutingSession();

		OmmBaseImpl& baseImpl;
		ActiveConfig& activeConfig;

		int activeChannelCount;
		UInt32 serviceIdCounter;

		EmaVector <ConsumerRoutingSessionChannel*> routingChannelList;

		bool initialLoginRefreshReceived;
		bool sentInitialLoginRefresh;
		LoginInfo	aggregatedLoginInfo;
		StatusMsg _statusMsg;

		bool enhancedItemRecovery;

		bool aggregateLoginRefreshInfo(RsslRDMLoginRefresh*);

		bool aggregateDirectory(Directory*, RsslMapEntryActions);

		// Sets up the SingleItem for a request with the 
		bool matchRequestToSessionChannel(SingleItem&);

		void processChannelEvent(ConsumerRoutingSessionChannel*, RsslReactorChannelEvent*);

		// Hash tables and a direct list of services 
		HashTable<UInt16, ConsumerRoutingService*, UInt16rHasher, UInt16Equal_To> serviceById;			// keyed by the concrete service for this channel
		// Note: hashTable.find returns a ** pointer.
		HashTable<EmaStringPtr, ConsumerRoutingService*, EmaStringPtrHasher, EmaStringPtrEqual_To> serviceByName;	// keyed by the concrete service for this channel
		EmaList< Directory* > serviceList;					// Contains all services in this routing session, all directory structures allocated will be put into here.

		EmaVector<EmaString> deletedServiceList;
		EmaVector<EmaString> addedServiceList;
		EmaVector<EmaString> updatedServiceList;

		ItemList pendingRequestList;			// Items that have had a service match, but have not been routed to a specific channel

		// Assumption for destructor and clear: All channels have already been closed and cleaned up 
		void clear();

		void closeChannel(RsslReactorChannel* pRsslReactorChannel);

		void closeReactorChannels();

	};

}

}

}

#endif