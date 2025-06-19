/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015,2019-2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "StreamId.h"

using namespace refinitiv::ema::access;

StreamId::StreamId( Int32 streamId )
{
    value = streamId;
}

Int32 StreamId::operator()() const
{
    return value;
}
