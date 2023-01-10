/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

using System.Net.Sockets;

namespace LSEG.Eta.Transports
{
    internal interface IProtocol
    {
        IChannel CreateChannel(ConnectOptions connectOptions, out Error error);

        IServer CreateServer(BindOptions bindOptions, out Error error);

        IChannel CreateChannel(AcceptOptions acceptOptions, IServer server, Socket socket, out Error error);

        void CloseChannel(IChannel channel);

        void CloseServer(IServer server);

        void Uninitialize(out Error error);
    }
}
