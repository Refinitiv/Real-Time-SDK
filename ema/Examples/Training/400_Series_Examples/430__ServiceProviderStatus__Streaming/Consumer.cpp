//****************************************************//
//** This file is preliminary and subject to change **//
//****************************************************//

///*
// *|---------------------------------------------------------------
// *| Confidential and Proprietary Information of Thomson Reuters.
// *| Copyright Thomson Reuters 2015
// *|---------------------------------------------------------------
// */

#include "Consumer.h"

using namespace thomsonreuters::ema::access;
using namespace thomsonreuters::ema::rdm;
using namespace std;

const UInt32 numberOfProviderSPSs = 3;
const EmaString topLevelSPSName( ".[SPSAMER" );
const EmaString serviceName( "ELEKTRON_DD" );
set< EmaString > requestedSPSs;

AppClient::AppClient( OmmConsumer& ommConsumer ) :
 _ommConsumer( ommConsumer )
{
}

void AppClient::onRefreshMsg( const RefreshMsg& refreshMsg, const OmmConsumerEvent& )
{
    cout << endl << "AppClient::onRefreshMsg" << endl;
    cout << refreshMsg << endl;

    if ( refreshMsg.getPayload().getDataType() == DataType::MapEnum )
    {
        if ( refreshMsg.hasName() )
        {
            const Map& map = refreshMsg.getPayload().getMap();
            UInt32 spsCount = 0;
            while ( !map.forth() )
            {
                const MapEntry& mapEntry = map.getEntry();
                if ( mapEntry.getLoadType() == DataType::FieldListEnum )
                {
                    const EmaBuffer& emaBuffer = mapEntry.getKey().getBuffer();
                    const EmaString spsName( emaBuffer.c_buf(), emaBuffer.length() );
                    if ( requestedSPSs.insert( spsName ).second )
                    {
                        cout << "Requesting SPS " << spsName << endl;
                        _ommConsumer.registerClient( ReqMsg().domainType( MMT_SERVICE_PROVIDER_STATUS ).serviceName( serviceName ).name( spsName ), *this );
                    }
                }

                // This just limits the number of provider level SPS we subscribe to
                if ( refreshMsg.getName() == topLevelSPSName )
                {
                    ++spsCount;
                    if ( spsCount == numberOfProviderSPSs )
                    {
                        break;
                    }
                }
            }
        }
    }
}

int main( int argc, char* argv[] )
{
	try
	{
		OmmConsumer consumer( OmmConsumerConfig().host( "localhost:14002" ).username( "user" ) );
		AppClient client( consumer );
		requestedSPSs.insert( topLevelSPSName );
		consumer.registerClient( ReqMsg().domainType( MMT_SERVICE_PROVIDER_STATUS ).serviceName( serviceName ).name( topLevelSPSName ), client );
		sleep( 60000 );
	}
	catch ( const OmmException& excp )
	{
		cout << excp << endl;
	}

	return 0;
}

