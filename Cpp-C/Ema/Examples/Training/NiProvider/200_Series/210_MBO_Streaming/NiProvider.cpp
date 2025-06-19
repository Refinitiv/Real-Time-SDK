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
		OmmProvider provider( OmmNiProviderConfig().username( "user" ) );
		UInt64 aaoHandle = 5;
		UInt64 aggHandle = 6;
		RefreshMsg refresh;
		UpdateMsg update;
		Map map;
		FieldList entryLoad, summary;

		provider.submit( refresh.domainType( MMT_MARKET_BY_ORDER ).serviceName( "TEST_NI_PUB" ).name( "AAO.V" )
			.state( OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "UnSolicited Refresh Completed" )
			.payload( map
				.summaryData( summary.clear().addEnum( 15, 840 ).addEnum( 53, 1 ).addEnum( 3423, 1 ).addEnum( 1709, 2 ).complete() )
				.addKeyAscii( "100", MapEntry::AddEnum, entryLoad.clear()
					.addRealFromDouble( 3427, 7.76 )
					.addRealFromDouble( 3429, 9600 )
					.addEnum( 3428, 2 )
					.addRmtes( 212, EmaBuffer( "Market Maker", 12 ) )
					.complete() )
				.complete() )
			.complete(), aaoHandle );

		provider.submit( refresh.clear().domainType( MMT_MARKET_BY_ORDER ).serviceName( "TEST_NI_PUB" ).name( "AGG.V" )
			.state( OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "UnSolicited Refresh Completed" )
			.payload( map.clear()
				.summaryData( summary.clear().addEnum( 15, 840 ).addEnum( 53, 1 ).addEnum( 3423, 1 ).addEnum( 1709, 2 ).complete() )
				.addKeyAscii( "222", MapEntry::AddEnum, entryLoad.clear()
					.addRealFromDouble( 3427, 9.22, OmmReal::ExponentNeg2Enum )
					.addRealFromDouble( 3429, 1200 )
					.addEnum( 3428, 2 )
					.addRmtes( 212, EmaBuffer( "Market Maker", 12 ) )
					.complete() )
				.complete() )
			.complete(), aggHandle );

		sleep( 1000 );

		for ( Int32 i = 0; i < 60; ++i )
		{
			provider.submit( update.clear().domainType( MMT_MARKET_BY_ORDER ).serviceName( "TEST_NI_PUB" ).name( "AAO.V" )
				.payload( map.clear()
					.addKeyAscii( "100", MapEntry::UpdateEnum, entryLoad.clear()
						.addRealFromDouble( 3427, 7.76 + i * 0.1, OmmReal::ExponentNeg2Enum )
						.addRealFromDouble( 3429, 9600 )
						.addEnum( 3428, 2 )
						.addRmtes( 212, EmaBuffer( "Market Maker", 12 ) )
						.complete() )
					.complete() ), aaoHandle );

			provider.submit( update.clear().domainType( MMT_MARKET_BY_ORDER ).serviceName( "TEST_NI_PUB" ).name( "AGG.V" )
				.payload( map.clear()
					.addKeyAscii( "222", MapEntry::UpdateEnum, entryLoad.clear()
						.addRealFromDouble( 3427, 9.22 + i * 0.1, OmmReal::ExponentNeg2Enum )
						.addRealFromDouble( 3429, 1200 )
						.addEnum( 3428, 2 )
						.addRmtes( 212, EmaBuffer( "Market Maker", 12 ) )
						.complete() )
					.complete() ), aggHandle );

			sleep( 1000 );
		}
	}
	catch ( const OmmException& excp )
	{
		cout << excp << endl;
	}
	return 0;
}
