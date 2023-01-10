/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Common;
using LSEG.Eta.PerfTools.Common;
using LSEG.Eta.Transports;

namespace LSEG.Eta.PerfTools.TransportPerf
{
    /// <summary>
    /// Process message interface. Enables different ways to process a message.
    /// </summary>
    public interface IProcessMsg
    {
        PerfToolsReturnCode ProcessMsg(TransportChannelHandler channelHandler, ClientChannelInfo channelInfo, ITransportBuffer msgBuffer, out Error? error);
    }
}
