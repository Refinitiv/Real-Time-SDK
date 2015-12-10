/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "OmmNiProviderEvent.h"
#include "ItemCallbackClient.h"

using namespace thomsonreuters::ema::access;

OmmNiProviderEvent::OmmNiProviderEvent() :
 _pItem( 0 )
{
}

OmmNiProviderEvent::~OmmNiProviderEvent()
{
}

UInt64 OmmNiProviderEvent::getHandle() const
{
	return (UInt64)_pItem;
}

void* OmmNiProviderEvent::getClosure() const
{
	return _pItem->getClosure();
}
