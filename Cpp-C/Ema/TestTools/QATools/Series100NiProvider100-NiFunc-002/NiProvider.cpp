///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

//APIQA this file is QATools standalone. See qa_readme.txt for details about this tool.

#include "NiProvider.h"

using namespace thomsonreuters::ema::access;
using namespace thomsonreuters::ema::rdm;
using namespace std;

int main( int argc, char* argv[] )
{
	try
	{
		sleep ( 5000 );

		OmmProvider provider( OmmNiProviderConfig().adminControlDirectory( OmmNiProviderConfig::UserControlEnum ).username( "user" ) );
		UInt64 sourceDirectoryHandle = 1;
		UInt64 ibmHandle = 5;
		RefreshMsg refresh;
		UpdateMsg update;
		FieldList fieldList;

		provider.submit( refresh.domainType( MMT_DIRECTORY ).filter( SERVICE_INFO_FILTER | SERVICE_STATE_FILTER )
			.payload( Map()
			.addKeyAscii( "TEST_NI_PUB", MapEntry::AddEnum, FilterList()
					.add( SERVICE_INFO_ID, FilterEntry::SetEnum, ElementList()
						.addArray( ENAME_CAPABILITIES, OmmArray()
							.addUInt( MMT_MARKET_PRICE )
							.addUInt( MMT_MARKET_BY_PRICE )
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

		provider.submit( refresh.clear().serviceName( "TEST_NI_PUB" ).name( "IBM.N" )
			.state( OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "UnSolicited Refresh Completed" )
			.payload( fieldList
				.addReal( 22, 14400, OmmReal::ExponentNeg2Enum )
				.addReal( 25, 14700, OmmReal::ExponentNeg2Enum )
				.addReal( 30, 9, OmmReal::Exponent0Enum )
				.addReal( 31, 19, OmmReal::Exponent0Enum )
				.complete() )
			.complete(), ibmHandle );

		sleep( 1000 );

		for ( Int32 i = 0; i < 60; i++ )
		{
			provider.submit( update.clear().serviceName( "TEST_NI_PUB" ).name( "IBM.N" )
				.payload( fieldList.clear()
					.addReal( 22, 14400 + i, OmmReal::ExponentNeg2Enum )
					.addReal( 30, 10 + i, OmmReal::Exponent0Enum )
					.complete() ), ibmHandle );

			sleep( 1000 );
		}
	}
	catch ( const OmmException& excp )
	{
		cout << excp << endl;
	}
	return 0;
}
//END APIQA
