///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

#include "NiProvider.h"

using namespace thomsonreuters::ema::access;
using namespace thomsonreuters::ema::rdm;
using namespace std;

void AppClient::onRefreshMsg( const RefreshMsg& refreshMsg, const OmmProviderEvent& event ) 
{
  cout << endl << "event channel info (refresh)" << endl << event.getChannelInformation()
	  << endl << refreshMsg << endl;
}

void AppClient::onStatusMsg( const StatusMsg& statusMsg, const OmmProviderEvent& event ) 
{
  cout << endl << "event channel info (status)" << endl << event.getChannelInformation() 
	  << endl << statusMsg << endl;
}

void AppClient::onClose( const ReqMsg& reqMsg, const OmmProviderEvent& event ) 
{
  cout << endl << "event channel info (close)" << endl << event.getChannelInformation() 
	  << endl << reqMsg << endl;
}
int main()
{
	try
	{
	    AppClient appClient;
		ChannelInformation ci;

		OmmProvider provider( OmmNiProviderConfig( "EmaConfig.xml" ).username( "user" ) );

		provider.getChannelInformation( ci );
		cout << "channel info (provider)" << endl << ci << endl;

		UInt64 loginHandle = provider.registerClient( ReqMsg().domainType( MMT_LOGIN )
													  .name( "user" ), appClient );

		UInt64 itemHandle = 5;
		provider.submit( RefreshMsg().serviceName( "NI_PUB" ).name( "IBM.N" )
						 .state( OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum,
								 "UnSolicited Refresh Completed" )
						 .payload( FieldList()
								   .addReal( 22, 3990, OmmReal::ExponentNeg2Enum )
								   .addReal( 25, 3994, OmmReal::ExponentNeg2Enum )
								   .addReal( 30, 9, OmmReal::Exponent0Enum )
								   .addReal( 31, 19, OmmReal::Exponent0Enum )
								   .complete() )
						 .complete(), itemHandle );
		sleep( 1000 );

		for ( Int32 i = 0; i < 60; i++ ) {
		  try {
			provider.submit( UpdateMsg().serviceName( "NI_PUB" ).name( "IBM.N" )
							 .payload( FieldList()
									   .addReal( 22, 3391 + i, OmmReal::ExponentNeg2Enum )
									   .addReal( 30, 10 + i, OmmReal::Exponent0Enum )
									   .complete() ), itemHandle );
		  }

		  // allows one to stop/start ADH which will generate a status message
		  catch ( OmmInvalidUsageException& e ) {
			cout << "Ignored invalid usage exception: " << e.getText() << endl;
		  }

		  sleep( 1000 );
		}
	}
	catch ( const OmmException& excp )
	{
		cout << excp << endl;
	}
	return 0;
}
