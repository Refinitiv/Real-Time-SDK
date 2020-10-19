///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2020 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

#include "IProvider.h"
#include <string.h>

using namespace refinitiv::ema::access;
using namespace refinitiv::ema::rdm;
using namespace std;

UInt64 itemHandle = 0;
UInt64 loginHandle = 0;
UInt64 loginClientHandle = 0;
UInt64 lastLatency = 0;

bool supportRTT = false;

void AppClient::processLoginRequest(const ReqMsg& reqMsg, const OmmProviderEvent& event)
{
	ElementList encodedElementList;

	/* Does consumer support RTT? */
	if (reqMsg.getAttrib().getDataType() == DataType::ElementListEnum)
	{
		const ElementList& elementList = reqMsg.getAttrib().getElementList();
		while (elementList.forth())
		{
			const ElementEntry& elementEntry = elementList.getEntry();

			if (elementEntry.getName() == ENAME_RTT && elementEntry.getLoadType() == DataType::UIntEnum && elementEntry.getUInt() == SUPPORT_RTT)
			{
				supportRTT = true;
				break;
			}
		}
	}

	/* This provider supports RTT. */
	if (supportRTT)
	{
		encodedElementList.addUInt(ENAME_RTT, SUPPORT_RTT);
	}

	encodedElementList.complete();

	event.getProvider().submit(RefreshMsg().
		domainType(MMT_LOGIN).
		name(reqMsg.getName()).
		nameType(USER_NAME).
		complete().
		attrib(encodedElementList).
		solicited(true).state(OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "Login accepted"),
		event.getHandle());

	if (loginHandle == 0)
	{
		loginHandle = event.getHandle();
	}

	if (loginClientHandle == 0)
	{
		loginClientHandle = event.getClientHandle();
	}
}

void AppClient::processMarketPriceRequest(const ReqMsg& reqMsg, const OmmProviderEvent& event)
{
	if (itemHandle != 0)
	{
		processInvalidItemRequest(reqMsg, event);
		return;
	}

	event.getProvider().submit(RefreshMsg().name(reqMsg.getName()).serviceName(reqMsg.getServiceName()).solicited(true).
		state(OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "Refresh Completed").
		payload(FieldList().
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

void AppClient::onGenericMsg(const GenericMsg& genericMsg, const OmmProviderEvent& event)
{
	if (genericMsg.getDomainType() == MMT_LOGIN && event.getHandle() == loginHandle && genericMsg.getPayload().getDataType() == DataType::ElementListEnum)
	{
		cout << "Received login RTT message from Consumer " << event.getHandle() << endl;

		TimeValue currTicks = GetTime::getTicks();

		const ElementList& elementList = genericMsg.getPayload().getElementList();
		while ( elementList.forth() )
		{
			const ElementEntry& elementEntry = elementList.getEntry();

			if ( elementEntry.getName() == ENAME_RTT_TICKS && elementEntry.getLoadType() == DataType::UIntEnum ) // "Ticks"
			{
				cout << "\tRTT Tick value is " << elementEntry.getUInt() << "us." << endl;

				lastLatency = (UInt64)(((double)currTicks - (double)elementEntry.getUInt()) / GetTime::ticksPerMicro());

				cout << "\tLast RTT message latency is " << lastLatency << "us." << endl;
			}
			else if ( elementEntry.getName() == ENAME_RTT_TCP_RETRANS && elementEntry.getLoadType() == DataType::UIntEnum ) // "TcpRetrans"
			{
				cout << "\tConsumer side TCP retransmissions: " << elementEntry.getUInt() << endl;
			}
		}
	}
}

int main(int argc, char* argv[])
{
	try
	{
		AppClient appClient;

		OmmProvider provider(OmmIProviderConfig().port("14002"), appClient);

		while (itemHandle == 0) sleep(1000);

		for (Int32 i = 0; i < 60; i++)
		{
			provider.submit(UpdateMsg().payload(FieldList().
				addReal(22, 3391 + i, OmmReal::ExponentNeg2Enum).
				addReal(30, 10 + i, OmmReal::Exponent0Enum).
				complete()), itemHandle);

			if (supportRTT)
			{
				ElementList elementList;

				elementList.addUInt(ENAME_RTT, lastLatency);
				elementList.addUInt(ENAME_RTT_TICKS, GetTime::getTicks());

				try
				{
					ChannelStatistics stats;
					provider.getConnectedClientChannelStats(loginClientHandle, stats);
					if (stats.hasTcpRetransmitCount())
					{
						elementList.addUInt(ENAME_RTT_TCP_RETRANS, stats.getTcpRetransmitCount());
					}
				}
				catch (const OmmException& excp)
				{
					cout << excp << endl;
				}

				elementList.complete();

				provider.submit(GenericMsg().domainType(MMT_LOGIN).
					payload( elementList ).
					providerDriven( true ).
					complete(), loginHandle);
			}

			sleep(1000);
		}
	}
	catch ( const OmmException& excp )
	{
		cout << excp << endl;
	}

	return 0;
}
