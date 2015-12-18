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
#include "Domain/MarketPrice//Include/Mp.h"

namespace marketprice = thomsonreuters::eoa::domain::marketprice;
namespace foundation = thomsonreuters::eoa::foundation;

// application defined client class for receiving and processing of ConsumerItem
class AppClient : public marketprice::Mp::ConsumerItemClient
{
protected :

	void onConsumerItemSync( const marketprice::Mp::ConsumerItem& , const marketprice::Mp::RefreshInfo& , void* ) override;

	void onConsumerItemUpdate( const marketprice::Mp::ConsumerItem& , const marketprice::Mp::UpdateInfo& , void* ) override;

	void onConsumerItemStatus( const marketprice::Mp::ConsumerItem& , const marketprice::Mp::StatusInfo& , void* ) override;

	void print( const marketprice::Mp::Quote& );
};

#endif // __eoa_consumer_h_
