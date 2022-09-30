/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

namespace Refinitiv.Eta.Transports
{
    internal interface IChannelSink
    {
        event ConnectionStateChangeHandler OnConnected;
        event ConnectionStateChangeHandler OnDisconnected;
        event ConnectionStateChangeHandler OnError;
    }
}
