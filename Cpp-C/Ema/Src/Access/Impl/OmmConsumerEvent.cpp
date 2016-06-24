/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "OmmConsumerEvent.h"
#include "ItemCallbackClient.h"

using namespace thomsonreuters::ema::access;

OmmConsumerEvent::OmmConsumerEvent() :
	_handle( 0 ),
	_closure( 0 ),
	_parentHandle( 0 )
{
}

OmmConsumerEvent::~OmmConsumerEvent()
{
}

UInt64 OmmConsumerEvent::getHandle() const
{
	return _handle;
}

void* OmmConsumerEvent::getClosure() const
{
	return _closure;
}

UInt64 OmmConsumerEvent::getParentHandle() const
{
	return _parentHandle;
}
