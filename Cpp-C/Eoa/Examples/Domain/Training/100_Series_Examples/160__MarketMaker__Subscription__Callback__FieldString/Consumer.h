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
#include "Domain/MarketMaker/Include/Mm.h"

namespace marketmaker = thomsonreuters::eoa::domain::marketmaker;
namespace foundation = thomsonreuters::eoa::foundation;

// application defined client class for receiving and processing of ConsumerItem
class AppClient : public marketmaker::Mm::ConsumerItemClient
{
protected :

	void onConsumerItemSync( const marketmaker::Mm::ConsumerItem& , const marketmaker::Mm::RefreshInfo& , void* ) override;

	void onConsumerItemPartial( const marketmaker::Mm::ConsumerItem& , const marketmaker::Mm::RefreshInfo& , void* ) override;

	void onConsumerItemUpdate( const marketmaker::Mm::ConsumerItem& , const marketmaker::Mm::UpdateInfo& , void* ) override;

	void onConsumerItemStatus( const marketmaker::Mm::ConsumerItem& , const marketmaker::Mm::StatusInfo& , void* ) override;

	void print( const marketmaker::Mm::MarketMakerList& );
};

#endif // __eoa_consumer_h_
