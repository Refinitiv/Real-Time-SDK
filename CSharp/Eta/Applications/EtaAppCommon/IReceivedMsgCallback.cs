/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using Refinitiv.Common.Interfaces;
using Refinitiv.Eta.Transports.Interfaces;

namespace Refinitiv.Eta.Example.Common
{
    /// <summary>
    /// Callback method used by <see cref="ProviderSession.Read(IChannel, out Transports.Error?, IReceivedMsgCallback)"/>
    /// </summary>
    public interface IReceivedMsgCallback
    {
        public void ProcessReceivedMsg(IChannel channel, ITransportBuffer messageBuf);

        public void ProcessChannelClose(IChannel channel);
    }
}
