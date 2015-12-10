/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "StreamId.h"

using namespace thomsonreuters::ema::access;

StreamId::StreamId( Int32 streamId )
{
    value = streamId;
}

Int32 StreamId::operator()() const
{
    return value;
}
