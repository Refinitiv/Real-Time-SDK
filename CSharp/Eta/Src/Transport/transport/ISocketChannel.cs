/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

using Refinitiv.Eta.Common;
using Refinitiv.Eta.Internal.Interfaces;
using System;
using System.Net;
using System.Net.Security;
using System.Net.Sockets;
using System.Collections.Generic;

namespace Refinitiv.Eta.Transports
{
    internal interface ISocketChannel : IChannelSink, IDisposable
    {
        bool IsConnected { get; set; }
        bool IsDisposed { get; }
        bool IsServer { get; }
        bool IsDataReady { get; set; }

        EndPoint RemoteEP { get; }
        int RemotePort { get; }

#if DEBUG
        Guid ChannelId { get; }
#endif
        bool Connect(ConnectOptions connectOptions, out Error error);
        bool Connect(ConnectOptions connectOptions, IPAddress remoteAddr, int port, bool isProxyEnabled, out Error error);

        void Disconnect();

        int Receive(ResultObject resultObject, out Error error);

        int Receive(ByteBuffer dstBuffer, out SocketError socketError);

        /// <summary>
        /// This is used for sending open handshake request only.
        /// </summary>
        /// <param name="packet">The connection request attributes</param>
        /// <param name="user_state">The current user state if any.</param>
        void Send(byte[] packet, object user_state = null);

        int Send(byte[] buffer, int position, int length, out Error error);

        int Send(IList<ArraySegment<byte>> buffers, out Error error);

        Socket Socket { get; }

        TcpClient TcpClient { get; }

        SslStream SslStream { get; set; }

        void SetReadWriteHandlers(bool isSslStream);

        bool FinishConnect(ChannelBase channel);

        bool IsEncrypted { get; }

        void PostProxyInit();
    }
}