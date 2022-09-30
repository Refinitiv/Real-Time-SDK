/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using System.Collections.Generic;
using System.Net.Sockets;
using Refinitiv.Eta.Transports;
using Refinitiv.Eta.Transports.Interfaces;

namespace Refinitiv.Eta.Internal.Interfaces
{
    internal abstract class ProtocolBase : IProtocol
    {
        protected IList<ChannelBase> Channels = new List<ChannelBase>();

        #region IProtocol

        public abstract IChannel CreateChannel(ConnectOptions connectOptions, out Error error);
        public abstract IChannel CreateChannel(AcceptOptions acceptOptions, IServer server, Socket socket, out Error error);
        public abstract IServer CreateServer(BindOptions bindOptions, out Error error);
        public abstract void Uninitialize(out Error error);

        public abstract Pool GetPool(int poolSpec);

        #endregion
    }
}
