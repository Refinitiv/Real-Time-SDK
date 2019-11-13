/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "OmmConsumerErrorClient.h"

using namespace thomsonreuters::ema::access;

OmmConsumerErrorClient::OmmConsumerErrorClient()
{
}

OmmConsumerErrorClient::~OmmConsumerErrorClient()
{
}

void OmmConsumerErrorClient::onInvalidHandle( UInt64, const EmaString& )
{
}

void OmmConsumerErrorClient::onInaccessibleLogFile( const EmaString&, const EmaString& )
{
}

void OmmConsumerErrorClient::onMemoryExhaustion( const EmaString& )
{
}

void OmmConsumerErrorClient::onInvalidUsage( const EmaString& )
{
}

void OmmConsumerErrorClient::onInvalidUsage(const EmaString&, Int32)
{
}

void OmmConsumerErrorClient::onSystemError( Int64, void*, const EmaString& )
{
}
