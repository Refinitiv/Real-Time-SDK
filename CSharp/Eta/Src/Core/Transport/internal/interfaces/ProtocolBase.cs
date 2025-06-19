/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System.Collections.Generic;
using System.Net.Sockets;
using LSEG.Eta.Transports;

namespace LSEG.Eta.Internal.Interfaces
{
    internal abstract class ProtocolBase : IProtocol
    {
        protected IList<IChannel> Channels = new List<IChannel>(100);

        protected IList<IServer> Servers = new List<IServer>(10);

        #region IProtocol

        public abstract IChannel CreateChannel(ConnectOptions connectOptions, out Error error);
        public abstract IChannel CreateChannel(AcceptOptions acceptOptions, IServer server, Socket socket, out Error error);
        public abstract IServer CreateServer(BindOptions bindOptions, out Error error);
        public abstract void Uninitialize(out Error error);

        public abstract Pool GetPool(int poolSpec);

        public abstract void CloseChannel(IChannel channel);

        public abstract void CloseServer(IServer server);

        #endregion
    }
}
