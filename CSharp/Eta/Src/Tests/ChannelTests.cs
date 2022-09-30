/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Collections.Generic;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

using Xunit;
using Xunit.Abstractions;
using Xunit.Categories;

using Refinitiv.Common.Logger;
using Refinitiv.Eta.Internal.Interfaces;
using Refinitiv.Eta.Tests;

namespace Refinitiv.Eta.Transports.Tests
{
    [Collection("Channel")]
    public class ChannelTests : IDisposable
    {
        static ChannelTests()
        {
            Refinitiv.Eta.Internal.ProtocolRegistry.Instance
                .Register(ConnectionType.SOCKET, new Refinitiv.Eta.Tests.MockProtocol());
        }
        public ChannelTests(ITestOutputHelper output)
        {
            XUnitLoggerProvider.Instance.Output = output;
            EtaLoggerFactory.Instance.AddProvider(XUnitLoggerProvider.Instance);
        }

        public void Dispose()
        {
            XUnitLoggerProvider.Instance.Output = null;
        }

        [Fact(Skip = "Broken")]
        [Category("Unit")]
        [Category("Channel")]
        public void BlockingChannelConnects()
        {
            var cnxnOptions = new ConnectOptions
            {
                ConnectionType = ConnectionType.SOCKET,
                Blocking = true,
            };
            cnxnOptions.UnifiedNetworkInfo.Address = "localhost";
            cnxnOptions.UnifiedNetworkInfo.ServiceName = MockProtocol.PortActionOnSignal.ToString();
            ChannelBase channel = new ChannelBase(new MockProtocol(), cnxnOptions, new MockChannel());

            // Step through the MockChannel states.
            var stimulator = Task.Factory.StartNew(() =>
            {
                Thread.Sleep(100);
                channel.SocketChannel.IsConnected = true;
                Thread.Sleep(100);
                channel.SocketChannel.IsDataReady = true;
            });

            try
            {
                // Succesful if does not throw.
                channel.Connect(out Error error);
            }
            finally
            {
                channel.Close(out Error error);
            }
        }
    }
}
