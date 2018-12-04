///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2018. All rights reserved.            --
///*|-----------------------------------------------------------------------------

#include "NiProvider.h"
#include <cstring>

using namespace thomsonreuters::ema::access;
using namespace thomsonreuters::ema::rdm;
using namespace std;

void printHelp()
{
	cout << endl << "Options:\n" << " -?\tShows this usage\n"
		<< " -ph Proxy host name \n"
		<< " -pp Proxy port number \n"
		<< " -spTLSv1 enable use of cryptographic protocol TLSv1 used with linux encrypted connections \n"
		<< " -spTLSv1.1 enable use of cryptographic protocol TLSv1.1 used with linux encrypted connections \n"
		<< " -spTLSv1.2 enable use of cryptographic protocol TLSv1.2 used with linux encrypted connections \n"
		<< " -libsslName name of the libssl.so shared library used with linux encrypted connections. \n"
		<< " -libcryptoName name of the libcrypto.so shared library used with linux encrypted connections \n" 
		//APIQA	
		<< " -reqDict enable to run the test with dictionary streaming \n"
		<< " -objectname ObjectName for encrypted connections \n" << endl;
}

// API QA to cover Recovery Test

AppClient::AppClient() :
        _bConnectionUp( false )
{
}

AppClient::~AppClient()
{
}

bool AppClient::isConnectionUp() const
{
        return _bConnectionUp;
}
// API QA to request dictionary
bool reqDict = false;
bool dumpDictionary = false;
DataDictionary dataDictionary;
bool fldComplete = false;
bool enumComplete = false;
void AppClient::decode(const Msg& msg, bool complete)
{
    switch (msg.getPayload().getDataType())
    {
        case DataType::SeriesEnum:
        {
            if (msg.getName() == "RWFFld")
            {
                dataDictionary.decodeFieldDictionary(msg.getPayload().getSeries(), DICTIONARY_NORMAL);

                if (complete)
                {
                    fldComplete = true;
                }
            }

            else if (msg.getName() == "RWFEnum")
            {
                dataDictionary.decodeEnumTypeDictionary(msg.getPayload().getSeries(), DICTIONARY_NORMAL);

                if (complete)
                {
                    enumComplete = true;
                }
            }

            if (fldComplete && enumComplete)
            {
                cout << endl << "Dictionary download complete" << endl;
                cout << "Dictionary Id : " << dataDictionary.getDictionaryId() << endl;
                cout << "Dictionary field version : " << dataDictionary.getFieldVersion() << endl;
                cout << "Number of dictionary entries : " << dataDictionary.getEntries().size() << endl;

                if ( dumpDictionary )
                    cout << dataDictionary << endl;
            }
        }
    }
}
// END API QA

void AppClient::onRefreshMsg( const RefreshMsg& refreshMsg, const OmmProviderEvent& ommEvent )
{
        cout << endl << "Handle: " << ommEvent.getHandle() << " Closure: " << ommEvent.getClosure() << endl;
        //cout << refreshMsg << endl;
		//API QA
		cout << endl << "Item Name: " << (refreshMsg.hasName() ? refreshMsg.getName() : EmaString("<not set>")) << endl
                << "Service Name: " << (refreshMsg.hasServiceName() ? refreshMsg.getServiceName() : EmaString("<not set>"));

        cout << endl << "Item State: " << refreshMsg.getState().toString() << endl;
           
        decode(refreshMsg, refreshMsg.getComplete());
        // API QA
        if ( refreshMsg.getState().getStreamState() == OmmState::OpenEnum )
        {
                if ( refreshMsg.getState().getDataState() == OmmState::OkEnum )
                        _bConnectionUp = true;
                else
                        _bConnectionUp = false;
        }
        else
                _bConnectionUp = false;
}

void AppClient::onStatusMsg( const StatusMsg& statusMsg, const OmmProviderEvent& ommEvent )
{
        cout << endl << "Handle: " << ommEvent.getHandle() << " Closure: " << ommEvent.getClosure() << endl;
        //cout << statusMsg << endl;
		//API QA
		cout << endl << "Item Name: " << (statusMsg.hasName() ? statusMsg.getName() : EmaString("<not set>")) << endl
                << "Service Name: " << (statusMsg.hasServiceName() ? statusMsg.getServiceName() : EmaString("<not set>"));
		// API QA
        if ( statusMsg.hasState() )
        {
                if ( statusMsg.getState().getStreamState() == OmmState::OpenEnum )
                {
                        if ( statusMsg.getState().getDataState() == OmmState::OkEnum )
                                _bConnectionUp = true;
                        else
                                _bConnectionUp = false;
                }
                else
                        _bConnectionUp = false;
        }
        else
                _bConnectionUp = true;
}

// End API QA
int main( int argc, char* argv[] )
{
	try
	{
		OmmNiProviderConfig config;
		int securityProtocol = 0;

		for (int i = 0; i < argc; i++)
		{
			if (strcmp(argv[i], "-?") == 0)
			{
				printHelp();
				return false;
			}
			else if (strcmp(argv[i], "-ph") == 0)
			{
				config.tunnelingProxyHostName(i < (argc - 1) ? argv[++i] : NULL);
			}
			else if (strcmp(argv[i], "-pp") == 0)
			{
				config.tunnelingProxyPort(i < (argc - 1) ? argv[++i] : NULL);
			}
			else if (strcmp(argv[i], "-spTLSv1") == 0)
			{
				securityProtocol |= OmmNiProviderConfig::ENC_TLSV1;
			}
			else if (strcmp(argv[i], "-spTLSv1.1") == 0)
			{
				securityProtocol |= OmmNiProviderConfig::ENC_TLSV1_1;
			}
			else if (strcmp(argv[i], "-spTLSv1.2") == 0)
			{
				securityProtocol |= OmmNiProviderConfig::ENC_TLSV1_2;
			}
			else if (strcmp(argv[i], "-libsslName") == 0)
			{
				config.tunnelingLibSslName(i < (argc - 1) ? argv[++i] : NULL);
			}
			else if (strcmp(argv[i], "-libcryptoName") == 0)
			{
				config.tunnelingLibCryptoName(i < (argc - 1) ? argv[++i] : NULL);
			}
			//APIQA
			else if (strcmp(argv[i], "-reqDict") == 0)
			{
				reqDict = true;
				dumpDictionary = true;
			}
			else if (strcmp(argv[i], "-objectname") == 0)
			{
				config.tunnelingObjectName(i < (argc - 1) ? argv[++i] : NULL);
			}
			//END APIQA
		}

		if (securityProtocol > 0)
			config.tunnelingSecurityProtocol(securityProtocol);
		// API QA
        AppClient appClient;
		// End API QA
		OmmProvider provider( config.operationModel( OmmNiProviderConfig::UserDispatchEnum ).username( "user" ).providerName( "Provider_3" ), appClient );
		UInt64 ibmHandle = 5;
		UInt64 triHandle = 6;
		UInt64 aaoHandle = 7;
		UInt64 aggHandle = 8;
		RefreshMsg refresh;
		UpdateMsg update;
		FieldList fieldList;
		Map map;
		FieldList entryLoad, summary;
		bool sendRefreshMsg = false;
		provider.dispatch( 100000 ); // calls to onRefreshMsg(), or onStatusMsg() execute on this thread
		
		if ( reqDict ) {		
            // Open Dictionary streams

            UInt64 fldHandle = provider.registerClient(ReqMsg().name("RWFFld").filter(DICTIONARY_NORMAL).serviceName("NI_PUB").domainType(MMT_DICTIONARY), appClient);
            UInt64 enumHandle = provider.registerClient(ReqMsg().name("RWFEnum").filter(DICTIONARY_NORMAL).serviceName("NI_PUB").domainType(MMT_DICTIONARY), appClient);
		}

		provider.submit( refresh.clear().serviceName( "NI_PUB" ).name( "IBM.N" )
			.state( OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "UnSolicited Refresh Completed" )
			.payload( fieldList.clear()
				.addReal( 22, 14400, OmmReal::ExponentNeg2Enum )
				.addReal( 25, 14700, OmmReal::ExponentNeg2Enum )
				.addReal( 30, 9, OmmReal::Exponent0Enum )
				.addReal( 31, 19, OmmReal::Exponent0Enum )
				.complete() )
			.complete(), ibmHandle );

		provider.submit( refresh.clear().serviceName( "NI_PUB" ).name( "TRI.N" )
			.state( OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "UnSolicited Refresh Completed" )
			.payload( fieldList.clear()
				.addReal( 22, 4100, OmmReal::ExponentNeg2Enum )
				.addReal( 25, 4200, OmmReal::ExponentNeg2Enum )
				.addReal( 30, 20, OmmReal::Exponent0Enum )
				.addReal( 31, 40, OmmReal::Exponent0Enum )
				.complete() )
			.complete(), triHandle );

		provider.submit( refresh.clear().domainType( MMT_MARKET_BY_ORDER ).serviceName( "NI_PUB" ).name( "AAO.V" )
			.state( OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "UnSolicited Refresh Completed" )
			.payload( map.clear()
				.summaryData( summary.clear().addEnum( 15, 840 ).addEnum( 53, 1 ).addEnum( 3423, 1 ).addEnum( 1709, 2 ).complete() )
				.addKeyAscii( "100", MapEntry::AddEnum, entryLoad.clear()
					.addRealFromDouble( 3427, 7.76 )
					.addRealFromDouble( 3429, 9600 )
					.addEnum( 3428, 2 )
					.addRmtes( 212, EmaBuffer( "Market Maker", 12 ) )
					.complete() )
				.complete() )
			.complete(), aaoHandle );

		provider.submit( refresh.clear().domainType( MMT_MARKET_BY_ORDER ).serviceName( "NI_PUB" ).name( "AGG.V" )
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
	    // API QA
		provider.dispatch( 1000000 );
		//sleep( 1000 );
		// END API QA

		for ( Int32 i = 0; i < 60;)
		{
			// API QA
			if ( appClient.isConnectionUp() )
			{
				if (sendRefreshMsg) 
				{
					provider.submit( refresh.clear().serviceName( "NI_PUB" ).name( "IBM.N" )
								.state( OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "UnSolicited Refresh Completed" )
								.payload( fieldList.clear()
										.addReal( 22, 14400, OmmReal::ExponentNeg2Enum )
										.addReal( 25, 14700, OmmReal::ExponentNeg2Enum )
										.addReal( 30, 9, OmmReal::Exponent0Enum )
										.addReal( 31, 19, OmmReal::Exponent0Enum )
										.complete() )
								.complete(), ibmHandle );

					provider.submit( refresh.clear().serviceName( "NI_PUB" ).name( "TRI.N" )
								.state( OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "UnSolicited Refresh Completed" )
								.payload( fieldList.clear()
										.addReal( 22, 4100, OmmReal::ExponentNeg2Enum )
										.addReal( 25, 4200, OmmReal::ExponentNeg2Enum )
										.addReal( 30, 20, OmmReal::Exponent0Enum )
										.addReal( 31, 40, OmmReal::Exponent0Enum )
										.complete() )
								.complete(), triHandle );

					provider.submit( refresh.clear().domainType( MMT_MARKET_BY_ORDER ).serviceName( "NI_PUB" ).name( "AAO.V" )
								.state( OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "UnSolicited Refresh Completed" )
								.payload( map.clear()
										.summaryData( summary.clear().addEnum( 15, 840 ).addEnum( 53, 1 ).addEnum( 3423, 1 ).addEnum( 1709, 2 ).complete() )
										.addKeyAscii( "100", MapEntry::AddEnum, entryLoad.clear()
												.addRealFromDouble( 3427, 7.76 )
												.addRealFromDouble( 3429, 9600 )
												.addEnum( 3428, 2 )
												.addRmtes( 212, EmaBuffer( "Market Maker", 12 ) )
												.complete() )
										.complete() )
								.complete(), aaoHandle );

					provider.submit( refresh.clear().domainType( MMT_MARKET_BY_ORDER ).serviceName( "NI_PUB" ).name( "AGG.V" )
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
					sendRefreshMsg = false;
				}
				else
				{
						provider.submit( update.clear().serviceName( "NI_PUB" ).name( "IBM.N" )
							.payload( fieldList.clear()
								.addReal( 22, 14400 + i, OmmReal::ExponentNeg2Enum )
								.addReal( 30, 10 + i, OmmReal::Exponent0Enum )
								.complete() ), ibmHandle );
						provider.submit( update.clear().serviceName( "NI_PUB" ).name( "TRI.N" )
							.payload( fieldList.clear()
								.addReal( 22, 4100 + i, OmmReal::ExponentNeg2Enum )
								.addReal( 30, 21 + i, OmmReal::Exponent0Enum )
								.complete() ), triHandle );
					
						provider.submit( update.clear().domainType( MMT_MARKET_BY_ORDER ).serviceName( "NI_PUB" ).name( "AAO.V" )
							.payload( map.clear()
								.addKeyAscii( "100", MapEntry::UpdateEnum, entryLoad.clear()
									.addRealFromDouble( 3427, 7.76 + i * 0.1, OmmReal::ExponentNeg2Enum )
									.addRealFromDouble( 3429, 9600 )
									.addEnum( 3428, 2 )
									.addRmtes( 212, EmaBuffer( "Market Maker", 12 ) )
									.complete() )
								.complete() ), aaoHandle );

						provider.submit( update.clear().domainType( MMT_MARKET_BY_ORDER ).serviceName( "NI_PUB" ).name( "AGG.V" )
							.payload( map.clear()
								.addKeyAscii( "222", MapEntry::UpdateEnum, entryLoad.clear()
									.addRealFromDouble( 3427, 9.22 + i * 0.1, OmmReal::ExponentNeg2Enum )
									.addRealFromDouble( 3429, 1200 )
									.addEnum( 3428, 2 )
									.addRmtes( 212, EmaBuffer( "Market Maker", 12 ) )
									.complete() )
								.complete() ), aggHandle );
						i++;
						//sleep( 1000 );
					}
			}
			else
			{
				sendRefreshMsg = true;
			}
			provider.dispatch( 1000000 );
		}
	}
	catch ( const OmmException& excp )
	{
		cout << excp << endl;
	}
	return 0;
}
