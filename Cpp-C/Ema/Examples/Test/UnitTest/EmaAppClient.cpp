/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2018-2021,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "EmaAppClient.h"

using namespace refinitiv::ema::access;
using namespace refinitiv::ema::rdm;

void AppClient::processLoginRequest(const ReqMsg& reqMsg, const OmmProviderEvent& event)
{
	event.getProvider().submit(RefreshMsg().domainType(MMT_LOGIN).name(reqMsg.getName()).nameType(USER_NAME).complete().
		attrib(ElementList().addUInt("SupportStandby", 1).complete()).solicited(true).state(OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "Login accepted"),
		event.getHandle());
}

void AppClient::onReqMsg(const ReqMsg& reqMsg, const OmmProviderEvent& event)
{
	switch (reqMsg.getDomainType())
	{
	case MMT_LOGIN:
		processLoginRequest(reqMsg, event);
		break;
	default:
		break;
	}
}
