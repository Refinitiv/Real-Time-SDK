/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "NiProvider.h"

using namespace refinitiv::ema::access;
using namespace refinitiv::ema::rdm;
using namespace std;

int main()
{
	try
	{
		OmmProvider provider( OmmNiProviderConfig().host( "localhost:14003" ).username( "user" ) );
		UInt64 itemHandle = 5;

		provider.submit( RefreshMsg().domainType( MMT_MARKET_BY_ORDER ).serviceName( "NI_PUB" ).name( "AAO.V" )
			.state( OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "UnSolicited Refresh Completed" )
			.payload( Map()
				.summaryData( FieldList().addEnum( 15, 840 ).addEnum( 53, 1 ).addEnum( 3423, 1 ).addEnum( 1709, 2 ).complete() )
				.addKeyAscii( "100", MapEntry::AddEnum, FieldList()
					.addRealFromDouble( 3427, 7.76, OmmReal::ExponentNeg2Enum )
					.addRealFromDouble( 3429, 9600 )
					.addEnum( 3428, 2 )
					.addRmtes( 212, EmaBuffer( "Market Maker", 12 ) )
					.complete() )
				.complete() )
			.complete(), itemHandle );

		sleep( 1000 );

		for ( Int32 i = 0; i < 60; ++i )
		{
			provider.submit( UpdateMsg().domainType( MMT_MARKET_BY_ORDER ).serviceName( "NI_PUB" ).name( "AAO.V" )
				.payload( Map()
					.addKeyAscii( "100", MapEntry::UpdateEnum, FieldList()
						.addRealFromDouble( 3427, 7.76 + i * 0.1, OmmReal::ExponentNeg2Enum )
						.addRealFromDouble( 3429, 9600 )
						.addEnum( 3428, 2 )
						.addRmtes( 212, EmaBuffer( "Market Maker", 12 ) )
						.complete() )
					.complete() ), itemHandle );

			sleep( 1000 );
		}
	}
	catch ( const OmmException& excp )
	{
		cout << excp << endl;
	}
	return 0;
}
