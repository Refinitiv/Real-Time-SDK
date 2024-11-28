/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020 LSEG. All rights reserved.                 --
 *|-----------------------------------------------------------------------------
 */

#include "OmmJsonConverterExceptionImpl.h"
#include "ChannelInfoImpl.h"

using namespace refinitiv::ema::access;

OmmJsonConverterExceptionImpl::OmmJsonConverterExceptionImpl() :
	OmmJsonConverterException()
{
}

OmmJsonConverterExceptionImpl::~OmmJsonConverterExceptionImpl()
{
}

OmmJsonConverterExceptionImpl::OmmJsonConverterExceptionImpl(const OmmJsonConverterExceptionImpl& other) :
	OmmJsonConverterException(other)
{
}

OmmJsonConverterExceptionImpl& OmmJsonConverterExceptionImpl::operator=(const OmmJsonConverterExceptionImpl& other)
{
	if (this == &other) return *this;

	OmmJsonConverterException::operator=(other);

	return *this;
}
const SessionInfo& OmmJsonConverterExceptionImpl::getSessionInfo() const
{
	return *pSessionInfo;
}

void OmmJsonConverterExceptionImpl::throwException(const char* text, Int32 errorCode, RsslReactorChannel* reactorChannel, ClientSession* clientSession, OmmProvider* pProvider)
{
	OmmJsonConverterExceptionImpl exception;

	if (pProvider != NULL)
	{
		ConsumerSessionInfo sessionInfo;
		ChannelInfoImpl::getChannelInformationImpl(reactorChannel, OmmCommonImpl::ConsumerEnum, const_cast<ChannelInformation&>(sessionInfo._channelInfo));

		exception.statusText(text);
		exception._errorCode = errorCode;
		exception.pSessionInfo = &sessionInfo;

		throw exception;
	}
	else
	{
		ProviderSessionInfo sessionInfo;

		if (clientSession)
		{
			sessionInfo._clientHandle = clientSession->getClientHandle();
			sessionInfo._handle = clientSession->getLoginHandle();
			sessionInfo._provider = pProvider;
			ChannelInfoImpl::getChannelInformationImpl(reactorChannel, OmmCommonImpl::IProviderEnum, const_cast<ChannelInformation&>(sessionInfo._channelInfo));
		}
		else
		{
			sessionInfo._provider = pProvider;

			ChannelInfoImpl::getChannelInformationImpl(reactorChannel, OmmCommonImpl::NiProviderEnum, const_cast<ChannelInformation&>(sessionInfo._channelInfo));
		}

		exception.statusText(text);
		exception._errorCode = errorCode;
		exception.pSessionInfo = &sessionInfo;

		throw exception;
	}
}
