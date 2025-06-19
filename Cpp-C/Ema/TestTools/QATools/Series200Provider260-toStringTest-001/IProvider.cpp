/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "IProvider.h"

using namespace refinitiv::ema::access;
using namespace refinitiv::ema::rdm;
using namespace std;

#define APP_DOMAIN 200

UInt64 itemHandle = 0;

void AppClient::processLoginRequest( const ReqMsg& reqMsg, const OmmProviderEvent& event )
{
	event.getProvider().submit(RefreshMsg().domainType(MMT_LOGIN).name(reqMsg.getName()).nameType(USER_NAME).complete().
		solicited( true ).state( OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "Login accepted" ),
		event.getHandle() );
}

void AppClient::processCustomDomainRequest(const ReqMsg& reqMsg, const OmmProviderEvent& event)
{
	if( itemHandle != 0 )
	{
		processInvalidItemRequest( reqMsg, event );
		return;
	}
	// API QA RTSDK-1643
	DataDictionary dictionary;
	dictionary.loadFieldDictionary("RDMFieldDictionary");
	dictionary.loadEnumTypeDictionary("enumtype.def");

	FieldList fieldList;
	fieldList.addReal(22, 3990, OmmReal::ExponentNeg2Enum).
		addReal(25, 3994, OmmReal::ExponentNeg2Enum).
		addReal(30, 9, OmmReal::Exponent0Enum).
		addReal(31, 19, OmmReal::Exponent0Enum).
		complete();
	cout << endl << "Error when FieldList.toString(): " << fieldList.toString() << endl;
	cout << endl << "Success when FieldList.toString(dictionary): \n" << fieldList.toString(dictionary) << endl; 

	ElementList elementList;
	elementList.addAscii("QATest_addAscii", "Test_addAscii")
		.addInt("QATest_addInt", 50000000)
		.addUInt("QATest_addUInt", 1).complete();
	cout << endl << "Error when ElementList.toString(): " << elementList.toString() << endl;
	cout << endl << "Success when ElementList.toString(dictionary): \n" << elementList.toString(dictionary) << endl;

	Map map;
	map.addKeyAscii("QATestMap", MapEntry::AddEnum, elementList).complete();
	cout << endl << "Error when Map.toString(): " << map.toString() << endl;
	cout << endl << "Success when Map.toString(dictionary): \n" << map.toString(dictionary) << endl;

	FilterList filterList;
	filterList.add(1, FilterEntry::UpdateEnum, fieldList);
	filterList.add(2, FilterEntry::UpdateEnum, elementList);
	filterList.add(3, FilterEntry::UpdateEnum, map);
	filterList.complete();
	cout << endl << "Error when FilterList.toString(): " << filterList.toString() << endl;
	cout << endl << "Success when FilterList.toString(dictionary): \n" << filterList.toString(dictionary) << endl;

	Series series;
	series.add(elementList);
	series.complete();
	cout << endl << "Error when Series.toString(): \n" << series.toString() << endl;
	cout << endl << "Success when Series.toString(dictionary): \n" << series.toString(dictionary) << endl;

	Vector vector;
	vector.add(1, VectorEntry::InsertEnum, filterList);
	vector.complete();
	cout << endl << "Error when Vector.toString(): " << vector.toString() << endl;
	cout << endl << "Success when Vector.toString(dictionary): \n" << vector.toString(dictionary) << endl;

	ReqMsg requestMsg;
	requestMsg.serviceName("DIRECT_FEED").name("IBM.N");
	cout << endl << "Error when ReqMsg.toString(): " << requestMsg.toString() << endl;
	cout << endl << "Success when ReqMsg.toString(dictionary): \n" << requestMsg.toString(dictionary) << endl;


	RefreshMsg refreshMsg;
	refreshMsg.domainType(reqMsg.getDomainType()).name(reqMsg.getName()).serviceId(reqMsg.getServiceId()).
		state(OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "Refresh Completed").solicited(true).privateStream(reqMsg.getPrivateStream()).
		complete();
	cout << endl << "Error when RefreshMsg.toString(): " << refreshMsg.toString() << endl;
	cout << endl << "Success when RefreshMsg.toString(dictionary): \n" << refreshMsg.toString(dictionary) << endl;

	GenericMsg genericMsg;
	genericMsg.domainType(reqMsg.getDomainType()).name("genericMsg").payload(
		RefreshMsg().name(reqMsg.getName()).serviceName(reqMsg.getServiceName()).
		state(OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "NestedMsg").
		payload(fieldList).
		complete());
	cout << endl << "Error when GenericMsg.toString(): " << genericMsg.toString() << endl;
	cout << endl << "Success when GenericMsg.toString(dictionary): \n" << genericMsg.toString(dictionary) << endl;

	StatusMsg statusMsg;
	statusMsg.name(reqMsg.getName()).serviceName(reqMsg.getServiceName()).
		domainType(reqMsg.getDomainType()).
		state(OmmState::ClosedEnum, OmmState::SuspectEnum, OmmState::NotFoundEnum, "Item not found");
	cout << endl << "Error when StatusMsg.toString(): " << statusMsg.toString() << endl;
	cout << endl << "Success when StatusMsg.toString(dictionary): \n" << statusMsg.toString(dictionary) << endl;

	UpdateMsg updateMsg;
	updateMsg.payload(fieldList);
	cout << endl << "Error when UpdateMsg.toString(): " << updateMsg.toString() << endl;
	cout << endl << "Success when UpdateMsg.toString(dictionary): \n" << updateMsg.toString(dictionary) << endl;

	PostMsg postMsg;
	postMsg.postId(1).serviceId(1).name(reqMsg.getName()).solicitAck(true).complete().payload(updateMsg);
	cout << endl << "Error when PostMsg.toString(): " << postMsg.toString() << endl;
	cout << endl << "Success when PostMsg.toString(dictionary): \n" << postMsg.toString(dictionary) << endl;

	AckMsg ackMsg;
	ackMsg.domainType(reqMsg.getDomainType());
	ackMsg.ackId(1);
	ackMsg.seqNum(1);
	cout << endl << "Error when AckMsg.toString(): " << ackMsg.toString() << endl;
	cout << endl << "Success when AckMsg.toString(dictionary): \n" << ackMsg.toString(dictionary) << endl;

	event.getProvider().submit(refreshMsg, event.getHandle());
	event.getProvider().submit(genericMsg, event.getHandle());
	//End API QA RTSDK-1643

	event.getProvider().submit(RefreshMsg().domainType(reqMsg.getDomainType()).name(reqMsg.getName()).serviceId(reqMsg.getServiceId()).
		state(OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "Refresh Completed").solicited( true ).privateStream( reqMsg.getPrivateStream() ).
		complete() , event.getHandle() );
	 
	event.getProvider().submit(GenericMsg().domainType(reqMsg.getDomainType()).name("genericMsg").payload(
		RefreshMsg().name( reqMsg.getName() ).serviceName( reqMsg.getServiceName() ).
		state( OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "NestedMsg" ).
		payload( FieldList().
			addReal( 22, 3990, OmmReal::ExponentNeg2Enum ).
			addReal( 25, 3994, OmmReal::ExponentNeg2Enum ).
			addReal( 30, 9, OmmReal::Exponent0Enum ).
			addReal( 31, 19, OmmReal::Exponent0Enum ).
			complete() ).
		complete() ) , event.getHandle() );

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
	case APP_DOMAIN:
		processCustomDomainRequest(reqMsg, event);
		break;
	default:
		processInvalidItemRequest( reqMsg, event );
		break;
	}
}

void AppClient::onGenericMsg(const refinitiv::ema::access::GenericMsg&, const refinitiv::ema::access::OmmProviderEvent& event)
{
	cout << endl << "Received:    GenericMsg" << endl << "Item Handle: " << event.getHandle() << endl << "Closure:     " << event.getClosure() << endl;
}

int main()
{
	try
	{
		AppClient appClient;

		OmmProvider provider( OmmIProviderConfig().operationModel( OmmIProviderConfig::UserDispatchEnum ), appClient );

		while ( itemHandle == 0 ) provider.dispatch( 1000 );

		for ( Int32 i = 0; i < 60; i++ )
		{
			provider.dispatch( 10000 );

			provider.submit( GenericMsg().domainType(APP_DOMAIN).name( "genericMsg" ).payload(
				UpdateMsg().payload( FieldList().
				addReal( 22, 3391 + i, OmmReal::ExponentNeg2Enum ) .
				addReal( 30, 10 + i, OmmReal::Exponent0Enum ).
				complete() ) ), itemHandle );

			sleep(1000);
		}
	}
	catch ( const OmmException& excp )
	{
		cout << excp << endl;
	}
	
	return 0;
}
