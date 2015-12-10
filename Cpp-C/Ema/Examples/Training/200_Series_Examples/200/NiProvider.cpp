///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

#include "NiProvider.h"

#include <iostream>

using namespace thomsonreuters::ema::access;
using namespace thomsonreuters::ema::rdm;
using namespace std;

int main( int argc, char* argv[] )
{ 
	try {
		OmmNiProvider provider( OmmNiProviderConfig().host( "localhost:14003" ).username( "user" ) );
		UInt64 itemHandleOne = 5;
		UInt64 itemHandleTwo = 11;
		UInt64 sourceDirectoryHandle = 7;

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
		provider.submit( RefreshMsg().serviceName( "NI_PUB" ).name( "TRI.N" )
				 .state( OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "UnSolicited Refresh Completed" )
				 .payload( FieldList().addReal( 22, 25, OmmReal::ExponentPos1Enum ).complete() ).complete(), itemHandleOne );

		// Encoding and sending Refresh Message
		provider.submit( RefreshMsg().serviceName( "NI_PUB" ).name( "IBM.N" )
				 .state( OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "UnSolicited Refresh Completed" )
				 .payload( FieldList().addReal( 22, 31, OmmReal::ExponentPos1Enum ).complete() ).complete(), itemHandleTwo );

		sleep( 2000 );

		// Encoding and sending Update Message
		provider.submit( UpdateMsg()
				.payload( FieldList().addReal( 22, 26, OmmReal::ExponentPos1Enum ).complete() ), itemHandleOne );

		// Encoding and sending Update Message
		provider.submit( UpdateMsg()
				.payload( FieldList().addReal( 22, 32, OmmReal::ExponentPos1Enum ).complete() ), itemHandleTwo );

		sleep( 10000 );


  } catch ( const OmmException& excp ) {
    cout << excp << endl;
  }
  return 0;
}

