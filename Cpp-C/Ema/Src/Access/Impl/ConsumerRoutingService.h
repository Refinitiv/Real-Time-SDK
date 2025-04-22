/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|         Copyright (C) 2024 LSEG. All rights reserved.             --
 *|-----------------------------------------------------------------------------
 */
 
// These classes are intended to represent a single consolidated service in a Consumer Session 
 
#ifndef __refinitiv_ema_access_ConsumerRoutingService_h
#define __refinitiv_ema_access_ConsumerRoutingService_h

#include "DirectoryCallbackClient.h"
#include "rtr/rsslTransport.h"
#include "rtr/rwfNet.h"
#include "rtr/rsslReactor.h" 
#include "EmaList.h"

namespace refinitiv {

namespace ema {

namespace access {


	// This class contains all of the routing information necessary for aggregating service information for request routing
	class ConsumerRoutingService : public Directory
	{
	public:
		enum AggregationResultEnum
		{
			SuccessNewServiceEnum,
			SuccessNoUpdateEnum,
			SuccessWithUpdateEnum,
			SuccessAndDeleteEnum,
			SuccessEnum,
			FailureEnum
		};

		ConsumerRoutingService(OmmBaseImpl&);
		virtual ~ConsumerRoutingService();

		EmaVector< ConsumerRoutingSessionChannel*> routingChannelList;  // List of channels that currently have this service.  This is initialized to the size of the total number of configured ConsumerRoutingSessionChannels
																		// and indexed by their position in the active configuration.

		int activeServiceCount;

		ConsumerRoutingService& clear();

		bool initialized;

		// Returns true if the aggregated service has materially changed, indicating that updates/refreshes should be fanned out to any open handles
		AggregationResultEnum aggregateDirectoryInfo(Directory&, RsslMapEntryActions action);

		AggregationResultEnum compareAggregatedService(Directory&);
	};

}
}
}

#endif