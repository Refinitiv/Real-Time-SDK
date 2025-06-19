/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2018-2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "OmmProviderErrorClient.h"

using namespace refinitiv::ema::access;


void OmmProviderErrorClient::onInvalidHandle( UInt64, const EmaString& )
{
}

void OmmProviderErrorClient::onInaccessibleLogFile( const EmaString&, const EmaString& )
{
}

void OmmProviderErrorClient::onMemoryExhaustion( const EmaString& )
{
}

void OmmProviderErrorClient::onInvalidUsage( const EmaString& )
{
}

void OmmProviderErrorClient::onInvalidUsage(const EmaString&, Int32 )
{
}

void OmmProviderErrorClient::onSystemError( Int64, void*, const EmaString& )
{
}


void OmmProviderErrorClient::onJsonConverter(const EmaString& text, Int32 errorCode, const ProviderSessionInfo& sessionInfo)
{
}

void OmmProviderErrorClient::onDispatchError(const EmaString&, Int32)
{
}

