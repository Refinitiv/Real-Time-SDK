///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

//APIQA This file is QATools standalone. See qa_readme.txt for details about this tool.

#include "IProvider.h"
#include <string.h>

using namespace refinitiv::ema::access;
using namespace refinitiv::ema::rdm;
using namespace std;
//APIQA 
Int32 numberOfUpdates = 600;
UInt64 itemHandle = 0;

UInt64 testFlag = 0;
bool isLogin = false;
EmaBuffer groupId;
EmaString link1("Link1");
EmaString link2("Link2");

EmaString statusText("Refresh Completed");

void AppClient::processLoginRequest( const ReqMsg& reqMsg, const OmmProviderEvent& event )
{
	event.getProvider().submit(RefreshMsg().domainType(MMT_LOGIN).name(reqMsg.getName()).nameType(USER_NAME).complete()
		.solicited( true ).solicited( true ).state( OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "Login accepted" ),
		event.getHandle() );

	isLogin = true;
}

void AppClient::processMarketPriceRequest( const ReqMsg& reqMsg, const OmmProviderEvent& event )
{
	if ( itemHandle != 0 )
	{
		event.getProvider().submit( StatusMsg().name(reqMsg.getName() ).serviceName( reqMsg.getServiceName() )
			.domainType( reqMsg.getDomainType() )
			.state( OmmState::ClosedEnum, OmmState::SuspectEnum, OmmState::TooManyItemsEnum, "This test tool support only one item." ),
			event.getHandle() );
		return;
	}

	event.getProvider().submit(RefreshMsg().clear().serviceId(reqMsg.getServiceId()).name(reqMsg.getName())
		.state(OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, statusText).solicited(true).itemGroup(groupId)
		.payload( FieldList().clear()
		    .addUInt( 1, 6560 )
		    .addUInt( 2, 66 )
		    .addUInt( 3855, 52832001 )
		    .addRmtes( 296, EmaBuffer("BOS", 3) )
		    .addTime( 375, 21, 0 )
		    .addTime( 1025, 14, 40, 32 )
		    .addReal( 22, 14400, OmmReal::ExponentNeg2Enum )
		    .addReal( 25, 14700, OmmReal::ExponentNeg2Enum )
		    .addReal( 30, 9, OmmReal::Exponent0Enum )
		    .addReal( 31, 19, OmmReal::Exponent0Enum )
		    .complete() )
		.complete(), event.getHandle() );

	itemHandle = event.getHandle();
}

void AppClient::processInvalidItemRequest( const ReqMsg& reqMsg, const OmmProviderEvent& event )
{
	event.getProvider().submit( StatusMsg().name( reqMsg.getName() ).serviceName( reqMsg.getServiceName() ).
		domainType( reqMsg.getDomainType() ).
		state( OmmState::ClosedEnum, OmmState::SuspectEnum, OmmState::NotFoundEnum, "Item not found" ),
		event.getHandle() );
}

void AppClient::onReqMsg( const ReqMsg& reqMsg, const OmmProviderEvent& event )
{
	switch ( reqMsg.getDomainType() )
	{
	case MMT_LOGIN:
		processLoginRequest( reqMsg, event );
		break;
	case MMT_MARKET_PRICE:
		processMarketPriceRequest( reqMsg, event );
		break;
	default:
		processInvalidItemRequest( reqMsg, event );
		break;
	}
}

namespace TestFlags
{
	const UInt64 serviceDown =                   0x0000000000000001;
	const UInt64 openWindowsAndLoadFactor =      0x0000000000000002;
	const UInt64 mergedToGroup =                 0x0000000000000004;
	const UInt64 groupStatusDataOk =			 0x0000000000000008;
	const UInt64 groupStatusDataSuspect =        0x0000000000000010;
	const UInt64 groupStatusClosedRecover =		 0x0000000000000020;
	const UInt64 changeDataType =                0x0000000000000040;
	const UInt64 chagneLinkTypeAndStateAndCode = 0x0000000000000080;
	const UInt64 infoAddCapabilities		   = 0x0000000000000100;
	const UInt64 sequencedMulticast            = 0x0000000000000200;
	const UInt64 acceptingRequest			   = 0x0000000000000400;
    const UInt64 mergedToGroupAndSendGroupStatusClosedRecover = 0x0000000000000800;
    const UInt64 serviceDelete 			= 0x0000000000001000;
};

void addSourceDirectory(OmmProvider& provider, UInt64 handle)
{
	switch (testFlag)
	{
	case TestFlags::serviceDown:
	{
		// Do nothing for the state filter
	}
	break;
	case TestFlags::openWindowsAndLoadFactor:
	{
		provider.submit(UpdateMsg().domainType(MMT_DIRECTORY).filter(SERVICE_LOAD_FILTER)
			.payload(Map()
			.addKeyUInt(1, MapEntry::UpdateEnum, FilterList()
			.add(SERVICE_LOAD_ID, FilterEntry::SetEnum, ElementList()
			.addUInt(ENAME_OPEN_LIMIT, 100)
			.addUInt(ENAME_OPEN_WINDOW, 50)
			.addUInt(ENAME_LOAD_FACT, 5555)
			.complete())
			.complete()).complete()), handle);
	}
	break;
	case TestFlags::mergedToGroup:
        case TestFlags::mergedToGroupAndSendGroupStatusClosedRecover:
	case TestFlags::groupStatusClosedRecover:
	case TestFlags::groupStatusDataSuspect:
	{
		provider.submit(UpdateMsg().domainType(MMT_DIRECTORY).filter(SERVICE_GROUP_FILTER)
			.payload(Map()
			.addKeyUInt(1, MapEntry::UpdateEnum, FilterList()
			.add(SERVICE_GROUP_ID, FilterEntry::SetEnum, ElementList()
			.addBuffer(ENAME_GROUP, EmaBuffer("5", 2))
			.addState(ENAME_STATUS, OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "")
			.complete())
			.complete()).complete()), handle);
	}
	break;
	case TestFlags::groupStatusDataOk:
	{   // Send initial state as suspect
		provider.submit(UpdateMsg().domainType(MMT_DIRECTORY).filter(SERVICE_GROUP_FILTER)
			.payload(Map()
			.addKeyUInt(1, MapEntry::UpdateEnum, FilterList()
			.add(SERVICE_GROUP_ID, FilterEntry::SetEnum, ElementList()
			.addBuffer(ENAME_GROUP, EmaBuffer("5", 2))
			.addState(ENAME_STATUS, OmmState::OpenEnum, OmmState::SuspectEnum, OmmState::NoneEnum, "")
			.complete())
			.complete()).complete()), handle);
	}
	break;
	case TestFlags::changeDataType:
	{
		provider.submit(UpdateMsg().domainType(MMT_DIRECTORY).filter(SERVICE_DATA_FILTER)
			.payload(Map()
			.addKeyUInt(1, MapEntry::UpdateEnum, FilterList()
			.add(SERVICE_DATA_ID, FilterEntry::SetEnum, ElementList()
			.addUInt(ENAME_TYPE, 1)
			.addAscii(ENAME_DATA, "Data")
			.complete())
			.complete()).complete()), handle);
	}
	break;
	case TestFlags::chagneLinkTypeAndStateAndCode:
	{
		provider.submit(UpdateMsg().domainType(MMT_DIRECTORY).filter(SERVICE_LINK_FILTER)
			.payload(Map()
			.addKeyUInt(1, MapEntry::UpdateEnum, FilterList()
			.add(SERVICE_LINK_ID, FilterEntry::SetEnum, Map().addKeyAscii( link1, MapEntry::AddEnum, 
			ElementList()
			.addUInt(ENAME_TYPE, 1)
			.addUInt(ENAME_LINK_STATE, 1)
			.addUInt(ENAME_LINK_CODE, 1)
			.addAscii(ENAME_TEXT, "Text of Link1")
			.complete())
			.addKeyAscii( link2, MapEntry::AddEnum,
			ElementList()
			.addUInt(ENAME_TYPE, 2)
			.addUInt(ENAME_LINK_STATE, 1)
			.addUInt(ENAME_LINK_CODE, 1)
			.addAscii(ENAME_TEXT, "Text of Link2")
			.complete())
			.complete()).complete()).complete()), handle);
	}
	break;
	case TestFlags::sequencedMulticast:
	{
		provider.submit(UpdateMsg().domainType(MMT_DIRECTORY).filter(64)
			.payload(Map()
			.addKeyUInt(1, MapEntry::UpdateEnum, FilterList()
			.add(7, FilterEntry::SetEnum, ElementList()
			.addAscii("ReferenceDataServerHost", "10.0.1.125")
			.addUInt("ReferenceDataServerPort", 19000)
			.addAscii("SnapshotServerHost", "10.0.1.125")
			.addUInt("SnapshotServerPort", 14002)
			.addAscii("GapRecoveryServerHost", "10.0.1.125")
			.addUInt("GapRecoveryServerPort", 14001)
			.addVector("StreamingMulticastChannels", Vector()
			.add(1, VectorEntry::InsertEnum, ElementList()
			.addAscii("MulticastGroup", "224.1.62.2")
			.addUInt("Port", 30001)
			.addUInt("Domain", 6).complete()).complete())
			.complete())
			.complete()).complete()), handle);
	}
	break;
	}
}

void updateSourceDirectory(OmmProvider& provider, UInt64 handle)
{
	switch (testFlag)
	{
		case TestFlags::serviceDelete:
                {
                       provider.submit(UpdateMsg().domainType(MMT_DIRECTORY)
                                .payload(Map()
                                .addKeyUInt(1, MapEntry::DeleteEnum, FilterList()
                                .complete()).complete()), handle); 
                }
                break;
		case TestFlags::serviceDown:
		{
			provider.submit(UpdateMsg().domainType(MMT_DIRECTORY).filter(SERVICE_STATE_FILTER)
				.payload(Map()
				.addKeyUInt(1, MapEntry::UpdateEnum, FilterList()
				.add(SERVICE_STATE_ID, FilterEntry::UpdateEnum, ElementList()
				.addUInt(ENAME_SVC_STATE, SERVICE_DOWN)
				.complete())
				.complete()).complete()), handle);
		}
		break;
		case TestFlags::acceptingRequest:
		{
			provider.submit(UpdateMsg().domainType(MMT_DIRECTORY).filter(SERVICE_STATE_FILTER)
				.payload(Map()
				.addKeyUInt(1, MapEntry::UpdateEnum, FilterList()
				.add(SERVICE_STATE_ID, FilterEntry::UpdateEnum, ElementList()
				.addUInt(ENAME_SVC_STATE, SERVICE_UP)
				.addUInt(ENAME_ACCEPTING_REQS, 0)
				.complete())
				.complete()).complete()), handle);
		}
		break;
		case TestFlags::openWindowsAndLoadFactor:
		{
			provider.submit(UpdateMsg().domainType(MMT_DIRECTORY).filter(SERVICE_LOAD_FILTER)
				.payload(Map()
				.addKeyUInt(1, MapEntry::UpdateEnum, FilterList()
				.add(SERVICE_LOAD_ID, FilterEntry::UpdateEnum, ElementList()
				.addUInt(ENAME_OPEN_WINDOW, 90)
				.addUInt(ENAME_LOAD_FACT, 9999)
				.complete())
				.complete()).complete()), handle);
		}
		break;
		case TestFlags::mergedToGroup:
                case TestFlags::mergedToGroupAndSendGroupStatusClosedRecover:
		{
			provider.submit(UpdateMsg().domainType(MMT_DIRECTORY).filter(SERVICE_GROUP_FILTER)
				.payload(Map()
				.addKeyUInt(1, MapEntry::UpdateEnum, FilterList()
				.add(SERVICE_GROUP_ID, FilterEntry::UpdateEnum, ElementList()
				.addBuffer(ENAME_GROUP, EmaBuffer("5", 2))
				.addBuffer(ENAME_MERG_TO_GRP, EmaBuffer("7", 2))
				.complete())
				.complete()).complete()), handle);


                       if( testFlag == TestFlags::mergedToGroupAndSendGroupStatusClosedRecover )
                       {
                            provider.submit(UpdateMsg().domainType(MMT_DIRECTORY).filter(SERVICE_GROUP_FILTER)
                                .payload(Map()
                                .addKeyUInt(1, MapEntry::UpdateEnum, FilterList()
                                .add(SERVICE_GROUP_ID, FilterEntry::UpdateEnum, ElementList()
                                .addBuffer(ENAME_GROUP, EmaBuffer("7", 2))
                                .addState(ENAME_STATUS, OmmState::ClosedRecoverEnum, OmmState::SuspectEnum, OmmState::NoneEnum, "")
                                .complete())
                                .complete()).complete()), handle);
                       }
		}
		break;
		case TestFlags::groupStatusDataOk:
		{  
			provider.submit(UpdateMsg().domainType(MMT_DIRECTORY).filter(SERVICE_GROUP_FILTER)
				.payload(Map()
				.addKeyUInt(1, MapEntry::UpdateEnum, FilterList()
				.add(SERVICE_GROUP_ID, FilterEntry::UpdateEnum, ElementList()
				.addBuffer(ENAME_GROUP, EmaBuffer("5", 2))
				.addState(ENAME_STATUS, OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "")
				.complete())
				.complete()).complete()), handle);
		}
		break;
		case TestFlags::groupStatusDataSuspect:
		{
			provider.submit(UpdateMsg().domainType(MMT_DIRECTORY).filter(SERVICE_GROUP_FILTER)
				.payload(Map()
				.addKeyUInt(1, MapEntry::UpdateEnum, FilterList()
				.add(SERVICE_GROUP_ID, FilterEntry::UpdateEnum, ElementList()
				.addBuffer(ENAME_GROUP, EmaBuffer("5", 2))
				.addState(ENAME_STATUS, OmmState::OpenEnum, OmmState::SuspectEnum, OmmState::NoneEnum, "")
				.complete())
				.complete()).complete()), handle);
		}
		break;
		case TestFlags::groupStatusClosedRecover:
		{
			provider.submit(UpdateMsg().domainType(MMT_DIRECTORY).filter(SERVICE_GROUP_FILTER)
				.payload(Map()
				.addKeyUInt(1, MapEntry::UpdateEnum, FilterList()
				.add(SERVICE_GROUP_ID, FilterEntry::UpdateEnum, ElementList()
				.addBuffer(ENAME_GROUP, EmaBuffer("5", 2))
				.addState(ENAME_STATUS, OmmState::ClosedRecoverEnum, OmmState::SuspectEnum, OmmState::NoneEnum, "")
				.complete())
				.complete()).complete()), handle);
		}
		break;
		case TestFlags::changeDataType:
		{
			provider.submit(UpdateMsg().domainType(MMT_DIRECTORY).filter(SERVICE_DATA_FILTER)
				.payload(Map()
				.addKeyUInt(1, MapEntry::UpdateEnum, FilterList()
				.add(SERVICE_DATA_ID, FilterEntry::UpdateEnum, ElementList()
                                .addUInt(ENAME_TYPE, 0)
				.addAscii(ENAME_DATA, "UpdatedData")
				.complete())
				.complete()).complete()), handle);
		}
		break;
		case TestFlags::chagneLinkTypeAndStateAndCode:
		{
			provider.submit(UpdateMsg().domainType(MMT_DIRECTORY).filter(SERVICE_LINK_FILTER)
				.payload(Map()
				.addKeyUInt(1, MapEntry::UpdateEnum, FilterList()
				.add(SERVICE_LINK_ID, FilterEntry::UpdateEnum, Map().addKeyAscii(link1, MapEntry::UpdateEnum,
				ElementList()
				.addUInt(ENAME_TYPE, 1)
				.addUInt(ENAME_LINK_STATE, 0)
				.addUInt(ENAME_LINK_CODE, 2)
				.addAscii(ENAME_TEXT, "Text of Link1")
				.complete())
				.addKeyAscii(link2, MapEntry::UpdateEnum,
				ElementList()
				.addUInt(ENAME_TYPE, 2)
				.addUInt(ENAME_LINK_STATE, 0)
				.addUInt(ENAME_LINK_CODE, 2)
				.addAscii(ENAME_TEXT, "Text of Link2")
				.complete())
				.complete()).complete()).complete()), handle);
		}
		break;
		case TestFlags::infoAddCapabilities:
		{
			provider.submit(UpdateMsg().domainType(MMT_DIRECTORY).filter(SERVICE_INFO_FILTER)
				.payload(Map()
				.addKeyUInt(1, MapEntry::UpdateEnum, FilterList()
				.add(SERVICE_INFO_ID, FilterEntry::UpdateEnum, ElementList()
				.addArray(ENAME_CAPABILITIES, OmmArray()
				.addUInt(MMT_MARKET_PRICE)
				.addUInt(MMT_MARKET_BY_PRICE)
				.addUInt(MMT_DICTIONARY)
				.addUInt(MMT_MARKET_MAKER)
				.complete())
				.complete())
				.complete()).complete()), handle);
		}
		break;
		case TestFlags::sequencedMulticast:
		{
			provider.submit(UpdateMsg().domainType(MMT_DIRECTORY).filter(64)
				.payload(Map()
				.addKeyUInt(1, MapEntry::UpdateEnum, FilterList()
				.add(7, FilterEntry::UpdateEnum, 
				ElementList()
				.addAscii("ReferenceDataServerHost", "10.0.5.125" )
				.addUInt("ReferenceDataServerPort", 19005)
				.addAscii("SnapshotServerHost", "10.0.5.125")
				.addUInt("SnapshotServerPort", 15002)
				.addAscii("GapRecoveryServerHost", "10.0.5.125")
				.addUInt("GapRecoveryServerPort", 15001)
				.addVector("StreamingMulticastChannels", 
				Vector()
				.add(1, VectorEntry::InsertEnum, 
				ElementList()
				.addAscii("MulticastGroup", "224.1.62.5")
				.addUInt("Port", 50001)
				.addUInt("Domain", 8).complete())
				.complete())
				.complete())
				.complete()).complete()), handle);
		}
		break;
	}
}

void printUsage()
{
	std::cout
		<< "Usage: SourceDirectoryUpdate [UpdateInfo]\n\n"
		<< "\t -serviceDown \n"
		<< "\t -openWindowsAndLoadFactor \n"
		<< "\t -mergedToGroup \n"
		<< "\t -mergedToGroupAndSendGroupStatusClosedRecover \n"
		<< "\t -groupStatusDataOk \n"
		<< "\t -groupStatusDataSuspect \n"
		<< "\t -groupStatusClosedRecover \n"
		<< "\t -changeDataType \n"
		<< "\t -chagneLinkTypeAndStateAndCode \n"
		<< "\t -infoAddCapabilities \n"
		<< "\t -MCastInformation \n"
		<< "\t -sequencedMulticast \n"
		<< "\t -acceptingRequest \n"
		<< "\t -serviceDelete \n"
		<< std::endl;
}

int main( int argc, char* argv[] )
{
	if (argc != 2)
	{
		printUsage();
		return 0;
	}
	else
	{
		if (strcmp(argv[1], "-serviceDown") == 0)
		{
			testFlag |= TestFlags::serviceDown;
		}
		else if (strcmp(argv[1], "-openWindowsAndLoadFactor") == 0)
		{
			testFlag |= TestFlags::openWindowsAndLoadFactor;
		}
		else if (strcmp(argv[1], "-mergedToGroup") == 0)
		{
			testFlag |= TestFlags::mergedToGroup;

			groupId.setFrom("5", 2);
		}
		else if (strcmp(argv[1], "-mergedToGroupAndSendGroupStatusClosedRecover") == 0)
                {
			testFlag |= TestFlags::mergedToGroupAndSendGroupStatusClosedRecover;

			groupId.setFrom("5", 2);
                }
		else if (strcmp(argv[1], "-groupStatusDataOk") == 0)
		{
			testFlag |= TestFlags::groupStatusDataOk;

			groupId.setFrom("5", 2);
		}
		else if (strcmp(argv[1], "-groupStatusDataSuspect") == 0)
		{
			testFlag |= TestFlags::groupStatusDataSuspect;

			groupId.setFrom("5", 2);
		}
		else if (strcmp(argv[1], "-groupStatusClosedRecover") == 0)
		{
			testFlag |= TestFlags::groupStatusClosedRecover;

			groupId.setFrom("5", 2);
		}
		else if (strcmp(argv[1], "-changeDataType") == 0)
		{
			testFlag |= TestFlags::changeDataType;
		}
		else if (strcmp(argv[1], "-chagneLinkTypeAndStateAndCode") == 0)
		{
			testFlag |= TestFlags::chagneLinkTypeAndStateAndCode;
		}
		else if (strcmp(argv[1], "-infoAddCapabilities") == 0)
		{
			testFlag |= TestFlags::infoAddCapabilities;
		}
		else if (strcmp(argv[1], "-sequencedMulticast") == 0)
		{
			testFlag |= TestFlags::sequencedMulticast;
		}
		else if (strcmp(argv[1], "-acceptingRequest") == 0)
		{
			testFlag |= TestFlags::acceptingRequest;
		}
		else if (strcmp(argv[1], "-serviceDelete") == 0)
		{
			testFlag |= TestFlags::serviceDelete;
		}

		if (testFlag == 0)
		{
			cout << "Invalid argument " << argv[1]  << endl;
			return 0;
		}
	}

	try
	{
		AppClient appClient;

		RefreshMsg refreshMsg;

		OmmProvider provider(OmmIProviderConfig().operationModel(OmmIProviderConfig::UserDispatchEnum), appClient);

		while (!itemHandle) provider.dispatch(1000);

		for (int index = 0; index < numberOfUpdates; index++)
		{
			provider.submit(UpdateMsg().payload(FieldList()
				.addTime( 1025, 14, 40, 32 )
				.addUInt( 3855, 52832001 )
				.addReal(22, 14400 + ((itemHandle & 0x1) ? 1 : 10), OmmReal::ExponentNeg2Enum)
				.addReal(30, 10 + ((itemHandle & 0x1) ? 10 : 20), OmmReal::Exponent0Enum)
				.addRmtes(296, EmaBuffer().setFrom("NAS", 3) )
				.complete()), itemHandle);

			if (index == 1)
			{
				addSourceDirectory(provider, 0);
			}
			else if (index == 2)
			{
				updateSourceDirectory(provider, 0);
			}

			if (index >= 2)
			{
				provider.dispatch(1000);
				sleep(3000);
			}

			provider.dispatch(1000);
		}
	}
	catch ( const OmmException& excp )
	{
		cout << excp << endl;
	}
	
	return 0;
}

//END APIQA
