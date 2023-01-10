/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using Xunit;
using Xunit.Categories;
using LSEG.Eta.ValueAdd.Reactor;
using System.Net.Sockets;
using LSEG.Eta.Transports;
using System;
using System.Threading.Tasks;
using System.Threading;
using LSEG.Eta.ValueAdd.Rdm;
using static LSEG.Eta.Rdm.Directory;
using LSEG.Eta.Common;
using LSEG.Eta.Codec;
using LSEG.Eta.Rdm;
using Buffer = LSEG.Eta.Codec.Buffer;
using System.Collections.Generic;

namespace LSEG.Eta.ValuedAdd.Tests
{
    [Collection("ValueAdded")]
    public class ReactorTests
    {
        private static int port = 14002;

        [Fact]
        [Category("Unit")]
        [Category("Reactor")]
        public void CreateAndShutdownReactorTest()
        {
            ReactorOptions reactorOptions = new ReactorOptions();
            reactorOptions.UserSpecObj = this;
            Reactor reactor = Reactor.CreateReactor(reactorOptions, out ReactorErrorInfo errorInfo);

            Assert.NotNull(reactor);
            Assert.NotNull(reactor.EventSocket);
            Assert.True(ReferenceEquals(this, reactor.UserSpecObj));

            Assert.Equal(ReactorReturnCode.SUCCESS, reactor.Shutdown(out errorInfo));
            Assert.Null(errorInfo);
        }

        [Fact]
        [Category("Unit")]
        [Category("Reactor")]
        public void DispatchFromInactiveReactorTest()
        {
            ReactorOptions reactorOptions = new ReactorOptions();
            reactorOptions.UserSpecObj = this;
            Reactor reactor = Reactor.CreateReactor(reactorOptions, out ReactorErrorInfo errorInfo);

            Assert.NotNull(reactor);
            Assert.NotNull(reactor.EventSocket);
            Assert.True(ReferenceEquals(this, reactor.UserSpecObj));

            Assert.Equal(ReactorReturnCode.SUCCESS, reactor.Shutdown(out errorInfo));
            Assert.Null(errorInfo);

            Assert.Equal(ReactorReturnCode.SHUTDOWN, reactor.Dispatch(new ReactorDispatchOptions(), out errorInfo));
            Assert.Equal("Reactor is not active, aborting.", errorInfo.Error.Text);
        }

        [Fact]
        [Category("Unit")]
        [Category("Reactor")]
        public void ReactorConnectInvalidPortTest()
        {
            ReactorOptions reactorOptions = new ReactorOptions();
            reactorOptions.UserSpecObj = this;
            Reactor reactor = Reactor.CreateReactor(reactorOptions, out ReactorErrorInfo errorInfo);
            Assert.NotNull(reactor);

            Socket reactorEventFD = reactor.EventSocket;

            ReactorConnectOptions connectOptons = new ReactorConnectOptions();
            ReactorConnectInfo connectInfo = new ReactorConnectInfo();

            connectInfo.ConnectOptions.UnifiedNetworkInfo.Address = "localhost";
            connectInfo.ConnectOptions.UnifiedNetworkInfo.ServiceName = "99999";
            connectOptons.ConnectionList.Add(connectInfo);

            ClientComponentTest clientComponentTest = new ClientComponentTest();

            ConsumerRole consumerRole = new ConsumerRole()
            {
                ChannelEventCallback = clientComponentTest,
                DefaultMsgCallback = clientComponentTest
            };

            ReactorReturnCode retVal = reactor.Connect(connectOptons, consumerRole, out errorInfo);
            Assert.Equal(ReactorReturnCode.SUCCESS, retVal);

            /* Received one channel down event as the limit is set to zero by default. */
            Assert.Equal(1, clientComponentTest.NumChannelDownEvent);

            ReactorDispatchOptions dispatchOpts = new ReactorDispatchOptions();

            Assert.False(clientComponentTest.ReactorChannel.InPool);

            while (reactorEventFD.Poll(1 * 1000 * 1000, SelectMode.SelectRead) != true) ;
            Assert.True(reactorEventFD.Available > 0);
            reactor.Dispatch(dispatchOpts, out errorInfo); /* This is the CLOSE_ACK event */
            Assert.True(reactorEventFD.Available == 0);

            Assert.True(clientComponentTest.ReactorChannel.InPool); /* ReactorChannel is returned to its pool */

            Assert.Equal(ReactorReturnCode.SUCCESS, reactor.Shutdown(out errorInfo));

            Assert.Null(errorInfo);
        }

        [Fact]
        [Category("Unit")]
        [Category("Reactor")]
        public void ReactorConnectInvalidRoleTest()
        {
            ReactorOptions reactorOptions = new ReactorOptions();
            reactorOptions.UserSpecObj = this;
            Reactor reactor = Reactor.CreateReactor(reactorOptions, out ReactorErrorInfo errorInfo);
            Assert.NotNull(reactor);

            Socket reactorEventFD = reactor.EventSocket;

            ReactorConnectOptions connectOptons = new ReactorConnectOptions();
            ReactorConnectInfo connectInfo = new ReactorConnectInfo();

            connectInfo.ConnectOptions.UnifiedNetworkInfo.Address = "localhost";
            connectInfo.ConnectOptions.UnifiedNetworkInfo.ServiceName = "99999";
            connectOptons.SetReconnectMaxDelay(300);
            connectOptons.SetReconnectMinDelay(100);
            connectOptons.SetReconnectAttempLimit(3);

            connectOptons.ConnectionList.Add(connectInfo);

            ServerComponentTest serverComponentTest = new ServerComponentTest();

            ProviderRole providerRole = new ProviderRole()
            {
                ChannelEventCallback = serverComponentTest,
                DefaultMsgCallback = serverComponentTest
            };

            ReactorReturnCode retVal = reactor.Connect(connectOptons, providerRole, out errorInfo);
            Assert.Equal(ReactorReturnCode.FAILURE, retVal);
            Assert.Equal("role must be Consumer or NIProvider Role, aborting.", errorInfo.Error.Text);

            Assert.Equal(ReactorReturnCode.SUCCESS, reactor.Shutdown(out errorInfo));

            Assert.Null(errorInfo);
        }

        [Fact]
        [Category("Unit")]
        [Category("Reactor")]
        public void ReactorAcceptInvalidRoleTest()
        {
            ReactorOptions reactorOptions = new ReactorOptions();
            reactorOptions.UserSpecObj = this;
            Reactor reactor = Reactor.CreateReactor(reactorOptions, out ReactorErrorInfo errorInfo);
            Assert.NotNull(reactor);

            Socket reactorEventFD = reactor.EventSocket;

            string serviceName = "20000";

            BindOptions bindOptions = new BindOptions
            {
                ServiceName = serviceName,
                ServerBlocking = false,
                ConnectionType = ConnectionType.SOCKET,
                ProtocolType = Transports.ProtocolType.RWF,
                UserSpecObject = this,
                InterfaceName = "localhost",
                MajorVersion = Codec.Codec.MajorVersion(),
                MinorVersion = Codec.Codec.MinorVersion(),
            };

            var server = Transport.Bind(bindOptions, out Error error);

            Assert.NotNull(server);
            Assert.Equal(ChannelState.ACTIVE, server.State);
            Assert.True(server.Socket.IsBound);

            ClientComponentTest clientComponentTest = new ClientComponentTest();

            ConsumerRole consumerRole = new ConsumerRole()
            {
                ChannelEventCallback = clientComponentTest,
                DefaultMsgCallback = clientComponentTest
            };

            ReactorAcceptOptions acceptOptions = new ReactorAcceptOptions();

            ReactorReturnCode retVal = reactor.Accept(server, acceptOptions, consumerRole, out errorInfo);
            Assert.Equal(ReactorReturnCode.FAILURE, retVal);
            Assert.Equal("role must be Provider Role, aborting.", errorInfo.Error.Text);

            Assert.Equal(ReactorReturnCode.SUCCESS, reactor.Shutdown(out errorInfo));

            Assert.Null(errorInfo);
        }

        [Fact]
        [Category("Unit")]
        [Category("Reactor")]
        public void ReactorConnectInvalidPortReconnectingTest()
        {
            ReactorOptions reactorOptions = new ReactorOptions();
            reactorOptions.UserSpecObj = this;
            Reactor reactor = Reactor.CreateReactor(reactorOptions, out ReactorErrorInfo errorInfo);
            Assert.NotNull(reactor);

            Socket reactorEventFD = reactor.EventSocket;

            ReactorConnectOptions connectOptons = new ReactorConnectOptions();
            ReactorConnectInfo connectInfo = new ReactorConnectInfo();

            connectInfo.ConnectOptions.UnifiedNetworkInfo.Address = "localhost";
            connectInfo.ConnectOptions.UnifiedNetworkInfo.ServiceName = "99999";
            connectOptons.SetReconnectMaxDelay(300);
            connectOptons.SetReconnectMinDelay(100);
            connectOptons.SetReconnectAttempLimit(3);

            connectOptons.ConnectionList.Add(connectInfo);

            ClientComponentTest clientComponentTest = new ClientComponentTest();

            ConsumerRole consumerRole = new ConsumerRole()
            {
                ChannelEventCallback = clientComponentTest,
                DefaultMsgCallback = clientComponentTest
            };

            ReactorReturnCode retVal = reactor.Connect(connectOptons, consumerRole, out errorInfo);
            Assert.Equal(ReactorReturnCode.SUCCESS, retVal);
            ReactorDispatchOptions dispatchOpts = new ReactorDispatchOptions();

            Assert.NotNull(clientComponentTest.ReactorChannel);

            Assert.Equal(1, clientComponentTest.NumChannelDownReconnectingEvent);

            while (reactorEventFD.Poll(10 * 1000 * 1000, SelectMode.SelectRead) != true) ;

            Assert.True(reactorEventFD.Available > 0);
            reactor.Dispatch(dispatchOpts, out errorInfo);
            Assert.True(reactorEventFD.Available == 0);

            Assert.Equal(2, clientComponentTest.NumChannelDownReconnectingEvent);

            while (reactorEventFD.Poll(10 * 1000 * 1000, SelectMode.SelectRead) != true) ;

            Assert.True(reactorEventFD.Available > 0);
            reactor.Dispatch(dispatchOpts, out errorInfo);
            Assert.True(reactorEventFD.Available == 0);

            Assert.Equal(3, clientComponentTest.NumChannelDownReconnectingEvent);

            while (reactorEventFD.Poll(10 * 1000 * 1000, SelectMode.SelectRead) != true) ;

            Assert.True(reactorEventFD.Available > 0);
            reactor.Dispatch(dispatchOpts, out errorInfo);
            Assert.True(reactorEventFD.Available == 0);

            Assert.Equal(1, clientComponentTest.NumChannelDownEvent);

            Assert.False(clientComponentTest.ReactorChannel.InPool);

            while (reactorEventFD.Poll(10 * 1000 * 1000, SelectMode.SelectRead) != true) ;
            Assert.True(reactorEventFD.Available > 0);
            reactor.Dispatch(dispatchOpts, out errorInfo); /* This is the CLOSE_ACK event */
            Assert.True(reactorEventFD.Available == 0);

            Assert.True(clientComponentTest.ReactorChannel.InPool); /* ReactorChannel is returned to its pool */

            Assert.Equal(ReactorReturnCode.SUCCESS, reactor.Shutdown(out errorInfo));

            Assert.Null(errorInfo);
        }

        private void ReactorConnectAndAcceptChannelUp(int testOption)
        {
            ReactorOptions reactorOptions = new ReactorOptions();
            reactorOptions.UserSpecObj = this;
            Reactor reactor = Reactor.CreateReactor(reactorOptions, out ReactorErrorInfo errorInfo);
            Assert.NotNull(reactor);

            Socket reactorEventFD = reactor.EventSocket;

            string serviceName = "19999";

            BindOptions bindOptions = new BindOptions
            {
                ServiceName = serviceName,
                ServerBlocking = false,
                ConnectionType = ConnectionType.SOCKET,
                ProtocolType = Transports.ProtocolType.RWF,
                UserSpecObject = this,
                InterfaceName = "localhost",
                MajorVersion = Codec.Codec.MajorVersion(),
                MinorVersion = Codec.Codec.MinorVersion(),
            };

            if(testOption == 1|| testOption == 2)
            {
                bindOptions.MinPingTimeout = 1;
                bindOptions.PingTimeout = 2;
            }

            ClientComponentTest clientComponentTest = new ClientComponentTest();
            ServerComponentTest serverComponentTest = new ServerComponentTest();

            var server = Transport.Bind(bindOptions, out Error error);

            Assert.NotNull(server);
            Assert.Equal(ChannelState.ACTIVE, server.State);
            Assert.True(server.Socket.IsBound);

            Func<IServer, ReactorReturnCode> acceptChannel = new Func<IServer, ReactorReturnCode>((acceptServer) =>
            {
                bool pollResult = acceptServer.Socket.Poll(-1, SelectMode.SelectRead);

                Assert.True(pollResult);

                ReactorAcceptOptions acceptOptions = new ReactorAcceptOptions();

                ProviderRole providerRole = new ProviderRole()
                {
                    ChannelEventCallback = serverComponentTest,
                    DefaultMsgCallback = serverComponentTest
                };

                ReactorReturnCode retCode = reactor.Accept(acceptServer, acceptOptions, providerRole, out ReactorErrorInfo reactorErrorInfo);

                return retCode;
            });

            Func<ReactorReturnCode> connectChannel = new Func<ReactorReturnCode>(() =>
            {
                ReactorConnectOptions connectOptons = new ReactorConnectOptions();
                connectOptons.SetReconnectMaxDelay(500);
                connectOptons.SetReconnectMinDelay(300);
                connectOptons.SetReconnectAttempLimit(3);

                if (testOption == 3)
                {
                    ReactorConnectInfo connectInfo1 = new ReactorConnectInfo();
                    connectInfo1.ConnectOptions.UnifiedNetworkInfo.Address = "localhost";
                    connectInfo1.ConnectOptions.UnifiedNetworkInfo.ServiceName = "19991";
                    connectOptons.ConnectionList.Add(connectInfo1);
                }

                ReactorConnectInfo connectInfo2 = new ReactorConnectInfo();
                connectInfo2.ConnectOptions.UnifiedNetworkInfo.Address = "localhost";
                connectInfo2.ConnectOptions.UnifiedNetworkInfo.ServiceName = serviceName;
                connectOptons.ConnectionList.Add(connectInfo2);

                if(testOption == 1|| testOption == 2)
                {
                    connectInfo2.ConnectOptions.PingTimeout = 2;
                }

                ConsumerRole consumerRole = new ConsumerRole()
                {
                    ChannelEventCallback = clientComponentTest,
                    DefaultMsgCallback = clientComponentTest
                };

                ReactorReturnCode retVal = reactor.Connect(connectOptons, consumerRole, out errorInfo);
                return retVal;
            });

            ReactorReturnCode acceptRetCode = ReactorReturnCode.FAILURE;
            ReactorReturnCode connectRetCode = ReactorReturnCode.FAILURE;

            Task acceptTask = Task.Factory.StartNew(() => { acceptRetCode = acceptChannel(server); });
            Task connectTask = Task.Factory.StartNew(() => { connectRetCode = connectChannel(); });

            Task.WaitAll(new[] { acceptTask, connectTask }, 15000);

            Assert.Equal(ReactorReturnCode.SUCCESS, acceptRetCode);
            Assert.Equal(ReactorReturnCode.SUCCESS, connectRetCode);

            ReactorDispatchOptions dispatchOpts = new ReactorDispatchOptions();

            while (reactorEventFD.Poll(1 * 1000 * 1000, SelectMode.SelectRead) != true) ;

            Assert.True(reactorEventFD.Available > 0);

            do
            {
                reactor.Dispatch(dispatchOpts, out errorInfo);
            } while (reactorEventFD.Available > 0);


            Assert.Equal(1, clientComponentTest.NumChannelUpEvent);
            Assert.Equal(1, clientComponentTest.NumChannelReadyEvent);

            Assert.Equal(ChannelState.ACTIVE, clientComponentTest.ReactorChannel.Channel.State);

            bool dispatch = reactorEventFD.Poll(1 * 1000 * 1000, SelectMode.SelectRead);

            if (dispatch)
            {
                reactor.Dispatch(dispatchOpts, out errorInfo);
            }

            Assert.Equal(1, serverComponentTest.NumChannelUpEvent);
            Assert.Equal(1, serverComponentTest.NumChannelReadyEvent);

            Assert.Equal(ChannelState.ACTIVE, serverComponentTest.ReactorChannel.Channel.State);

            if(testOption == 1)
            {
                /* Closes the server channel */
                serverComponentTest.ReactorChannel.Close(out errorInfo);

                while (reactorEventFD.Poll(-1, SelectMode.SelectRead) != true) ;

                reactor.Dispatch(dispatchOpts, out errorInfo); // The CLOSE_ACK event

                reactorEventFD.Poll(7 * 1000 * 1000, SelectMode.SelectRead);

                reactor.Dispatch(dispatchOpts, out errorInfo);

                Assert.Equal(1, clientComponentTest.NumChannelDownReconnectingEvent);
            }
            else if (testOption == 2)
            {
                /* Closes the client channel */
                clientComponentTest.ReactorChannel.Close(out errorInfo);

                while (reactorEventFD.Poll(-1, SelectMode.SelectRead) != true) ;

                reactor.Dispatch(dispatchOpts, out errorInfo); // The CLOSE_ACK event

                reactorEventFD.Poll(7 * 1000 * 1000, SelectMode.SelectRead);

                reactor.Dispatch(dispatchOpts, out errorInfo);

                Assert.Equal(1, serverComponentTest.NumChannelDownEvent);
            }

            Assert.Equal(TransportReturnCode.SUCCESS, server.Close(out error));

            Assert.Equal(ReactorReturnCode.SUCCESS, reactor.Shutdown(out errorInfo));
            Assert.Null(errorInfo);

            if(testOption == 0)
            {
                Assert.Equal(1, clientComponentTest.NumChannelDownEvent);
                Assert.Equal(1, serverComponentTest.NumChannelDownEvent);
            }
            else if (testOption == 1)
            {
                Assert.Equal(1, clientComponentTest.NumChannelDownEvent);
                Assert.Equal(0, serverComponentTest.NumChannelDownEvent);
            }
            else if (testOption == 2)
            {
                Assert.Equal(0, clientComponentTest.NumChannelDownEvent);
                Assert.Equal(1, serverComponentTest.NumChannelDownEvent);
            }

            Assert.Equal(0, reactor.ChannelCount);
        }

        [Fact]
        [Category("Unit")]
        [Category("Reactor")]
        public void ReactorConnectAndAcceptChannelUpTest()
        {
            ReactorConnectAndAcceptChannelUp(0);
        }

        [Fact]
        [Category("Unit")]
        [Category("Reactor")]
        public void ReactorConnectAndAcceptChannelUp_CloseServerChannelTest()
        {
            ReactorConnectAndAcceptChannelUp(1);
        }

        [Fact]
        [Category("Unit")]
        [Category("Reactor")]
        public void ReactorConnectAndAcceptChannelUp_CloseClientChannelTest()
        {
            ReactorConnectAndAcceptChannelUp(2);
        }

        [Fact]
        [Category("Unit")]
        [Category("Reactor")]
        public void ReactorReconnectChannelUpTest()
        {
            ReactorConnectAndAcceptChannelUp(3);
        }

        [Fact]
        [Category("Unit")]
        [Category("Reactor")]
        public void ReactorPingTimeoutTest()
        {
            ReactorOptions reactorOptions = new ReactorOptions();
            reactorOptions.UserSpecObj = this;
            Reactor reactor = Reactor.CreateReactor(reactorOptions, out ReactorErrorInfo errorInfo);
            Assert.NotNull(reactor);

            PingHandler.TrackPings(true); /* Enables ping tracking */

            Socket reactorEventFD = reactor.EventSocket;

            string serviceName = "19999";

            BindOptions bindOptions = new BindOptions
            {
                ServiceName = serviceName,
                ServerBlocking = false,
                ConnectionType = ConnectionType.SOCKET,
                ProtocolType = Transports.ProtocolType.RWF,
                UserSpecObject = this,
                InterfaceName = "localhost",
                MajorVersion = Codec.Codec.MajorVersion(),
                MinorVersion = Codec.Codec.MinorVersion(),
            };

            bindOptions.MinPingTimeout = 2;
            bindOptions.PingTimeout = 4;

            ClientComponentTest clientComponentTest = new ClientComponentTest();
            ServerComponentTest serverComponentTest = new ServerComponentTest();

            var server = Transport.Bind(bindOptions, out Error error);

            Assert.NotNull(server);
            Assert.Equal(ChannelState.ACTIVE, server.State);
            Assert.True(server.Socket.IsBound);

            Func<IServer, ReactorReturnCode> acceptChannel = new Func<IServer, ReactorReturnCode>((acceptServer) =>
            {
                bool pollResult = acceptServer.Socket.Poll(-1, SelectMode.SelectRead);

                Assert.True(pollResult);

                ReactorAcceptOptions acceptOptions = new ReactorAcceptOptions();

                ProviderRole providerRole = new ProviderRole()
                {
                    ChannelEventCallback = serverComponentTest,
                    DefaultMsgCallback = serverComponentTest
                };

                ReactorReturnCode retCode = reactor.Accept(acceptServer, acceptOptions, providerRole, out ReactorErrorInfo reactorErrorInfo);

                return retCode;
            });

            Func<ReactorReturnCode> connectChannel = new Func<ReactorReturnCode>(() =>
            {
                ReactorConnectOptions connectOptons = new ReactorConnectOptions();
                connectOptons.SetReconnectMaxDelay(500);
                connectOptons.SetReconnectMinDelay(300);
                connectOptons.SetReconnectAttempLimit(3);

                ReactorConnectInfo connectInfo2 = new ReactorConnectInfo();
                connectInfo2.ConnectOptions.UnifiedNetworkInfo.Address = "localhost";
                connectInfo2.ConnectOptions.UnifiedNetworkInfo.ServiceName = serviceName;
                connectOptons.ConnectionList.Add(connectInfo2);

                connectInfo2.ConnectOptions.PingTimeout = 4;

                ConsumerRole consumerRole = new ConsumerRole()
                {
                    ChannelEventCallback = clientComponentTest,
                    DefaultMsgCallback = clientComponentTest
                };

                ReactorReturnCode retVal = reactor.Connect(connectOptons, consumerRole, out errorInfo);
                return retVal;
            });

            ReactorReturnCode acceptRetCode = ReactorReturnCode.FAILURE;
            ReactorReturnCode connectRetCode = ReactorReturnCode.FAILURE;

            Task acceptTask = Task.Factory.StartNew(() => { acceptRetCode = acceptChannel(server); });
            Task connectTask = Task.Factory.StartNew(() => { connectRetCode = connectChannel(); });

            Task.WaitAll(new[] { acceptTask, connectTask });

            Assert.Equal(ReactorReturnCode.SUCCESS, acceptRetCode);
            Assert.Equal(ReactorReturnCode.SUCCESS, connectRetCode);

            ReactorDispatchOptions dispatchOpts = new ReactorDispatchOptions();

            while (reactorEventFD.Poll(1 * 1000 * 1000, SelectMode.SelectRead) != true) ;

            Assert.True(reactorEventFD.Available > 0);

            do
            {
                reactor.Dispatch(dispatchOpts, out errorInfo);
            } while (reactorEventFD.Available > 0);


            Assert.Equal(1, clientComponentTest.NumChannelUpEvent);
            Assert.Equal(1, clientComponentTest.NumChannelReadyEvent);

            Assert.Equal(ChannelState.ACTIVE, clientComponentTest.ReactorChannel.Channel.State);

            bool dispatch = reactorEventFD.Poll(1 * 1000 * 1000, SelectMode.SelectRead);

            if (dispatch)
            {
                reactor.Dispatch(dispatchOpts, out errorInfo);
            }

            Assert.Equal(1, serverComponentTest.NumChannelUpEvent);
            Assert.Equal(1, serverComponentTest.NumChannelReadyEvent);

            Assert.Equal(ChannelState.ACTIVE, serverComponentTest.ReactorChannel.Channel.State);

            int loopCount = 10;

            do
            {
                System.Threading.Thread.Sleep(1000);
                reactor.Dispatch(dispatchOpts, out errorInfo);
                loopCount--;
            }while (loopCount > 0);

            Assert.Equal(3, serverComponentTest.ReactorChannel.GetPingHandler().GetPingReceived());
            Assert.Equal(3, serverComponentTest.ReactorChannel.GetPingHandler().GetPingSent());

            Assert.Equal(3, clientComponentTest.ReactorChannel.GetPingHandler().GetPingReceived());
            Assert.Equal(3, clientComponentTest.ReactorChannel.GetPingHandler().GetPingSent());

            Assert.Equal(TransportReturnCode.SUCCESS, server.Close(out error));

            Assert.Equal(ReactorReturnCode.SUCCESS, reactor.Shutdown(out errorInfo));
            Assert.Null(errorInfo);


            Assert.Equal(1, clientComponentTest.NumChannelDownEvent);
            Assert.Equal(1, serverComponentTest.NumChannelDownEvent);

            Assert.Equal(0, reactor.ChannelCount);
        }

        [Fact]
        [Category("Unit")]
        [Category("Reactor")]
        [Category("AdminDomain")]
        public void ReactorConsumerWithDefaultLoginAndDirectory()
        {
            ReactorErrorInfo errorInfo;
            ReactorOptions reactorOptions = new ReactorOptions();
            reactorOptions.UserSpecObj = this;
            Reactor consumerReactor = Reactor.CreateReactor(reactorOptions, out errorInfo);
            Reactor providerReactor = Reactor.CreateReactor(reactorOptions, out errorInfo);

            Socket consumerReactorEventFD = consumerReactor.EventSocket;
            Socket providerReactorEventFD = providerReactor.EventSocket;

            string serviceName = port.ToString();
            port++;

            BindOptions bindOptions = new BindOptions
            {
                ServiceName = serviceName,
                ServerBlocking = false,
                ConnectionType = ConnectionType.SOCKET,
                ProtocolType = Transports.ProtocolType.RWF,
                UserSpecObject = this,
                InterfaceName = "localhost",
                MajorVersion = Codec.Codec.MajorVersion(),
                MinorVersion = Codec.Codec.MinorVersion(),
                MinPingTimeout = 1,
                PingTimeout = 255
            };

            var server = Transport.Bind(bindOptions, out Error error);

            Assert.NotNull(server);
            Assert.Equal(ChannelState.ACTIVE, server.State);
            Assert.True(server.Socket.IsBound);

            ClientComponentTest clientComponentTest = new ClientComponentTest();
            ServerComponentTest serverComponentTest = new ServerComponentTest();

            ReactorReturnCode acceptRetCode = ReactorReturnCode.FAILURE;
            ReactorReturnCode connectRetCode = ReactorReturnCode.FAILURE;

            ConsumerRole consumerRole = CreateConsumerRole(clientComponentTest, true, true, false);
            ReactorConnectOptions connectOptions = CreateDefaultReactorConnectOptions(serviceName, 3);

            Task acceptTask = Task.Factory.StartNew(() =>
            {  
                acceptRetCode = ServerAccept(providerReactor, serverComponentTest)(server); 
            });

            Task connectTask = Task.Factory.StartNew(() =>
            {
                connectRetCode = ClientConnect(consumerReactor, consumerRole, connectOptions)();
            });

            Task.WaitAll(new[] { acceptTask, connectTask });

            Assert.Equal(ReactorReturnCode.SUCCESS, acceptRetCode);
            Assert.Equal(ReactorReturnCode.SUCCESS, connectRetCode);

            ReactorDispatchOptions dispatchOpts = new ReactorDispatchOptions();

            DispatchReactorEvent(providerReactorEventFD, providerReactor, dispatchOpts, 1 * 1000 * 1000);

            Assert.Equal(1, serverComponentTest.NumChannelUpEvent);
            Assert.Equal(1, serverComponentTest.NumChannelReadyEvent);
            Assert.Equal(ChannelState.ACTIVE, serverComponentTest.ReactorChannel.Channel.State);

            DispatchReactorEvent(consumerReactorEventFD, consumerReactor, dispatchOpts, 1 * 1000 * 1000);

            Assert.Equal(1, clientComponentTest.NumChannelUpEvent);
            Assert.Equal(ChannelState.ACTIVE, clientComponentTest.ReactorChannel.Channel.State);

            Dispatch(providerReactor, 400);

            Assert.Equal(1, serverComponentTest.NumLoginMsgEvent);

            Dispatch(consumerReactor, 400);

            Assert.Equal(1, clientComponentTest.NumLoginMsgEvent);

            Dispatch(providerReactor, 400);

            Assert.Equal(1, serverComponentTest.NumDirectoryMsgEvent);

            Dispatch(consumerReactor, 400);

            Assert.Equal(1, clientComponentTest.NumDirectoryMsgEvent);
            Assert.Equal(1, clientComponentTest.NumChannelReadyEvent);

            Assert.Equal(ReactorReturnCode.SUCCESS, consumerReactor.Shutdown(out errorInfo));
            Assert.Equal(ReactorReturnCode.SUCCESS, providerReactor.Shutdown(out errorInfo));
        }

        [Fact]
        [Category("Unit")]
        [Category("Reactor")]
        [Category("AdminDomain")]
        public void ReactorConsumerWithDefaultLoginAndDirectoryRaiseEvent()
        {
            ReactorErrorInfo errorInfo;
            ReactorOptions reactorOptions = new ReactorOptions();
            reactorOptions.UserSpecObj = this;
            Reactor consumerReactor = Reactor.CreateReactor(reactorOptions, out errorInfo);
            Reactor providerReactor = Reactor.CreateReactor(reactorOptions, out errorInfo);

            Socket consumerReactorEventFD = consumerReactor.EventSocket;
            Socket providerReactorEventFD = providerReactor.EventSocket;

            string serviceName = port.ToString();
            port++;

            BindOptions bindOptions = new BindOptions
            {
                ServiceName = serviceName,
                ServerBlocking = false,
                ConnectionType = ConnectionType.SOCKET,
                ProtocolType = Transports.ProtocolType.RWF,
                UserSpecObject = this,
                InterfaceName = "localhost",
                MajorVersion = Codec.Codec.MajorVersion(),
                MinorVersion = Codec.Codec.MinorVersion(),
                MinPingTimeout = 1,
                PingTimeout = 255
            };

            var server = Transport.Bind(bindOptions, out Error error);

            Assert.NotNull(server);
            Assert.Equal(ChannelState.ACTIVE, server.State);
            Assert.True(server.Socket.IsBound);

            ClientComponentTest clientComponentTest = new ClientComponentTest();
            ServerComponentTest serverComponentTest = new ServerComponentTest();

            clientComponentTest.MsgReturnCode = ReactorCallbackReturnCode.RAISE;
            serverComponentTest.MsgReturnCode = ReactorCallbackReturnCode.RAISE;

            ReactorReturnCode acceptRetCode = ReactorReturnCode.FAILURE;
            ReactorReturnCode connectRetCode = ReactorReturnCode.FAILURE;

            ConsumerRole consumerRole = CreateConsumerRole(clientComponentTest, true, true, false);
            ReactorConnectOptions connectOptions = CreateDefaultReactorConnectOptions(serviceName, 3);

            Task acceptTask = Task.Factory.StartNew(() =>
            {
                acceptRetCode = ServerAccept(providerReactor, serverComponentTest)(server);
            });

            Task connectTask = Task.Factory.StartNew(() =>
            {
                connectRetCode = ClientConnect(consumerReactor, consumerRole, connectOptions)();
            });

            Task.WaitAll(new[] { acceptTask, connectTask });

            Assert.Equal(ReactorReturnCode.SUCCESS, acceptRetCode);
            Assert.Equal(ReactorReturnCode.SUCCESS, connectRetCode);

            ReactorDispatchOptions dispatchOpts = new ReactorDispatchOptions();
            
            DispatchReactorEvent(providerReactorEventFD, providerReactor, dispatchOpts, 1 * 1000 * 1000);

            Assert.Equal(1, serverComponentTest.NumChannelUpEvent);
            Assert.Equal(1, serverComponentTest.NumChannelReadyEvent);
            Assert.Equal(ChannelState.ACTIVE, serverComponentTest.ReactorChannel.Channel.State);

            DispatchReactorEvent(consumerReactorEventFD, consumerReactor, dispatchOpts, 1 * 1000 * 1000);

            Assert.Equal(1, clientComponentTest.NumChannelUpEvent);
            Assert.Equal(0, clientComponentTest.NumChannelReadyEvent);
            Assert.Equal(ChannelState.ACTIVE, clientComponentTest.ReactorChannel.Channel.State);

            Dispatch(providerReactor, 400);

            Assert.Equal(1, serverComponentTest.NumLoginMsgEvent);
            Assert.Equal(1, serverComponentTest.NumDefaultMsgEvent);

            Dispatch(consumerReactor, 400);

            Assert.Equal(1, clientComponentTest.NumLoginMsgEvent);
            Assert.Equal(1, clientComponentTest.NumDefaultMsgEvent);

            Dispatch(providerReactor, 400);

            Assert.Equal(1, serverComponentTest.NumDirectoryMsgEvent);
            Assert.Equal(2, serverComponentTest.NumDefaultMsgEvent);

            Dispatch(consumerReactor, 1000);

            Assert.Equal(1, clientComponentTest.NumDirectoryMsgEvent);
            Assert.Equal(2, clientComponentTest.NumDefaultMsgEvent);
            Assert.Equal(1, clientComponentTest.NumChannelReadyEvent);

            Assert.Equal(ReactorReturnCode.SUCCESS, consumerReactor.Shutdown(out errorInfo));
            Assert.Equal(ReactorReturnCode.SUCCESS, providerReactor.Shutdown(out errorInfo));
        }

        [Fact]
        [Category("Unit")]
        [Category("Reactor")]
        [Category("AdminDomain")]
        public void ReactorConsumerWithDefaultLoginDirectoryDictionary()
        {
            ReactorErrorInfo errorInfo;
            ReactorOptions reactorOptions = new ReactorOptions();
            reactorOptions.UserSpecObj = this;
            Reactor consumerReactor = Reactor.CreateReactor(reactorOptions, out errorInfo);
            Reactor providerReactor = Reactor.CreateReactor(reactorOptions, out errorInfo);
           
            Socket consumerReactorEventFD = consumerReactor.EventSocket;
            Socket providerReactorEventFD = providerReactor.EventSocket;

            string serviceName = port.ToString();
            port++;

            BindOptions bindOptions = new BindOptions
            {
                ServiceName = serviceName,
                ServerBlocking = false,
                ConnectionType = ConnectionType.SOCKET,
                ProtocolType = Transports.ProtocolType.RWF,
                UserSpecObject = this,
                InterfaceName = "localhost",
                MajorVersion = Codec.Codec.MajorVersion(),
                MinorVersion = Codec.Codec.MinorVersion(),
                MinPingTimeout = 1,
                PingTimeout = 255
            };

            var server = Transport.Bind(bindOptions, out Error error);

            Assert.NotNull(server);
            Assert.Equal(ChannelState.ACTIVE, server.State);
            Assert.True(server.Socket.IsBound);

            ClientComponentTest clientComponentTest = new ClientComponentTest();
            ServerComponentTest serverComponentTest = new ServerComponentTest();

            ReactorReturnCode acceptRetCode = ReactorReturnCode.FAILURE;
            ReactorReturnCode connectRetCode = ReactorReturnCode.FAILURE;

            ConsumerRole consumerRole = CreateConsumerRole(clientComponentTest, true, true, true);
            ReactorConnectOptions connectOptions = CreateDefaultReactorConnectOptions(serviceName, 3);

            Task acceptTask = Task.Factory.StartNew(() =>
            {
                acceptRetCode = ServerAccept(providerReactor, serverComponentTest)(server);
            });

            Task connectTask = Task.Factory.StartNew(() =>
            {
                connectRetCode = ClientConnect(consumerReactor, consumerRole, connectOptions)();
            });

            Task.WaitAll(new[] { acceptTask, connectTask });

            Assert.Equal(ReactorReturnCode.SUCCESS, acceptRetCode);
            Assert.Equal(ReactorReturnCode.SUCCESS, connectRetCode);

            ReactorDispatchOptions dispatchOpts = new ReactorDispatchOptions();

            DispatchReactorEvent(providerReactorEventFD, providerReactor, dispatchOpts, 1 * 1000 * 1000);

            Assert.Equal(1, serverComponentTest.NumChannelUpEvent);
            Assert.Equal(1, serverComponentTest.NumChannelReadyEvent);
            Assert.Equal(ChannelState.ACTIVE, serverComponentTest.ReactorChannel.Channel.State);

            DispatchReactorEvent(consumerReactorEventFD, consumerReactor, dispatchOpts, 1 * 1000 * 1000);

            Assert.Equal(1, clientComponentTest.NumChannelUpEvent);
            Assert.Equal(ChannelState.ACTIVE, clientComponentTest.ReactorChannel.Channel.State);

            Dispatch(providerReactor, 400);

            Assert.Equal(1, serverComponentTest.NumLoginMsgEvent);

            Dispatch(consumerReactor, 400);

            Assert.Equal(1, clientComponentTest.NumLoginMsgEvent);

            Dispatch(providerReactor, 400);

            Assert.Equal(1, serverComponentTest.NumDirectoryMsgEvent);

            Dispatch(consumerReactor, 400);

            Assert.Equal(1, clientComponentTest.NumDirectoryMsgEvent);
  
            Dispatch(providerReactor, 400);

            Assert.Equal(2, serverComponentTest.NumDictionaryMsgEvent);

            Dispatch(consumerReactor, 400);

            Assert.Equal(2, clientComponentTest.NumDictionaryMsgEvent);
            Assert.Equal(1, clientComponentTest.NumChannelReadyEvent);

            Dispatch(providerReactor, 400);

            Assert.Equal(4, serverComponentTest.NumDictionaryMsgEvent);

            Assert.Equal(ReactorReturnCode.SUCCESS, consumerReactor.Shutdown(out errorInfo));
        }

        [Fact]
        [Category("Unit")]
        [Category("Reactor")]
        [Category("AdminDomain")]
        public void ReactorConsumerWithCustomLoginAndDirectory()
        {
            ReactorErrorInfo errorInfo;
            ReactorOptions reactorOptions = new ReactorOptions();
            reactorOptions.UserSpecObj = this;
            Reactor consumerReactor = Reactor.CreateReactor(reactorOptions, out errorInfo);
            Reactor providerReactor = Reactor.CreateReactor(reactorOptions, out errorInfo);

            Socket consumerReactorEventFD = consumerReactor.EventSocket;
            Socket providerReactorEventFD = providerReactor.EventSocket;

            string serviceName = port.ToString();
            port++;

            BindOptions bindOptions = new BindOptions
            {
                ServiceName = serviceName,
                ServerBlocking = false,
                ConnectionType = ConnectionType.SOCKET,
                ProtocolType = Transports.ProtocolType.RWF,
                UserSpecObject = this,
                InterfaceName = "localhost",
                MajorVersion = Codec.Codec.MajorVersion(),
                MinorVersion = Codec.Codec.MinorVersion(),
                MinPingTimeout = 1,
                PingTimeout = 255
            };

            var server = Transport.Bind(bindOptions, out Error error);

            Assert.NotNull(server);
            Assert.Equal(ChannelState.ACTIVE, server.State);
            Assert.True(server.Socket.IsBound);

            ClientComponentTest clientComponentTest = new ClientComponentTest();
            ServerComponentTest serverComponentTest = new ServerComponentTest();

            ReactorReturnCode acceptRetCode = ReactorReturnCode.FAILURE;
            ReactorReturnCode connectRetCode = ReactorReturnCode.FAILURE;

            clientComponentTest.InitTestLoginRequest(1, "UnitTest", "12345", "UnitTestUser", false);
            clientComponentTest.InitDefaultTestDirectoryRequest(-1, ServiceFilterFlags.INFO | ServiceFilterFlags.STATE);
            
            ConsumerRole consumerRole = CreateConsumerRole(clientComponentTest, false, false, false);
            ReactorConnectOptions connectOptions = CreateDefaultReactorConnectOptions(serviceName, 3);

            Task acceptTask = Task.Factory.StartNew(() =>
            {
                acceptRetCode = ServerAccept(providerReactor, serverComponentTest)(server);
            });

            Task connectTask = Task.Factory.StartNew(() =>
            {
                connectRetCode = ClientConnect(consumerReactor, consumerRole, connectOptions)();
            });

            Task.WaitAll(new[] { acceptTask, connectTask });

            Assert.Equal(ReactorReturnCode.SUCCESS, acceptRetCode);
            Assert.Equal(ReactorReturnCode.SUCCESS, connectRetCode);

            ReactorDispatchOptions dispatchOpts = new ReactorDispatchOptions();

            DispatchReactorEvent(providerReactorEventFD, providerReactor, dispatchOpts, 1 * 1000 * 1000);

            Assert.Equal(1, serverComponentTest.NumChannelUpEvent);
            Assert.Equal(1, serverComponentTest.NumChannelReadyEvent);
            Assert.Equal(ChannelState.ACTIVE, serverComponentTest.ReactorChannel.Channel.State);

            DispatchReactorEvent(consumerReactorEventFD, consumerReactor, dispatchOpts, 1 * 1000 * 1000);

            Assert.Equal(1, clientComponentTest.NumChannelUpEvent);
            Assert.Equal(ChannelState.ACTIVE, clientComponentTest.ReactorChannel.Channel.State);

            serverComponentTest.CheckLoginMsg = msg =>
            {
                Assert.Equal(LoginMsgType.REQUEST, msg.LoginMsgType);
                LoginRequest request = msg.LoginRequest;
                Assert.Equal("UnitTestUser", request.UserName.ToString());
                Assert.Equal("UnitTest", request.LoginAttrib.ApplicationName.ToString());
                Assert.Equal("12345", request.LoginAttrib.ApplicationId.ToString());
            };

            serverComponentTest.CheckDirectoryMsg = msg =>
            {
                Assert.Equal(DirectoryMsgType.REQUEST, msg.DirectoryMsgType);
                DirectoryRequest request = msg.DirectoryRequest;
                Assert.Equal(-1, request.StreamId);
                Assert.Equal(ServiceFilterFlags.INFO | ServiceFilterFlags.STATE, request.Filter);
            };

            Dispatch(providerReactor, 400);

            Assert.Equal(1, serverComponentTest.NumLoginMsgEvent);

            Dispatch(consumerReactor, 400);

            Assert.Equal(1, clientComponentTest.NumLoginMsgEvent);

            Dispatch(providerReactor, 400);

            Assert.Equal(1, serverComponentTest.NumDirectoryMsgEvent);

            Dispatch(consumerReactor, 400);

            Assert.Equal(1, clientComponentTest.NumDirectoryMsgEvent);
            Assert.Equal(1, clientComponentTest.NumChannelReadyEvent);

            Assert.Equal(ReactorReturnCode.SUCCESS, consumerReactor.Shutdown(out errorInfo));
            Assert.Equal(ReactorReturnCode.SUCCESS, providerReactor.Shutdown(out errorInfo));
        }

        [Fact]
        [Category("Unit")]
        [Category("Reactor")]
        [Category("AdminDomain")]
        public void ReactorInitAdminDomainsAndSendItemRefresh()
        {
            ReactorErrorInfo errorInfo;
            ReactorOptions reactorOptions = new ReactorOptions();
            reactorOptions.UserSpecObj = this;
            Reactor consumerReactor = Reactor.CreateReactor(reactorOptions, out errorInfo);
            Reactor providerReactor = Reactor.CreateReactor(reactorOptions, out errorInfo);

            Socket consumerReactorEventFD = consumerReactor.EventSocket;
            Socket providerReactorEventFD = providerReactor.EventSocket;

            string serviceName = port.ToString();
            port++;

            BindOptions bindOptions = new BindOptions
            {
                ServiceName = serviceName,
                ServerBlocking = false,
                ConnectionType = ConnectionType.SOCKET,
                ProtocolType = Transports.ProtocolType.RWF,
                UserSpecObject = this,
                InterfaceName = "localhost",
                MajorVersion = Codec.Codec.MajorVersion(),
                MinorVersion = Codec.Codec.MinorVersion(),
                MinPingTimeout = 1,
                PingTimeout = 255
            };

            var server = Transport.Bind(bindOptions, out Error error);

            Assert.NotNull(server);
            Assert.Equal(ChannelState.ACTIVE, server.State);
            Assert.True(server.Socket.IsBound);

            ClientComponentTest clientComponentTest = new ClientComponentTest();
            ServerComponentTest serverComponentTest = new ServerComponentTest();

            clientComponentTest.OnDefaultMsgReceived = eventMsg =>
            {
                var msg = eventMsg.Msg;
                if (msg.DomainType == (int)DomainType.MARKET_PRICE && msg.MsgClass == MsgClasses.REFRESH)
                {
                    clientComponentTest.NumMarketPriceRefresh++;
                }
            };

            ReactorReturnCode acceptRetCode = ReactorReturnCode.FAILURE;
            ReactorReturnCode connectRetCode = ReactorReturnCode.FAILURE;

            ConsumerRole consumerRole = CreateConsumerRole(clientComponentTest, true, true, false);
            ReactorConnectOptions connectOptions = CreateDefaultReactorConnectOptions(serviceName, 3);

            Task acceptTask = Task.Factory.StartNew(() =>
            {
                acceptRetCode = ServerAccept(providerReactor, serverComponentTest)(server);
            });

            Task connectTask = Task.Factory.StartNew(() =>
            {
                connectRetCode = ClientConnect(consumerReactor, consumerRole, connectOptions)();
            });

            Task.WaitAll(new[] { acceptTask, connectTask });

            Assert.Equal(ReactorReturnCode.SUCCESS, acceptRetCode);
            Assert.Equal(ReactorReturnCode.SUCCESS, connectRetCode);

            ReactorDispatchOptions dispatchOpts = new ReactorDispatchOptions();

            DispatchReactorEvent(providerReactorEventFD, providerReactor, dispatchOpts, 1 * 1000 * 1000);

            Assert.Equal(1, serverComponentTest.NumChannelUpEvent);
            Assert.Equal(1, serverComponentTest.NumChannelReadyEvent);
            Assert.Equal(ChannelState.ACTIVE, serverComponentTest.ReactorChannel.Channel.State);

            DispatchReactorEvent(consumerReactorEventFD, consumerReactor, dispatchOpts, 1 * 1000 * 1000);

            Assert.Equal(1, clientComponentTest.NumChannelUpEvent);
            Assert.Equal(ChannelState.ACTIVE, clientComponentTest.ReactorChannel.Channel.State);

            Dispatch(providerReactor, 400);

            Assert.Equal(1, serverComponentTest.NumLoginMsgEvent);

            Dispatch(consumerReactor, 400);

            Assert.Equal(1, clientComponentTest.NumLoginMsgEvent);

            Dispatch(providerReactor, 400);

            Assert.Equal(1, serverComponentTest.NumDirectoryMsgEvent);

            Dispatch(consumerReactor, 400);

            Assert.Equal(1, clientComponentTest.NumDirectoryMsgEvent);
            Assert.Equal(1, clientComponentTest.NumChannelReadyEvent);

            clientComponentTest.InitDefaultTestMarketPriceRequest(0, 6, null);
            consumerReactor.SubmitChannel(clientComponentTest.ReactorChannel, clientComponentTest.MarketPriceRequestMsg, new ReactorSubmitOptions(), out ReactorErrorInfo errorInfo1);

            DispatchReactorEvent(consumerReactorEventFD, consumerReactor, dispatchOpts, 1 * 1000 * 1000); // read FLUSH_DONE
            Dispatch(providerReactor, 400);

            Assert.Equal(1, serverComponentTest.NumDefaultMsgEvent);

            Dispatch(consumerReactor, 700);

            Assert.Equal(1, clientComponentTest.NumDefaultMsgEvent);
            Assert.Equal(1, clientComponentTest.NumMarketPriceRefresh);

            Assert.Equal(ReactorReturnCode.SUCCESS, consumerReactor.Shutdown(out errorInfo));
            Assert.Equal(ReactorReturnCode.SUCCESS, providerReactor.Shutdown(out errorInfo));
        }

        [Fact]
        [Category("Unit")]
        [Category("Reactor")]
        [Category("AdminDomain")]
        public void ReactorConsumerDispatchWithChannelDown()
        {
            ReactorErrorInfo errorInfo;
            ReactorOptions reactorOptions = new ReactorOptions();
            reactorOptions.UserSpecObj = this;
            Reactor consumerReactor = Reactor.CreateReactor(reactorOptions, out errorInfo);
            Reactor providerReactor = Reactor.CreateReactor(reactorOptions, out errorInfo);

            Socket consumerReactorEventFD = consumerReactor.EventSocket;
            Socket providerReactorEventFD = providerReactor.EventSocket;

            string serviceName = port.ToString();
            port++;

            BindOptions bindOptions = new BindOptions
            {
                ServiceName = serviceName,
                ServerBlocking = false,
                ConnectionType = ConnectionType.SOCKET,
                ProtocolType = Transports.ProtocolType.RWF,
                UserSpecObject = this,
                InterfaceName = "localhost",
                MajorVersion = Codec.Codec.MajorVersion(),
                MinorVersion = Codec.Codec.MinorVersion(),
                MinPingTimeout = 1,
                PingTimeout = 255
            };

            var server = Transport.Bind(bindOptions, out Error error);

            Assert.NotNull(server);
            Assert.Equal(ChannelState.ACTIVE, server.State);
            Assert.True(server.Socket.IsBound);

            ClientComponentTest clientComponentTest = new ClientComponentTest();
            ServerComponentTest serverComponentTest = new ServerComponentTest();

            ReactorReturnCode acceptRetCode = ReactorReturnCode.FAILURE;
            ReactorReturnCode connectRetCode = ReactorReturnCode.FAILURE;

            ConsumerRole consumerRole = CreateConsumerRole(clientComponentTest, true, true, false);
            ReactorConnectOptions connectOptions = CreateDefaultReactorConnectOptions(serviceName, 0);

            Task acceptTask = Task.Factory.StartNew(() =>
            {
                acceptRetCode = ServerAccept(providerReactor, serverComponentTest)(server);
            });

            Task connectTask = Task.Factory.StartNew(() =>
            {
                connectRetCode = ClientConnect(consumerReactor, consumerRole, connectOptions)();
            });

            Task.WaitAll(new[] { acceptTask, connectTask });

            Assert.Equal(ReactorReturnCode.SUCCESS, acceptRetCode);
            Assert.Equal(ReactorReturnCode.SUCCESS, connectRetCode);

            ReactorDispatchOptions dispatchOpts = new ReactorDispatchOptions();

            DispatchReactorEvent(providerReactorEventFD, providerReactor, dispatchOpts, 1 * 1000 * 1000);

            Assert.Equal(1, serverComponentTest.NumChannelUpEvent);
            Assert.Equal(1, serverComponentTest.NumChannelReadyEvent);
            Assert.Equal(ChannelState.ACTIVE, serverComponentTest.ReactorChannel.Channel.State);

            DispatchReactorEvent(consumerReactorEventFD, consumerReactor, dispatchOpts, 1 * 1000 * 1000);

            Assert.Equal(1, clientComponentTest.NumChannelUpEvent);
            Assert.Equal(ChannelState.ACTIVE, clientComponentTest.ReactorChannel.Channel.State);

            Dispatch(providerReactor, 400);

            Assert.Equal(1, serverComponentTest.NumLoginMsgEvent);

            IChannel channel = clientComponentTest.ReactorChannel.Channel;
            clientComponentTest.ReactorChannel.SetChannel(new ReadNullChannel(channel));

            Dispatch(consumerReactor, 400);

            Assert.Equal(1, clientComponentTest.NumChannelDownEvent);

            Assert.Equal(ReactorReturnCode.SUCCESS, providerReactor.Shutdown(out errorInfo));
        }

        [Fact]
        [Category("Unit")]
        [Category("Reactor")]
        [Category("AdminDomain")]
        public void ReactorSubmitWithCodecNoBuffers()
        {
            ReactorErrorInfo errorInfo;
            ReactorOptions reactorOptions = new ReactorOptions();
            reactorOptions.UserSpecObj = this;
            Reactor consumerReactor = Reactor.CreateReactor(reactorOptions, out errorInfo);
            Reactor providerReactor = Reactor.CreateReactor(reactorOptions, out errorInfo);

            Socket consumerReactorEventFD = consumerReactor.EventSocket;
            Socket providerReactorEventFD = providerReactor.EventSocket;

            string serviceName = port.ToString();
            port++;

            BindOptions bindOptions = new BindOptions
            {
                ServiceName = serviceName,
                ServerBlocking = false,
                ConnectionType = ConnectionType.SOCKET,
                ProtocolType = Transports.ProtocolType.RWF,
                UserSpecObject = this,
                InterfaceName = "localhost",
                MajorVersion = Codec.Codec.MajorVersion(),
                MinorVersion = Codec.Codec.MinorVersion(),
                MinPingTimeout = 1,
                PingTimeout = 255
            };

            var server = Transport.Bind(bindOptions, out Error error);

            Assert.NotNull(server);
            Assert.Equal(ChannelState.ACTIVE, server.State);
            Assert.True(server.Socket.IsBound);

            ClientComponentTest clientComponentTest = new ClientComponentTest();
            ServerComponentTest serverComponentTest = new ServerComponentTest();

            ReactorReturnCode acceptRetCode = ReactorReturnCode.FAILURE;
            ReactorReturnCode connectRetCode = ReactorReturnCode.FAILURE;

            ConsumerRole consumerRole = CreateConsumerRole(clientComponentTest, false, false, false);
            ReactorConnectOptions connectOptions = CreateDefaultReactorConnectOptions(serviceName, 0);

            Assert.Null(consumerRole.RdmLoginRequest);
            Assert.Null(consumerRole.RdmDirectoryRequest);

            Task acceptTask = Task.Factory.StartNew(() =>
            {
                acceptRetCode = ServerAccept(providerReactor, serverComponentTest)(server);
            });

            Task connectTask = Task.Factory.StartNew(() =>
            {
                connectRetCode = ClientConnect(consumerReactor, consumerRole, connectOptions)();
            });

            Task.WaitAll(new[] { acceptTask, connectTask });

            Assert.Equal(ReactorReturnCode.SUCCESS, acceptRetCode);
            Assert.Equal(ReactorReturnCode.SUCCESS, connectRetCode);

            ReactorDispatchOptions dispatchOpts = new ReactorDispatchOptions();

            DispatchReactorEvent(providerReactorEventFD, providerReactor, dispatchOpts, 1 * 1000 * 1000);

            Assert.Equal(1, serverComponentTest.NumChannelUpEvent);
            Assert.Equal(1, serverComponentTest.NumChannelReadyEvent);
            Assert.Equal(ChannelState.ACTIVE, serverComponentTest.ReactorChannel.Channel.State);

            DispatchReactorEvent(consumerReactorEventFD, consumerReactor, dispatchOpts, 1 * 1000 * 1000);

            Assert.Equal(1, clientComponentTest.NumChannelUpEvent);
            Assert.Equal(1, clientComponentTest.NumChannelReadyEvent);
            Assert.Equal(ChannelState.ACTIVE, clientComponentTest.ReactorChannel.Channel.State);

            IChannel channel = clientComponentTest.ReactorChannel.Channel;
            WriteFlushFailChannel writeFlushFailChannel = new WriteFlushFailChannel(channel);

            Msg msg = new Msg();

            msg.MsgClass = MsgClasses.REQUEST;
            msg.StreamId = 1;
            msg.DomainType = (int)DomainType.LOGIN;
            msg.ContainerType = Codec.DataTypes.ELEMENT_LIST;

            msg.MsgKey.ApplyHasNameType();
            msg.MsgKey.ApplyHasName();
            msg.MsgKey.ApplyHasIdentifier();
            msg.MsgKey.ApplyHasAttrib();

            msg.MsgKey.Name.Data("TRI.N");
            msg.MsgKey.NameType = InstrumentNameTypes.RIC;
            msg.MsgKey.Identifier = 0x7fff;

            Buffer encodedAttrib = new Buffer();
            encodedAttrib.Data("ENCODED ATTRIB");
            msg.MsgKey.AttribContainerType = Codec.DataTypes.OPAQUE;
            msg.MsgKey.EncodedAttrib = encodedAttrib;

            ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
            Assert.Equal(ReactorReturnCode.SUCCESS, clientComponentTest.ReactorChannel.Submit(msg, submitOptions, out errorInfo));

            Dispatch(providerReactor, 400);

            Assert.Equal(1, serverComponentTest.NumLoginMsgEvent);
            clientComponentTest.ReactorChannel.SetChannel(writeFlushFailChannel);

            writeFlushFailChannel.writeRetVal = TransportReturnCode.SUCCESS;
            writeFlushFailChannel.flushRetVal = TransportReturnCode.SUCCESS;
            writeFlushFailChannel.noBuffers = true;

            Assert.Equal(ReactorReturnCode.NO_BUFFERS, clientComponentTest.ReactorChannel.Submit(msg, submitOptions, out errorInfo));
            
            Dispatch(consumerReactor, 400); // Read FLUSH_DONE event internally

            Thread.Sleep(1000);
            Assert.Equal(0, writeFlushFailChannel.releaseBufferCount);
            Assert.Equal(1, writeFlushFailChannel.flushCount);

            Assert.Equal(ReactorReturnCode.SUCCESS, providerReactor.Shutdown(out errorInfo));
            Assert.Equal(ReactorReturnCode.SUCCESS, consumerReactor.Shutdown(out errorInfo));
        }

        [Fact]
        [Category("Unit")]
        [Category("Reactor")]
        [Category("AdminDomain")]
        public void ReactorSubmitRdmMsgResizeBuffer()
        {
            ReactorErrorInfo errorInfo;
            ReactorOptions reactorOptions = new ReactorOptions();
            reactorOptions.UserSpecObj = this;
            Reactor consumerReactor = Reactor.CreateReactor(reactorOptions, out errorInfo);
            Reactor providerReactor = Reactor.CreateReactor(reactorOptions, out errorInfo);

            Socket consumerReactorEventFD = consumerReactor.EventSocket;
            Socket providerReactorEventFD = providerReactor.EventSocket;

            string serviceName = port.ToString();
            port++;

            BindOptions bindOptions = new BindOptions
            {
                ServiceName = serviceName,
                ServerBlocking = false,
                ConnectionType = ConnectionType.SOCKET,
                ProtocolType = Transports.ProtocolType.RWF,
                UserSpecObject = this,
                InterfaceName = "localhost",
                MajorVersion = Codec.Codec.MajorVersion(),
                MinorVersion = Codec.Codec.MinorVersion(),
                MinPingTimeout = 1,
                PingTimeout = 255
            };

            var server = Transport.Bind(bindOptions, out Error error);

            Assert.NotNull(server);
            Assert.Equal(ChannelState.ACTIVE, server.State);
            Assert.True(server.Socket.IsBound);

            ClientComponentTest clientComponentTest = new ClientComponentTest();
            ServerComponentTest serverComponentTest = new ServerComponentTest();

            ReactorReturnCode acceptRetCode = ReactorReturnCode.FAILURE;
            ReactorReturnCode connectRetCode = ReactorReturnCode.FAILURE;

            ConsumerRole consumerRole = CreateConsumerRole(clientComponentTest, false, false, false);
            ReactorConnectOptions connectOptions = CreateDefaultReactorConnectOptions(serviceName, 0);

            Assert.Null(consumerRole.RdmLoginRequest);
            Assert.Null(consumerRole.RdmDirectoryRequest);

            Task acceptTask = Task.Factory.StartNew(() =>
            {
                acceptRetCode = ServerAccept(providerReactor, serverComponentTest)(server);
            });

            Task connectTask = Task.Factory.StartNew(() =>
            {
                connectRetCode = ClientConnect(consumerReactor, consumerRole, connectOptions)();
            });

            Task.WaitAll(new[] { acceptTask, connectTask });

            Assert.Equal(ReactorReturnCode.SUCCESS, acceptRetCode);
            Assert.Equal(ReactorReturnCode.SUCCESS, connectRetCode);

            ReactorDispatchOptions dispatchOpts = new ReactorDispatchOptions();

            DispatchReactorEvent(providerReactorEventFD, providerReactor, dispatchOpts, 1 * 1000 * 1000);

            Assert.Equal(1, serverComponentTest.NumChannelUpEvent);
            Assert.Equal(1, serverComponentTest.NumChannelReadyEvent);
            Assert.Equal(ChannelState.ACTIVE, serverComponentTest.ReactorChannel.Channel.State);

            DispatchReactorEvent(consumerReactorEventFD, consumerReactor, dispatchOpts, 1 * 1000 * 1000);

            Assert.Equal(1, clientComponentTest.NumChannelUpEvent);
            Assert.Equal(1, clientComponentTest.NumChannelReadyEvent);
            Assert.Equal(ChannelState.ACTIVE, clientComponentTest.ReactorChannel.Channel.State);

            IChannel channel = clientComponentTest.ReactorChannel.Channel;
            WriteFlushFailChannel writeFlushFailChannel = new WriteFlushFailChannel(channel);

            LoginRequest loginRequest = new LoginRequest();
            loginRequest.InitDefaultRequest(1);

            ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
            Assert.Equal(ReactorReturnCode.SUCCESS, clientComponentTest.ReactorChannel.Submit(loginRequest, submitOptions, out errorInfo));

            DispatchReactorEvent(consumerReactorEventFD, consumerReactor, dispatchOpts, 1000 * 1000);

            Dispatch(providerReactor, 400);

            Assert.Equal(1, serverComponentTest.NumLoginMsgEvent);

            clientComponentTest.ReactorChannel.SetChannel(writeFlushFailChannel);

            writeFlushFailChannel.writeRetVal = TransportReturnCode.SUCCESS;
            writeFlushFailChannel.flushRetVal = TransportReturnCode.SUCCESS;
            writeFlushFailChannel.smallBuffer = true;

            Assert.Equal(ReactorReturnCode.SUCCESS, clientComponentTest.ReactorChannel.Submit(loginRequest, submitOptions, out errorInfo));

            Thread.Sleep(1000);
            Assert.Equal(1, writeFlushFailChannel.releaseBufferCount);
            Assert.Equal(0, writeFlushFailChannel.flushCount);

            Assert.Equal(ReactorReturnCode.SUCCESS, providerReactor.Shutdown(out errorInfo));
            Assert.Equal(ReactorReturnCode.SUCCESS, consumerReactor.Shutdown(out errorInfo));
        }

        [Fact]
        [Category("Unit")]
        [Category("Reactor")]
        [Category("AdminDomain")]
        public void ReactorSubmitWithWriteFail()
        {
            ReactorErrorInfo errorInfo;
            ReactorOptions reactorOptions = new ReactorOptions();
            reactorOptions.UserSpecObj = this;
            Reactor consumerReactor = Reactor.CreateReactor(reactorOptions, out errorInfo);
            Reactor providerReactor = Reactor.CreateReactor(reactorOptions, out errorInfo);

            Socket consumerReactorEventFD = consumerReactor.EventSocket;
            Socket providerReactorEventFD = providerReactor.EventSocket;

            string serviceName = port.ToString();
            port++;

            BindOptions bindOptions = new BindOptions
            {
                ServiceName = serviceName,
                ServerBlocking = false,
                ConnectionType = ConnectionType.SOCKET,
                ProtocolType = Transports.ProtocolType.RWF,
                UserSpecObject = this,
                InterfaceName = "localhost",
                MajorVersion = Codec.Codec.MajorVersion(),
                MinorVersion = Codec.Codec.MinorVersion(),
                MinPingTimeout = 1,
                PingTimeout = 255
            };

            var server = Transport.Bind(bindOptions, out Error error);

            Assert.NotNull(server);
            Assert.Equal(ChannelState.ACTIVE, server.State);
            Assert.True(server.Socket.IsBound);

            ClientComponentTest clientComponentTest = new ClientComponentTest();
            ServerComponentTest serverComponentTest = new ServerComponentTest();

            ReactorReturnCode acceptRetCode = ReactorReturnCode.FAILURE;
            ReactorReturnCode connectRetCode = ReactorReturnCode.FAILURE;

            ConsumerRole consumerRole = CreateConsumerRole(clientComponentTest, false, false, false);
            ReactorConnectOptions connectOptions = CreateDefaultReactorConnectOptions(serviceName, 0);

            Assert.Null(consumerRole.RdmLoginRequest);
            Assert.Null(consumerRole.RdmDirectoryRequest);

            Task acceptTask = Task.Factory.StartNew(() =>
            {
                acceptRetCode = ServerAccept(providerReactor, serverComponentTest)(server);
            });

            Task connectTask = Task.Factory.StartNew(() =>
            {
                connectRetCode = ClientConnect(consumerReactor, consumerRole, connectOptions)();
            });

            Task.WaitAll(new[] { acceptTask, connectTask });

            Assert.Equal(ReactorReturnCode.SUCCESS, acceptRetCode);
            Assert.Equal(ReactorReturnCode.SUCCESS, connectRetCode);

            ReactorDispatchOptions dispatchOpts = new ReactorDispatchOptions();

            DispatchReactorEvent(providerReactorEventFD, providerReactor, dispatchOpts, 1 * 1000 * 1000);

            Assert.Equal(1, serverComponentTest.NumChannelUpEvent);
            Assert.Equal(1, serverComponentTest.NumChannelReadyEvent);
            Assert.Equal(ChannelState.ACTIVE, serverComponentTest.ReactorChannel.Channel.State);

            DispatchReactorEvent(consumerReactorEventFD, consumerReactor, dispatchOpts, 1 * 1000 * 1000);

            Assert.Equal(1, clientComponentTest.NumChannelUpEvent);
            Assert.Equal(1, clientComponentTest.NumChannelReadyEvent);
            Assert.Equal(ChannelState.ACTIVE, clientComponentTest.ReactorChannel.Channel.State);

            IChannel channel = clientComponentTest.ReactorChannel.Channel;
            WriteFlushFailChannel writeFlushFailChannel = new WriteFlushFailChannel(channel);

            LoginRequest loginRequest = new LoginRequest();
            loginRequest.InitDefaultRequest(1);

            ITransportBuffer msgBuf = clientComponentTest.ReactorChannel.Channel.GetBuffer(1000, false, out Error trError);
            Assert.NotNull(msgBuf);

            EncodeIterator encIter =new EncodeIterator();
            encIter.Clear();
            encIter.SetBufferAndRWFVersion(msgBuf, clientComponentTest.ReactorChannel.Channel.MajorVersion, clientComponentTest.ReactorChannel.Channel.MinorVersion);

            CodecReturnCode ret = loginRequest.Encode(encIter);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);

            ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
            Assert.Equal(ReactorReturnCode.SUCCESS, clientComponentTest.ReactorChannel.Submit(msgBuf, submitOptions, out errorInfo));

            DispatchReactorEvent(consumerReactorEventFD, consumerReactor, dispatchOpts, 1000 * 1000);

            Dispatch(providerReactor, 400);

            Assert.Equal(1, serverComponentTest.NumLoginMsgEvent);

            clientComponentTest.ReactorChannel.SetChannel(writeFlushFailChannel);

            writeFlushFailChannel.writeRetVal = TransportReturnCode.FAILURE;

            Assert.Equal(ReactorReturnCode.FAILURE, clientComponentTest.ReactorChannel.Submit(msgBuf, submitOptions, out errorInfo));

            Thread.Sleep(1000);
            Assert.Equal(0, writeFlushFailChannel.flushCount);

            Assert.Equal(ReactorReturnCode.SUCCESS, providerReactor.Shutdown(out errorInfo));
            Assert.Equal(ReactorReturnCode.SUCCESS, consumerReactor.Shutdown(out errorInfo));
        }

        [Fact]
        [Category("Unit")]
        [Category("Reactor")]
        [Category("AdminDomain")]
        public void ReactorSubmitWithFlushFail()
        {
            ReactorErrorInfo errorInfo;
            ReactorOptions reactorOptions = new ReactorOptions();
            reactorOptions.UserSpecObj = this;
            Reactor consumerReactor = Reactor.CreateReactor(reactorOptions, out errorInfo);
            Reactor providerReactor = Reactor.CreateReactor(reactorOptions, out errorInfo);

            Socket consumerReactorEventFD = consumerReactor.EventSocket;
            Socket providerReactorEventFD = providerReactor.EventSocket;

            string serviceName = port.ToString();
            port++;

            BindOptions bindOptions = new BindOptions
            {
                ServiceName = serviceName,
                ServerBlocking = false,
                ConnectionType = ConnectionType.SOCKET,
                ProtocolType = Transports.ProtocolType.RWF,
                UserSpecObject = this,
                InterfaceName = "localhost",
                MajorVersion = Codec.Codec.MajorVersion(),
                MinorVersion = Codec.Codec.MinorVersion(),
                MinPingTimeout = 1,
                PingTimeout = 255
            };

            var server = Transport.Bind(bindOptions, out Error error);

            Assert.NotNull(server);
            Assert.Equal(ChannelState.ACTIVE, server.State);
            Assert.True(server.Socket.IsBound);

            ClientComponentTest clientComponentTest = new ClientComponentTest();
            ServerComponentTest serverComponentTest = new ServerComponentTest();

            ReactorReturnCode acceptRetCode = ReactorReturnCode.FAILURE;
            ReactorReturnCode connectRetCode = ReactorReturnCode.FAILURE;

            ConsumerRole consumerRole = CreateConsumerRole(clientComponentTest, false, false, false);
            ReactorConnectOptions connectOptions = CreateDefaultReactorConnectOptions(serviceName, 0);

            Assert.Null(consumerRole.RdmLoginRequest);
            Assert.Null(consumerRole.RdmDirectoryRequest);

            Task acceptTask = Task.Factory.StartNew(() =>
            {
                acceptRetCode = ServerAccept(providerReactor, serverComponentTest)(server);
            });

            Task connectTask = Task.Factory.StartNew(() =>
            {
                connectRetCode = ClientConnect(consumerReactor, consumerRole, connectOptions)();
            });

            Task.WaitAll(new[] { acceptTask, connectTask });

            Assert.Equal(ReactorReturnCode.SUCCESS, acceptRetCode);
            Assert.Equal(ReactorReturnCode.SUCCESS, connectRetCode);

            ReactorDispatchOptions dispatchOpts = new ReactorDispatchOptions();

            DispatchReactorEvent(providerReactorEventFD, providerReactor, dispatchOpts, 1 * 1000 * 1000);

            Assert.Equal(1, serverComponentTest.NumChannelUpEvent);
            Assert.Equal(1, serverComponentTest.NumChannelReadyEvent);
            Assert.Equal(ChannelState.ACTIVE, serverComponentTest.ReactorChannel.Channel.State);

            DispatchReactorEvent(consumerReactorEventFD, consumerReactor, dispatchOpts, 1 * 1000 * 1000);

            Assert.Equal(1, clientComponentTest.NumChannelUpEvent);
            Assert.Equal(1, clientComponentTest.NumChannelReadyEvent);
            Assert.Equal(ChannelState.ACTIVE, clientComponentTest.ReactorChannel.Channel.State);

            IChannel channel = clientComponentTest.ReactorChannel.Channel;
            WriteFlushFailChannel writeFlushFailChannel = new WriteFlushFailChannel(channel);

            LoginRequest loginRequest = new LoginRequest();
            loginRequest.InitDefaultRequest(1);

            ITransportBuffer msgBuf = clientComponentTest.ReactorChannel.Channel.GetBuffer(1000, false, out Error trError);
            Assert.NotNull(msgBuf);

            EncodeIterator encIter = new EncodeIterator();
            encIter.Clear();
            encIter.SetBufferAndRWFVersion(msgBuf, clientComponentTest.ReactorChannel.Channel.MajorVersion, clientComponentTest.ReactorChannel.Channel.MinorVersion);

            CodecReturnCode ret = loginRequest.Encode(encIter);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);

            ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
            Assert.Equal(ReactorReturnCode.SUCCESS, clientComponentTest.ReactorChannel.Submit(msgBuf, submitOptions, out errorInfo));

            DispatchReactorEvent(consumerReactorEventFD, consumerReactor, dispatchOpts, 1000 * 1000);

            Dispatch(providerReactor, 400);

            Assert.Equal(1, serverComponentTest.NumLoginMsgEvent);

            clientComponentTest.ReactorChannel.SetChannel(writeFlushFailChannel);

            writeFlushFailChannel.writeRetVal = (TransportReturnCode)1;
            writeFlushFailChannel.flushRetVal = TransportReturnCode.FAILURE;

            Assert.Equal(ReactorReturnCode.SUCCESS, clientComponentTest.ReactorChannel.Submit(msgBuf, submitOptions, out errorInfo));

            Thread.Sleep(1000);
            Assert.Equal(1, writeFlushFailChannel.flushCount);

            DispatchReactorEventIfNeeded(consumerReactorEventFD, consumerReactor, dispatchOpts, 1 * 1000 * 1000);

            Assert.Equal(1, clientComponentTest.NumChannelDownEvent);
            Assert.Equal(ReactorReturnCode.SUCCESS, providerReactor.Shutdown(out errorInfo));
        }

        [Fact]
        [Category("Unit")]
        [Category("Reactor")]
        [Category("AdminDomain")]
        public void ReactorNIProviderWithDefaultLoginDirectoryDictionary()
        {
            ReactorErrorInfo errorInfo;
            ReactorOptions reactorOptions = new ReactorOptions();
            reactorOptions.UserSpecObj = this;
            Reactor reactor = Reactor.CreateReactor(reactorOptions, out errorInfo);

            Socket reactorEventFD = reactor.EventSocket;

            string serviceName = port.ToString();
            port++;

            BindOptions bindOptions = new BindOptions
            {
                ServiceName = serviceName,
                ServerBlocking = false,
                ConnectionType = ConnectionType.SOCKET,
                ProtocolType = Transports.ProtocolType.RWF,
                UserSpecObject = this,
                InterfaceName = "localhost",
                MajorVersion = Codec.Codec.MajorVersion(),
                MinorVersion = Codec.Codec.MinorVersion(),
                MinPingTimeout = 1,
                PingTimeout = 255
            };

            var server = Transport.Bind(bindOptions, out Error error);

            Assert.NotNull(server);
            Assert.Equal(ChannelState.ACTIVE, server.State);
            Assert.True(server.Socket.IsBound);

            ClientComponentTest clientComponentTest = new ClientComponentTest();           
            ServerComponentTest serverComponentTest = new ServerComponentTest();

            serverComponentTest.CheckDirectoryMsg = msg => { Assert.Equal(DirectoryMsgType.REFRESH, msg.DirectoryMsgType); Assert.NotNull(msg.DirectoryRefresh); };

            ReactorReturnCode acceptRetCode = ReactorReturnCode.FAILURE;
            ReactorReturnCode connectRetCode = ReactorReturnCode.FAILURE;

            NIProviderRole niProviderRole = CreateNiProviderRole(clientComponentTest, true);
            ReactorConnectOptions connectOptions = CreateDefaultReactorConnectOptions(serviceName, 3);

            Task acceptTask = Task.Factory.StartNew(() =>
            {
                acceptRetCode = ServerAccept(reactor, serverComponentTest)(server);
            });

            Task connectTask = Task.Factory.StartNew(() =>
            {
                connectRetCode = ClientConnect(reactor, niProviderRole, connectOptions)();
            });

            Task.WaitAll(new[] { acceptTask, connectTask });

            Assert.Equal(ReactorReturnCode.SUCCESS, acceptRetCode);
            Assert.Equal(ReactorReturnCode.SUCCESS, connectRetCode);

            ReactorDispatchOptions dispatchOpts = new ReactorDispatchOptions();

            DispatchReactorEvent(reactorEventFD, reactor, dispatchOpts, 1 * 1000 * 1000);

            Assert.Equal(1, serverComponentTest.NumChannelUpEvent);
            Assert.Equal(1, serverComponentTest.NumChannelReadyEvent);
            Assert.Equal(ChannelState.ACTIVE, serverComponentTest.ReactorChannel.Channel.State);

            Dispatch(reactor, 400);

            Assert.Equal(1, clientComponentTest.NumChannelUpEvent);
            Assert.Equal(ChannelState.ACTIVE, clientComponentTest.ReactorChannel.Channel.State);

            Dispatch(reactor, 400);

            Assert.Equal(1, serverComponentTest.NumLoginMsgEvent);

            Dispatch(reactor, 400);

            Assert.Equal(1, clientComponentTest.NumLoginMsgEvent);

            Dispatch(reactor, 400);

            Assert.Equal(1, clientComponentTest.NumChannelReadyEvent);
            Assert.Equal(1, serverComponentTest.NumDirectoryMsgEvent);

            Assert.Equal(ReactorReturnCode.SUCCESS, reactor.Shutdown(out errorInfo));
        }

        [Fact]
        [Category("Unit")]
        [Category("Reactor")]
        [Category("AdminDomain")]
        public void ReactorSubmitPackedBuffer()
        {
            ReactorErrorInfo errorInfo;
            ReactorOptions reactorOptions = new ReactorOptions();
            reactorOptions.UserSpecObj = this;
            Reactor consumerReactor = Reactor.CreateReactor(reactorOptions, out errorInfo);
            Reactor providerReactor = Reactor.CreateReactor(reactorOptions, out errorInfo);

            Socket consumerReactorEventFD = consumerReactor.EventSocket;
            Socket providerReactorEventFD = providerReactor.EventSocket;

            string serviceName = port.ToString();
            port++;

            BindOptions bindOptions = new BindOptions
            {
                ServiceName = serviceName,
                ServerBlocking = false,
                ConnectionType = ConnectionType.SOCKET,
                ProtocolType = Transports.ProtocolType.RWF,
                UserSpecObject = this,
                InterfaceName = "localhost",
                MajorVersion = Codec.Codec.MajorVersion(),
                MinorVersion = Codec.Codec.MinorVersion(),
                MinPingTimeout = 1,
                PingTimeout = 255
            };

            var server = Transport.Bind(bindOptions, out Error error);

            Assert.NotNull(server);
            Assert.Equal(ChannelState.ACTIVE, server.State);
            Assert.True(server.Socket.IsBound);

            ClientComponentTest clientComponentTest = new ClientComponentTest();
            ServerComponentTest serverComponentTest = new ServerComponentTest();

            ReactorReturnCode acceptRetCode = ReactorReturnCode.FAILURE;
            ReactorReturnCode connectRetCode = ReactorReturnCode.FAILURE;

            ConsumerRole consumerRole = CreateConsumerRole(clientComponentTest, false, false, false);
            ReactorConnectOptions connectOptions = CreateDefaultReactorConnectOptions(serviceName, 0);

            Assert.Null(consumerRole.RdmLoginRequest);
            Assert.Null(consumerRole.RdmDirectoryRequest);

            Task acceptTask = Task.Factory.StartNew(() =>
            {
                acceptRetCode = ServerAccept(providerReactor, serverComponentTest)(server);
            });

            Task connectTask = Task.Factory.StartNew(() =>
            {
                connectRetCode = ClientConnect(consumerReactor, consumerRole, connectOptions)();
            });

            Task.WaitAll(new[] { acceptTask, connectTask });

            Assert.Equal(ReactorReturnCode.SUCCESS, acceptRetCode);
            Assert.Equal(ReactorReturnCode.SUCCESS, connectRetCode);

            ReactorDispatchOptions dispatchOpts = new ReactorDispatchOptions();

            DispatchReactorEventIfNeeded(providerReactorEventFD, providerReactor, dispatchOpts, 1 * 1000 * 1000);

            Assert.Equal(1, serverComponentTest.NumChannelUpEvent);
            Assert.Equal(1, serverComponentTest.NumChannelReadyEvent);
            Assert.Equal(ChannelState.ACTIVE, serverComponentTest.ReactorChannel.Channel.State);

            DispatchReactorEventIfNeeded(consumerReactorEventFD, consumerReactor, dispatchOpts, 1 * 1000 * 1000);

            Assert.Equal(1, clientComponentTest.NumChannelUpEvent);
            Assert.Equal(1, clientComponentTest.NumChannelReadyEvent);
            Assert.Equal(ChannelState.ACTIVE, clientComponentTest.ReactorChannel.Channel.State);

            LoginRequest loginRequest = new LoginRequest();
            loginRequest.InitDefaultRequest(1);

            ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
            Assert.Equal(ReactorReturnCode.SUCCESS, clientComponentTest.ReactorChannel.Submit(loginRequest, submitOptions, out errorInfo));

            DispatchReactorEventIfNeeded(consumerReactorEventFD, consumerReactor, dispatchOpts, 1000 * 1000);

            Dispatch(providerReactor, 400);

            Assert.Equal(1, serverComponentTest.NumLoginMsgEvent);

            Dispatch(consumerReactor, 400);

            Assert.Equal(1, clientComponentTest.NumLoginMsgEvent);

            ReactorChannel chnl = clientComponentTest.ReactorChannel;

            List<string> itemNames = new List<string>();
            List<int> streamIds = new List<int>();
            
            serverComponentTest.CheckDefaultMsg = msg =>
            {
                itemNames.Add(msg.MsgKey.Name.ToString());
                streamIds.Add(msg.StreamId);
            };

            ITransportBuffer buffer = clientComponentTest.ReactorChannel.GetBuffer(100, true, out errorInfo);
            EncodeIterator encIter = new EncodeIterator();

            encIter.SetBufferAndRWFVersion(buffer, chnl.MajorVersion, chnl.MinorVersion);
            clientComponentTest.InitDefaultTestMarketPriceRequest(0, 6, "TRI.N");
            clientComponentTest.MarketPriceRequestMsg.Encode(encIter);
            chnl.PackBuffer(buffer, out errorInfo);

            encIter.Clear();
            encIter.SetBufferAndRWFVersion(buffer, chnl.MajorVersion, chnl.MinorVersion);
            clientComponentTest.InitDefaultTestMarketPriceRequest(0, 7, "IBM.N");
            clientComponentTest.MarketPriceRequestMsg.Encode(encIter);
            chnl.PackBuffer(buffer, out errorInfo);

            encIter.Clear();
            encIter.SetBufferAndRWFVersion(buffer, chnl.MajorVersion, chnl.MinorVersion);
            clientComponentTest.InitDefaultTestMarketPriceRequest(0, 8, "ABC.N");
            clientComponentTest.MarketPriceRequestMsg.Encode(encIter);

            submitOptions.Clear();
            submitOptions.WriteArgs.Priority = WritePriorities.HIGH;
            submitOptions.WriteArgs.Flags = WriteFlags.DIRECT_SOCKET_WRITE;

            consumerReactor.SubmitChannel(clientComponentTest.ReactorChannel, buffer, submitOptions, out errorInfo);

            DispatchReactorEventIfNeeded(consumerReactorEventFD, consumerReactor, dispatchOpts, 1 * 1000 * 1000);
            Dispatch(providerReactor, 1000);

            Assert.Equal(3, itemNames.Count);
            Assert.Contains("TRI.N", itemNames);
            Assert.Contains("IBM.N", itemNames);
            Assert.Contains("ABC.N", itemNames);

            Assert.Equal(3, streamIds.Count);
            Assert.Contains(6, streamIds);
            Assert.Contains(7, streamIds);
            Assert.Contains(8, streamIds);

            Assert.Equal(ReactorReturnCode.SUCCESS, providerReactor.Shutdown(out errorInfo));
            Assert.Equal(ReactorReturnCode.SUCCESS, consumerReactor.Shutdown(out errorInfo));
        }

        private void DispatchReactorEvent(Socket reactorEventFD, Reactor reactor, ReactorDispatchOptions dispatchOptions, int pollTime)
        {
            while (reactorEventFD.Poll(pollTime, SelectMode.SelectRead) != true) ;

            Assert.True(reactorEventFD.Available > 0);
            ReactorErrorInfo errorInfo;
            do
            {
                reactor.Dispatch(dispatchOptions, out errorInfo);
            } while (reactorEventFD.Available > 0);
        }

        private void DispatchReactorEventIfNeeded(Socket reactorEventFD, Reactor reactor, ReactorDispatchOptions dispatchOptions, int pollTime)
        {
            bool dispatch = reactorEventFD.Poll(pollTime, SelectMode.SelectRead);

            if (dispatch)
            {
                ReactorErrorInfo errorInfo;
                do
                {
                    reactor.Dispatch(dispatchOptions, out errorInfo);
                } while (reactorEventFD.Available > 0);
            }          
        }

        private void Dispatch(Reactor reactor, int timeout)
        {
            ReactorDispatchOptions dispatchOptions = new ReactorDispatchOptions();
            ReactorErrorInfo reactorErrorInfo;
            long endTime = DateTimeOffset.Now.ToUnixTimeMilliseconds() + timeout;
            do
            {
                while (reactor.Dispatch(dispatchOptions, out reactorErrorInfo) > ReactorReturnCode.SUCCESS);
            } while (endTime > DateTimeOffset.Now.ToUnixTimeMilliseconds());
        }

        private Func<ReactorReturnCode> ClientConnect(Reactor reactor, ReactorRole clientRole, ReactorConnectOptions connectOptons)
        {
            return () =>
            {                 
                return reactor.Connect(connectOptons, clientRole, out ReactorErrorInfo errorInfo);
            };
        }

        private Func<IServer, ReactorReturnCode> ServerAccept(Reactor reactor, ServerComponentTest serverComponentTest)
        {
            return (acceptServer) =>
            {
                bool pollResult = acceptServer.Socket.Poll(-1, SelectMode.SelectRead);

                Assert.True(pollResult);

                ReactorAcceptOptions acceptOptions = new ReactorAcceptOptions();

                ProviderRole providerRole = new ProviderRole()
                {
                    ChannelEventCallback = serverComponentTest,
                    LoginMsgCallback = serverComponentTest,
                    DirectoryMsgCallback = serverComponentTest,
                    DictionaryMsgCallback = serverComponentTest,
                    DefaultMsgCallback = serverComponentTest
                };

                return reactor.Accept(acceptServer, acceptOptions, providerRole, out ReactorErrorInfo reactorErrorInfo);
            };
        }

        private ReactorConnectOptions CreateDefaultReactorConnectOptions(string serviceName, int reconnectAttemptLimit)
        {
            ReactorConnectOptions connectOptons = new ReactorConnectOptions();
            connectOptons.SetReconnectMaxDelay(500);
            connectOptons.SetReconnectMinDelay(300);
            connectOptons.SetReconnectAttempLimit(reconnectAttemptLimit);

            ReactorConnectInfo connectInfo = new ReactorConnectInfo();
            connectInfo.ConnectOptions.UnifiedNetworkInfo.Address = "localhost";
            connectInfo.ConnectOptions.UnifiedNetworkInfo.ServiceName = serviceName;
            connectInfo.ConnectOptions.PingTimeout = 255;
            connectInfo.ConnectOptions.MajorVersion = Codec.Codec.MajorVersion();
            connectInfo.ConnectOptions.MinorVersion = Codec.Codec.MinorVersion();
            connectInfo.ConnectOptions.Blocking = false;
            connectOptons.ConnectionList.Add(connectInfo);

            return connectOptons;
        }

        private ReactorConnectOptions CreateReactorConnectOptionsWithBackup(string serviceName, string backupServiceName, int reconnectAttemptLimit)
        {
            ReactorConnectOptions connectOptons = new ReactorConnectOptions();
            connectOptons.SetReconnectMaxDelay(500);
            connectOptons.SetReconnectMinDelay(300);
            connectOptons.SetReconnectAttempLimit(reconnectAttemptLimit);

            ReactorConnectInfo connectInfo1 = new ReactorConnectInfo();
            connectInfo1.ConnectOptions.UnifiedNetworkInfo.Address = "localhost";
            connectInfo1.ConnectOptions.UnifiedNetworkInfo.ServiceName = serviceName;
            connectInfo1.ConnectOptions.PingTimeout = 1;
            connectInfo1.ConnectOptions.MajorVersion = Codec.Codec.MajorVersion();
            connectInfo1.ConnectOptions.MinorVersion = Codec.Codec.MinorVersion();
            connectInfo1.ConnectOptions.Blocking = false;
            connectOptons.ConnectionList.Add(connectInfo1);

            ReactorConnectInfo connectInfo2 = new ReactorConnectInfo();
            connectInfo2.ConnectOptions.UnifiedNetworkInfo.Address = "localhost";
            connectInfo2.ConnectOptions.UnifiedNetworkInfo.ServiceName = backupServiceName;
            connectInfo2.ConnectOptions.PingTimeout = 1;
            connectInfo2.ConnectOptions.MajorVersion = Codec.Codec.MajorVersion();
            connectInfo2.ConnectOptions.MinorVersion = Codec.Codec.MinorVersion();
            connectInfo2.ConnectOptions.Blocking = false;
            connectOptons.ConnectionList.Add(connectInfo2);

            return connectOptons;
        }

        private ConsumerRole CreateConsumerRole(ClientComponentTest clientCallbackHandler, bool login, bool directory, bool dictionary)
        {
            ConsumerRole consumerRole = new ConsumerRole()
            {
                ChannelEventCallback = clientCallbackHandler,
                LoginMsgCallback = clientCallbackHandler,
                DirectoryMsgCallback = clientCallbackHandler,
                DictionaryMsgCallback = clientCallbackHandler,
                DefaultMsgCallback = clientCallbackHandler
            };

            if (login)
            {
                consumerRole.InitDefaultRDMLoginRequest();
            }
            else
            {
                consumerRole.RdmLoginRequest = clientCallbackHandler.LoginRequestMsg;
            }
            if (directory)
            {
                consumerRole.InitDefaultRDMDirectoryRequest();
            }
            else
            {
                consumerRole.RdmDirectoryRequest = clientCallbackHandler.DirectoryRequestMsg;
            }
            if (dictionary)
            {
                consumerRole.DictionaryDownloadMode = DictionaryDownloadMode.FIRST_AVAILABLE;
            }
            else
            {
                consumerRole.DictionaryDownloadMode = DictionaryDownloadMode.NONE;
            }

            return consumerRole;
        }

        private NIProviderRole CreateNiProviderRole(ClientComponentTest clientCallbackHandler, bool login)
        {
            NIProviderRole niProviderRole = new NIProviderRole()
            {
                ChannelEventCallback = clientCallbackHandler,
                LoginMsgCallback = clientCallbackHandler,
                DictionaryMsgCallback = clientCallbackHandler,
                DefaultMsgCallback = clientCallbackHandler
            };

            if (login)
            {
                niProviderRole.InitDefaultRDMLoginRequest();
            }
            else
            {
                niProviderRole.RdmLoginRequest = clientCallbackHandler.LoginRequestMsg;
            }
            niProviderRole.InitDefaultRDMDirectoryRefresh("RMDS_PUB", 1);

            return niProviderRole;
        }

        public class ReadNullChannel : IChannel
        {
            IChannel m_Channel;

            public ChannelState State => m_Channel.State;

            public Socket Socket => m_Channel.Socket;

            public Socket OldSocket => m_Channel.OldSocket;

            public int MajorVersion => m_Channel.MajorVersion;

            public int MinorVersion => m_Channel.MinorVersion;

            public Transports.ProtocolType ProtocolType => m_Channel.ProtocolType;

            public int PingTimeOut => m_Channel.PingTimeOut;

            public object UserSpecObject => m_Channel.UserSpecObject;

            public bool Blocking => m_Channel.Blocking;

            public ConnectionType ConnectionType => m_Channel.ConnectionType;

            public string HostName => m_Channel.HostName;

            public int Port => m_Channel.Port;

            public ReadNullChannel(IChannel channel)
            {
                m_Channel = channel;
            }

            public ITransportBuffer Read(ReadArgs readArgs, out Error error)
            {
                // fake out read() returning null here
                error = new Error()
                {
                    ErrorId = TransportReturnCode.FAILURE
                };
                readArgs.ReadRetVal = TransportReturnCode.FAILURE;
                return null;
            }

            public TransportReturnCode Init(InProgInfo inProg, out Error error)
            {
                error = null;
                return TransportReturnCode.SUCCESS;
            }

            public ITransportBuffer GetBuffer(int size, bool packedBuffer, out Error error)
            {
                error = null;
                return null;
            }

            public TransportReturnCode ReleaseBuffer(ITransportBuffer buffer, out Error error)
            {
                error = null;
                return TransportReturnCode.SUCCESS;
            }

            public TransportReturnCode Write(ITransportBuffer buffer, WriteArgs writeArgs, out Error error)
            {
                error = null;
                return TransportReturnCode.SUCCESS;
            }

            public TransportReturnCode Ping(out Error error)
            {
                error = null;
                return TransportReturnCode.SUCCESS;
            }

            public TransportReturnCode Flush(out Error error)
            {
                error = null;
                return TransportReturnCode.SUCCESS;
            }

            public TransportReturnCode Close(out Error error)
            {
                error = null;
                return TransportReturnCode.SUCCESS;
            }

            public TransportReturnCode Info(ChannelInfo info, out Error error)
            {
                error = null;
                return TransportReturnCode.SUCCESS;
            }

            public TransportReturnCode IOCtl(IOCtlCode code, object value, out Error error)
            {
                error = null;
                return TransportReturnCode.SUCCESS;
            }

            public TransportReturnCode IOCtl(IOCtlCode code, int value, out Error error)
            {
                error = null;
                return TransportReturnCode.SUCCESS;
            }

            public TransportReturnCode BufferUsage(out Error error)
            {
                error = null;
                return TransportReturnCode.SUCCESS;
            }

            public TransportReturnCode PackBuffer(ITransportBuffer buffer, out Error error)
            {
                error = null;
                return TransportReturnCode.SUCCESS;
            }
        }

        public class WriteFlushFailChannel : IChannel
        {
            internal TransportReturnCode writeRetVal = TransportReturnCode.SUCCESS;
            internal TransportReturnCode flushRetVal = TransportReturnCode.SUCCESS;
            internal bool resetRetValAfterFlush = false;
            internal int flushCount = 0;
            internal int releaseBufferCount = 0;
            internal bool noBuffers = false;
            internal bool smallBuffer = false;
            internal ChannelState _state = ChannelState.ACTIVE;
            private IChannel _channel;

            class TestTransportBuffer : ITransportBuffer
            {
                const int smallBufSize = 1;
                const int bufSize = 1000;
                ByteBuffer byteBuffer = new ByteBuffer(bufSize);
                ByteBuffer smallByteBuffer = new ByteBuffer(smallBufSize);

                WriteFlushFailChannel parentChannel;

                public TestTransportBuffer(WriteFlushFailChannel channel)
                {
                    parentChannel = channel;
                }

                ByteBuffer ITransportBuffer.Data
                {
                    get
                    {
                        if (parentChannel.smallBuffer == false)
                        {
                            return byteBuffer;
                        }
                        else
                        {
                            parentChannel.smallBuffer = false;
                            return smallByteBuffer;
                        }
                    }
                }

                public bool IsOwnedByApp { get; set; }

                public int Length()
                {
                    if (parentChannel.smallBuffer == false)
                    {
                        return bufSize;
                    }
                    else
                    {
                        parentChannel.smallBuffer = false;
                        return smallBufSize;
                    }
                }

                public int Capacity()
                {
                    if (parentChannel.smallBuffer == false)
                    {
                        return bufSize;
                    }
                    else
                    {
                        parentChannel.smallBuffer = false;
                        return smallBufSize;
                    }
                }

                public int GetDataStartPosition()
                {
                    return 0;
                }

                public int Copy(ByteBuffer destination)
                {
                    return 0;
                }
            }

            public WriteFlushFailChannel(IChannel channel)
            {
                _channel = channel;
            }

            public ChannelState State => _channel.State;

            public Socket Socket => _channel.Socket;

            public Socket OldSocket => _channel.OldSocket;

            public int MajorVersion => _channel.MajorVersion;

            public int MinorVersion => _channel.MinorVersion;

            public Transports.ProtocolType ProtocolType => _channel.ProtocolType;

            public int PingTimeOut => _channel.PingTimeOut;

            public object UserSpecObject => _channel.UserSpecObject;

            public bool Blocking => _channel.Blocking;

            public ConnectionType ConnectionType => _channel.ConnectionType;

            public string HostName => _channel.HostName;

            public int Port => _channel.Port;

            public TransportReturnCode BufferUsage(out Error error)
            {
                error = null;
                return TransportReturnCode.SUCCESS;
            }

            public TransportReturnCode Close(out Error error)
            {
                error = null;
                return TransportReturnCode.SUCCESS;
            }

            public TransportReturnCode Flush(out Error error)
            {
                TransportReturnCode ret = flushRetVal;
                if (resetRetValAfterFlush)
                {
                    flushRetVal = TransportReturnCode.SUCCESS;
                    resetRetValAfterFlush = false;
                }
                flushCount++;
                error = new Error();
                return ret;
            }

            public ITransportBuffer GetBuffer(int size, bool packedBuffer, out Error error)
            {
                error = null;
                if (!noBuffers)
                {
                    return new TestTransportBuffer(this);
                }
                else
                {
                    error = new Error();
                    return null;
                }
            }

            public TransportReturnCode Info(ChannelInfo info, out Error error)
            {
                error = null;
                return TransportReturnCode.SUCCESS;
            }

            public TransportReturnCode Init(InProgInfo inProg, out Error error)
            {
                error = null;
                return TransportReturnCode.SUCCESS;
            }

            public TransportReturnCode IOCtl(IOCtlCode code, object value, out Error error)
            {
                error = null;
                return TransportReturnCode.SUCCESS;
            }

            public TransportReturnCode IOCtl(IOCtlCode code, int value, out Error error)
            {
                error = null;
                return TransportReturnCode.SUCCESS;
            }

            public TransportReturnCode PackBuffer(ITransportBuffer buffer, out Error error)
            {
                error = null;
                return TransportReturnCode.SUCCESS;
            }

            public TransportReturnCode Ping(out Error error)
            {
                error = null;
                return TransportReturnCode.SUCCESS;
            }

            public ITransportBuffer Read(ReadArgs readArgs, out Error error)
            {
                error = new Error()
                {
                    ErrorId = TransportReturnCode.FAILURE
                };
                readArgs.ReadRetVal = TransportReturnCode.FAILURE;
                return null;
            }

            public TransportReturnCode ReleaseBuffer(ITransportBuffer buffer, out Error error)
            {
                releaseBufferCount++;
                error = null;
                return TransportReturnCode.SUCCESS;
            }

            public TransportReturnCode Write(ITransportBuffer buffer, WriteArgs writeArgs, out Error error)
            {
                error = new Error();
                return writeRetVal;
            }
        }      

    }
}
