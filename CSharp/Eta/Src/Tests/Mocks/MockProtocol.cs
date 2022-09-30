/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Collections.Generic;
using System.Linq;
using System.Net.Sockets;
using System.Text;
using System.Threading;

using Refinitiv.Eta.Common;
using Refinitiv.Eta.Internal;
using Refinitiv.Eta.Internal.Interfaces;
using Refinitiv.Eta.Transports;
using Refinitiv.Eta.Transports.Interfaces;

namespace Refinitiv.Eta.Tests
{
    internal class MockProtocol : ProtocolBase
    {
        public const int PortActionAfter50ms = 14001;
        public const int PortActionOnSignal = 14002;

        /// <summary>
        /// This field induces behavior into the MockChannel
        /// specifically to test Transport.Connect GlobalLocking;
        /// This is not expected behavior on a real Protocol.
        /// </summary>
        public static bool TestMultipleConnectsFail = false;

        static ReaderWriterLockSlim _slimLock = new ReaderWriterLockSlim(LockRecursionPolicy.SupportsRecursion);
        static WriteLocker _locker = new WriteLocker(_slimLock);

        public override IChannel CreateChannel(ConnectOptions connectionOptions, out Error error)
        {
            error = null;
            try
            {
                _locker.Enter();

                var channelBase = new ChannelBase(this, connectionOptions, new MockChannel());

                if (channelBase.Connect(out error) != TransportReturnCode.SUCCESS)
                {
                    return null;
                }

                return channelBase;
            }
            catch(Exception exp)
            {
                error = new Error(
                    errorId: TransportReturnCode.FAILURE,
                    text: exp.Message,
                    exception: exp,
                    sysError: 0);
            }
            finally
            {
                _locker.Exit();
            }

            return null;
        }

        public override IChannel CreateChannel(AcceptOptions acceptOptions, IServer server, Socket socket, out Error error)
        {
            throw new NotImplementedException();
        }

        public override IServer CreateServer(BindOptions bindOptions, out Error error)
        {
            throw new NotImplementedException();
        }

        public override Pool GetPool(int poolSpec)
        {
            throw new NotImplementedException();
        }

        public override void Uninitialize(out Error error)
        {
            try
            {
                _locker.Enter();

                var activeChannels = Channels
                    .Where((c) => c.State == ChannelState.ACTIVE || c.State == ChannelState.INITIALIZING);

                foreach (var channel in activeChannels)
                {
                    channel.Close(out error);
                }

                error = null;
            }
            finally
            {
                _locker.Exit();
            }
        }
    }
}
