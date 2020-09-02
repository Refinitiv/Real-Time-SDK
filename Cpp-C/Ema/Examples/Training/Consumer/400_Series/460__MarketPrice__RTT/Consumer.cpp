///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2020 Refinitiv. All rights reserved.              --
///*|-----------------------------------------------------------------------------

#include "Consumer.h"

using namespace rtsdk::ema::access;
using namespace rtsdk::ema::rdm;
using namespace std;

void AppClient::onRefreshMsg( const RefreshMsg& refreshMsg, const OmmConsumerEvent& ) 
{
	cout << refreshMsg << endl;		// defaults to refreshMsg.toString()
}

void AppClient::onUpdateMsg( const UpdateMsg& updateMsg, const OmmConsumerEvent& ) 
{
	cout << updateMsg << endl;		// defaults to updateMsg.toString()
}

void AppClient::onStatusMsg( const StatusMsg& statusMsg, const OmmConsumerEvent& ) 
{
	cout << statusMsg << endl;		// defaults to statusMsg.toString()
}

void AppClient::onGenericMsg( const GenericMsg& genericMsg, const OmmConsumerEvent& event )
{
	if ( genericMsg.getDomainType() == MMT_LOGIN && genericMsg.getPayload().getDataType() == DataType::ElementListEnum )
	{
		cout << "Received login RTT message for Channel " << event.getHandle() << endl;

		const ElementList& elementList = genericMsg.getPayload().getElementList();
		while ( elementList.forth() )
		{
			const ElementEntry& elementEntry = elementList.getEntry();

			if ( elementEntry.getName() == ENAME_RTT && elementEntry.getLoadType() == DataType::UIntEnum ) // "RoundTripLatency"
			{
				cout << "\tLast Latency: " << elementEntry.getUInt() << endl;
			}
			else if ( elementEntry.getName() == ENAME_RTT_TICKS && elementEntry.getLoadType() == DataType::UIntEnum ) // "Ticks"
			{
				cout << "\tTicks: " << elementEntry.getUInt() << endl;
			}
			else if ( elementEntry.getName() == ENAME_RTT_TCP_RETRANS && elementEntry.getLoadType() == DataType::UIntEnum ) // "TcpRetrans"
			{
				cout << "\tProvider side TCP Retransmissions: " << elementEntry.getUInt() << endl;
			}
		}

		cout << endl;
	}
}

int main( int argc, char* argv[] )
{ 
	try { 
		AppClient client;

		OmmConsumer consumer( OmmConsumerConfig().consumerName( "Consumer_2" ) );

		// register client to receive MARKET_PRICE item updates
		consumer.registerClient( ReqMsg().domainType(MMT_MARKET_PRICE).serviceName("DIRECT_FEED").name("IBM.N"), client );

		// register client to receive RTT messages in LOGIN stream
		consumer.registerClient( ReqMsg().domainType(MMT_LOGIN), client );

		sleep( 60000 );				// API calls onRefreshMsg(), onUpdateMsg(), onStatusMsg(), or onGenericMsg
	} catch ( const OmmException& excp ) {
		cout << excp << endl;
	}

	return 0;
}
