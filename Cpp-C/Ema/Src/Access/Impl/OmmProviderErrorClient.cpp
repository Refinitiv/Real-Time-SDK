/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2018. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "OmmProviderErrorClient.h"

using namespace thomsonreuters::ema::access;


OmmProviderErrorClient::OmmProviderErrorClient()
{
}

OmmProviderErrorClient::~OmmProviderErrorClient()
{
}

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

void OmmProviderErrorClient::onSystemError( Int64, void*, const EmaString& )
{
}
