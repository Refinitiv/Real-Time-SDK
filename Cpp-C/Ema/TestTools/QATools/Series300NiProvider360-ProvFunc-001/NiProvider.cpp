///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------
//APIQA this file is QATools standalone. See qa_readme.txt for details about this tool.

//APIQA
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//END APIQA
#include "NiProvider.h"

using namespace thomsonreuters::ema::access;
using namespace thomsonreuters::ema::rdm;
using namespace std;

//APIQA
int numOfItemUpdateForTest = 600;
bool userDispatch = false; //test case for diff dispatch mode with channel set
bool dirAdminControl = false; //test case admin Control with channel set
//APIQA ESDK-1601
bool testChannelInfoWithLoginHandle = false;
//END APIQA ESDK-1601
//END APIQA
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

//APIQA
bool AppClient::sendRefreshMsg() const
{
	return _sendRefreshMsg;
}

void AppClient::sendRefreshMsg(bool sending)
{
	_sendRefreshMsg = sending;
}
//END APIQA
void AppClient::onRefreshMsg( const RefreshMsg& refreshMsg, const OmmProviderEvent& ommEvent )
{
	cout << endl << "Handle: " << ommEvent.getHandle() << " Closure: " << ommEvent.getClosure() << endl;
	cout << refreshMsg << endl;

    // API QA ESDK-1601
	cout << endl << "event channel info (refresh)" << endl << ommEvent.getChannelInformation() << endl;
	// END API QA ESDK-1601

	if ( refreshMsg.getState().getStreamState() == OmmState::OpenEnum )
	{
		//APIQA
		if ( refreshMsg.getState().getDataState() == OmmState::OkEnum )
		{
			_bConnectionUp = true;
			_sendRefreshMsg = true;
		}
		//END APIQA
		else
			_bConnectionUp = false;
	}
	else
		_bConnectionUp = false;
}

void AppClient::onStatusMsg( const StatusMsg& statusMsg, const OmmProviderEvent& ommEvent )
{
	cout << endl << "Handle: " << ommEvent.getHandle() << " Closure: " << ommEvent.getClosure() << endl;
	cout << statusMsg << endl;

    // API QA ESDK-1601
	cout << endl << "event channel info (status)" << endl << ommEvent.getChannelInformation() << endl;   
	// END API QA ESDK-1601
        
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
void AppClient::onClose( const ReqMsg& reqMsg, const OmmProviderEvent& ommEvent )
{
	cout << endl << "event channel info (close)" << endl << ommEvent.getChannelInformation()
         << endl << reqMsg << endl;
}
//APIQA
void sendDirRefresh(OmmProvider& provider)
{
	RefreshMsg refresh;
	UInt64 serviceId = 1;
	UInt64 sourceDirectoryHandle = 1;
	provider.submit(refresh.domainType(MMT_DIRECTORY).filter(SERVICE_INFO_FILTER | SERVICE_STATE_FILTER)
		.payload(Map()
			.addKeyUInt(serviceId, MapEntry::AddEnum, FilterList()
				.add(SERVICE_INFO_ID, FilterEntry::SetEnum, ElementList()
					.addAscii(ENAME_NAME, "NI_PUB")
					.addArray(ENAME_CAPABILITIES, OmmArray()
						.addUInt(MMT_MARKET_PRICE)
						.addUInt(MMT_MARKET_BY_PRICE)
						.complete())
					.addArray(ENAME_DICTIONARYS_USED, OmmArray()
						.addAscii("RWFFld")
						.addAscii("RWFEnum")
						.complete())
					.complete())
				.add(SERVICE_STATE_ID, FilterEntry::SetEnum, ElementList()
					.addUInt(ENAME_SVC_STATE, SERVICE_UP)
					.complete())
				.complete())
			.complete()).complete(), sourceDirectoryHandle);
}
void printUsage(bool reflect)
{
	if (!reflect)
	{
		cout << "\nOptions:\n"
			<< "  -?\tShows this usage\n\n"
			<< "  -numOfUpdatesForApp \tSend the number of item updates for the whole test [default = 600]\n"
			<< "  -userDispatch \tUse UserDispatch Operation Model [default = false]\n"
			<< "  -dirAdminControl \tSet if user controls sending directory msg [default = false]\n"
			<< "  -testChannelInfoWithLoginHandle \tSet if test getting channel info when register login on function registerClient() default = false]\n"
			<< "\n";
		exit(-1);
	}
	else
	{
		cout << "\nOptions will be used:\n"
			 << "  -numOfUpdatesForApp \t" << numOfItemUpdateForTest << endl;
		cout << "  -userDispatch \t" << userDispatch << endl;
		cout << "  -dirAdminControl \t" << dirAdminControl << endl;
		cout << "  -testChannelInfoWithLoginHandle \t" << testChannelInfoWithLoginHandle << "\n\n";
	}
}

void  readCommandlineArgs(int argc, char* argv[])
{
	int iargs = 1;
	while (iargs < argc)
	{
		if (0 == strcmp("-?", argv[iargs]))
		{
			printUsage(false);
		}
		else if (strcmp("-numOfUpdatesForApp", argv[iargs]) == 0)
		{
			++iargs; if (iargs == argc) printUsage(false);
			numOfItemUpdateForTest = atoi(argv[iargs++]);
		}
		else if (strcmp("-userDispatch", argv[iargs]) == 0)
		{
			++iargs; if (iargs == argc) printUsage(false);
			userDispatch = (strcmp("true", argv[iargs++]) == 0 ? true : false);
		}
		else if (strcmp("-dirAdminControl", argv[iargs]) == 0)
		{
			++iargs; if (iargs == argc) printUsage(false);
			dirAdminControl = (strcmp("true", argv[iargs++]) == 0 ? true : false);
		}
		else if (strcmp("-testChannelInfoWithLoginHandle", argv[iargs]) == 0)
		{
			++iargs; if (iargs == argc) printUsage(false);
			testChannelInfoWithLoginHandle = (strcmp("true", argv[iargs++]) == 0 ? true : false);
		}
		else
		{
			cout << "Invalid argument: " << argv[iargs] << endl;
			printUsage(false);
		}
	}
	printUsage(true);
	sleep(2000);
}
//END APIQA
int main( int argc, char* argv[] )
{
	//APIQA
	OmmNiProviderConfig* config = new OmmNiProviderConfig();
	try
	{
		AppClient appClient;

		UInt64 triHandle = 6;
		RefreshMsg refresh;
		UpdateMsg update;
		FieldList fieldList;

		//APIQA
		readCommandlineArgs(argc, argv);
		if (userDispatch)
			config->operationModel(OmmNiProviderConfig::UserDispatchEnum);
		if (dirAdminControl)
			config->adminControlDirectory(OmmNiProviderConfig::UserControlEnum);
                
		// API QA ESDK-1601
		OmmProvider *pOmmprovider = 0;
        if (testChannelInfoWithLoginHandle) 
            pOmmprovider = new OmmProvider(config->username("user"));
		else
            pOmmprovider = new OmmProvider(config->username("user"), appClient);

		OmmProvider& provider = *pOmmprovider;
        ChannelInformation ci;
		provider.getChannelInformation( ci );
        cout << "channel info (provider)" << endl << ci << endl;

        if (testChannelInfoWithLoginHandle) 
            UInt64 loginHandle = provider.registerClient( ReqMsg().domainType( MMT_LOGIN ).name( "user" ), appClient );
		// END API QA ESDK-1601

		if (dirAdminControl)
			sendDirRefresh(provider);
		if (userDispatch)
			provider.dispatch(1000000);
		// END APIQA

		provider.submit( refresh.clear().serviceName( "NI_PUB" ).name( "TRI.N" )
			.state( OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "UnSolicited Refresh Completed" )
			.payload( fieldList.clear()
				.addReal( 22, 4100, OmmReal::ExponentNeg2Enum )
				.addReal( 25, 4200, OmmReal::ExponentNeg2Enum )
				.addReal( 30, 20, OmmReal::Exponent0Enum )
				.addReal( 31, 40, OmmReal::Exponent0Enum )
				.complete() )
			.complete(), triHandle );
		//APIQA
		if (userDispatch)
			provider.dispatch(1000000);
		appClient.sendRefreshMsg(false);

		for ( Int32 i = 0; i < numOfItemUpdateForTest; i++ )
		{
			if ( appClient.isConnectionUp() )
			{
				if (appClient.sendRefreshMsg())
				{
					try {
						provider.submit(refresh.clear().serviceName("NI_PUB").name("TRI.N")
							.state(OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "UnSolicited Refresh Completed")
							.payload(fieldList.clear()
								.addReal(22, 4100, OmmReal::ExponentNeg2Enum)
								.addReal(25, 4200, OmmReal::ExponentNeg2Enum)
								.addReal(30, 20, OmmReal::Exponent0Enum)
								.addReal(31, 40, OmmReal::Exponent0Enum)
								.complete())
							.complete(), triHandle);
						appClient.sendRefreshMsg(false);
					}
					catch (const OmmException& excp)
					{
						cout << excp << endl;
					}
				}
				else
				{
					try {
						provider.submit(update.clear().serviceName("NI_PUB").name("TRI.N")
							.payload(fieldList.clear()
								.addReal(22, 4100 + i, OmmReal::ExponentNeg2Enum)
								.addReal(30, 21 + i, OmmReal::Exponent0Enum)
								.complete()), triHandle);
					}
					catch (const OmmException& excp)
					{
						cout << excp << endl;
						sleep(1000);
					}
				}
			}

			if (userDispatch)
				provider.dispatch(1000000);

			sleep(1000);
		//END APIQA
		}
	}
	catch ( const OmmException& excp )
	{
		cout << excp << endl;
	}
	return 0;
}
