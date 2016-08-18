///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

#include "NiProvider.h"

using namespace thomsonreuters::ema::access;
using namespace thomsonreuters::ema::rdm;
using namespace std;

int main( int argc, char* argv[] )
{
	try
	{
		OmmProvider provider( OmmNiProviderConfig().adminControlDirectory( OmmNiProviderConfig::UserControlEnum ).username( "user" ) );
		UInt64 serviceId = 0; 
		UInt64 sourceDirectoryHandle = 1;
		UInt64 aaoHandle = 5;
		RefreshMsg refresh;
		UpdateMsg update;
		Map map;
		FieldList entryLoad, summary;

		provider.submit( refresh.domainType( MMT_DIRECTORY ).filter( SERVICE_INFO_FILTER | SERVICE_STATE_FILTER )
			.payload( map
			.addKeyUInt( serviceId, MapEntry::AddEnum, FilterList()
					.add( SERVICE_INFO_ID, FilterEntry::SetEnum, ElementList()
						.addAscii( ENAME_NAME, "TEST_NI_PUB" )
						.addArray( ENAME_CAPABILITIES, OmmArray()
							.addUInt( MMT_MARKET_PRICE )
							.addUInt( MMT_MARKET_BY_ORDER )
							.complete( ) )
						.addArray( ENAME_DICTIONARYS_USED, OmmArray()
							.addAscii( "RWFFld" )
							.addAscii( "RWFEnum" )
							.complete( ) )
						.complete() )
					.add( SERVICE_STATE_ID, FilterEntry::SetEnum, ElementList()
						.addUInt( ENAME_SVC_STATE, SERVICE_UP )
						.complete() )
					.complete() )
				.complete() ).complete(), sourceDirectoryHandle );

		provider.submit( refresh.domainType( MMT_MARKET_BY_ORDER ).serviceName( "TEST_NI_PUB" ).name( "AAO.V" )
			.state( OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "UnSolicited Refresh Completed" )
			.payload( map.clear()
				.summaryData( summary.addEnum( 15, 840 ).addEnum( 53, 1 ).addEnum( 3423, 1 ).addEnum( 1709, 2 ).complete() )
				.addKeyAscii( "100", MapEntry::AddEnum, entryLoad.clear()
					.addRealFromDouble( 3427, 7.76 )
					.addRealFromDouble( 3429, 9600 )
					.addEnum( 3428, 2 )
					.addRmtes( 212, EmaBuffer( "Market Maker", 12 ) )
					.complete() )
				.complete() )
			.complete(), aaoHandle );

		sleep( 1000 );

		for ( Int32 i = 0; i < 60; ++i )
		{
			provider.submit( update.clear().domainType( MMT_MARKET_BY_ORDER ).serviceName( "TEST_NI_PUB" ).name( "AAO.V" )
				.payload( map.clear()
					.addKeyAscii( "100", MapEntry::UpdateEnum, entryLoad.clear()
						.addRealFromDouble( 3427, 7.76 + i * 0.1 )
						.addRealFromDouble( 3429, 9600 )
						.addEnum( 3428, 2 )
						.addRmtes( 212, EmaBuffer( "Market Maker", 12 ) )
						.complete() )
					.complete() ), aaoHandle );

			sleep( 1000 );
		}
	}
	catch ( const OmmException& excp )
	{
		cout << excp << endl;
	}
	return 0;
}
