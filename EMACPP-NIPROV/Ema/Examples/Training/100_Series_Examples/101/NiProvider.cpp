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
	try {
		OmmNiProvider provider( OmmNiProviderConfig().host( "132.88.227.196:14003" ).username( "user" ) );
		UInt64 itemHandle = 5;
		UInt64 sourceDirectoryHandle = 11;

		// Encoding and sending Source Directory Message
		FilterList filterListPayload;
		filterListPayload.add( SERVICE_INFO_ID, FilterEntry::SetEnum, 
				   ElementList().addAscii( ENAME_NAME, "NI_PUB" )
				   .addArray( ENAME_CAPABILITIES, OmmArray().addUInt( MMT_MARKET_PRICE ).complete( ) ).complete( ) );
		filterListPayload.add( SERVICE_STATE_ID, FilterEntry::SetEnum, 
				   ElementList().addUInt( ENAME_SVC_STATE, SERVICE_UP ).complete() ).complete();

		provider.submit( RefreshMsg().domainType( MMT_DIRECTORY ).filter( SERVICE_INFO_FILTER | SERVICE_STATE_FILTER )
				 .payload( Map().addKeyUInt( 0, MapEntry::AddEnum, filterListPayload ).complete() ).complete(), sourceDirectoryHandle );

		// Encoding and sending Refresh Message
		provider.submit( RefreshMsg().serviceId( 0 ).name( "TRI.N" )
		     .state( OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "UnSolicited Refresh Completed" )
		     .payload( FieldList()
			       .addReal( 22, 3990, OmmReal::ExponentNeg2Enum )
			       .addReal( 25, 3994, OmmReal::ExponentNeg2Enum )
			       .addReal( 30, 9, OmmReal::Exponent0Enum )
			       .addReal( 31, 19, OmmReal::Exponent0Enum )
			       .complete() )
		     .complete(), itemHandle );

		sleep( 2000 );

		for ( int i = 0; i < 50; i++ )
		{
			// Encoding and sending Update Message
			provider.submit( UpdateMsg()
				.payload( FieldList()
					.addReal( 22, 3391 + i, OmmReal::ExponentNeg2Enum )
					.addReal( 30, 10 + i, OmmReal::Exponent0Enum )
					.complete() ), itemHandle );
			sleep ( 1000 );
		}

		sleep( 10000 );

	} catch ( const OmmException& excp ) {
		cout << excp << endl;
	}
	return 0;
}
