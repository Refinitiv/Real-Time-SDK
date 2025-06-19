/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2018-2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "IProvider.h"

using namespace refinitiv::ema::access;
using namespace refinitiv::ema::rdm;
using namespace std;

UInt64 itemHandle = 0;

void AppClient::processLoginRequest(const ReqMsg& reqMsg, const OmmProviderEvent& event)
{
	event.getProvider().submit(RefreshMsg().domainType(MMT_LOGIN).name(reqMsg.getName()).nameType(USER_NAME).complete().
		solicited(true).state(OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "Login accepted"),
		event.getHandle());
}

void AppClient::processMarketPriceRequest(const ReqMsg& reqMsg, const OmmProviderEvent& event)
{
	if (itemHandle != 0)
	{
		processInvalidItemRequest(reqMsg, event);
		return;
	}

	event.getProvider().submit(RefreshMsg().name(reqMsg.getName()).serviceName(reqMsg.getServiceName()).
		state(OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "Refresh Completed").solicited(true).
		payload(FieldList().
			addAscii(3, reqMsg.getName()).
			addEnum(15, 840).
			addReal(21, 3900, OmmReal::ExponentNeg2Enum).
			addReal(22, 3990, OmmReal::ExponentNeg2Enum).
			addReal(25, 3994, OmmReal::ExponentNeg2Enum).
			addReal(30, 9, OmmReal::Exponent0Enum).
			addReal(31, 19, OmmReal::Exponent0Enum).
			complete()).
		complete(), event.getHandle());

	itemHandle = event.getHandle();
}

void AppClient::processInvalidItemRequest(const ReqMsg& reqMsg, const OmmProviderEvent& event)
{
	event.getProvider().submit(StatusMsg().name(reqMsg.getName()).serviceName(reqMsg.getServiceName()).
		domainType(reqMsg.getDomainType()).
		state(OmmState::ClosedEnum, OmmState::SuspectEnum, OmmState::NotFoundEnum, "Item not found"),
		event.getHandle());
}

void AppClient::onReqMsg(const ReqMsg& reqMsg, const OmmProviderEvent& event)
{
	switch (reqMsg.getDomainType())
	{
	case MMT_LOGIN:
		processLoginRequest(reqMsg, event);
		break;
	case MMT_MARKET_PRICE:
		processMarketPriceRequest(reqMsg, event);
		break;
	default:
		processInvalidItemRequest(reqMsg, event);
		break;
	}
}
//APIQA

void createProgrammaticConfig(const EmaString& dictConfigPath, Map& configMap)
{
	Map innerMap;
	ElementList elementList;

	innerMap.addKeyAscii("Dictionary_1", MapEntry::AddEnum,
		ElementList()
		.addEnum("DictionaryType", 0)
		.addAscii("RdmFieldDictionaryFileName", dictConfigPath + ("/RDMFieldDictionary"))
		.addAscii("EnumTypeDefFileName", dictConfigPath + ("/enumtype.def"))
		.addAscii("RdmFieldDictionaryItemName", "RWFFld3")
		.addAscii("EnumTypeDefItemName", "RWFEnum3").complete()).complete();

	elementList.addMap("DictionaryList", innerMap);
	elementList.complete();
	innerMap.clear();

	configMap.addKeyAscii("DictionaryGroup", MapEntry::AddEnum, elementList);
	elementList.clear();
}

//END APIQA
int main(int argc, char* argv[])
{
	try
	{
		EmaString dictConfigPath(".");
		if (argc == 2)
			dictConfigPath.clear().append(argv[1]);

		AppClient appClient;
		Map configMap;
		createProgrammaticConfig(dictConfigPath, configMap);

		OmmProvider provider(OmmIProviderConfig().config(configMap).operationModel(OmmIProviderConfig::UserDispatchEnum), appClient);

		while (itemHandle == 0) provider.dispatch(1000);

		for (Int32 i = 0; i < 60; i++)
		{
			provider.dispatch(10000);

			provider.submit(UpdateMsg().payload(FieldList().
				addReal(22, 3391 + i, OmmReal::ExponentNeg2Enum).
				addReal(25, 3994 + i, OmmReal::ExponentNeg2Enum).
				addReal(30, 10 + i, OmmReal::Exponent0Enum).
				addReal(31, 19 + i, OmmReal::Exponent0Enum).
				complete()), itemHandle);

			sleep(1000);

		}
	}
	catch (const OmmException& excp)
	{
		cout << excp << endl;
	}

	return 0;
}
