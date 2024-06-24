///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2019 LSEG. All rights reserved.                 --
///*|-----------------------------------------------------------------------------

#include "NiProvider.h"

using namespace refinitiv::ema::access;
using namespace std;

int main()
{
	try
	{
		OmmProvider provider( OmmNiProviderConfig().username( "user" ) );
		UInt64 itemNumber = 1000;
		int sleepTime = 1000;
		RefreshMsg refresh;
		UpdateMsg update;
		FieldList fieldList;

		EmaString itemName( 0, 256 );
		EmaString serviceName( "TEST_NI_PUB" );
		EmaString statusText( "UnSolicited Refresh Completed" );
		unsigned long long start = getCurrentTime();

		for ( UInt64 handle = 0; handle < itemNumber; ++handle )
		{
			itemName.set( "RTR", 3 ).append( handle ).append( ".N" );
			provider.submit( refresh.clear().serviceName( serviceName ).name( itemName )
				.state( OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, statusText )
				.payload( fieldList.clear()
					.addUInt( 1, 6560 )
					.addUInt( 2, 66 )
					.addUInt( 3855, 52832001 )
					.addRmtes( 296, EmaBuffer( "BOS", 3 ) )
					.addTime( 375, 21, 0 )
					.addTime( 1025, 14, 40, 32 )
					.addReal( 22, 14400, OmmReal::ExponentNeg2Enum )
					.addReal( 25, 14700, OmmReal::ExponentNeg2Enum )
					.addReal( 30, 9, OmmReal::Exponent0Enum )
					.addReal( 31, 19, OmmReal::Exponent0Enum )
					.complete() )
				.complete(), handle );
		}

		unsigned long long end = getCurrentTime();
		float timeSpent = ( float )( end - start ) / ( float ) 1000;

		cout << "total refresh count = " << itemNumber
		     << "\ttotal time = " << timeSpent << " sec"
		     << "\trefresh rate = " << ( float )itemNumber / timeSpent << " refresh per sec" << endl;

		unsigned long long midpoint = end = start = getCurrentTime();

		UInt64 updateCount = 0;

		while ( start + 300000 > end )
		{
			for ( UInt64 handle = 0; handle < itemNumber; ++handle )
			{
				itemName.set( "RTR", 3 ).append( handle ).append( ".N" );
				provider.submit( update.clear().serviceName( serviceName ).name( itemName )
					.payload(fieldList.clear()
						.addTime( 1025, 14, 40, 32 )
						.addUInt( 3855, 52832001 )
						.addReal( 22, 14400 + ( ( handle & 0x1 ) ? 1 : 10 ), OmmReal::ExponentNeg2Enum )
						.addReal( 30, 10 + ( ( handle & 0x1 ) ? 10 : 20 ), OmmReal::Exponent0Enum )
						.addRmtes( 296, EmaBuffer( "NAS", 3 ) )
						.complete() ), handle );

				++updateCount;
			}

			sleep( sleepTime );

			end = getCurrentTime();

			if ( end >= midpoint + 1000 )
			{
				float timeSpent = ( float )( end - midpoint ) / ( float ) 1000;

				cout << "update count = " << updateCount
				     << "\tupdate rate = " << ( float )updateCount / timeSpent << " update per sec" << endl;

				updateCount = 0;

				midpoint = end;
			}
		}
	}
	catch ( const OmmException& excp )
	{
		cout << excp << endl;
	}
	return 0;
}
