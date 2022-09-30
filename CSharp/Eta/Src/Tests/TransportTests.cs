/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Threading;
using System.Threading.Tasks;

using Xunit;
using Xunit.Abstractions;
using Xunit.Categories;

using Refinitiv.Common.Logger;
using Refinitiv.Eta.Internal;
using Refinitiv.Eta.Transports.Interfaces;
using Refinitiv.Eta.Internal.Interfaces;
using Refinitiv.Eta.Tests;
using System.Text;
using Refinitiv.Eta.Transports.Internal;
using Refinitiv.Common.Interfaces;
using System.Net.Sockets;
using System.Collections.Generic;
using System.Net.Security;
using System.Runtime.InteropServices;

namespace Refinitiv.Eta.Transports.Tests
{
    #region Transport Initialization
    /// <summary>
    /// This set of test will test expected result of each test
    /// Independent of each other, sequence isn't an issue.
    /// </summary>
    [Collection("Transport")]
    public class TransportInitializationTests
    {
        [Fact]
        [Category("Unit")]
        [Category("Transport")]
        public void TransportInitializeNotInitialize()
        {
            Transport.Clear();
            Assert.True(TransportReturnCode.INIT_NOT_INITIALIZED == Transport.Uninitialize());
        }
        [Fact]
        [Category("Unit")]
        [Category("Transport")]
        public void TransportInitializedOK()
        {
            Transport.Clear();
            InitArgs initArgs = new InitArgs { GlobalLocking = true };
            Assert.True(TransportReturnCode.SUCCESS == Transport.Initialize(initArgs, out Error error));
        }
        [Fact]
        [Category("Unit")]
        [Category("Transport")]
        public void TransportInitializeSubsequentInit()
        {
            // transport still open
            Transport.Clear();
            InitArgs initArgs = new InitArgs { GlobalLocking = true };
            Transport.Initialize(initArgs, out Error error);
            Transport.Initialize(initArgs, out error);
            Assert.True(TransportReturnCode.SUCCESS == Transport.Initialize(initArgs, out error));
            Assert.True(TransportReturnCode.SUCCESS == Transport.Uninitialize());
        }

        [Fact]
        [Category("Unit")]
        [Category("Transport")]
        public void TransportUninitializeOK()
        {
            Transport.Clear();
            InitArgs initArgs = new InitArgs { GlobalLocking = true };
            Transport.Initialize(initArgs, out Error error);
            Assert.True(TransportReturnCode.SUCCESS == Transport.Uninitialize());
        }

        [Fact]
        [Category("Unit")]
        [Category("Transport")]
        public void TransportUninitializeMoreThanOne()
        {
            Transport.Clear();
            InitArgs initArgs = new InitArgs { GlobalLocking = true };
            Transport.Initialize(initArgs, out Error error);
            Transport.Uninitialize();
            Transport.Uninitialize();
            Assert.True(TransportReturnCode.INIT_NOT_INITIALIZED == Transport.Uninitialize());
        }

    }
    #endregion

    #region Transport Connect
    /// <summary>
    /// Validate the Transport.Connect and Transport.GlobalLocking behavior:
    /// 1) Transport must be initialized before any connection attempt.
    /// 2) Transport.Connect must request a supported Protocol.
    /// 3) Transport.Connect can deliver a Blocking Channel.
    /// 4) Transport.Connect can deliver a Non-blocking Channel.
    /// 5) Transport.Connect w/ GlobalLocking only allows a single
    ///    Channel.Connect call to be occurring at one time.
    /// 6) Transport.Connect w/out GlobalLocking allows multiple
    ///    Channel.Connect call to be occurring at one time.
    /// </summary>
    [Collection("Transport")]
    public class TransportConnectTests : IDisposable
    {
        public TransportConnectTests(ITestOutputHelper output)
        {
            XUnitLoggerProvider.Instance.Output = output;
            EtaLoggerFactory.Instance.AddProvider(XUnitLoggerProvider.Instance);
        }

        public void Dispose()
        {
            XUnitLoggerProvider.Instance.Output = null;
        }

        static TransportConnectTests()
        {
            ProtocolRegistry.Instance
                .Register(ConnectionType.ENCRYPTED, new Refinitiv.Eta.Tests.MockProtocol());
        }

        /// <summary>
        /// Channel was created, supports IChannel, there was no error, and the Channel.State
        /// is as expected for the ConnectionType.Blocking value.
        /// </summary>
        /// <param name="connectOptions"></param>
        private IChannel AssertChannelExists(ConnectOptions connectOptions)
        {
            if (string.IsNullOrEmpty(connectOptions.UnifiedNetworkInfo.ServiceName))
                connectOptions.UnifiedNetworkInfo.ServiceName = MockProtocol.PortActionAfter50ms.ToString();

            var channel = Transport.Connect(connectOptions, out Error error);
            if (error != null)
                EtaLogger.Instance.Error($"{DateTime.UtcNow:HH:mm:ss.ffff} Test - Error: {error}");
            Assert.Null(error);

            Assert.IsAssignableFrom<IChannel>(channel);

            EtaLogger.Instance.Trace($"{DateTime.UtcNow:HH:mm:ss.ffff} Test - Channel: {channel}");

            Assert.True((((ChannelBase)channel).State == ChannelState.ACTIVE)
                      || ((ChannelBase)channel).State == ChannelState.INITIALIZING && !connectOptions.Blocking);

            return channel;
        }


        [Fact]
        [Category("Unit")]
        [Category("Transport")]
        public void TransportConnectBeforeInitializeFail()
        {
            try
            {
                Transport.Clear();

                var channel = Transport.Connect(new ConnectOptions
                {
                    ConnectionType = ConnectionType.SOCKET,
                    Blocking = true
                }, out Error error);

                Assert.Null(channel);
                Assert.Equal(TransportReturnCode.INIT_NOT_INITIALIZED,
                    error.ErrorId);
                Assert.Null(error.Channel);
            }
            finally
            {
                Transport.Clear();
            }
        }

        [Fact]
        [Category("Unit")]
        [Category("Transport")]
        public void TransportConnectNotSupportedProtocolFail()
        {
            try
            {
                InitArgs initArgs = new InitArgs { GlobalLocking = true };
                Transport.Initialize(initArgs, out Error error);

                var actual = Transport.Connect(new ConnectOptions { ConnectionType = ConnectionType.ENCRYPTED }, out error);

                Assert.Null(actual);
            }
            finally
            {
                Transport.Uninitialize();

            }
        }

        [Fact(Skip = "Broken")]
        [Category("Unit")]
        [Category("Transport")]
        [Category("Channel")]
        public void TransportConnectBlockingProtocolOk()
        {
            IChannel channel = null;
            try
            {
                InitArgs initArgs = new InitArgs { GlobalLocking = true };

                var actual = Transport.Initialize(initArgs, out Error error);
                if (error != null)
                    EtaLogger.Instance.Trace($"{error}");

                Assert.Equal(TransportReturnCode.SUCCESS, actual);

                var cnxnOptions = new ConnectOptions
                {
                    ConnectionType = ConnectionType.SOCKET,
                    Blocking = true
                };
                cnxnOptions.UnifiedNetworkInfo.Address = "localhost";
                cnxnOptions.UnifiedNetworkInfo.ServiceName = MockProtocol.PortActionAfter50ms.ToString();

                channel = AssertChannelExists(cnxnOptions);
            }
            finally
            {
                if (channel != null)
                {
                    channel.Close(out Error error);
                    Assert.Null(error);
                }
                Transport.Uninitialize();
            }
        }

        [Fact(Skip ="Broken")]
        [Category("Unit")]
        [Category("Transport")]
        [Category("Channel")]
        async public void TransportConnectBlockingOneAtATime()
        {
            try
            {
                InitArgs initArgs = new InitArgs { GlobalLocking = false };
                var result = Transport.Initialize(initArgs, out Error error);

                Assert.Equal(TransportReturnCode.SUCCESS, result);

                Func<IChannel> connect = new Func<IChannel>(() =>
                {
                    var cnxnOptions = new ConnectOptions
                    {
                        ConnectionType = ConnectionType.SOCKET,
                        Blocking = true
                    };
                    cnxnOptions.UnifiedNetworkInfo.Address = "localhost";
                    cnxnOptions.UnifiedNetworkInfo.ServiceName = MockProtocol.PortActionAfter50ms.ToString();

                    IChannel channel = null;
                    try
                    {
                        channel = Transport.Connect(cnxnOptions,
                                                 out Error connectError);
                        if (connectError != null)
                            EtaLogger.Instance.Trace($"{DateTime.UtcNow:HH:mm:ss.ffff} Test ConnectError: {connectError}");

                        return channel;
                    }
                    finally
                    {
                        channel.Close(out Error closeError);
                        Assert.Null(closeError);
                    }
                });

                MockProtocol.TestMultipleConnectsFail = true;

                var taskA = Task.Factory.StartNew(connect);
                var taskB = Task.Factory.StartNew(connect);

                var channelA = taskA.Result;
                var channelB = taskB.Result;

                Assert.NotNull(channelA);
                Assert.IsAssignableFrom<IChannel>(channelA);

                await taskB;
            }
            finally
            {
                Transport.Uninitialize();
            }
        }

        [Fact(Skip = "Broken")]
        [Category("Unit")]
        [Category("Transport")]
        [Category("Channel")]
        public void TransportConnectNonBlockingChannelAsyncConnect()
        {
            Error connectError = null;
            Error initError = null;

            try
            {
                InitArgs initArgs = new InitArgs { GlobalLocking = false };

                var returnCode = Transport.Initialize(initArgs, out Error error);
                Assert.Equal(TransportReturnCode.SUCCESS, returnCode);

                Func<int, IChannel> connect = new Func<int, IChannel>((port) =>
                {
                    IChannel result = null;
                    var connectionOptions = new ConnectOptions
                    {
                        ConnectionType = ConnectionType.SOCKET,
                        Blocking = false      
                    };
                    connectionOptions.UnifiedNetworkInfo.Address = "localhost";
                    connectionOptions.UnifiedNetworkInfo.ServiceName = port.ToString();
                    result = Transport.Connect(connectionOptions,
                                             out connectError);

                    return result;
                });

                Func<IChannel, TransportReturnCode> initPoll = new Func<IChannel, TransportReturnCode>((chnl) =>
                {
                    TransportReturnCode initReturnCode;
                    do
                    {
                        Thread.Sleep(10);
                        InProgInfo inProgInfo = new InProgInfo();
                        initReturnCode = chnl.Init(inProgInfo, out initError);
                    } while (initReturnCode != TransportReturnCode.SUCCESS);
                    return initReturnCode;
                });

                // Create a MockChannel that will pend on external connect signal before
                // changing its state.
                IChannel channel = connect(MockProtocol.PortActionOnSignal);
                var initTask = Task.Factory.StartNew<TransportReturnCode>(() => initPoll(channel));

                // Assert Channel created initializing.
                Assert.Equal(TransportReturnCode.CHAN_INIT_IN_PROGRESS, channel.Init(new InProgInfo(), out connectError));

                // Fondle internals
                IInternalChannel baseChannel = channel as IInternalChannel;
                Assert.NotNull(baseChannel);

                Thread.Sleep(250);
                // Assert Channel created still initializing.
                Assert.Equal(TransportReturnCode.CHAN_INIT_IN_PROGRESS, channel.Init(new InProgInfo(), out connectError));

                // Externally Set Connected Signal
                baseChannel.SocketChannel.IsConnected = true;

                Thread.Sleep(50);
                baseChannel.SocketChannel.IsDataReady = true;

                int waitms = 15000;
                if (initTask.Wait(waitms))
                    Assert.Equal(TransportReturnCode.SUCCESS, initTask.Result);
                else
                    throw new TransportException($"Channel (NonBlocking) Timeout: ({waitms})ms");

            }
            finally
            {
                Transport.Clear();
            }
        }

    }
    #endregion

    #region Transport Initialization ThreadSafety
    /// <summary>
    /// step 1
    /// Test the ability to run TaskB within TaskA
    /// step 2
    /// Test the ability to run TaskA within TaskB
    /// step 3
    /// Created 30 Task random intervals
    /// </summary>
    [Collection("Transport")]
    public class TransportTestsThreadSafety
    {
        [Fact]
        [Category("Unit")]
        [Category("Transport")]
        public void TaskBRunsWithinTaskA()
        {
            object result1 = null;
            object result2 = null;
            var threadId = Thread.CurrentThread.ManagedThreadId;
            Transport.Clear(); // 
            Task taskA = new Task<TransportReturnCode>(
                () =>
                {
                    InitArgs initArgs = new InitArgs { GlobalLocking = true };
                    Transport.Initialize(initArgs, error: out Error error);
                    threadId = Thread.CurrentThread.ManagedThreadId;
                    Thread.Sleep(2000);
                    result1 = Transport.Uninitialize();
                    Assert.Equal(TransportReturnCode.SUCCESS, result1);
                    return (TransportReturnCode)result1;


                });
            Task taskB = new Task<TransportReturnCode>(
                () =>
                {
                    threadId = Thread.CurrentThread.ManagedThreadId;
                    InitArgs initArgs = new InitArgs { GlobalLocking = true };
                    Transport.Initialize(initArgs, error: out Error error);
                    Thread.Sleep(1000);
                    result2 = Transport.Uninitialize();
                    Assert.Equal(TransportReturnCode.SUCCESS, result2);
                    return (TransportReturnCode)result2;
                });
            taskA.Start();
            Thread.Sleep(250);
            taskB.Start();
            Task.WaitAll(taskA, taskB);
            Assert.Equal(TransportReturnCode.INIT_NOT_INITIALIZED, Transport.Uninitialize());
        }

        [Fact]
        [Category("Unit")]
        [Category("Transport")]
        public void TaskARunsWithinTaskB()
        {
            object result1 = null;
            object result2 = null;
            var threadId = Thread.CurrentThread.ManagedThreadId;
            Transport.Clear(); // 
            Task taskA = new Task<TransportReturnCode>(
                () =>
                {
                    InitArgs initArgs = new InitArgs { GlobalLocking = true };
                    Transport.Initialize(initArgs, error: out Error error);
                    threadId = Thread.CurrentThread.ManagedThreadId;
                    Thread.Sleep(2000);
                    result1 = Transport.Uninitialize();
                    Assert.Equal(TransportReturnCode.SUCCESS, result1);
                    return (TransportReturnCode)result1;
                });
            Task taskB = new Task<TransportReturnCode>(
                () =>
                {
                    var threadId2 = Thread.CurrentThread.ManagedThreadId;
                    InitArgs initArgs = new InitArgs { GlobalLocking = true };
                    Transport.Initialize(initArgs, error: out Error error);
                    Thread.Sleep(2000);
                    result2 = Transport.Uninitialize();
                    Assert.Equal(TransportReturnCode.SUCCESS, result2);
                    return (TransportReturnCode)result2;
                });
            taskA.Start();
            Thread.Sleep(500);
            taskB.Start();
            Task.WaitAll(taskA, taskB);
            Assert.Equal(TransportReturnCode.INIT_NOT_INITIALIZED, Transport.Uninitialize());
        }

        [Fact(Skip = "TakesTooLong")]
        [Category("Unit")]
        [Category("Transport")]
        public void MultipleTaskVariableIntervals()
        {
            object[] resultList = new object[30];
            Random randomInteval = new Random();

            Task<TransportReturnCode>[] taskArray = new Task<TransportReturnCode>[30];
            int randomInt;
            var threadId = Thread.CurrentThread.ManagedThreadId;

            Transport.Clear();
            for (int i = 0; i < taskArray.Length; i++)
            {
                randomInt = randomInteval.Next(0, 3000);

                taskArray[i] = Task<TransportReturnCode>.Factory.StartNew(
                            () =>
                            {
                                InitArgs initArgs = new InitArgs { GlobalLocking = true };
                                Transport.Initialize(initArgs, error: out Error error);
                                Thread.Sleep(randomInt);
                                return Transport.Uninitialize();
                            });//).Result;
                resultList[i] = taskArray[i].Result;
                Assert.Equal(TransportReturnCode.SUCCESS, resultList[i]);
            }
            Task.WaitAll(taskArray, 6000);
            Assert.Equal(TransportReturnCode.INIT_NOT_INITIALIZED, Transport.Uninitialize());
        }
    }
    #endregion

    #region Transport Protocol Tests
    [Collection("Transport")]
    public class TransportTestsProtocol : IDisposable
    {
        public TransportTestsProtocol(ITestOutputHelper output)
        {
            XUnitLoggerProvider.Instance.Output = output;
            EtaLoggerFactory.Instance.AddProvider(XUnitLoggerProvider.Instance);
        }

        public void Dispose()
        {
            XUnitLoggerProvider.Instance.Output = null;
        }

        static TransportTestsProtocol()
        {
            ProtocolRegistry.Instance
                .Register(ConnectionType.SOCKET, new Refinitiv.Eta.Tests.MockProtocol());
        }

        /// <summary>
        /// Channel was created, supports IChannel, there was no error, and the Channel.State
        /// is as expected for the ConnectionType.Blocking value.
        /// </summary>
        /// <param name="connectOptions"></param>
        private void AssertChannelExists(ConnectOptions connectOptions)
        {
            var channel = Transport.Connect(connectOptions, out Error error);
            if (error != null)
                EtaLogger.Instance.Trace($"{error}");

            Assert.IsAssignableFrom<IChannel>(channel);
            Assert.Null(error);
            Assert.True((((ChannelBase)channel).State == ChannelState.ACTIVE)
                      || ((ChannelBase)channel).State == ChannelState.INITIALIZING && !connectOptions.Blocking);
        }
    }
    #endregion

    #region Transport Global Locking Tests

    [Collection("Transport")]
    public class TransportGlobalLockingTests : IDisposable
    {

        public TransportGlobalLockingTests(ITestOutputHelper output)
        {
            XUnitLoggerProvider.Instance.Output = output;
            EtaLoggerFactory.Instance.AddProvider(XUnitLoggerProvider.Instance);
        }

        public void Dispose()
        {
            XUnitLoggerProvider.Instance.Output = null;
        }

        [Fact]
        [Category("Unit")]
        [Category("Transport")]
        public void TransportInitializeCannotChangeGlobalLocking()
        {
            InitArgs initArgsTrue = new InitArgs()
            { GlobalLocking = true };
            InitArgs initArgsFalse = new InitArgs()
            { GlobalLocking = false };

            try
            {
                Transport.Initialize(initArgsTrue, out Error error);
                var result = Transport.Initialize(initArgsFalse, out error);

                Assert.Equal(TransportReturnCode.FAILURE, result);
                Assert.Equal(TransportReturnCode.INIT_NOT_INITIALIZED, error.ErrorId);
            }
            finally
            {
                Transport.Clear();
            }
        }


    }

    #endregion

    #region Transport Select

    [Collection("Transport"), Category("Unit"), Category("Transport")]
    public class TransportSelectTests
    {
        ITestOutputHelper _output;

        public TransportSelectTests(ITestOutputHelper output)
        {
            _output = output;
            ProtocolRegistry.Instance
                           .Register(ConnectionType.SOCKET, new Refinitiv.Eta.Tests.MockProtocol());
        }
    }

    #endregion

    #region Transport Write and Flush
    [Category("Unit")]
    [Category("Transport")]
    public class TransportWriteFlushTests
    {
        [Fact]
        public void TransportGetBufferInActiveChannelTest()
        {
            ConnectOptions conOpt = new ConnectOptions();
            conOpt.UnifiedNetworkInfo.ServiceName = "3000";
            MockChannel mockChannel = new MockChannel();
            IChannel channel = new ChannelBase(new MockProtocol(), conOpt, mockChannel);

            Assert.True(channel.State == ChannelState.INACTIVE);

            ITransportBuffer transport = channel.GetBuffer(500, false, out Error error);
            Assert.Null(transport);
            Assert.NotNull(error);
            Assert.True(channel == error.Channel);
            Assert.True(TransportReturnCode.FAILURE == error.ErrorId);
            Assert.Equal("Channel is not in active state for GetBuffer", error.Text);
            Assert.Equal(0, error.SysError);
        }

        [Fact]
        public void TransportDirectWriteTest()
        {
            ConnectOptions conOpt = new ConnectOptions();
            MockChannel mockChannel = new MockChannel();

            InitArgs initArgs = new InitArgs
            {
                GlobalLocking = true
            };

            Transport.Initialize(initArgs, out Error error);

            ChannelBase channel = new ChannelBase(conOpt, mockChannel, ChannelState.ACTIVE, 8192);

            ITransportBuffer messageBuffer = channel.GetBuffer(500, false, out error);

            Assert.NotNull(messageBuffer);
            Assert.Null(error);

            byte[] dataByteArray = Encoding.ASCII.GetBytes("ABCDEFGHIJKL123456789");

            messageBuffer.Data.Put(dataByteArray);

            TransportReturnCode ret = channel.Write(messageBuffer, new WriteArgs()
            {
                Flags = WriteFlags.DIRECT_SOCKET_WRITE
            }, out error );

            Assert.Equal(TransportReturnCode.SUCCESS, ret);
            Assert.Null(error);
            mockChannel.GetNetworkBuffer().Flip();

            TransportBuffer expectedBuffer = new TransportBuffer(new Common.ByteBuffer(100), RipcDataMessage.HeaderSize, true);
            expectedBuffer.Data.Put(dataByteArray);

            int ripcMsgLength = expectedBuffer.Length + RipcDataMessage.HeaderSize;

            expectedBuffer.Data.WriteAt(0, (short)(ripcMsgLength));  // Populate RIPC message length
            expectedBuffer.Data.WriteAt(2, (byte)RipcFlags.DATA);  // Populate RIPC message header

            expectedBuffer.Data.Flip();

            // Compares the contents of the two buffers
            Assert.Equal(0, mockChannel.GetNetworkBuffer().Position);
            Assert.Equal(ripcMsgLength, mockChannel.GetNetworkBuffer().Limit);
            Assert.Equal(expectedBuffer.Data.Limit, mockChannel.GetNetworkBuffer().Limit);
            Assert.True(expectedBuffer.Data.Equals(mockChannel.GetNetworkBuffer()));
            Assert.Equal(TransportReturnCode.SUCCESS, channel.Close(out error));

            Transport.Uninitialize();
        }

        [Fact]
        public void TransportWriteAndFlushTest()
        {
            ConnectOptions conOpt = new ConnectOptions();
            MockChannel mockChannel = new MockChannel();

            InitArgs initArgs = new InitArgs
            {
                GlobalLocking = true
            };

            Transport.Initialize(initArgs, out Error error);

            ChannelBase channel = new ChannelBase(conOpt, mockChannel, ChannelState.ACTIVE, 8192);

            ITransportBuffer messageBuffer = channel.GetBuffer(500, false, out error);

            Assert.NotNull(messageBuffer);
            Assert.Null(error);

            byte[] dataByteArray = Encoding.ASCII.GetBytes("ABCDEFGHIJKL123456789");

            TransportBuffer expectedBuffer = new TransportBuffer(new Common.ByteBuffer(100), RipcDataMessage.HeaderSize, true);
            expectedBuffer.Data.Put(dataByteArray);

            int ripcMsgLength = expectedBuffer.Length + RipcDataMessage.HeaderSize;

            messageBuffer.Data.Put(dataByteArray);

            TransportReturnCode ret = channel.Write(messageBuffer, new WriteArgs(), out error);

            Assert.True(ret > TransportReturnCode.SUCCESS && (int)ret == ripcMsgLength);
            Assert.Null(error);

            ret = channel.Flush(out error);
            Assert.Equal(TransportReturnCode.SUCCESS, ret);
            Assert.Null(error);
            mockChannel.GetNetworkBuffer().Flip();

            expectedBuffer.Data.WriteAt(0, (short)(ripcMsgLength));  // Populate RIPC message length
            expectedBuffer.Data.WriteAt(2, (byte)RipcFlags.DATA);  // Populate RIPC message header

            expectedBuffer.Data.Flip();

            // Compares the contents of the two buffers
            Assert.Equal(0, mockChannel.GetNetworkBuffer().Position);
            Assert.Equal(ripcMsgLength, mockChannel.GetNetworkBuffer().Limit);
            Assert.Equal(expectedBuffer.Data.Limit, mockChannel.GetNetworkBuffer().Limit);
            Assert.True(expectedBuffer.Data.Equals(mockChannel.GetNetworkBuffer()));
            Assert.Equal(TransportReturnCode.SUCCESS, channel.Close(out error));

            Transport.Uninitialize();
        }

        [Fact]
        public void TransportPartialFlushTest()
        {
            ConnectOptions conOpt = new ConnectOptions();
            MockChannel mockChannel = new MockChannel();

            InitArgs initArgs = new InitArgs
            {
                GlobalLocking = true
            };

            Transport.Initialize(initArgs, out Error error);

            ChannelBase channel = new ChannelBase(conOpt, mockChannel, ChannelState.ACTIVE, 8192);

            ITransportBuffer messageBuffer = channel.GetBuffer(500, false, out error);

            Assert.NotNull(messageBuffer);
            Assert.Null(error);

            byte[] dataByteArray = Encoding.ASCII.GetBytes("ABCDEFGHIJKL123456789");

            TransportBuffer expectedBuffer = new TransportBuffer(new Common.ByteBuffer(100), RipcDataMessage.HeaderSize, true);
            expectedBuffer.Data.Put(dataByteArray);

            int ripcMsgLength = expectedBuffer.Length + RipcDataMessage.HeaderSize;

            messageBuffer.Data.Put(dataByteArray);

            TransportReturnCode ret = channel.Write(messageBuffer, new WriteArgs(), out error);

            Assert.True(ret > TransportReturnCode.SUCCESS && (int)ret == ripcMsgLength);
            Assert.Null(error);

            // Limit maximum number of bytes per write call to 10
            mockChannel.MaxWrite = 10;

            ret = channel.Flush(out error);
            Assert.True(ret > TransportReturnCode.SUCCESS && (int)ret == 14);
            Assert.Null(error);

            ret = channel.Flush(out error);
            Assert.True(ret > TransportReturnCode.SUCCESS && (int)ret == 4);
            Assert.Null(error);

            ret = channel.Flush(out error);
            Assert.True(ret == TransportReturnCode.SUCCESS);
            Assert.Null(error);

            mockChannel.GetNetworkBuffer().Flip();

            expectedBuffer.Data.WriteAt(0, (short)(ripcMsgLength));  // Populate RIPC message length
            expectedBuffer.Data.WriteAt(2, (byte)RipcFlags.DATA);  // Populate RIPC message header

            expectedBuffer.Data.Flip();

            // Compares the contents of the two buffers
            Assert.Equal(0, mockChannel.GetNetworkBuffer().Position);
            Assert.Equal(ripcMsgLength, mockChannel.GetNetworkBuffer().Limit);
            Assert.Equal(expectedBuffer.Data.Limit, mockChannel.GetNetworkBuffer().Limit);
            Assert.True(expectedBuffer.Data.Equals(mockChannel.GetNetworkBuffer()));
            Assert.Equal(TransportReturnCode.SUCCESS, channel.Close(out error));

            Transport.Uninitialize();
        }

        [Fact]
        public void TransportPartialFlushWith2BuffersTest()
        {
            ConnectOptions conOpt = new ConnectOptions();
            MockChannel mockChannel = new MockChannel();

            InitArgs initArgs = new InitArgs
            {
                GlobalLocking = true
            };

            Transport.Initialize(initArgs, out Error error);

            ChannelBase channel = new ChannelBase(conOpt, mockChannel, ChannelState.ACTIVE, 8192);

            ITransportBuffer messageBuffer = channel.GetBuffer(100, false, out error);

            Assert.NotNull(messageBuffer);
            Assert.Null(error);

            byte[] dataByteArray = Encoding.ASCII.GetBytes("ABCDEF");

            TransportBuffer expectedBuffer = new TransportBuffer(new Common.ByteBuffer(50), RipcDataMessage.HeaderSize, true);
            expectedBuffer.Data.Put(dataByteArray);

            int ripcMsgLength = expectedBuffer.Length + RipcDataMessage.HeaderSize;

            messageBuffer.Data.Put(dataByteArray);

            TransportReturnCode ret = channel.Write(messageBuffer, new WriteArgs(), out error);

            Assert.True(ret > TransportReturnCode.SUCCESS && (int)ret == ripcMsgLength);
            Assert.Null(error);

            ITransportBuffer messageBuffer2 = channel.GetBuffer(100, false, out error);

            Assert.NotNull(messageBuffer2);
            Assert.Null(error);

            byte[] dataByteArray2 = Encoding.ASCII.GetBytes("123456789");

            TransportBuffer expectedBuffer2 = new TransportBuffer(new Common.ByteBuffer(50), RipcDataMessage.HeaderSize, true);
            expectedBuffer2.Data.Put(dataByteArray2);

            int ripcMsgLength2 = expectedBuffer2.Length + RipcDataMessage.HeaderSize;

            messageBuffer2.Data.Put(dataByteArray2);

            ret = channel.Write(messageBuffer2, new WriteArgs(), out error);

            Assert.True(ret > TransportReturnCode.SUCCESS && (int)ret == (ripcMsgLength + ripcMsgLength2));
            Assert.Null(error);

            // Limit maximum number of bytes per write call to 10
            mockChannel.MaxWrite = 10;

            ret = channel.Flush(out error);
            Assert.True(ret > TransportReturnCode.SUCCESS && (int)ret == 2);
            Assert.Null(error);

            ret = channel.Flush(out error);
            Assert.True(ret == TransportReturnCode.SUCCESS);
            Assert.Null(error);

            mockChannel.GetNetworkBuffer().Flip();

            expectedBuffer.Data.WriteAt(0, (short)(ripcMsgLength));  // Populate RIPC message length
            expectedBuffer.Data.WriteAt(2, (byte)RipcFlags.DATA);  // Populate RIPC message header

            expectedBuffer.Data.Flip();

            expectedBuffer2.Data.WriteAt(0, (short)(ripcMsgLength2));  // Populate RIPC message length
            expectedBuffer2.Data.WriteAt(2, (byte)RipcFlags.DATA);  // Populate RIPC message header

            expectedBuffer2.Data.Flip();

            // Compares the contents of the buffers
            Assert.Equal(0, mockChannel.GetNetworkBuffer().Position);
            Assert.Equal(ripcMsgLength + ripcMsgLength2, mockChannel.GetNetworkBuffer().Limit);
            Assert.Equal(expectedBuffer.Data.Limit + expectedBuffer2.Data.Limit, mockChannel.GetNetworkBuffer().Limit);

            TransportBuffer totalExpectedBuffers = new TransportBuffer(new Common.ByteBuffer(100), RipcDataMessage.HeaderSize, false);
            totalExpectedBuffers.Data.Put(expectedBuffer.Data).Put(expectedBuffer2.Data);

            Assert.True(totalExpectedBuffers.Data.Equals(mockChannel.GetNetworkBuffer()));
            Assert.Equal(TransportReturnCode.SUCCESS, channel.Close(out error));

            Transport.Uninitialize();
        }

        [Fact]
        public void TransportWriteFlushDefaultStrategyTest()
        {
            // DEFAULT_PRIORITY_FLUSH_ORDER = "HMHLHM"
            ConnectOptions conOpt = new ConnectOptions();
            MockChannel mockChannel = new MockChannel();

            InitArgs initArgs = new InitArgs
            {
                GlobalLocking = true
            };

            Transport.Initialize(initArgs, out Error error);

            ChannelBase channel = new ChannelBase(conOpt, mockChannel, ChannelState.ACTIVE, 8192);

            ITransportBuffer messageBuffer = channel.GetBuffer(10, false, out error);

            Assert.NotNull(messageBuffer);
            Assert.Null(error);

            byte[] dataByteArray = Encoding.ASCII.GetBytes("LOW");

            TransportBuffer expectedBuffer = new TransportBuffer(new Common.ByteBuffer(10), RipcDataMessage.HeaderSize, true);
            expectedBuffer.Data.Put(dataByteArray);

            int ripcMsgLength = expectedBuffer.Length + RipcDataMessage.HeaderSize;

            messageBuffer.Data.Put(dataByteArray);

            TransportReturnCode ret = channel.Write(messageBuffer, new WriteArgs() { Priority = WritePriorities.LOW }, out error);

            Assert.True(ret > TransportReturnCode.SUCCESS && (int)ret == ripcMsgLength);
            Assert.Null(error);

            ITransportBuffer messageBuffer2 = channel.GetBuffer(10, false, out error);

            Assert.NotNull(messageBuffer2);
            Assert.Null(error);

            byte[] dataByteArray2 = Encoding.ASCII.GetBytes("MEDIUM");

            TransportBuffer expectedBuffer2 = new TransportBuffer(new Common.ByteBuffer(10), RipcDataMessage.HeaderSize, true);
            expectedBuffer2.Data.Put(dataByteArray2);

            int ripcMsgLength2 = expectedBuffer2.Length + RipcDataMessage.HeaderSize;

            messageBuffer2.Data.Put(dataByteArray2);

            ret = channel.Write(messageBuffer2, new WriteArgs() { Priority = WritePriorities.MEDIUM }, out error);

            Assert.True(ret > TransportReturnCode.SUCCESS && (int)ret == (ripcMsgLength + ripcMsgLength2));
            Assert.Null(error);

            ITransportBuffer messageBuffer3 = channel.GetBuffer(10, false, out error);

            Assert.NotNull(messageBuffer3);
            Assert.Null(error);

            byte[] dataByteArray3 = Encoding.ASCII.GetBytes("HIGH");

            TransportBuffer expectedBuffer3 = new TransportBuffer(new Common.ByteBuffer(10), RipcDataMessage.HeaderSize, true);
            expectedBuffer3.Data.Put(dataByteArray3);

            int ripcMsgLength3 = expectedBuffer3.Length + RipcDataMessage.HeaderSize;

            messageBuffer3.Data.Put(dataByteArray3);

            ret = channel.Write(messageBuffer3, new WriteArgs() { Priority = WritePriorities.HIGH }, out error);

            Assert.True(ret > TransportReturnCode.SUCCESS && (int)ret == (ripcMsgLength + ripcMsgLength2 + ripcMsgLength3));
            Assert.Null(error);

            ret = channel.Flush(out error);
            Assert.True(ret == TransportReturnCode.SUCCESS);
            Assert.Null(error);

            mockChannel.GetNetworkBuffer().Flip();

            expectedBuffer.Data.WriteAt(0, (short)(ripcMsgLength));  // Populate RIPC message length
            expectedBuffer.Data.WriteAt(2, (byte)RipcFlags.DATA);  // Populate RIPC message header
            expectedBuffer.Data.Flip();

            expectedBuffer2.Data.WriteAt(0, (short)(ripcMsgLength2));  // Populate RIPC message length
            expectedBuffer2.Data.WriteAt(2, (byte)RipcFlags.DATA);  // Populate RIPC message header
            expectedBuffer2.Data.Flip();

            expectedBuffer3.Data.WriteAt(0, (short)(ripcMsgLength3));  // Populate RIPC message length
            expectedBuffer3.Data.WriteAt(2, (byte)RipcFlags.DATA);  // Populate RIPC message header
            expectedBuffer3.Data.Flip();

            // Compares the contents of the buffers
            Assert.Equal(0, mockChannel.GetNetworkBuffer().Position);
            Assert.Equal(ripcMsgLength + ripcMsgLength2 + ripcMsgLength3, mockChannel.GetNetworkBuffer().Limit);
            Assert.Equal(expectedBuffer.Data.Limit + expectedBuffer2.Data.Limit + expectedBuffer3.Data.Limit, mockChannel.GetNetworkBuffer().Limit);

            TransportBuffer totalExpectedBuffers = new TransportBuffer(new Common.ByteBuffer(100), RipcDataMessage.HeaderSize, false);
            totalExpectedBuffers.Data.Put(expectedBuffer3.Data).Put(expectedBuffer2.Data).Put(expectedBuffer.Data);

            Assert.True(totalExpectedBuffers.Data.Equals(mockChannel.GetNetworkBuffer()));
            Assert.Equal(TransportReturnCode.SUCCESS, channel.Close(out error));

            Transport.Uninitialize();
        }

        [Fact]
        public void TransportWriteFlushConfigureStrategyTest()
        {
            // Override the flush strategy to "LMH"
            ConnectOptions conOpt = new ConnectOptions();
            MockChannel mockChannel = new MockChannel();

            InitArgs initArgs = new InitArgs
            {
                GlobalLocking = true
            };

            Transport.Initialize(initArgs, out Error error);

            ChannelBase channel = new ChannelBase(conOpt, mockChannel, ChannelState.ACTIVE, 8192, "LMH");

            ITransportBuffer messageBuffer = channel.GetBuffer(10, false, out error);

            Assert.NotNull(messageBuffer);
            Assert.Null(error);

            byte[] dataByteArray = Encoding.ASCII.GetBytes("LOW");

            TransportBuffer expectedBuffer = new TransportBuffer(new Common.ByteBuffer(10), RipcDataMessage.HeaderSize, true);
            expectedBuffer.Data.Put(dataByteArray);

            int ripcMsgLength = expectedBuffer.Length + RipcDataMessage.HeaderSize;

            messageBuffer.Data.Put(dataByteArray);

            TransportReturnCode ret = channel.Write(messageBuffer, new WriteArgs() { Priority = WritePriorities.LOW }, out error);

            Assert.True(ret > TransportReturnCode.SUCCESS && (int)ret == ripcMsgLength);
            Assert.Null(error);

            ITransportBuffer messageBuffer2 = channel.GetBuffer(10, false, out error);

            Assert.NotNull(messageBuffer2);
            Assert.Null(error);

            byte[] dataByteArray2 = Encoding.ASCII.GetBytes("MEDIUM");

            TransportBuffer expectedBuffer2 = new TransportBuffer(new Common.ByteBuffer(10), RipcDataMessage.HeaderSize, true);
            expectedBuffer2.Data.Put(dataByteArray2);

            int ripcMsgLength2 = expectedBuffer2.Length + RipcDataMessage.HeaderSize;

            messageBuffer2.Data.Put(dataByteArray2);

            ret = channel.Write(messageBuffer2, new WriteArgs() { Priority = WritePriorities.MEDIUM }, out error);

            Assert.True(ret > TransportReturnCode.SUCCESS && (int)ret == (ripcMsgLength + ripcMsgLength2));
            Assert.Null(error);

            ITransportBuffer messageBuffer3 = channel.GetBuffer(10, false, out error);

            Assert.NotNull(messageBuffer3);
            Assert.Null(error);

            byte[] dataByteArray3 = Encoding.ASCII.GetBytes("HIGH");

            TransportBuffer expectedBuffer3 = new TransportBuffer(new Common.ByteBuffer(10), RipcDataMessage.HeaderSize, true);
            expectedBuffer3.Data.Put(dataByteArray3);

            int ripcMsgLength3 = expectedBuffer3.Length + RipcDataMessage.HeaderSize;

            messageBuffer3.Data.Put(dataByteArray3);

            ret = channel.Write(messageBuffer3, new WriteArgs() { Priority = WritePriorities.HIGH }, out error);

            Assert.True(ret > TransportReturnCode.SUCCESS && (int)ret == (ripcMsgLength + ripcMsgLength2 + ripcMsgLength3));
            Assert.Null(error);

            ret = channel.Flush(out error);
            Assert.True(ret == TransportReturnCode.SUCCESS);
            Assert.Null(error);

            mockChannel.GetNetworkBuffer().Flip();

            expectedBuffer.Data.WriteAt(0, (short)(ripcMsgLength));  // Populate RIPC message length
            expectedBuffer.Data.WriteAt(2, (byte)RipcFlags.DATA);  // Populate RIPC message header
            expectedBuffer.Data.Flip();

            expectedBuffer2.Data.WriteAt(0, (short)(ripcMsgLength2));  // Populate RIPC message length
            expectedBuffer2.Data.WriteAt(2, (byte)RipcFlags.DATA);  // Populate RIPC message header
            expectedBuffer2.Data.Flip();

            expectedBuffer3.Data.WriteAt(0, (short)(ripcMsgLength3));  // Populate RIPC message length
            expectedBuffer3.Data.WriteAt(2, (byte)RipcFlags.DATA);  // Populate RIPC message header
            expectedBuffer3.Data.Flip();

            // Compares the contents of the buffers
            Assert.Equal(0, mockChannel.GetNetworkBuffer().Position);
            Assert.Equal(ripcMsgLength + ripcMsgLength2 + ripcMsgLength3, mockChannel.GetNetworkBuffer().Limit);
            Assert.Equal(expectedBuffer.Data.Limit + expectedBuffer2.Data.Limit + expectedBuffer3.Data.Limit, mockChannel.GetNetworkBuffer().Limit);

            TransportBuffer totalExpectedBuffers = new TransportBuffer(new Common.ByteBuffer(100), RipcDataMessage.HeaderSize, false);
            totalExpectedBuffers.Data.Put(expectedBuffer.Data).Put(expectedBuffer2.Data).Put(expectedBuffer3.Data); 

            Assert.True(totalExpectedBuffers.Data.Equals(mockChannel.GetNetworkBuffer()));
            Assert.Equal(TransportReturnCode.SUCCESS, channel.Close(out error));

            Transport.Uninitialize();
        }

        [Fact]
        public void TransportWriteFlushHighWaterMarkTest()
        {
            ConnectOptions conOpt = new ConnectOptions();
            MockChannel mockChannel = new MockChannel();

            InitArgs initArgs = new InitArgs
            {
                GlobalLocking = true
            };

            Transport.Initialize(initArgs, out Error error);

            ChannelBase channel = new ChannelBase(conOpt, mockChannel, ChannelState.ACTIVE, 20);

            ITransportBuffer messageBuffer = channel.GetBuffer(100, false, out error);

            Assert.NotNull(messageBuffer);
            Assert.Null(error);

            byte[] dataByteArray = Encoding.ASCII.GetBytes("ABCDEF");

            TransportBuffer expectedBuffer = new TransportBuffer(new Common.ByteBuffer(50), RipcDataMessage.HeaderSize, true);
            expectedBuffer.Data.Put(dataByteArray);

            int ripcMsgLength = expectedBuffer.Length + RipcDataMessage.HeaderSize;

            messageBuffer.Data.Put(dataByteArray);

            TransportReturnCode ret = channel.Write(messageBuffer, new WriteArgs(), out error);

            Assert.True(ret > TransportReturnCode.SUCCESS && (int)ret == ripcMsgLength);
            Assert.Null(error);

            ITransportBuffer messageBuffer2 = channel.GetBuffer(100, false, out error);

            Assert.NotNull(messageBuffer2);
            Assert.Null(error);

            byte[] dataByteArray2 = Encoding.ASCII.GetBytes("123456789");

            TransportBuffer expectedBuffer2 = new TransportBuffer(new Common.ByteBuffer(50), RipcDataMessage.HeaderSize, true);
            expectedBuffer2.Data.Put(dataByteArray2);

            int ripcMsgLength2 = expectedBuffer2.Length + RipcDataMessage.HeaderSize;

            messageBuffer2.Data.Put(dataByteArray2);

            // Flush to network as the buffer size reaches the high water mark
            ret = channel.Write(messageBuffer2, new WriteArgs(), out error);

            Assert.True(ret == TransportReturnCode.SUCCESS);
            Assert.Null(error);

            mockChannel.GetNetworkBuffer().Flip();

            expectedBuffer.Data.WriteAt(0, (short)(ripcMsgLength));  // Populate RIPC message length
            expectedBuffer.Data.WriteAt(2, (byte)RipcFlags.DATA);  // Populate RIPC message header

            expectedBuffer.Data.Flip();

            expectedBuffer2.Data.WriteAt(0, (short)(ripcMsgLength2));  // Populate RIPC message length
            expectedBuffer2.Data.WriteAt(2, (byte)RipcFlags.DATA);  // Populate RIPC message header

            expectedBuffer2.Data.Flip();

            // Compares the contents of the buffers
            Assert.Equal(0, mockChannel.GetNetworkBuffer().Position);
            Assert.Equal(ripcMsgLength + ripcMsgLength2, mockChannel.GetNetworkBuffer().Limit);
            Assert.Equal(expectedBuffer.Data.Limit + expectedBuffer2.Data.Limit, mockChannel.GetNetworkBuffer().Limit);

            TransportBuffer totalExpectedBuffers = new TransportBuffer(new Common.ByteBuffer(100), RipcDataMessage.HeaderSize, false);
            totalExpectedBuffers.Data.Put(expectedBuffer.Data).Put(expectedBuffer2.Data);

            Assert.True(totalExpectedBuffers.Data.Equals(mockChannel.GetNetworkBuffer()));
            Assert.Equal(TransportReturnCode.SUCCESS, channel.Close(out error));

            Transport.Uninitialize();
        }

        [Fact]
        public void TransportWriteWouldBlockTest()
        {
            ConnectOptions conOpt = new ConnectOptions();
            MockChannel mockChannel = new MockChannel();

            InitArgs initArgs = new InitArgs
            {
                GlobalLocking = true
            };

            Transport.Initialize(initArgs, out Error error);

            ChannelBase channel = new ChannelBase(conOpt, mockChannel, ChannelState.ACTIVE, 8192);

            ITransportBuffer messageBuffer = channel.GetBuffer(500, false, out error);

            Assert.NotNull(messageBuffer);
            Assert.Null(error);

            byte[] dataByteArray = Encoding.ASCII.GetBytes("ABCDEFGHIJKL123456789");

            messageBuffer.Data.Put(dataByteArray);

            mockChannel.SocketWriteAction = MockChannel.WriteActions.WOULD_BLOCK;

            TransportReturnCode ret = channel.Write(messageBuffer, new WriteArgs()
            {
                Flags = WriteFlags.DIRECT_SOCKET_WRITE
            }, out error);

            Assert.Equal(TransportReturnCode.WRITE_FLUSH_FAILED, ret);
            Assert.NotNull(error);
            Assert.Equal(TransportReturnCode.WRITE_FLUSH_FAILED, error.ErrorId);
            Assert.Equal((int)SocketError.WouldBlock, error.SysError);
            Assert.Equal("An operation on a nonblocking socket cannot be completed immediately", error.Text);

            mockChannel.SocketWriteAction = MockChannel.WriteActions.NORMAL;

            // Flush again
            ret = channel.Flush(out error);
            Assert.True(ret == TransportReturnCode.SUCCESS);
            Assert.Null(error);

            mockChannel.GetNetworkBuffer().Flip();

            TransportBuffer expectedBuffer = new TransportBuffer(new Common.ByteBuffer(100), RipcDataMessage.HeaderSize, true);
            expectedBuffer.Data.Put(dataByteArray);

            int ripcMsgLength = expectedBuffer.Length + RipcDataMessage.HeaderSize;

            expectedBuffer.Data.WriteAt(0, (short)(ripcMsgLength));  // Populate RIPC message length
            expectedBuffer.Data.WriteAt(2, (byte)RipcFlags.DATA);  // Populate RIPC message header

            expectedBuffer.Data.Flip();

            // Compares the contents of the two buffers
            Assert.Equal(0, mockChannel.GetNetworkBuffer().Position);
            Assert.Equal(ripcMsgLength, mockChannel.GetNetworkBuffer().Limit);
            Assert.Equal(expectedBuffer.Data.Limit, mockChannel.GetNetworkBuffer().Limit);
            Assert.True(expectedBuffer.Data.Equals(mockChannel.GetNetworkBuffer()));
            Assert.Equal(TransportReturnCode.SUCCESS, channel.Close(out error));

            Transport.Uninitialize();
        }

        [Fact]
        public void TransportWriteFailedTest()
        {
            ConnectOptions conOpt = new ConnectOptions();
            MockChannel mockChannel = new MockChannel();

            InitArgs initArgs = new InitArgs
            {
                GlobalLocking = true
            };

            Transport.Initialize(initArgs, out Error error);

            ChannelBase channel = new ChannelBase(conOpt, mockChannel, ChannelState.ACTIVE, 8192);

            ITransportBuffer messageBuffer = channel.GetBuffer(500, false, out error);

            Assert.NotNull(messageBuffer);
            Assert.Null(error);

            byte[] dataByteArray = Encoding.ASCII.GetBytes("ABCDEFGHIJKL123456789");

            messageBuffer.Data.Put(dataByteArray);

            mockChannel.SocketWriteAction = MockChannel.WriteActions.ERROR;

            TransportReturnCode ret = channel.Write(messageBuffer, new WriteArgs()
            {
                Flags = WriteFlags.DIRECT_SOCKET_WRITE
            }, out error);

            Assert.Equal(TransportReturnCode.FAILURE, ret);
            Assert.NotNull(error);
            Assert.Equal(TransportReturnCode.FAILURE, error.ErrorId);
            Assert.Equal((int)SocketError.ConnectionReset, error.SysError);
            Assert.Equal("The connection was reset by the remote peer", error.Text);
            Assert.Equal(ChannelState.CLOSED, channel.State);
          
        }
    }

    #endregion

    #region Transport Server Test

    [CollectionDefinition("Non-Parallel Collection", DisableParallelization = true)]
    public class NonParallelCollectionDefinitionClass
    {
    }

    [Collection("Non-Parallel Collection")]
    [Category("Unit")]
    [Category("Transport")]
    public class TransportServerTests
    {
        [Fact]
        public void ServerBindToAPortAndCloseTest()
        {
            InitArgs initArgs = new InitArgs
            {
                GlobalLocking = true
            };

            Assert.Equal(TransportReturnCode.SUCCESS, Transport.Initialize(initArgs, out Error error));

            BindOptions bindOptions = new BindOptions
            {
                ServiceName = "15999",
                ServerBlocking = false,
                ConnectionType = ConnectionType.SOCKET,
                ProtocolType = ProtocolType.RWF,
                UserSpecObject = this,
                InterfaceName = "localhost"
            };

            var server = Transport.Bind(bindOptions, out error);

            Assert.NotNull(server);
            Assert.Equal(ChannelState.ACTIVE, server.State);
            Assert.Equal(int.Parse(bindOptions.ServiceName), server.PortNumber);
            Assert.Equal(bindOptions.ConnectionType, server.ConnectionType);
            Assert.Equal(this, server.UserSpecObject);
            Assert.NotNull(server.Socket);
            Assert.True(server.Socket.IsBound);
            Assert.Equal(this, server.UserSpecObject);

            server.Close(out error);
            Assert.Equal(ChannelState.CLOSED, server.State);
            Assert.Null(server.Socket);

            Assert.Equal(TransportReturnCode.SUCCESS, Transport.Uninitialize());
        }

        [Fact]
        public void ServerAcceptChannelAndCloseTest()
        {
            InitArgs initArgs = new InitArgs
            {
                GlobalLocking = true
            };

            Assert.Equal(TransportReturnCode.SUCCESS, Transport.Initialize(initArgs, out Error error));

            string serviceName = "19999";

            BindOptions bindOptions = new BindOptions
            {
                ServiceName = serviceName,
                ServerBlocking = false,
                ConnectionType = ConnectionType.SOCKET,
                ProtocolType = ProtocolType.RWF,
                UserSpecObject = this,
                InterfaceName = "localhost",
                MajorVersion = Codec.Codec.MajorVersion(),
                MinorVersion = Codec.Codec.MinorVersion(),
            };

            var server = Transport.Bind(bindOptions, out error);

            Assert.NotNull(server);
            Assert.Equal(ChannelState.ACTIVE, server.State);
            Assert.True(server.Socket.IsBound);

            Func<IServer, IChannel> acceptChannel = new Func<IServer, IChannel>((acceptServer) =>
            {
                AcceptOptions acceptOptions = new AcceptOptions();
                IChannel serverChannel = null;

                bool pollResult = acceptServer.Socket.Poll(-1, SelectMode.SelectRead);

                Assert.True(pollResult);

                serverChannel = acceptServer.Accept(acceptOptions, out Error acceptError);
                Assert.Null(acceptError);

                return serverChannel;
            });

            Func<IChannel> connectChannel = new Func<IChannel>(() =>
           {
               IChannel returnChannel = null;
               ConnectOptions connectOptions = new ConnectOptions
               {
                   Blocking = false,
                   ConnectionType = ConnectionType.SOCKET,
                   MajorVersion = Codec.Codec.MajorVersion(),
                   MinorVersion = Codec.Codec.MinorVersion(),
                   ProtocolType = ProtocolType.RWF,
                   UserSpecObject = this
               };
               connectOptions.UnifiedNetworkInfo.Address = "localhost";
               connectOptions.UnifiedNetworkInfo.ServiceName = serviceName;

               returnChannel = Transport.Connect(connectOptions, out error);
               Assert.Null(error);

               return returnChannel;
           });

            IChannel srvChannel = null;
            IChannel channel = null;

            Task acceptTask = Task.Factory.StartNew(() => { srvChannel = acceptChannel(server); });
            Task connectTask = Task.Factory.StartNew(() => { channel = connectChannel(); });

            Task.WaitAll(new[] { acceptTask, connectTask });

            Assert.NotNull(srvChannel);
            Assert.NotNull(channel);
            Assert.Equal(ChannelState.INITIALIZING, srvChannel.State);
            Assert.Equal(this, srvChannel.UserSpecObject);
            Assert.NotNull(srvChannel.Socket);
            Assert.True(srvChannel.Socket.Connected);

            Assert.Equal(ChannelState.INITIALIZING, channel.State);
            Assert.Equal(this, channel.UserSpecObject);
            Assert.NotNull(channel.Socket);
            Assert.True(channel.Socket.Connected);

            Assert.Equal(TransportReturnCode.SUCCESS, srvChannel.Close(out error));
            Assert.Equal(ChannelState.INACTIVE, srvChannel.State);

            Assert.Equal(TransportReturnCode.SUCCESS, channel.Close(out error));
            Assert.Equal(ChannelState.INACTIVE, channel.State);

            Assert.Equal(TransportReturnCode.SUCCESS, server.Close(out error));
            Assert.Equal(ChannelState.CLOSED, server.State);
            Assert.Null(server.Socket);

            Assert.Equal(TransportReturnCode.SUCCESS, Transport.Uninitialize());
        }

        private void ServerAndClientRipcHandShake(ConnectionsVersions connectionsVersion, bool nakMount = false, bool blocking = false)
        {
            InitArgs initArgs = new InitArgs
            {
                GlobalLocking = blocking ? false : true
            };

            Assert.Equal(TransportReturnCode.SUCCESS, Transport.Initialize(initArgs, out Error error));

            string serviceName = "19999";

            BindOptions bindOptions = new BindOptions
            {
                ServiceName = serviceName,
                ChannelIsBlocking = blocking,
                ConnectionType = ConnectionType.SOCKET,
                ProtocolType = ProtocolType.RWF,
                UserSpecObject = this,
                InterfaceName = "localhost",
                MajorVersion = Codec.Codec.MajorVersion(),
                MinorVersion = Codec.Codec.MinorVersion(),
            };

            var server = Transport.Bind(bindOptions, out error);

            Assert.NotNull(server);
            Assert.Equal(ChannelState.ACTIVE, server.State);
            Assert.True(server.Socket.IsBound);

            Func<IServer, IChannel> acceptChannel = new Func<IServer, IChannel>((acceptServer) =>
            {
                AcceptOptions acceptOptions = new AcceptOptions();
                IChannel serverChannel = null;

                bool pollResult = acceptServer.Socket.Poll(-1, SelectMode.SelectRead);
                Assert.True(pollResult);

                acceptOptions.NakMount = nakMount;

                serverChannel = acceptServer.Accept(acceptOptions, out Error acceptError);
                Assert.Null(acceptError);

                return serverChannel;
            });

            Func<IChannel> connectChannel = new Func<IChannel>(() =>
            {
                IChannel returnChannel = null;
                ConnectOptions connectOptions = new ConnectOptions
                {
                    Blocking = blocking,
                    ConnectionType = ConnectionType.SOCKET,
                    MajorVersion = Codec.Codec.MajorVersion(),
                    MinorVersion = Codec.Codec.MinorVersion(),
                    ProtocolType = ProtocolType.RWF,
                    UserSpecObject = this
                };
                connectOptions.UnifiedNetworkInfo.Address = "localhost";
                connectOptions.UnifiedNetworkInfo.ServiceName = serviceName;

                returnChannel = Transport.Connect(connectOptions, out error);
                Assert.Null(error);

                return returnChannel;
            });

            IChannel channel = null, srvChannel = null;

            Task acceptTask = Task.Factory.StartNew(() => { srvChannel = acceptChannel(server); });
            Task connectTask = Task.Factory.StartNew(() => { channel = connectChannel(); });

            Task.WaitAll(new[] { acceptTask, connectTask });

            if (blocking == false)
            {
                /* Non blocking mode */
                Assert.NotNull(srvChannel);
                Assert.Equal(ChannelState.INITIALIZING, srvChannel.State);
                Assert.NotNull(srvChannel.Socket);
                Assert.NotNull(channel);
                Assert.Equal(ChannelState.INITIALIZING, channel.State);
                Assert.Equal(this, channel.UserSpecObject);
                Assert.NotNull(channel.Socket);
                Assert.True(channel.Socket.Connected);
            }
            else
            {
                Assert.NotNull(srvChannel);
                Assert.Equal(ChannelState.ACTIVE, srvChannel.State);
                Assert.NotNull(srvChannel.Socket);
                Assert.NotNull(channel);
                Assert.Equal(ChannelState.ACTIVE, channel.State);
                Assert.Equal(this, channel.UserSpecObject);
                Assert.NotNull(channel.Socket);
                Assert.True(channel.Socket.Connected);

                Assert.Equal(TransportReturnCode.SUCCESS, channel.Close(out error));
                Assert.Equal(ChannelState.CLOSED, channel.State);

                Assert.Equal(TransportReturnCode.SUCCESS, srvChannel.Close(out error));
                Assert.Equal(ChannelState.CLOSED, channel.State);

                Assert.Equal(TransportReturnCode.SUCCESS, server.Close(out error));
                Assert.Equal(ChannelState.CLOSED, server.State);

                Assert.Equal(TransportReturnCode.SUCCESS, Transport.Uninitialize());
                return;
            }

            Func<IChannel, bool, TransportReturnCode> initChannel = new Func<IChannel, bool, TransportReturnCode>((channelArg, isClient) =>
            {
                bool exit = false;
                InProgInfo inProg = new InProgInfo();
                TransportReturnCode initRet;
                Error initError;

                if(blocking)
                {
                    if (isClient)
                    {
                        initRet = channelArg.Init(inProg, out initError);
                    }
                    else
                    {
                        channelArg.Socket.Poll(-1, SelectMode.SelectRead);

                        initRet = channelArg.Init(inProg, out initError);
                    }

                    return initRet;
                }

                do
                {
                    if (isClient)
                    {
                        initRet = channelArg.Init(inProg, out initError);
                        Assert.Null(initError);

                        if (initRet == TransportReturnCode.SUCCESS || initRet == TransportReturnCode.FAILURE)
                        {
                            Assert.True(false); // Unexpected return code.
                        }
                    }

                    channelArg.Socket.Poll(-1, SelectMode.SelectRead);

                    initRet = channelArg.Init(inProg, out initError);

                    if (nakMount)
                    {
                        Assert.NotNull(initError);

                        if(isClient)
                        {
                            Assert.Equal("Connection refused.", initError.Text);
                        }
                        else
                        {
                            Assert.Equal("Rejected the connection request by the server.", initError.Text);
                        }

                        return initRet;
                    }
                    else
                    {
                        Assert.Null(initError);
                    }

                    if (initRet == TransportReturnCode.SUCCESS || initRet == TransportReturnCode.FAILURE)
                    {
                        exit = true;
                    }
                    else if (initRet == TransportReturnCode.CHAN_INIT_IN_PROGRESS)
                    {
                        continue;
                    }
                    else
                    {
                        Assert.True(false); // Unexpected return code.
                    }

                }
                while (!exit);

                return initRet;
            });

            TransportReturnCode srvHandShakeRet = TransportReturnCode.SUCCESS, clientHandShakeRet = TransportReturnCode.SUCCESS;

            // Overrides the default starting RIPC version
            if(connectionsVersion != ConnectionsVersions.VERSION14)
                ((ChannelBase)channel).StartingConnectVersion(connectionsVersion);

            Task serverHandShake = Task.Factory.StartNew(() => { srvHandShakeRet = initChannel(srvChannel, false); });
            Task clientHandShake = Task.Factory.StartNew(() => { clientHandShakeRet = initChannel(channel, true); });

            Task.WaitAll(new[] { serverHandShake, clientHandShake });

            if (nakMount == false)
            {
                Assert.Equal(TransportReturnCode.SUCCESS, srvHandShakeRet);
                Assert.Equal(TransportReturnCode.SUCCESS, clientHandShakeRet);
                Assert.Equal(ChannelState.ACTIVE, srvChannel.State);
                Assert.Equal(ChannelState.ACTIVE, channel.State);

                Assert.Equal(TransportReturnCode.SUCCESS, srvChannel.Close(out error));
                Assert.Equal(ChannelState.CLOSED, srvChannel.State);

                Assert.Equal(TransportReturnCode.SUCCESS, channel.Close(out error));
                Assert.Equal(ChannelState.CLOSED, channel.State);

                Assert.Equal(TransportReturnCode.SUCCESS, server.Close(out error));
                Assert.Equal(ChannelState.CLOSED, server.State);
            }
            else
            {
                Assert.Equal(TransportReturnCode.FAILURE, srvHandShakeRet);
                Assert.Equal(TransportReturnCode.CHAN_INIT_REFUSED, clientHandShakeRet);
                Assert.Equal(ChannelState.INITIALIZING, srvChannel.State);
                Assert.Equal(ChannelState.INITIALIZING, channel.State);

                Assert.Equal(TransportReturnCode.SUCCESS, srvChannel.Close(out error));
                Assert.Equal(ChannelState.INACTIVE, srvChannel.State);

                Assert.Equal(TransportReturnCode.SUCCESS, channel.Close(out error));
                Assert.Equal(ChannelState.INACTIVE, channel.State);

                Assert.Equal(TransportReturnCode.SUCCESS, server.Close(out error));
                Assert.Equal(ChannelState.CLOSED, server.State);
            }

            Assert.Equal(TransportReturnCode.SUCCESS, Transport.Uninitialize());
        }

        [Fact]
        public void RipcHandShakeVersion14AckTest()
        {
            ServerAndClientRipcHandShake(ConnectionsVersions.VERSION14);
        }

        [Fact]
        public void RipcHandShakeVersion13AckTest()
        {
            ServerAndClientRipcHandShake(ConnectionsVersions.VERSION13);
        }

        [Fact]
        public void RipcHandShakeVersion12AckTest()
        {
            ServerAndClientRipcHandShake(ConnectionsVersions.VERSION12);
        }

        [Fact]
        public void RipcHandShakeVersion11AckTest()
        {
            ServerAndClientRipcHandShake(ConnectionsVersions.VERSION11);
        }

        [Fact]
        public void RipcHandShakeVersion14NckTest()
        {
            ServerAndClientRipcHandShake(ConnectionsVersions.VERSION14, true);
        }

        [Fact]
        public void RipcHandShakeVersion13NckTest()
        {
            ServerAndClientRipcHandShake(ConnectionsVersions.VERSION13, true);
        }

        [Fact]
        public void RipcHandShakeVersion12NckTest()
        {
            ServerAndClientRipcHandShake(ConnectionsVersions.VERSION12, true);
        }

        [Fact]
        public void RipcHandShakeVersion11NckTest()
        {
            ServerAndClientRipcHandShake(ConnectionsVersions.VERSION11, true);
        }

        [Fact]
        public void RipcHandShakeVersion14AckBlockingTest()
        {
            ServerAndClientRipcHandShake(ConnectionsVersions.VERSION14, false, true);
        }

        [Fact]
        public void RipcHandShakeVersion13AckBlockingTest()
        {
            ServerAndClientRipcHandShake(ConnectionsVersions.VERSION13, false, true);
        }

        [Fact]
        public void RipcHandShakeVersion12AckBlockingTest()
        {
            ServerAndClientRipcHandShake(ConnectionsVersions.VERSION12, false, true);
        }

        [Fact]
        public void RipcHandShakeVersion11AckBlockingTest()
        {
            ServerAndClientRipcHandShake(ConnectionsVersions.VERSION11, false, true);
        }

        List<TlsCipherSuite> cipherSuites = new List<TlsCipherSuite>();

        public TransportServerTests()
        {
            cipherSuites.Add(TlsCipherSuite.TLS_ECDHE_ECDSA_WITH_AES_256_GCM_SHA384);
            cipherSuites.Add(TlsCipherSuite.TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256);
            cipherSuites.Add(TlsCipherSuite.TLS_ECDHE_RSA_WITH_AES_256_GCM_SHA384);
            cipherSuites.Add(TlsCipherSuite.TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256);
        }

        private void ServerEncryptionBindToAPortWithInvalid(string certificate, string privateKey)
        {
            InitArgs initArgs = new InitArgs
            {
                GlobalLocking = true
            };

            Assert.Equal(TransportReturnCode.SUCCESS, Transport.Initialize(initArgs, out Error error));

            BindOptions bindOptions = new BindOptions
            {
                ServiceName = "16999",
                ServerBlocking = false,
                ConnectionType = ConnectionType.ENCRYPTED,
                ProtocolType = ProtocolType.RWF,
                UserSpecObject = this,
                InterfaceName = "localhost",
                BindEncryptionOpts =
                {
                    EncryptionProtocolFlags = EncryptionProtocolFlags.ENC_TLSV1_2,
                    ServerCertificate = certificate,
                    ServerPrivateKey = privateKey,
                }
            };

            var server = Transport.Bind(bindOptions, out error);

            Assert.Null(server);
            Assert.NotNull(error);
            Assert.StartsWith("Failed to validate server's certificate and private key files.", error.Text);

            Assert.Equal(TransportReturnCode.SUCCESS, Transport.Uninitialize());
        }

        [Fact]
        public void ServerEncryptionBindToAPortWithInvalidCertTest()
        {
            ServerEncryptionBindToAPortWithInvalid("certificate.invalid.crt", "certificate.invalid.crt");
        }

        [Fact]
        public void ServerEncryptionBindToAPortWithOutSpecifyingCertTest()
        {
            ServerEncryptionBindToAPortWithInvalid(null, null);
        }


        private void ServerEncryptionAcceptChannelAndClose(EncryptionProtocolFlags protocolFlags, bool clientEncrypted = true)
        {
            InitArgs initArgs = new InitArgs
            {
                GlobalLocking = true
            };

            Assert.Equal(TransportReturnCode.SUCCESS, Transport.Initialize(initArgs, out Error error));

            string serviceName = "17999";

            BindOptions bindOptions = new BindOptions
            {
                ServiceName = serviceName,
                ServerBlocking = false,
                ConnectionType = ConnectionType.ENCRYPTED,
                ProtocolType = ProtocolType.RWF,
                UserSpecObject = this,
                InterfaceName = "localhost",
                MajorVersion = Codec.Codec.MajorVersion(),
                MinorVersion = Codec.Codec.MinorVersion(),
                BindEncryptionOpts =
                {
                    EncryptionProtocolFlags = protocolFlags,
                    ServerCertificate = "certificate.test.crt",
                    ServerPrivateKey = "certificate.test.key"
                }
            };

            var server = Transport.Bind(bindOptions, out error);

            Assert.NotNull(server);
            Assert.Equal(ChannelState.ACTIVE, server.State);
            Assert.True(server.Socket.IsBound);

            Func<IServer, IChannel> acceptChannel = new Func<IServer, IChannel>((acceptServer) =>
            {
                AcceptOptions acceptOptions = new AcceptOptions();
                IChannel serverChannel = null;

                bool pollResult = acceptServer.Socket.Poll(-1, SelectMode.SelectRead);

                Assert.True(pollResult);

                serverChannel = acceptServer.Accept(acceptOptions, out Error acceptError);

                Assert.Null(acceptError);

                return serverChannel;
            });

            Func<IChannel> connectChannel = new Func<IChannel>(() =>
            {
                IChannel returnChannel = null;
                ConnectOptions connectOptions = new ConnectOptions
                {
                    Blocking = false,
                    ConnectionType = clientEncrypted == true ? ConnectionType.ENCRYPTED : ConnectionType.SOCKET,
                    MajorVersion = Codec.Codec.MajorVersion(),
                    MinorVersion = Codec.Codec.MinorVersion(),
                    ProtocolType = ProtocolType.RWF,
                    UserSpecObject = this,

                    EncryptionOpts =
                    {
                        EncryptionProtocolFlags = protocolFlags,
                        EncryptedProtocol = ConnectionType.SOCKET
                    }
                };
                connectOptions.UnifiedNetworkInfo.Address = "localhost";
                connectOptions.UnifiedNetworkInfo.ServiceName = serviceName;

                returnChannel = Transport.Connect(connectOptions, out error);

                Assert.Null(error);

                return returnChannel;
            });

            IChannel srvChannel = null;
            IChannel channel = null;

            Task acceptTask = Task.Factory.StartNew(() => { srvChannel = acceptChannel(server); });
            Task connectTask = Task.Factory.StartNew(() => { channel = connectChannel(); });

            Task.WaitAll(new[] { acceptTask, connectTask }, 7000);

            Assert.NotNull(srvChannel);
            Assert.NotNull(channel);
            Assert.Equal(ChannelState.INITIALIZING, srvChannel.State);
            Assert.Equal(this, srvChannel.UserSpecObject);
            Assert.NotNull(srvChannel.Socket);
            Assert.True(srvChannel.Socket.Connected);

            Assert.Equal(ChannelState.INITIALIZING, channel.State);
            Assert.Equal(this, channel.UserSpecObject);
            Assert.NotNull(channel.Socket);
            Assert.True(channel.Socket.Connected);

            Assert.Equal(TransportReturnCode.SUCCESS, srvChannel.Close(out error));
            Assert.Equal(ChannelState.INACTIVE, srvChannel.State);

            Assert.Equal(TransportReturnCode.SUCCESS, channel.Close(out error));
            Assert.Equal(ChannelState.INACTIVE, channel.State);

            Assert.Equal(TransportReturnCode.SUCCESS, server.Close(out error));
            Assert.Equal(ChannelState.CLOSED, server.State);
            Assert.Null(server.Socket);

            Assert.Equal(TransportReturnCode.SUCCESS, Transport.Uninitialize());
        }

        [Fact]
        public void ServerEncryptionAcceptChannelAndClose_DefaultProtocol_Test()
        {
            ServerEncryptionAcceptChannelAndClose(EncryptionProtocolFlags.ENC_NONE);
        }

        [Fact]
        public void ServerEncryptionAcceptChannelAndClose_TLSV1_2Protocol_Test()
        {
            ServerEncryptionAcceptChannelAndClose(EncryptionProtocolFlags.ENC_TLSV1_2);
        }

        [Fact]
        public void ServerEncryptionAcceptChannelAndClose_ConnectionMismatch_Test()
        {
            ServerEncryptionAcceptChannelAndClose(EncryptionProtocolFlags.ENC_NONE, false);
        }

        private void ServerAndClientEncryptionRipcHandShake(ConnectionsVersions connectionsVersion, bool blocking = false, List<TlsCipherSuite> cipherSuites = null)
        {
            bool expectedError = false;

            if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows) && cipherSuites != null)
            {
                // Windows doesn't support setting Cipher suites so we are expected an error
                expectedError = true;
            }

            InitArgs initArgs = new InitArgs
            {
                GlobalLocking = blocking ? false : true
            };

            Assert.Equal(TransportReturnCode.SUCCESS, Transport.Initialize(initArgs, out Error error));

            string serviceName = "18999";

            BindOptions bindOptions = new BindOptions
            {
                ServiceName = serviceName,
                ChannelIsBlocking = blocking,
                ConnectionType = ConnectionType.ENCRYPTED,
                ProtocolType = ProtocolType.RWF,
                UserSpecObject = this,
                InterfaceName = "localhost",
                MajorVersion = Codec.Codec.MajorVersion(),
                MinorVersion = Codec.Codec.MinorVersion(),

                BindEncryptionOpts =
                {
                    EncryptionProtocolFlags = EncryptionProtocolFlags.ENC_TLSV1_2,
                    ServerCertificate = "certificate.test.crt",
                    ServerPrivateKey = "certificate.test.key",
                    TlsCipherSuites = cipherSuites
                }
            };

            var server = Transport.Bind(bindOptions, out error);

            Assert.NotNull(server);
            Assert.Equal(ChannelState.ACTIVE, server.State);
            Assert.True(server.Socket.IsBound);

            Func<IServer, IChannel> acceptChannel = new Func<IServer, IChannel>((acceptServer) =>
            {
                AcceptOptions acceptOptions = new AcceptOptions();
                IChannel serverChannel = null;

                bool pollResult = acceptServer.Socket.Poll(-1, SelectMode.SelectRead);
                Assert.True(pollResult);

                acceptOptions.NakMount = false;

                serverChannel = acceptServer.Accept(acceptOptions, out Error acceptError);

                if (!expectedError || !blocking)
                {
                    Assert.Null(acceptError);
                }
                else
                {
                    Assert.NotNull(acceptError);
                    Assert.StartsWith("Failed to create a client encrypted channel. Reason:CipherSuitesPolicy is not supported on this platform.", acceptError.Text);
                }

                return serverChannel;
            });

            Func<IChannel> connectChannel = new Func<IChannel>(() =>
            {
                IChannel returnChannel = null;
                ConnectOptions connectOptions = new ConnectOptions
                {
                    Blocking = blocking,
                    ConnectionType = ConnectionType.ENCRYPTED,
                    MajorVersion = Codec.Codec.MajorVersion(),
                    MinorVersion = Codec.Codec.MinorVersion(),
                    ProtocolType = ProtocolType.RWF,
                    UserSpecObject = this,

                    EncryptionOpts =
                    {
                        EncryptionProtocolFlags = EncryptionProtocolFlags.ENC_TLSV1_2,
                        EncryptedProtocol = ConnectionType.SOCKET,
                        TlsCipherSuites = cipherSuites
                    }
                };
                connectOptions.UnifiedNetworkInfo.Address = "localhost";
                connectOptions.UnifiedNetworkInfo.ServiceName = serviceName;

                returnChannel = Transport.Connect(connectOptions, out error);

                if (!expectedError || !blocking)
                {
                    Assert.Null(error);
                }
                else
                {
                    Assert.NotNull(error);
                    Assert.StartsWith("Failed to create an encrypted connection to the remote endpoint. Reason:CipherSuitesPolicy is not supported on this platform.", error.Text);
                }

                return returnChannel;
            });

            IChannel channel = null, srvChannel = null;

            Task acceptTask = Task.Factory.StartNew(() => { srvChannel = acceptChannel(server); });
            Task connectTask = Task.Factory.StartNew(() => { channel = connectChannel(); });

            Task.WaitAll(new[] { acceptTask, connectTask });

            if (blocking == false)
            {
                /* Non blocking mode */
                Assert.NotNull(srvChannel);
                Assert.Equal(ChannelState.INITIALIZING, srvChannel.State);
                Assert.NotNull(srvChannel.Socket);
                Assert.NotNull(channel);
                Assert.Equal(ChannelState.INITIALIZING, channel.State);
                Assert.Equal(this, channel.UserSpecObject);
                Assert.NotNull(channel.Socket);
                Assert.True(channel.Socket.Connected);
            }
            else
            {
                if (!expectedError)
                {
                    Assert.NotNull(srvChannel);
                    Assert.Equal(ChannelState.ACTIVE, srvChannel.State);
                    Assert.NotNull(srvChannel.Socket);
                    Assert.NotNull(channel);
                    Assert.Equal(ChannelState.ACTIVE, channel.State);
                    Assert.Equal(this, channel.UserSpecObject);
                    Assert.NotNull(channel.Socket);
                    Assert.True(channel.Socket.Connected);

                    Assert.Equal(TransportReturnCode.SUCCESS, channel.Close(out error));
                    Assert.Equal(ChannelState.CLOSED, channel.State);

                    Assert.Equal(TransportReturnCode.SUCCESS, srvChannel.Close(out error));
                    Assert.Equal(ChannelState.CLOSED, channel.State);
                }
                else
                {
                    Assert.Null(srvChannel);
                    Assert.Null(channel);
                }

                Assert.Equal(TransportReturnCode.SUCCESS, server.Close(out error));
                Assert.Equal(ChannelState.CLOSED, server.State);

                Assert.Equal(TransportReturnCode.SUCCESS, Transport.Uninitialize());
                return;
            }

            Func<IChannel, bool, TransportReturnCode> initChannel = new Func<IChannel, bool, TransportReturnCode>((channelArg, isClient) =>
            {
                bool exit = false;
                InProgInfo inProg = new InProgInfo();
                TransportReturnCode initRet;
                Error initError;

                if (blocking)
                {
                    if (isClient)
                    {
                        initRet = channelArg.Init(inProg, out initError);
                    }
                    else
                    {
                        channelArg.Socket.Poll(-1, SelectMode.SelectRead);

                        initRet = channelArg.Init(inProg, out initError);
                    }

                    return initRet;
                }

                do
                {
                    if (isClient)
                    {
                        initRet = channelArg.Init(inProg, out initError);

                        if (!expectedError)
                        {
                            Assert.Null(initError);
                        }
                        else
                        {
                            Assert.NotNull(initError);
                            Assert.StartsWith("Failed to create an encrypted connection to the remote endpoint. Reason:CipherSuitesPolicy is not supported on this platform.", initError.Text);
                            return initRet;
                        }

                        if (initRet == TransportReturnCode.SUCCESS || initRet == TransportReturnCode.FAILURE)
                        {
                            Assert.True(false); // Unexpected return code.
                        }
                    }

                    channelArg.Socket.Poll(-1, SelectMode.SelectRead);

                    initRet = channelArg.Init(inProg, out initError);

                    if (!expectedError)
                    {
                        Assert.Null(initError);
                    }
                    else
                    {
                        Assert.NotNull(initError);
                        Assert.StartsWith("Failed to create a client encrypted channel. Reason:CipherSuitesPolicy is not supported on this platform.", initError.Text);
                        return initRet;
                    }

                    if (initRet == TransportReturnCode.SUCCESS || initRet == TransportReturnCode.FAILURE)
                    {
                        exit = true;
                    }
                    else if (initRet == TransportReturnCode.CHAN_INIT_IN_PROGRESS)
                    {
                        continue;
                    }
                    else
                    {
                        Assert.True(false); // Unexpected return code.
                    }

                }
                while (!exit);

                return initRet;
            });

            TransportReturnCode srvHandShakeRet = TransportReturnCode.SUCCESS, clientHandShakeRet = TransportReturnCode.SUCCESS;

            // Overrides the default starting RIPC version
            if (connectionsVersion != ConnectionsVersions.VERSION14)
                ((ChannelBase)channel).StartingConnectVersion(connectionsVersion);

            Task serverHandShake = Task.Factory.StartNew(() => { srvHandShakeRet = initChannel(srvChannel, false); });
            Task clientHandShake = Task.Factory.StartNew(() => { clientHandShakeRet = initChannel(channel, true); });

            Task.WaitAll(new[] { serverHandShake, clientHandShake });

            if (!expectedError)
            {
                Assert.Equal(TransportReturnCode.SUCCESS, srvHandShakeRet);
                Assert.Equal(TransportReturnCode.SUCCESS, clientHandShakeRet);
                Assert.Equal(ChannelState.ACTIVE, srvChannel.State);
                Assert.Equal(ChannelState.ACTIVE, channel.State);
                Assert.Equal(TransportReturnCode.SUCCESS, srvChannel.Close(out error));
                Assert.Equal(ChannelState.CLOSED, srvChannel.State);

                Assert.Equal(TransportReturnCode.SUCCESS, channel.Close(out error));
                Assert.Equal(ChannelState.CLOSED, channel.State);
            }
            else
            {
                Assert.Equal(TransportReturnCode.FAILURE, srvHandShakeRet);
                Assert.Equal(TransportReturnCode.FAILURE, clientHandShakeRet);
                Assert.Equal(ChannelState.INITIALIZING, srvChannel.State);
                Assert.Equal(ChannelState.INITIALIZING, channel.State);
                Assert.Equal(TransportReturnCode.SUCCESS, srvChannel.Close(out error));
                Assert.Equal(ChannelState.INACTIVE, srvChannel.State);

                Assert.Equal(TransportReturnCode.SUCCESS, channel.Close(out error));
                Assert.Equal(ChannelState.INACTIVE, channel.State);
            }

            Assert.Equal(TransportReturnCode.SUCCESS, server.Close(out error));
            Assert.Equal(ChannelState.CLOSED, server.State);

            Assert.Equal(TransportReturnCode.SUCCESS, Transport.Uninitialize());
        }

        [Fact]
        public void ServerAndClientEncryptionRipcHandShakeVersion14AckTest()
        {
            ServerAndClientEncryptionRipcHandShake(ConnectionsVersions.VERSION14);
        }

        [Fact]
        public void ServerAndClientEncryptionRipcHandShakeVersion13AckTest()
        {
            ServerAndClientEncryptionRipcHandShake(ConnectionsVersions.VERSION13);
        }

        [Fact]
        public void ServerAndClientEncryptionRipcHandShakeVersion12AckTest()
        {
            ServerAndClientEncryptionRipcHandShake(ConnectionsVersions.VERSION12);
        }

        [Fact]
        public void ServerAndClientEncryptionRipcHandShakeVersion11AckTest()
        {
            ServerAndClientEncryptionRipcHandShake(ConnectionsVersions.VERSION11);
        }

        [Fact]
        public void ServerAndClientEncryptionRipcHandShakeVersion14Ack_BlockingTest()
        {
            ServerAndClientEncryptionRipcHandShake(ConnectionsVersions.VERSION14, true);
        }

        [Fact]
        public void ServerAndClientEncryptionRipcHandShakeVersion13Ack_BlockingTest()
        {
            ServerAndClientEncryptionRipcHandShake(ConnectionsVersions.VERSION13, true);
        }

        [Fact]
        public void ServerAndClientEncryptionRipcHandShakeVersion12Ack_BlockingTest()
        {
            ServerAndClientEncryptionRipcHandShake(ConnectionsVersions.VERSION12, true);
        }

        [Fact]
        public void ServerAndClientEncryptionRipcHandShakeVersion11Ack_BlockingTest()
        {
            ServerAndClientEncryptionRipcHandShake(ConnectionsVersions.VERSION11, true);
        }

        [Fact]
        public void ServerAndClientEncryptionRipcHandShakeVersion14Ack_withCipherSuitesTest()
        {
            ServerAndClientEncryptionRipcHandShake(ConnectionsVersions.VERSION14, false, cipherSuites);
        }

        [Fact]
        public void ServerAndClientEncryptionRipcHandShakeVersion13Ack_withCipherSuitesTest()
        {
            ServerAndClientEncryptionRipcHandShake(ConnectionsVersions.VERSION13, false, cipherSuites);
        }

        [Fact]
        public void ServerAndClientEncryptionRipcHandShakeVersion12Ack_withCipherSuitesTest()
        {
            ServerAndClientEncryptionRipcHandShake(ConnectionsVersions.VERSION12, false, cipherSuites);
        }

        [Fact]
        public void ServerAndClientEncryptionRipcHandShakeVersion11Ack_withCipherSuitesTest()
        {
            ServerAndClientEncryptionRipcHandShake(ConnectionsVersions.VERSION11, false, cipherSuites);
        }

        [Fact]
        public void ServerAndClientEncryptionRipcHandShakeVersion14Ack_withCipherSuites_BlockingTest()
        {
            ServerAndClientEncryptionRipcHandShake(ConnectionsVersions.VERSION14, true, cipherSuites);
        }

        [Fact]
        public void ServerAndClientEncryptionRipcHandShakeVersion13Ack_withCipherSuites_BlockingTest()
        {
            ServerAndClientEncryptionRipcHandShake(ConnectionsVersions.VERSION13, true, cipherSuites);
        }

        [Fact]
        public void ServerAndClientEncryptionRipcHandShakeVersion12Ack_withCipherSuites_BlockingTest()
        {
            ServerAndClientEncryptionRipcHandShake(ConnectionsVersions.VERSION12, true, cipherSuites);
        }

        [Fact]
        public void ServerAndClientEncryptionRipcHandShakeVersion11Ack_withCipherSuites_BlockingTest()
        {
            ServerAndClientEncryptionRipcHandShake(ConnectionsVersions.VERSION11, true, cipherSuites);
        }
    }
    #endregion
}
