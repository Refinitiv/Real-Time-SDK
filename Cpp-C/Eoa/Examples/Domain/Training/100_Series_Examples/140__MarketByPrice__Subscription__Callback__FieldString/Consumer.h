///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

#ifndef __eoa_consumer_h_
#define __eoa_consumer_h_

#include <iostream>
#include <thread>
#include <chrono>
#include "Domain/MarketByPrice//Include/Mbp.h"

namespace marketbyprice = thomsonreuters::eoa::domain::marketbyprice;
namespace foundation = thomsonreuters::eoa::foundation;

// application defined client class for receiving and processing of ConsumerItem
class AppClient : public marketbyprice::Mbp::ConsumerItemClient
{
protected :

	void onConsumerItemSync( const marketbyprice::Mbp::ConsumerItem& , const marketbyprice::Mbp::RefreshInfo& , void* ) override;

	void onConsumerItemPartial( const marketbyprice::Mbp::ConsumerItem& , const marketbyprice::Mbp::RefreshInfo& , void* ) override;

	void onConsumerItemUpdate( const marketbyprice::Mbp::ConsumerItem& , const marketbyprice::Mbp::UpdateInfo& , void* ) override;

	void onConsumerItemStatus( const marketbyprice::Mbp::ConsumerItem& , const marketbyprice::Mbp::StatusInfo& , void* ) override;

	void print( const marketbyprice::Mbp::OrderBook& );
};

#endif // __eoa_consumer_h_
