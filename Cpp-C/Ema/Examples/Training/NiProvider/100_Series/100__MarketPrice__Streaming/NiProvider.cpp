///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

#include "NiProvider.h"

using namespace thomsonreuters::ema::access;
using namespace std;

int main()
{
	try
	{
		OmmProvider provider( OmmNiProviderConfig().host( "localhost:14003" ).username( "user" ) );
		UInt64 itemHandle = 5;

		provider.submit( RefreshMsg().serviceName( "NI_PUB" ).name( "IBM.N" )
			.state( OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "UnSolicited Refresh Completed" )
			.payload( FieldList()
				.addReal( 22, 3990, OmmReal::ExponentNeg2Enum )
				.addReal( 25, 3994, OmmReal::ExponentNeg2Enum )
				.addReal( 30, 9, OmmReal::Exponent0Enum )
				.addReal( 31, 19, OmmReal::Exponent0Enum )
				.complete() )
			.complete(), itemHandle );

		sleep( 1000 );

		for ( Int32 i = 0; i < 60; i++ )
		{
			provider.submit( UpdateMsg().serviceName( "NI_PUB" ).name( "IBM.N" )
				.payload( FieldList()
					.addReal( 22, 3391 + i, OmmReal::ExponentNeg2Enum )
					.addReal( 30, 10 + i, OmmReal::Exponent0Enum )
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
