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
                     .payload( FieldList()
                               .addReal( 22, 3990, OmmReal::ExponentNeg2Enum )
                               .addReal( 25, 3994, OmmReal::ExponentNeg2Enum )
                               .addReal( 30, 9, OmmReal::Exponent0Enum )
                               .addReal( 31, 19, OmmReal::Exponent0Enum )
                               .complete() )
                     .complete(), itemHandleOne );

		// Encoding and sending Refresh Message
                provider.submit( RefreshMsg().serviceName( "NI_PUB" ).name( "IBM.N" )
                     .state( OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "UnSolicited Refresh Completed" )
                     .payload( FieldList()
                               .addReal( 22, 1190, OmmReal::ExponentNeg2Enum )
                               .addReal( 25, 1194, OmmReal::ExponentNeg2Enum )
                               .addReal( 30, 239, OmmReal::Exponent0Enum )
                               .addReal( 31, 41, OmmReal::Exponent0Enum )
                               .complete() )
                     .complete(), itemHandleTwo );

		sleep( 2000 );

                for ( int i = 0; i < 50; i++ )
                {
                        // Encoding and sending Update Message
                        provider.submit( UpdateMsg()
                                .payload( FieldList()
                                        .addReal( 22, 3391 + i, OmmReal::ExponentNeg2Enum )
                                        .addReal( 30, 10 + i, OmmReal::Exponent0Enum )
                                        .complete() ), itemHandleOne );

                        // Encoding and sending Update Message
                        provider.submit( UpdateMsg()
                                .payload( FieldList()
                                        .addReal( 22, 1191 + i, OmmReal::ExponentNeg2Enum )
                                        .addReal( 30, 240 + i, OmmReal::Exponent0Enum )
                                        .complete() ), itemHandleTwo );

                        sleep ( 1000 );
                }

		sleep( 10000 );


  } catch ( const OmmException& excp ) {
    cout << excp << endl;
  }
  return 0;
}

