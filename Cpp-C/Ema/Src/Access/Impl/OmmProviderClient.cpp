/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "OmmProviderClient.h"

using namespace thomsonreuters::ema::access;

OmmProviderClient::OmmProviderClient()
{
}

OmmProviderClient::~OmmProviderClient()
{
}

void OmmProviderClient::onRefreshMsg( const RefreshMsg&, const OmmProviderEvent& ) 
{
}

void OmmProviderClient::onStatusMsg( const StatusMsg&, const OmmProviderEvent& ) 
{
}

void OmmProviderClient::onGenericMsg( const GenericMsg&, const OmmProviderEvent& ) 
{
}

void OmmProviderClient::onAllMsg( const Msg&, const OmmProviderEvent& ) 
{
}

void OmmProviderClient::onPostMsg( const PostMsg&, const OmmProviderEvent& ) 
{
}

void OmmProviderClient::onReqMsg( const ReqMsg&, const OmmProviderEvent& ) 
{
}

void OmmProviderClient::onReissue( const ReqMsg&, const OmmProviderEvent& ) 
{
}

void OmmProviderClient::onClose( const ReqMsg&, const OmmProviderEvent& ) 
{
}
