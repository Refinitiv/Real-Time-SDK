/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Diagnostics;
using System.Threading;
using System.Collections.Generic;
using System.Net.Sockets;

using Xunit;
using Xunit.Abstractions;
using Xunit.Categories;

using LSEG.Eta.Common;
using LSEG.Eta.Transports;
using System.Text;
using System.Reflection;

namespace LSEG.Eta.Transports.Tests;


/// <summary>
/// Creates a Channel between a client and server.  Messages are sent from client to
/// server, and the received message is verified to be identical to what was sent.
/// </summary>
///
/// <p>
/// The test options include <ul>
/// <li>An array of messages sizes; one message will be sent of each message size.</li>
/// <li>The compression type and level for the channel.</li>
/// <li>The content of the message from {@link MessageContentType}:
/// UNIFORM (all 1's), SEQUENCE (1,2,3...), or RANDOM (good for
/// simulating poor compression). </li>
/// </ul>
///
/// The client/server framework for this test was based on {@link TransportLockJunit}.
///
[Category("TransportMessage")]
public class TransportMessageTests
{
    #region Utilities

    internal enum RunningState
    {
        INACTIVE, INITIALIZING, RUNNING, TERMINATED
    };

    internal enum MessageContentType
    {
        UNIFORM, SEQUENCE, RANDOM, MIXED_RANDOM_START, MIXED_UNIFORM_START
    };

    /// <summary>
    /// When PrintReceivedData is true this is the number of initial bytes that will be printed.
    /// </summary>
    const int PRINT_NUM_BYTES = 80;

    ITestOutputHelper output;

    internal interface IDataHandler
    {
        void HandleMessage(ITransportBuffer buffer, ITestOutputHelper output);

        int ReceivedCount { get; }

        void SetExpectedMessage(ByteBuffer b);

        List<bool> ComparisonResults { get; }
    };


    /// This server will accept connects and process messages until instructed to
    /// terminate. It will keep track of the following statistics:
    ///
    /// <ul>
    /// <li>writeCount per priority.
    /// <li>messageCount per priority (verify if this is different from
    /// writeCount).
    /// <li>bytesWritten per priority.
    /// <li>uncompressedBytesWritten per priority.
    /// </ul>
    internal class EtaNetServer
    {
        bool DEBUG = false;

        private BindOptions m_BindOptions;
        private AcceptOptions m_AcceptOptions;
        private IServer m_Server;
        private bool m_GlobalLocking;
        private IDataHandler m_DataHandler;

        private volatile bool m_Running = true;
        private volatile RunningState m_RunningState = RunningState.INACTIVE;

        private Error m_Error = new Error();
        private ReadArgs m_ReadArgs = new ReadArgs();
        private InProgInfo m_InProgInfo = new InProgInfo();
        private const int TIMEOUTMS = 1000; // 100 milliseconds

        // registered clients
        private Dictionary<Socket, IChannel> m_SocketChannelMap = new Dictionary<Socket, IChannel>();

        private ITestOutputHelper output;

        /// A blocking EtaNetServer is not supported for unit tests.
        internal EtaNetServer(BindOptions bindOptions, AcceptOptions acceptOptions,
               bool globalLocking, IDataHandler dataHandler, ITestOutputHelper output)
        {
            Debug.Assert(bindOptions != null);
            Debug.Assert(acceptOptions != null);

            m_BindOptions = bindOptions;
            m_AcceptOptions = acceptOptions;
            m_GlobalLocking = globalLocking;
            m_DataHandler = dataHandler;
            this.output = output;
        }

        /// Check if the server is running.
        public RunningState State { get => m_RunningState; }

        /// Instructs the server to terminate.
        public void Terminate()
        {
            if (DEBUG)
                output.WriteLine("EtaNetServer: Terminate() entered.");

            m_Running = false;
        }

        public long MessageCount { get; private set; }

        public long CompressedBytesRead { get; private set; }

        public long BytesRead { get; private set; }

        /// Error message that caused the server to abort.
        public string ErrorMsg { get; private set; }


        /// Sets up the Server by calling bind, and if non-blocking registers for
        /// Accept.
        private bool SetupServer()
        {
            if (DEBUG)
                output.WriteLine("EtaNetServer: SetupServer() entered");

            if (m_BindOptions.ChannelIsBlocking)
            {
                ErrorMsg = "Blocking EtaNetServer is not supported in junits.";
                return false;
            }

            if (DEBUG)
                output.WriteLine("EtaNetServer: setupServer() binding");

            m_Server = Transport.Bind(m_BindOptions, out m_Error);
            if (m_Server == null)
            {
                ErrorMsg = "errorCode=" + m_Error.ErrorId + " errorText=" + m_Error.Text;
                return false;
            }

            if (DEBUG)
                output.WriteLine("EtaNetServer: SetupServer() completed successfully");

            return true;
        }

        private void CloseSockets()
        {
            if (DEBUG)
                output.WriteLine("EtaNetServer: CloseSockets() entered");

            if (m_Server != null)
            {
                m_Server.Close(out m_Error);
                m_Server = null;
            }

            foreach (IChannel chan in m_SocketChannelMap.Values)
            {
                chan.Close(out _);
            }

            m_SocketChannelMap.Clear();
        }

        private bool VerifyChannelInfoCompressionType(IChannel channel)
        {
            ChannelInfo channelInfo = new ChannelInfo();
            if (channel.Info(channelInfo, out m_Error) != TransportReturnCode.SUCCESS)
            {
                ErrorMsg = "VerifyChannelInfoCompressionType() channel.info() failed. error="
                        + m_Error.Text;
                return false;
            }

            if (channelInfo.CompressionType != m_BindOptions.CompressionType)
            {
                ErrorMsg = "VerifyChannelInfoCompressionType() channelInfo.CompressionType("
                    + channelInfo.CompressionType
                    + ") did not match bindOptions.CompressionType("
                    + m_BindOptions.CompressionType + ")";
                return false;
            }

            return true;
        }

        private bool InitializeChannel(IChannel channel)
        {
            if (DEBUG)
                output.WriteLine("EtaNetServer: InitializeChannel() entered");

            m_InProgInfo.Clear();
            TransportReturnCode ret = channel.Init(m_InProgInfo, out m_Error);
            if (ret == TransportReturnCode.SUCCESS)
            {
                if (DEBUG)
                    output.WriteLine("EtaNetServer: InitializeChannel() SUCCESS - verifying CompressionType is "
                            + m_BindOptions.CompressionType);

                // SUCCESS, channel is ready, first verify compression Type
                // is what the server specified in BindOptions.
                return VerifyChannelInfoCompressionType(channel);
            }
            else if (ret == TransportReturnCode.CHAN_INIT_IN_PROGRESS)
            {
                return true;
            }
            else
            {
                ErrorMsg = "InitializeChannel failed, TransportReturnCode=" + ret;
                return false;
            }
        }

        private bool ProcessRead(IChannel channel)
        {
            m_ReadArgs.Clear();
            do
            {
                ITransportBuffer msgBuf = channel.Read(m_ReadArgs, out m_Error);
                if (msgBuf != null)
                {
                    if (DEBUG)
                        output.WriteLine("Message read: " + msgBuf.Length());
                    m_DataHandler.HandleMessage(msgBuf, output);
                }
                else
                {
                    if (m_ReadArgs.ReadRetVal == TransportReturnCode.READ_PING)
                    {
                        // Note that we are not tracking client pings.
                        return true;
                    }
                    else if (m_ReadArgs.ReadRetVal == TransportReturnCode.READ_WOULD_BLOCK)
                    {
                        output.WriteLine("Channel READ_WOULD_BLOCK");
                        return true;
                    }
                }
            }
            while (m_ReadArgs.ReadRetVal > TransportReturnCode.SUCCESS);

            if (m_ReadArgs.ReadRetVal != TransportReturnCode.SUCCESS && m_Running)
            {
                // ignore this error if we are terminating, which would mean that the client terminated as expected.
                ErrorMsg = "processRead(): channel.read() returned " + m_ReadArgs.ReadRetVal
                        + ", " + m_Error.Text;
                return false;
            }
            else
            {
                return true;
            }
        }

        private bool ProcessSelector()
        {
            List<Socket> readSockets = new List<Socket>();
            readSockets.AddRange(m_SocketChannelMap.Keys);
            readSockets.Add(m_Server.Socket);

            Socket.Select(readSockets, null, null, TIMEOUTMS);

            foreach (var sock in readSockets)
            {
                if (sock == m_Server.Socket)
                {
                    if (DEBUG)
                        output.WriteLine("EtaNetServer: ProcessSelector() accepting a connection");

                    IChannel channel = m_Server.Accept(m_AcceptOptions, out m_Error);
                    if (channel != null)
                    {
                        m_SocketChannelMap.Add(channel.Socket, channel);
                    }
                    else
                    {
                        ErrorMsg = "server.Accept() failed to return a valid Channel, error="
                            + m_Error.Text;
                        return false;
                    }
                }

                else
                {
                    IChannel channel = m_SocketChannelMap[sock];
                    if (DEBUG)
                        output.WriteLine("EtaNetServer: ProcessSelector() channel is readable, channelState="
                            + channel.State);

                    if (channel.State == ChannelState.ACTIVE)
                    {
                        if (!ProcessRead(channel))
                            return false;
                    }
                    else if (channel.State == ChannelState.INITIALIZING)
                    {
                        if (!InitializeChannel(channel))
                            return false;
                    }
                    else
                    {
                        ErrorMsg = $"channel state ({channel.State}) is no longer ACTIVE, aborting.";
                        return false;
                    }

                }
            }

            return true;
        }

        public void Run()
        {
            m_RunningState = RunningState.INITIALIZING;

            if (DEBUG)
                output.WriteLine("EtaNetServer: Run() entered");

            // Initialize Transport
            InitArgs initArgs = new InitArgs();
            initArgs.GlobalLocking = m_GlobalLocking;
            if (Transport.Initialize(initArgs, out m_Error) != TransportReturnCode.SUCCESS)
            {
                ErrorMsg = "errorCode=" + m_Error.ErrorId + " errorText=" + m_Error.Text;
                m_Running = false;
            }
            else if (!SetupServer())
            {
                m_Running = false;
            }
            else
            {
                m_RunningState = RunningState.RUNNING;
            }

            while (m_Running)
            {
                if (!ProcessSelector())
                    m_Running = false;
            }

            CloseSockets();

            if (ErrorMsg != null)
                output.WriteLine("EtaNetServer: Run(): error occurred, " + ErrorMsg);

            if (Transport.Uninitialize() != TransportReturnCode.SUCCESS && ErrorMsg == null)
            {
                ErrorMsg = "Transport.Uninitialize() failed.";
                return;
            }

            m_RunningState = RunningState.TERMINATED;
            if (DEBUG)
                output.WriteLine("EtaNetServer: Run() completed");
        }

    } // end of EtaNetServer class


    /// This client takes a channel and sends messages until instructed to
    /// terminate.
    ///
    /// It keeps track of the following statistics:
    /// <ul>
    /// <li>writeCount per priority.
    /// <li>messageCount per priority (verify if this is different from
    /// writeCount).
    /// <li>bytesWritten per priority.
    /// <li>uncompressedBytesWritten per priority.
    /// </ul>
    class EtaNetClient
    {
        bool DEBUG = false;

        private IChannel m_Channel;

        private int m_Id;
        private WritePriorities m_Priority;
        private bool m_GlobalLocking;

        private volatile bool m_Running = true;

        private Error m_Error = new Error();

        private Random m_Gen;

        private ITestOutputHelper output;

        internal WriteArgs WriteArgs { get; private set; } = new WriteArgs();

        public EtaNetClient(int id, WritePriorities priority, IChannel channel, bool globalLocking, ITestOutputHelper output)
        {
            Debug.Assert(channel != null);

            m_Id = id;

            if (priority <= WritePriorities.LOW)
                m_Priority = priority;
            else
                m_Priority = WritePriorities.LOW;

            m_Channel = channel;
            m_GlobalLocking = globalLocking;

            m_Gen = new Random(2289374);

            this.output = output;

            if (DEBUG)
                output.WriteLine("EtaNetClient id=" + m_Id + ": created. priority=" + m_Priority);
        }

        private volatile RunningState _state = RunningState.INACTIVE;

        /// Check if client is running.
        public RunningState State { get => _state; private set { _state = value; } }

        /// Instructs the server to terminate.
        public void Terminate()
        {
            if (DEBUG)
                output.WriteLine("EtaNetClient id=" + m_Id + ": Terminate() entered.");

            m_Running = false;
        }

        public long MessageCount { get; private set; }

        public long UncompressedBytesWritten { get; private set; }

        public long BytesWritten { get; private set; }

        /// Error message that caused the server to abort.
        public string ErrorMsg { get; private set; }

        /// <param name="dataBuf">user data to put in each of the messages</param>
        /// <param name="packCount">number of packed messages to send</param>
        internal TransportReturnCode WritePackedMessages(PackedTestArgs args)
        {
            TransportReturnCode retVal = TransportReturnCode.SUCCESS;
            int userBytes = 0;

            int payloadSize = (args.PackedMessageSize + RipcDataMessage.PackedHeaderSize) * args.PackedCount;
            // Packed buffer
            ITransportBuffer msgBuf = m_Channel.GetBuffer(payloadSize, true, out m_Error);
            Assert.True(msgBuf != null, $"{m_Error?.ErrorId} : {m_Error?.Text}");

            int availableRemaining = payloadSize - msgBuf.Length();
            if (args.Debug)
                output.WriteLine("[Packing] start payloadSize=" + payloadSize
                    + " msgBuf.Length=" + msgBuf.Length()
                    + " availableRemaining=" + availableRemaining);

            for (int p = 1; p <= args.PackedCount; p++)
            {
                if (args.Debug)
                    output.WriteLine("[Packing] #" + p + " availableRemaining=" + availableRemaining);

                // populate buffer with payload
                ByteBuffer dataBuf = new ByteBuffer(args.PackedMessageSize);
                PopulateMessage(dataBuf, dataBuf.Capacity, args.MessageContent);

                args.SetExpectedMessage(dataBuf); // for comparison on the receive side
                dataBuf.Flip();
                msgBuf.Data.Put(dataBuf.Contents);

                userBytes += dataBuf.Limit;

                // pack
                availableRemaining = (int)m_Channel.PackBuffer(msgBuf, out var packError);
                Assert.True(availableRemaining >= (int)TransportReturnCode.SUCCESS,
                    $"Failure packing buffer: {(TransportReturnCode)availableRemaining} - {packError?.ToString()}");
            }

            if (args.Debug)
                output.WriteLine("[Packing] availableRemaining at end of packing:" + availableRemaining);

            WriteArgs.Clear();

            do
            {
                if (retVal != TransportReturnCode.SUCCESS)
                    output.WriteLine("WritePackedMessages(): last retVal=" + retVal
                        + " msgBufPos=" + msgBuf.Data.Position
                        + " msgBufLimit=" + msgBuf.Data.Limit);
                retVal = m_Channel.Write(msgBuf, WriteArgs, out m_Error);
                if (!m_Running)
                {
                    output.WriteLine($"EtaNetClient id={m_Id}: WritePackedMessages(): Write() retVal={retVal} while Run time expired.");
                    try
                    {
                        if (DEBUG)
                            Thread.Sleep(250);
                        else
                            Thread.Sleep(1);
                    }
                    catch (ThreadInterruptedException e)
                    {
                        output.WriteLine(e.Message);
                    }
                }
            }
            while (retVal == TransportReturnCode.WRITE_CALL_AGAIN);

            if (retVal >= TransportReturnCode.SUCCESS)
            {
                ++MessageCount;
                BytesWritten += WriteArgs.BytesWritten;
                UncompressedBytesWritten += WriteArgs.UncompressedBytesWritten;

                if (DEBUG)
                    output.WriteLine("EtaNetClient id=" + m_Id
                        + ": WritePackedMessages(): BytesWritten=" + BytesWritten
                        + " UncompressedBytesWritten=" + UncompressedBytesWritten
                        + " userBytes=" + userBytes);

                retVal = m_Channel.Flush(out m_Error);

                if (retVal < TransportReturnCode.SUCCESS)
                {
                    ErrorMsg = "WritePackedMessages(): flush failed, retVal=" + retVal + " error="
                            + m_Error.Text;

                    output.WriteLine("EtaNetClient id=" + m_Id + ": WritePackedMessages(): error="
                                + ErrorMsg);
                    return retVal;
                }
            }
            else
            {
                ErrorMsg = "WritePackedMessage(): Write() failed, retVal=" + retVal + " error="
                        + m_Error.Text;

                output.WriteLine("EtaNetClient id=" + m_Id + ": WritePackedMessages(): error="
                            + ErrorMsg);
                return retVal;
            }

            return retVal;
        }

        internal TransportReturnCode WriteMessage(ByteBuffer dataBuf)
        {
            TransportReturnCode retVal;
            ITransportBuffer msgBuf = null;

            while (msgBuf == null)
            {
                // Not packed
                msgBuf = m_Channel.GetBuffer(dataBuf.Limit, false, out m_Error);
                if (msgBuf == null)
                {
                    output.WriteLine("EtaNetClient id=" + m_Id
                        + ": WriteMessage(): no msgBufs available, errorId=" + m_Error.ErrorId
                        + " error=" + m_Error.Text + ", attempting to flush.");

                    retVal = m_Channel.Flush(out m_Error);
                    if (DEBUG)
                        output.WriteLine("EtaNetClient id=" + m_Id
                                + ": WriteMessage(): Flush() returned retVal=" + retVal);

                    if (retVal < TransportReturnCode.SUCCESS)
                    {
                        ErrorMsg = "WriteMessage(): no msgBufs available to write and flush failed, retVal="
                                + retVal + " error=" + m_Error.Text;

                        output.WriteLine("EtaNetClient id=" + m_Id + ": WriteMessage(): error="
                                    + ErrorMsg);
                        return retVal;
                    }

                    try
                    {
                        if (DEBUG)
                            Thread.Sleep(250);
                        else
                            Thread.Sleep(1);
                    }
                    catch (ThreadInterruptedException e)
                    {
                        output.WriteLine(e.Message);
                    }

                    if (!m_Running)
                    {
                        ErrorMsg = "WriteMessage(): no msgBufs available to write and Run time expired";
                        return TransportReturnCode.FAILURE;
                    }
                }
            }

            // populate msgBuf with data.
            dataBuf.Flip();
            for (int n = 0; n < dataBuf.Limit; n++)
            {
                msgBuf.Data.Put(new byte[] { dataBuf.ReadByte() });
            }

            // write
            WriteArgs.Clear();
            WriteArgs.Priority = m_Priority;

            int origLen = msgBuf.Length(), origPos = msgBuf.Data.Position, origLimit = msgBuf.Data.Limit;
            retVal = 0;

            do
            {
                if (retVal != 0)
                    output.WriteLine("WriteMessage(): last retVal=" + retVal
                        + " origLen=" + origLen + " origPos=" + origPos
                        + " origLimit=" + origLimit + " msgBufPos=" + msgBuf.Data.Position
                        + " msgBufLimit=" + msgBuf.Data.Limit);
                retVal = m_Channel.Write(msgBuf, WriteArgs, out m_Error);
                if (!m_Running)
                {
                    output.WriteLine("EtaNetClient id=" + m_Id + ": WriteMessage(): Write() retVal=" + retVal + " while Run time expired.");
                    try
                    {
                        if (DEBUG)
                            Thread.Sleep(250);
                        else
                            Thread.Sleep(1);
                    }
                    catch (ThreadInterruptedException e)
                    {
                        output.WriteLine(e.Message);
                    }
                }
            }
            while (retVal == TransportReturnCode.WRITE_CALL_AGAIN);

            if (retVal >= TransportReturnCode.SUCCESS)
            {
                ++MessageCount;
                BytesWritten += WriteArgs.BytesWritten;
                UncompressedBytesWritten += WriteArgs.UncompressedBytesWritten;

                if (DEBUG)
                    output.WriteLine("EtaNetClient id=" + m_Id + ": WriteMessage(): _bytesWritten="
                            + BytesWritten + " _uncompressedBytesWritten="
                            + UncompressedBytesWritten
                            + " userBytes=" + dataBuf.Limit);

                retVal = m_Channel.Flush(out m_Error);

                if (retVal < TransportReturnCode.SUCCESS)
                {
                    ErrorMsg = "WriteMessage(): flush failed, retVal=" + retVal + " error="
                            + m_Error.Text;

                    output.WriteLine("EtaNetClient id=" + m_Id + ": WriteMessage(): error="
                                + ErrorMsg);
                    return retVal;
                }
            }
            else
            {
                ErrorMsg = "WriteMessage(): write failed, retVal=" + retVal + " error="
                        + m_Error.Text;

                output.WriteLine("EtaNetClient id=" + m_Id + ": WriteMessage(): error="
                            + ErrorMsg);
                return retVal;
            }

            return 0;
        }

        internal void PopulateMessage(ByteBuffer buf, int messageSize, MessageContentType type)
        {
            byte[] data = new byte[messageSize];

            switch (type)
            {
                case MessageContentType.RANDOM:
                    m_Gen.NextBytes(data);
                    break;
                case MessageContentType.UNIFORM:
                default:
                    for (int idx = 0; idx < messageSize; idx++)
                    {
                        data[idx] = 1;
                    }
                    break;
                case MessageContentType.SEQUENCE:
                    for (int idx = 0; idx < messageSize; idx++)
                    {
                        data[idx] = (byte)(idx % 255);
                    }
                    break;
                case MessageContentType.MIXED_RANDOM_START:
                case MessageContentType.MIXED_UNIFORM_START:
                    bool random = false;
                    if (type == MessageContentType.MIXED_RANDOM_START)
                        random = true;
                    for (int idx = 0; idx < messageSize; idx++)
                    {
                        data[idx] = random
                            ? (byte)(m_Gen.Next() % 255)
                            : (byte)7;

                        // approximately the amount of data for a fragment
                        if (idx % 6136 == 0)
                            random = !random;
                    }
                    break;

            }
            buf.Put(data);
            buf.Flip();
        }

        public void Run()
        {
            State = RunningState.INITIALIZING;

            if (DEBUG)
                output.WriteLine("EtaNetClient id=" + m_Id + ": Run() running");

            // Initialize Transport
            InitArgs initArgs = new InitArgs();
            initArgs.GlobalLocking = m_GlobalLocking;
            if (Transport.Initialize(initArgs, out m_Error) != TransportReturnCode.SUCCESS)
            {
                ErrorMsg = "Run(): errorCode=" + m_Error.ErrorId + " errorText=" + m_Error.Text;
                m_Running = false;
            }

            // get channel info for maxFragmentSize.
            ChannelInfo channelInfo = new ChannelInfo();
            if (m_Channel.Info(channelInfo, out m_Error) != TransportReturnCode.SUCCESS)
            {
                ErrorMsg = "Run(): channel.Info() failed. errorCode=" + m_Error.ErrorId
                        + " errorText=" + m_Error.Text;
                m_Running = false;
            }
            output.WriteLine($"Run(): maxFragmentSize={channelInfo.MaxFragmentSize}");

            State = RunningState.RUNNING;

            while (m_Running) ; // DRT for now not doing anything in client thread

            if (Transport.Uninitialize() != TransportReturnCode.SUCCESS && ErrorMsg == null)
            {
                ErrorMsg = "EtaNetClient id=" + m_Id + ": Transport.uninitialize() failed.";
            }

            State = RunningState.TERMINATED;
            output.WriteLine("EtaNetClient id=" + m_Id + ": Run() complete. messageCount=" + MessageCount);

        }
    } // end of EtaNetClient class

    public TransportMessageTests(ITestOutputHelper output)
    {
        this.output = output;
    }

    public BindOptions DefaultBindOptions(String portNumber)
    {
        BindOptions bindOptions = new BindOptions();
        bindOptions.MajorVersion = Codec.Codec.MajorVersion();
        bindOptions.MinorVersion = Codec.Codec.MinorVersion();
        bindOptions.ProtocolType = (ProtocolType)Codec.Codec.ProtocolType();
        bindOptions.ConnectionType = ConnectionType.SOCKET;
        bindOptions.ServiceName = portNumber;
        bindOptions.ServerToClientPings = false;
        return bindOptions;
    }

    public BindOptions EncryptedBindOptions(string portNumber, EncryptionProtocolFlags protocol)
    {
        BindOptions bindOptions = new BindOptions();
        bindOptions.ConnectionType = ConnectionType.ENCRYPTED;
        bindOptions.ProtocolType = ProtocolType.RWF;
        bindOptions.BindEncryptionOpts.EncryptionProtocolFlags = protocol;
        bindOptions.BindEncryptionOpts.ServerCertificate = "certificate.test.crt";
        bindOptions.BindEncryptionOpts.ServerPrivateKey = "certificate.test.key";
        bindOptions.ServiceName = portNumber;
        bindOptions.SysRecvBufSize = 64 * 1024;

        return bindOptions;
    }

    public AcceptOptions DefaultAcceptOptions()
    {
        AcceptOptions acceptOptions = new AcceptOptions();
        return acceptOptions;
    }

    public ConnectOptions DefaultConnectOptions(string portNumber)
    {
        ConnectOptions connectOptions = new ConnectOptions();
        connectOptions.MajorVersion = Codec.Codec.MajorVersion();
        connectOptions.MinorVersion = Codec.Codec.MinorVersion();
        connectOptions.ProtocolType = (ProtocolType)Codec.Codec.ProtocolType();
        connectOptions.ConnectionType = ConnectionType.SOCKET;
        connectOptions.UnifiedNetworkInfo.Address = "localhost";
        connectOptions.UnifiedNetworkInfo.ServiceName = portNumber;
        return connectOptions;
    }

    public ConnectOptions EncryptedConnectOptions(string portNumber, EncryptionProtocolFlags protocol)
    {
        ConnectOptions connectOptions = new ConnectOptions();

        connectOptions.ConnectionType = ConnectionType.ENCRYPTED;
        connectOptions.EncryptionOpts.EncryptionProtocolFlags = protocol;
        connectOptions.EncryptionOpts.EncryptedProtocol = ConnectionType.SOCKET;
        connectOptions.UnifiedNetworkInfo.Address = "localhost";
        connectOptions.UnifiedNetworkInfo.ServiceName = portNumber;
        connectOptions.MajorVersion = Codec.Codec.MajorVersion();
        connectOptions.MinorVersion = Codec.Codec.MinorVersion();
        connectOptions.ProtocolType = (ProtocolType)Codec.Codec.ProtocolType();

        return connectOptions;
    }

    /// Start and initialize the client channels.
    public IChannel StartClientChannel(int guaranteedOutputBuffers,
            bool blocking, bool writeLocking, CompressionType compressionType, string portNumber, int? sysBufSize, EncryptionProtocolFlags? encryption)
    {
        output.WriteLine("StartClientChannel(): entered");

        IChannel channel;
        ConnectOptions connectOptions = encryption > EncryptionProtocolFlags.ENC_NONE
            ? EncryptedConnectOptions(portNumber, (EncryptionProtocolFlags)encryption)
            : DefaultConnectOptions(portNumber);

        connectOptions.Blocking = blocking;
        connectOptions.CompressionType = compressionType;
        connectOptions.ChannelWriteLocking = writeLocking;
        connectOptions.GuaranteedOutputBuffers = guaranteedOutputBuffers;
        if (sysBufSize is not null)
        {
            connectOptions.SysSendBufSize = sysBufSize.Value;
            connectOptions.SysRecvBufSize = sysBufSize.Value;
        }
        Error error = new Error();
        InProgInfo inProgInfo = new InProgInfo();

        if ((channel = Transport.Connect(connectOptions, out error)) == null)
        {
            output.WriteLine("StartClientChannel(): Transport.Connect() for channel "
                  + "failed, errorId=" + error.ErrorId + " error=" + error.Text);
            return null;
        }
        output.WriteLine("StartClientChannel(): Channel is INITIALIZING");

        // loop until all connections are ACTIVE.
        DateTime timeout = DateTime.Now + TimeSpan.FromSeconds(20); // 20 second timeout
        bool initializing;

        do
        {
            initializing = false;
            TransportReturnCode retVal;
            if (channel.State != ChannelState.INITIALIZING)
            {
                continue;
            }

            if ((retVal = channel.Init(inProgInfo, out error)) < TransportReturnCode.SUCCESS)
            {
                output.WriteLine("StartClientChannel(): channel.Init() "
                        + " failed, errorId=" + error.ErrorId + " error=" + error.Text);
                return null;
            }
            else if (retVal == TransportReturnCode.CHAN_INIT_IN_PROGRESS)
            {
                initializing = true;
            }
            else
            {
                output.WriteLine("StartClientChannel(): Channel is ACTIVE");
            }
        }
        while (initializing && DateTime.Now < timeout);

        if (!initializing)
        {
            output.WriteLine("StartClientChannel() initialized");
            return channel;
        }
        else
        {
            output.WriteLine("StartClientChannel(): failed to initialize channel");
            return null;
        }
    }

    /// Wait for server state of RUNNING or TERMINATED.
    bool WaitForStateRunning(EtaNetServer server)
    {
        try
        {
            while (true)
            {
                if (server.State == RunningState.RUNNING)
                    return true;
                else if (server.State == RunningState.TERMINATED)
                    return false;
                else
                    Thread.Sleep(100);
            }
        }
        catch (ThreadInterruptedException e)
        {
            output.WriteLine(e.Message);
        }
        return false;
    }

    private void TerminateServerAndClients(Thread serverThread,
            EtaNetServer server,
            Thread clientThread,
            EtaNetClient etaNetClient,
            IChannel channel)
    {
        output.WriteLine("TerminateServerAndClients(): stopping clients");

        // instruct clients to stop
        etaNetClient?.Terminate();

        output.WriteLine("TerminateServerAndClients(): waiting for clients to finish");

        // wait for all clients to finish
        DateTime timeout = DateTime.Now + TimeSpan.FromSeconds(120);
        bool stillRunning;
        do
        {
            stillRunning = false;
            if (etaNetClient?.State == RunningState.RUNNING)
                stillRunning = true;

            try
            {
                Thread.Sleep(1000);
            }
            catch (ThreadInterruptedException e)
            {
                output.WriteLine(e.Message);
            }

            if (DateTime.Now > timeout)
            {
                output.WriteLine("TerminateServerAndClients(): failed to stop clients after 10 seconds.");
                break;
            }
        }
        while (stillRunning);

        if (!stillRunning)
        {
            output.WriteLine("TerminateServerAndClients(): flushing client channels.");
            Error error = new Error();
            TransportReturnCode retVal;
            do
            {
                retVal = channel?.Flush(out error) ?? TransportReturnCode.SUCCESS;
                try
                {
                    Thread.Sleep(1);
                }
                catch (ThreadInterruptedException e)
                {
                    output.WriteLine(e.Message);
                }
            }
            while (retVal > TransportReturnCode.SUCCESS);

            if (retVal != TransportReturnCode.SUCCESS)
                output.WriteLine("TerminateServerAndClients(): channel.Flush() failed. retVal="
                        + retVal + " errorId=" + error.ErrorId + " error=" + error.Text);
        }
        else
        {
            output.WriteLine("TerminateServerAndClients(): skipping the flushing client channels, due to non-responsive client threads.");
        }


        output.WriteLine("TerminateServerAndClients(): terminating server");
        server?.Terminate();

        output.WriteLine("TerminateServerAndClients(): waiting for server to finish");
        // wait for server to terminate
        bool serverStillRunning = false;
        do
        {
            serverStillRunning = false;
            if (server?.State == RunningState.RUNNING)
                serverStillRunning = true;

            try
            {
                Thread.Sleep(100);
            }
            catch (ThreadInterruptedException e)
            {
                output.WriteLine(e.Message);
            }
        }
        while (serverStillRunning);

        // join all client threads
        if (!stillRunning)
        {
            output.WriteLine("TerminateServerAndClients(): joining client threads");
            try
            {
                clientThread?.Join();
            }
            catch (ThreadInterruptedException e)
            {
                output.WriteLine(e.Message);
            }
        }
        else
        {
            output.WriteLine("TerminateServerAndClients(): skipping the joining of client threads, due to non-responsive client threads.");
        }

        try
        {
            serverThread.Join();
        }
        catch (ThreadInterruptedException e)
        {
            output.WriteLine(e.Message);
        }

        output.WriteLine("TerminateServerAndClients(): closing channels");

        // close all channels
        if (!stillRunning)
        {
            Error error = new Error();
            channel?.Close(out error); // ignore return code since server will close the channels as well, just prior to this call.
        }
        else
        {
            output.WriteLine("TerminateServerAndClients(): skipping the closing of client channels, due to non-responsive client threads.");
        }

        output.WriteLine("TerminateServerAndClients(): completed");
    }

    internal class PackedTestArgs : IDataHandler
    {
        // set default test args
        internal TimeSpan RunTime = TimeSpan.FromSeconds(30);
        internal int GuaranteedOutputBuffers = 7000;
        internal bool GlobalLocking = true;
        internal bool WriteLocking = true;
        internal bool Blocking = false;
        internal CompressionType CompressionType = CompressionType.NONE;
        internal int CompressionLevel = 6;
        internal int PackedMessageSize = 100;
        internal int PackedCount = 1;
        internal MessageContentType MessageContent = MessageContentType.UNIFORM;
        internal bool PrintReceivedData = false;
        internal bool Debug = false;
        internal int expectedTotalBytes = -1;
        internal int expectedUncompressedBytes = -1;


        private static int m_PortNumber = 15100;
        internal string PORT_NUMBER;

        private PackedTestArgs() { }

        public static PackedTestArgs GetInstance()
        {
            PackedTestArgs args = new PackedTestArgs();
            args.PORT_NUMBER = String.Format("{0}", m_PortNumber++);
            Console.WriteLine("PACKED PORT NUMBER:" + args.PORT_NUMBER);
            return args;
        }

        // DataHandler support
        List<ByteBuffer> m_ExpectedMessages = new List<ByteBuffer>(20);

        public virtual List<bool> ComparisonResults { get; private set; } = new List<bool>(20);

        public override string ToString()
        {
            return $"RunTime={RunTime}\n"
                + $"GuaranteedOutputBuffers={GuaranteedOutputBuffers}\n"
                + $"GlobalLocking={GlobalLocking}\twriteLocking={WriteLocking}\tblocking={Blocking}\n"
                + $"CompressionType={CompressionType}\tcompressionLevel={CompressionLevel}\tdataType={MessageContent}";
        }

        public void HandleMessage(ITransportBuffer buffer, ITestOutputHelper output)
        {
            ++ReceivedCount;
            if (Debug)
                output.WriteLine("[Packed] ReceivedCount=" + ReceivedCount + " len=" + buffer.Length() + " pos=" + buffer.Data.Position);

            ByteBuffer receivedChunk = new ByteBuffer(buffer.Length());
            receivedChunk.Put(buffer.Data.Contents, buffer.Data.ReadPosition, buffer.Length());

            int receivedPos = buffer.Data.Position;
            if (PrintReceivedData)
            {
                StringBuilder sb = new StringBuilder();
                sb.Append("RECEIVE: ");
                int num = Math.Min(buffer.Length(), PRINT_NUM_BYTES);

                for (int n = 0; n < num; n++)
                {
                    sb.Append(string.Format("{0:x2} ", buffer.Data.ReadByte()));
                }
                output.WriteLine(sb.ToString());
            }

            ByteBuffer receivedMsg = buffer.Data;

            ByteBuffer expected = m_ExpectedMessages[ReceivedCount - 1];

            if (PrintReceivedData)
            {
                StringBuilder sb = new StringBuilder();
                sb.Append("EXPECT:  ");
                expected.Flip();
                int num = Math.Min(expected.Contents.Length - expected.Position, PRINT_NUM_BYTES);

                for (int n = 0; n < num; n++)
                {
                    sb.Append(String.Format("{0:x2} ", expected.ReadByte()));
                }
                output.WriteLine(sb.ToString());
                // todo: restore receivedMsg.Position = receivedPos;
                receivedMsg.WritePosition = receivedPos;
                //receivedMsg.Flip();
                receivedMsg.ReadPosition = receivedPos;
            }

            expected.Flip();
            receivedMsg.Flip();
            if (PrintReceivedData)
            {
                output.WriteLine(String.Empty);
                output.WriteLine($"WritePosition received={receivedMsg.WritePosition} expected={expected.WritePosition}");
            }

            //if (expected.Equals(receivedMsg))
            if (expected.Equals(receivedChunk))
            {
                if (PrintReceivedData)
                    output.WriteLine("\tTRUE");
                ComparisonResults.Add(true);
            }
            else
            {
                if (PrintReceivedData)
                    output.WriteLine("\tFALSE");
                ComparisonResults.Add(false);
            }
        }

        public int ReceivedCount { get; private set; }

        public void SetExpectedMessage(ByteBuffer b)
        {
            m_ExpectedMessages.Add(b);
        }
    }

    internal class TestArgs : IDataHandler
    {
        // set default test args
        internal TimeSpan RunTime = TimeSpan.FromSeconds(30);
        internal int GuaranteedOutputBuffers = 7000;
        internal bool GlobalLocking = true;
        internal bool WriteLocking = true;
        internal bool Blocking = false;
        internal CompressionType CompressionType = CompressionType.NONE;
        internal int CompressionLevel = 6;
        internal int[] MessageSizes;
        internal MessageContentType MessageContent = MessageContentType.UNIFORM;
        internal bool PrintReceivedData = false;
        internal bool Debug = false;
        internal int ExpectedTotalBytes = -1;
        internal int ExpectedUncompressedBytes = -1;
        internal bool Encrypted = false;
        internal EncryptionProtocolFlags EncryptionProtocol = EncryptionProtocolFlags.ENC_NONE;
        internal int? SysBufSize;
        private static int portNumber = 15200;
        internal string PORT_NUMBER;

        private TestArgs() { }

        public static TestArgs GetInstance()
        {
            TestArgs args = new TestArgs();
            args.PORT_NUMBER = String.Format("{0}", portNumber++);
            Console.WriteLine("PORT NUMBER:" + args.PORT_NUMBER);
            return args;
        }

        // DataHandler support
        List<ByteBuffer> m_ExpectedMessages = new List<ByteBuffer>();

        public virtual List<bool> ComparisonResults { get; } = new List<bool>();

        public override string ToString()
        {
            return $"RunTime={RunTime}\n"
                + $"GuaranteedOutputBuffers={GuaranteedOutputBuffers}\n"
                + $"GlobalLocking={GlobalLocking}\twriteLocking={WriteLocking}\tblocking={Blocking}\n"
                + $"CompressionType={CompressionType}\tcompressionLevel={CompressionLevel}\tdataType={MessageContent}";
        }

        public void HandleMessage(ITransportBuffer buffer, ITestOutputHelper output)
        {
            if (Debug)
                output.WriteLine("Expecting=" + m_ExpectedMessages.Count + " ReceivedCount=" + ReceivedCount
                    + " len=" + buffer.Length() + " pos=" + buffer.Data.Position);

            int receivedPos = buffer.Data.Position;
            if (PrintReceivedData)
            {
                StringBuilder sb = new StringBuilder();
                sb.Append("RECEIVE: ");
                int num = buffer.Length();
                if (num > PRINT_NUM_BYTES)
                    num = PRINT_NUM_BYTES;
                for (int n = 0; n < num; n++)
                {
                    sb.Append(String.Format("{0:x2} ", buffer.Data.ReadByte()));
                }
                output.WriteLine(sb.ToString());
                output.WriteLine("");
            }

            ByteBuffer receivedMsg = buffer.Data;

            ByteBuffer expected = m_ExpectedMessages[ReceivedCount];

            if (PrintReceivedData)
            {
                StringBuilder sb = new StringBuilder();
                sb.Append("EXPECT:  ");
                expected.Flip();
                // int num = expected.Limit - expected.Position;
                int num = expected.Contents.Length - expected.Position;
                if (num > PRINT_NUM_BYTES)
                    num = PRINT_NUM_BYTES;
                for (int n = 0; n < num; n++)
                {
                    sb.Append(String.Format("{0:x2} ", expected.ReadByte()));
                }
                output.WriteLine(sb.ToString());
                output.WriteLine("");
                // todo receivedMsg.position(receivedPos);
            }

            expected.Flip();
            receivedMsg.Flip();
            if (PrintReceivedData)
                output.WriteLine($"WritePosition received={receivedMsg.WritePosition} expected={expected.WritePosition}");

            if (expected.Equals(receivedMsg))
            {
                if (PrintReceivedData)
                    output.WriteLine("\tTRUE");
                ComparisonResults.Add(true);
            }
            else
            {
                if (PrintReceivedData)
                    output.WriteLine("\tFALSE");
                ComparisonResults.Add(false);
            }
            ++ReceivedCount;
        }

        public int ReceivedCount { get; private set; } = 0;

        public void SetExpectedMessage(ByteBuffer b)
        {
            m_ExpectedMessages.Add(b);
        }
    }

    #endregion

    #region Tests

    // Primarily for debugging during development
    [Fact, Category("Unit")]
    public void test0()
    {
        TestArgs args = TestArgs.GetInstance();

        args.RunTime = TimeSpan.FromSeconds(30);
        args.GuaranteedOutputBuffers = 700;
        args.GlobalLocking = false;
        args.WriteLocking = true;
        args.Blocking = true;
        args.CompressionType = CompressionType.LZ4;
        args.CompressionLevel = 6;

        int[] sizes = { 6140 };
        args.MessageSizes = sizes;
        args.PrintReceivedData = false;
        args.Debug = false;
        args.MessageContent = MessageContentType.RANDOM;

        TestRunner("test0: debugging", args);
    }

    [Fact, Category("Unit")]
    // Verify zlib level 0 split/re-combine with CompFragment
    public void test1()
    {
        TestArgs args = TestArgs.GetInstance();

        args.RunTime = TimeSpan.FromSeconds(30);
        args.GuaranteedOutputBuffers = 700;
        args.GlobalLocking = true;
        args.WriteLocking = true;
        args.Blocking = false;
        args.CompressionType = CompressionType.ZLIB;
        args.CompressionLevel = 0;

        int[] sizes = { 6142 };
        args.MessageSizes = sizes;
        args.PrintReceivedData = false;
        args.MessageContent = MessageContentType.UNIFORM;

        TestRunner("test1: basic", args);
    }

    // No compression: message sizes from no-frag to fragmentation
    [Fact, Category("Unit")]
    public void test2()
    {
        TestArgs args = TestArgs.GetInstance();

        args.RunTime = TimeSpan.FromSeconds(30);
        args.GuaranteedOutputBuffers = 700;
        args.GlobalLocking = true;
        args.WriteLocking = true;
        args.Blocking = false;

        args.CompressionType = CompressionType.NONE;
        args.CompressionLevel = 0;

        int[] sizes = { 6140, 6141, 6142, 6143, 6144, 6145, 6146, 6147, 6148, 6149 };
        args.MessageSizes = sizes;
        args.PrintReceivedData = false;
        args.MessageContent = MessageContentType.UNIFORM;

        TestRunner("test2: no compression fragmentation boundary", args);
    }

    // lz4 compression growth: messages sizes from no-frag to fragmentation
    [Fact, Category("Unit")]
    public void test3()
    {
        TestArgs args = TestArgs.GetInstance();

        args.RunTime = TimeSpan.FromSeconds(30);
        args.GuaranteedOutputBuffers = 700;
        args.GlobalLocking = true;
        args.WriteLocking = true;
        args.Blocking = false;
        args.CompressionType = CompressionType.LZ4;
        args.CompressionLevel = 6;

        int[] sizes = { 6100, 6101, 6102, 6103, 6104, 6105, 6106, 6107, 6108, 6109, 6110 };
        args.MessageSizes = sizes;
        args.PrintReceivedData = false;
        args.MessageContent = MessageContentType.RANDOM;

        TestRunner("test3: lz4 fragmentation boundary test poor compression", args);
    }

    // zlib compression growth: message sizes from no-frag to fragmentation
    [Fact, Category("Unit")]
    public void test4()
    {
        TestArgs args = TestArgs.GetInstance();

        args.RunTime = TimeSpan.FromSeconds(30);
        args.GuaranteedOutputBuffers = 700;
        args.GlobalLocking = true;
        args.WriteLocking = true;
        args.Blocking = false;
        args.CompressionType = CompressionType.ZLIB;
        args.CompressionLevel = 6;

        int[] sizes = { 6123, 6124, 6125, 6126, 6127, 6128, 6129, 6130, 6131, 6132, 6133, 6134, 6135, 6136, 6137, 6138, 6139, 6140 };
        args.MessageSizes = sizes;
        args.PrintReceivedData = false;
        args.MessageContent = MessageContentType.RANDOM;

        TestRunner("test4: zlib fragmentation boundary test poor compression", args);
    }

    // Alternate fragments with Random and Uniform data.
    // Starting with Random forces compression fragmentation on the first fragment
    [Fact, Category("Unit")]
    public void test5()
    {
        TestArgs args = TestArgs.GetInstance();

        args.RunTime = TimeSpan.FromSeconds(30);
        args.GuaranteedOutputBuffers = 700;
        args.GlobalLocking = true;
        args.WriteLocking = true;
        args.Blocking = false;
        args.CompressionType = CompressionType.LZ4;
        args.CompressionLevel = 6;

        int[] sizes = { 50000 };
        args.MessageSizes = sizes;
        args.PrintReceivedData = false;
        args.MessageContent = MessageContentType.MIXED_RANDOM_START;

        TestRunner("test5: mixed data random start", args);
    }

    [Fact, Category("Unit")]
    public void test5z()
    {
        TestArgs args = TestArgs.GetInstance();

        args.RunTime = TimeSpan.FromSeconds(30);
        args.GuaranteedOutputBuffers = 700;
        args.GlobalLocking = true;
        args.WriteLocking = true;
        args.Blocking = false;
        args.CompressionType = CompressionType.ZLIB;
        args.CompressionLevel = 6;

        int[] sizes = { 50000 };
        args.MessageSizes = sizes;
        args.PrintReceivedData = false;
        args.MessageContent = MessageContentType.MIXED_RANDOM_START;

        TestRunner("test5: mixed data random start", args);
    }

    // Alternate fragments with Random and Uniform data.
    // Starting with Uniform forces compression fragmentation on the second fragment
    [Fact, Category("Unit")]
    public void test6()
    {
        TestArgs args = TestArgs.GetInstance();

        args.RunTime = TimeSpan.FromSeconds(30);
        args.GuaranteedOutputBuffers = 700;
        args.GlobalLocking = true;
        args.WriteLocking = true;
        args.Blocking = false;
        args.CompressionType = CompressionType.LZ4;
        args.CompressionLevel = 6;

        int[] sizes = { 50000 };
        args.MessageSizes = sizes;
        args.PrintReceivedData = false;
        args.MessageContent = MessageContentType.MIXED_UNIFORM_START;

        TestRunner("test6: mised data uniform start", args);
    }

    [Fact, Category("Unit")]
    public void test6z()
    {
        TestArgs args = TestArgs.GetInstance();

        args.RunTime = TimeSpan.FromSeconds(30);
        args.GuaranteedOutputBuffers = 700;
        args.GlobalLocking = true;
        args.WriteLocking = true;
        args.Blocking = false;
        args.CompressionType = CompressionType.ZLIB;
        args.CompressionLevel = 6;

        int[] sizes = { 50000 };
        args.MessageSizes = sizes;
        args.PrintReceivedData = false;
        args.MessageContent = MessageContentType.MIXED_UNIFORM_START;

        TestRunner("test6z: mixed data uniform start", args);
    }

    // Alternate fragments with Random and Uniform data.
    // Starting with Uniform forces compression fragmentation on the second fragment
    [Fact, Category("Unit")]
    public void test7()
    {
        TestArgs args = TestArgs.GetInstance();

        args.RunTime = TimeSpan.FromSeconds(30);
        args.GuaranteedOutputBuffers = 700;
        args.GlobalLocking = true;
        args.WriteLocking = true;
        args.Blocking = false;
        args.CompressionType = CompressionType.LZ4;
        args.CompressionLevel = 6;

        int[] sizes = { 500000 };
        args.MessageSizes = sizes;
        args.PrintReceivedData = false;
        args.MessageContent = MessageContentType.MIXED_UNIFORM_START;

        TestRunner("test7: mixed data large message", args);
    }

    [Fact, Category("Unit")]
    public void test8()
    {
        TestArgs args = TestArgs.GetInstance();

        args.RunTime = TimeSpan.FromSeconds(30);
        args.GuaranteedOutputBuffers = 700;
        args.GlobalLocking = true;
        args.WriteLocking = true;
        args.Blocking = false;
        args.CompressionType = CompressionType.NONE;
        args.CompressionLevel = 0;

        int[] sizes = { 7000 };
        args.MessageSizes = sizes;
        args.PrintReceivedData = false;
        args.MessageContent = MessageContentType.RANDOM;

        // frag 1: 6147
        // frag 2:  869
        args.ExpectedTotalBytes = 7016;

        TestRunner("test8: ", args);
    }

    // fragment + compressed frag testing writeArgs bytes written
    [Fact, Category("Unit")]
    public void test8lz4()
    {
        TestArgs args = TestArgs.GetInstance();

        args.RunTime = TimeSpan.FromSeconds(30);
        args.GuaranteedOutputBuffers = 700;
        args.GlobalLocking = true;
        args.WriteLocking = true;
        args.Blocking = false;
        args.CompressionType = CompressionType.LZ4;
        args.CompressionLevel = 0;

        int[] sizes = { 7000 };
        args.MessageSizes = sizes;
        args.PrintReceivedData = false;
        args.MessageContent = MessageContentType.RANDOM;

        // frag 1: 6147
        // frag 1b: 29
        // frag 2: 874
        args.ExpectedTotalBytes = 7050;

        // frag 1: 10
        // frag 1b: 3
        // frag 2: 6
        // data: 7000
        args.ExpectedUncompressedBytes = 7019;

        TestRunner("test8lz4: fragment with compFragment testing write args bytes written", args);
    }

    // fragment + compressed frag testing writeArgs bytes written
    [Fact, Category("Unit")]
    public void test8z()
    {
        TestArgs args = TestArgs.GetInstance();

        args.RunTime = TimeSpan.FromSeconds(30);
        args.GuaranteedOutputBuffers = 700;
        args.GlobalLocking = true;
        args.WriteLocking = true;
        args.Blocking = false;
        args.CompressionType = CompressionType.ZLIB;
        args.CompressionLevel = 0;
        args.Debug = false;

        int[] sizes = { 7000 };
        args.MessageSizes = sizes;
        args.PrintReceivedData = false;
        args.MessageContent = MessageContentType.RANDOM;

        // frag 1: 6147
        // frag 1b: 15
        // frag 2: 879
        args.ExpectedTotalBytes = 7041;

        // frag 1: 10
        // frag 1b: 3
        // frag 2: 6
        // data: 7000
        args.ExpectedUncompressedBytes = 7019;

        TestRunner("test8z: zlib fragment with compFragment testing write args bytes written", args);
    }

    // compressed frag normal: testing writeArgs bytes written
    [Fact, Category("Unit")]
    public void test9lz4()
    {
        TestArgs args = TestArgs.GetInstance();

        args.RunTime = TimeSpan.FromSeconds(30);
        args.GuaranteedOutputBuffers = 700;
        args.GlobalLocking = true;
        args.WriteLocking = true;
        args.Blocking = false;
        args.CompressionType = CompressionType.LZ4;
        args.CompressionLevel = 0;

        int[] sizes = { 6140 };
        args.MessageSizes = sizes;
        args.PrintReceivedData = false;
        args.MessageContent = MessageContentType.RANDOM;

        // 1: 6147
        // 1b: 25
        args.ExpectedTotalBytes = 6172;

        // 1: 3
        // 1b: 3
        // data: 6140
        args.ExpectedUncompressedBytes = 6146;

        TestRunner("test9lz4: compFragment normal testing write args bytes written", args);
    }

    [Fact, Category("Unit")]
    public void test10()
    {
        TestArgs args = TestArgs.GetInstance();

        args.RunTime = TimeSpan.FromSeconds(30);
        args.GuaranteedOutputBuffers = 700;
        args.GlobalLocking = true;
        args.WriteLocking = true;
        args.Blocking = false;
        args.CompressionType = CompressionType.NONE;
        args.CompressionLevel = 0;

        args.MessageSizes = new int[] { 12300 };
        args.PrintReceivedData = false;
        args.Debug = false;
        args.MessageContent = MessageContentType.RANDOM;

        TestRunner("test10: ", args);
    }

    // Designed so that the last fragment size is below the compression threshold
    [Fact, Category("Unit")]
    public void test10lz4()
    {
        TestArgs args = TestArgs.GetInstance();

        args.RunTime = TimeSpan.FromSeconds(30);
        args.GuaranteedOutputBuffers = 700;
        args.GlobalLocking = true;
        args.WriteLocking = true;
        args.Blocking = false;
        args.CompressionType = CompressionType.LZ4;
        args.CompressionLevel = 0;

        int[] sizes = { 12300 };
        args.MessageSizes = sizes;
        args.PrintReceivedData = false;
        args.MessageContent = MessageContentType.RANDOM;

        TestRunner("test10lz4: last fragment not compressed", args);
    }

    // Designed so that the last fragment size is below the compression threshold
    [Fact, Category("Unit")]
    public void test10z()
    {
        TestArgs args = TestArgs.GetInstance();

        args.RunTime = TimeSpan.FromSeconds(30);
        args.GuaranteedOutputBuffers = 700;
        args.GlobalLocking = true;
        args.WriteLocking = true;
        args.Blocking = false;
        args.CompressionType = CompressionType.ZLIB;
        args.CompressionLevel = 0;

        int[] sizes = { 12300 };
        args.MessageSizes = sizes;
        args.PrintReceivedData = false;
        args.MessageContent = MessageContentType.RANDOM;

        TestRunner("test10z: last fragment not compressed", args);
    }

    [Fact, Category("Unit")]
    public void test11z()
    {
        TestArgs args = TestArgs.GetInstance();

        args.RunTime = TimeSpan.FromSeconds(30);
        args.GuaranteedOutputBuffers = 700;
        args.GlobalLocking = true;
        args.WriteLocking = true;
        args.Blocking = false;
        args.CompressionType = CompressionType.ZLIB;
        args.CompressionLevel = 0;

        int[] sizes = { 6128, 6129 };
        args.MessageSizes = sizes;
        args.PrintReceivedData = false;
        args.MessageContent = MessageContentType.RANDOM;

        TestRunner("test11z: ", args);
    }

    [Fact, Category("Unit")]
    public void ptest1()
    {
        PackedTestArgs args = PackedTestArgs.GetInstance();

        args.RunTime = TimeSpan.FromSeconds(30);
        args.GuaranteedOutputBuffers = 700;
        args.GlobalLocking = true;
        args.WriteLocking = true;
        args.Blocking = false;
        args.CompressionType = CompressionType.LZ4;
        args.CompressionLevel = 0;

        args.PackedMessageSize = 612;
        args.PackedCount = 10;
        args.PrintReceivedData = false;
        args.Debug = false;
        args.MessageContent = MessageContentType.RANDOM;

        TestRunnerPacked("ptest1: packing with random data and lz4 ", args, output);
    }

    [Fact, Category("Unit")]
    public void ptest2()
    {
        PackedTestArgs args = PackedTestArgs.GetInstance();

        args.RunTime = TimeSpan.FromSeconds(30);
        args.GuaranteedOutputBuffers = 700;
        args.GlobalLocking = true;
        args.WriteLocking = true;
        args.Blocking = false;
        args.CompressionType = CompressionType.ZLIB;
        args.CompressionLevel = 6;

        args.PackedMessageSize = 612;
        args.PackedCount = 10;
        args.PrintReceivedData = false;
        args.Debug = false;
        args.MessageContent = MessageContentType.RANDOM;

        TestRunnerPacked("ptest2: packing with random data and zlib ", args, output);
    }

    [Fact, Category("Unit")]
    public void ptest3()
    {
        PackedTestArgs args = PackedTestArgs.GetInstance();

        args.RunTime = TimeSpan.FromSeconds(30);
        args.GuaranteedOutputBuffers = 700;
        args.GlobalLocking = true;
        args.WriteLocking = true;
        args.Blocking = false;
        args.CompressionType = CompressionType.NONE;
        args.CompressionLevel = 6;

        args.PackedMessageSize = 612;
        args.PackedCount = 10;
        args.PrintReceivedData = false;
        args.Debug = false;
        args.MessageContent = MessageContentType.RANDOM;

        TestRunnerPacked("ptest3: packing with random data and no compression ", args, output);
    }

    [Theory, Category("Unit")]
    [InlineData(EncryptionProtocolFlags.ENC_TLSV1_2)]
#if TEST_TLSV1_3
    [InlineData(EncryptionProtocolFlags.ENC_TLSV1_3)]
#endif
    public void test0_encrypted(EncryptionProtocolFlags encryptionProtocol)
    {
        TestArgs args = TestArgs.GetInstance();

        args.RunTime = TimeSpan.FromSeconds(30);
        args.GuaranteedOutputBuffers = 700;
        args.GlobalLocking = false;
        args.WriteLocking = true;
        args.Blocking = true;
        args.CompressionType = CompressionType.LZ4;
        args.CompressionLevel = 6;

        int[] sizes = { 6140 };
        args.MessageSizes = sizes;
        args.PrintReceivedData = false;
        args.Debug = false;
        args.MessageContent = MessageContentType.RANDOM;
        args.Encrypted = true;
        args.EncryptionProtocol = encryptionProtocol;

        TestRunner("test0_encrypted: debugging", args);
    }

    [Theory, Category("Unit")]
    [InlineData(EncryptionProtocolFlags.ENC_TLSV1_2)]
#if TEST_TLSV1_3
    [InlineData(EncryptionProtocolFlags.ENC_TLSV1_3)]
#endif
    public void test1_encrypted(EncryptionProtocolFlags encryptionProtocol)
    {
        TestArgs args = TestArgs.GetInstance();

        args.RunTime = TimeSpan.FromSeconds(30);
        args.GuaranteedOutputBuffers = 700;
        args.GlobalLocking = true;
        args.WriteLocking = true;
        args.Blocking = false;
        args.CompressionType = CompressionType.ZLIB;
        args.CompressionLevel = 0;

        int[] sizes = { 6142 };
        args.MessageSizes = sizes;
        args.PrintReceivedData = false;
        args.MessageContent = MessageContentType.UNIFORM;
        args.Encrypted = true;
        args.EncryptionProtocol = encryptionProtocol;

        TestRunner("test1_encrypted: basic", args);
    }

    // No compression: message sizes from no-frag to fragmentation
    [Theory, Category("Unit")]
    [InlineData(EncryptionProtocolFlags.ENC_TLSV1_2)]
#if TEST_TLSV1_3
    [InlineData(EncryptionProtocolFlags.ENC_TLSV1_3)]
#endif
    public void test2_encrypted(EncryptionProtocolFlags encryptionProtocol)
    {
        TestArgs args = TestArgs.GetInstance();

        args.RunTime = TimeSpan.FromSeconds(30);
        args.GuaranteedOutputBuffers = 700;
        args.GlobalLocking = true;
        args.WriteLocking = true;
        args.Blocking = false;
        args.Encrypted = true;
        args.EncryptionProtocol = encryptionProtocol;

        args.CompressionType = CompressionType.NONE;
        args.CompressionLevel = 0;

        int[] sizes = { 6140, 6141, 6142, 6143, 6144, 6145, 6146, 6147, 6148, 6149 };
        args.MessageSizes = sizes;
        args.PrintReceivedData = false;
        args.MessageContent = MessageContentType.UNIFORM;

        TestRunner("test2_encrypted: no compression fragmentation boundary", args);
    }

    // lz4 compression growth: messages sizes from no-frag to fragmentation
    [Theory, Category("Unit")]
    [InlineData(EncryptionProtocolFlags.ENC_TLSV1_2)]
#if TEST_TLSV1_3
    [InlineData(EncryptionProtocolFlags.ENC_TLSV1_3)]
#endif
    public void test3_encrypted(EncryptionProtocolFlags encryptionProtocol)
    {
        TestArgs args = TestArgs.GetInstance();

        args.RunTime = TimeSpan.FromSeconds(30);
        args.GuaranteedOutputBuffers = 700;
        args.GlobalLocking = false;
        args.WriteLocking = true;
        args.Blocking = true;
        args.CompressionType = CompressionType.LZ4;
        args.CompressionLevel = 6;
        args.Encrypted = true;
        args.EncryptionProtocol = encryptionProtocol;

        int[] sizes = { 6100, 6101, 6102, 6103, 6104, 6105, 6106, 6107, 6108, 6109, 6110 };
        args.MessageSizes = sizes;
        args.PrintReceivedData = false;
        args.MessageContent = MessageContentType.RANDOM;

        TestRunner("test3_encrypted: lz4 fragmentation boundary test poor compression", args);
    }

    // zlib compression growth: message sizes from no-frag to fragmentation
    [Theory, Category("Unit")]
    [InlineData(EncryptionProtocolFlags.ENC_TLSV1_2)]
#if TEST_TLSV1_3
    [InlineData(EncryptionProtocolFlags.ENC_TLSV1_3)]
#endif
    public void test4_encrypted(EncryptionProtocolFlags encryptionProtocol)
    {
        TestArgs args = TestArgs.GetInstance();

        args.RunTime = TimeSpan.FromSeconds(30);
        args.GuaranteedOutputBuffers = 700;
        args.GlobalLocking = true;
        args.WriteLocking = true;
        args.Blocking = false;
        args.CompressionType = CompressionType.ZLIB;
        args.CompressionLevel = 6;
        args.Encrypted = true;
        args.EncryptionProtocol = encryptionProtocol;

        int[] sizes = { 6123, 6124, 6125, 6126, 6127, 6128, 6129, 6130, 6131, 6132, 6133, 6134, 6135, 6136, 6137, 6138, 6139, 6140 };
        args.MessageSizes = sizes;
        args.PrintReceivedData = false;
        args.MessageContent = MessageContentType.RANDOM;

        TestRunner("test4_encrypted: zlib fragmentation boundary test poor compression", args);
    }

    // Alternate fragments with Random and Uniform data.
    // Starting with Random forces compression fragmentation on the first fragment
    [Theory, Category("Unit")]
    [InlineData(EncryptionProtocolFlags.ENC_TLSV1_2)]
#if TEST_TLSV1_3
    [InlineData(EncryptionProtocolFlags.ENC_TLSV1_3)]
#endif
    public void test5_encrypted(EncryptionProtocolFlags encryptionProtocol)
    {
        TestArgs args = TestArgs.GetInstance();

        args.RunTime = TimeSpan.FromSeconds(30);
        args.GuaranteedOutputBuffers = 700;
        args.GlobalLocking = true;
        args.WriteLocking = true;
        args.Blocking = false;
        args.CompressionType = CompressionType.LZ4;
        args.CompressionLevel = 6;
        args.Encrypted = true;
        args.EncryptionProtocol = encryptionProtocol;

        int[] sizes = { 50000 };
        args.MessageSizes = sizes;
        args.PrintReceivedData = false;
        args.MessageContent = MessageContentType.MIXED_RANDOM_START;

        TestRunner("test5_encrypted: mixed data random start", args);
    }

    [Theory, Category("Unit")]
    [InlineData(EncryptionProtocolFlags.ENC_TLSV1_2)]
#if TEST_TLSV1_3
    [InlineData(EncryptionProtocolFlags.ENC_TLSV1_3)]
#endif
    public void test5z_encrypted(EncryptionProtocolFlags encryptionProtocol)
    {
        TestArgs args = TestArgs.GetInstance();

        args.RunTime = TimeSpan.FromSeconds(30);
        args.GuaranteedOutputBuffers = 700;
        args.GlobalLocking = true;
        args.WriteLocking = true;
        args.Blocking = false;
        args.CompressionType = CompressionType.ZLIB;
        args.CompressionLevel = 6;
        args.Encrypted = true;
        args.EncryptionProtocol = encryptionProtocol;

        int[] sizes = { 50000 };
        args.MessageSizes = sizes;
        args.PrintReceivedData = false;
        args.MessageContent = MessageContentType.MIXED_RANDOM_START;

        TestRunner("test5_encrypted: mixed data random start", args);
    }

    // Alternate fragments with Random and Uniform data.
    // Starting with Uniform forces compression fragmentation on the second fragment
    [Theory, Category("Unit")]
    [InlineData(EncryptionProtocolFlags.ENC_TLSV1_2)]
#if TEST_TLSV1_3
    [InlineData(EncryptionProtocolFlags.ENC_TLSV1_3)]
#endif
    public void test6_encrypted(EncryptionProtocolFlags encryptionProtocol)
    {
        TestArgs args = TestArgs.GetInstance();

        args.RunTime = TimeSpan.FromSeconds(30);
        args.GuaranteedOutputBuffers = 700;
        args.GlobalLocking = true;
        args.WriteLocking = true;
        args.Blocking = false;
        args.CompressionType = CompressionType.LZ4;
        args.CompressionLevel = 6;
        args.Encrypted = true;
        args.EncryptionProtocol = encryptionProtocol;

        int[] sizes = { 50000 };
        args.MessageSizes = sizes;
        args.PrintReceivedData = false;
        args.MessageContent = MessageContentType.MIXED_UNIFORM_START;

        TestRunner("test6_encrypted: mised data uniform start", args);
    }

    [Theory, Category("Unit")]
    [InlineData(EncryptionProtocolFlags.ENC_TLSV1_2)]
#if TEST_TLSV1_3
    [InlineData(EncryptionProtocolFlags.ENC_TLSV1_3)]
#endif
    public void test6z_encrypted(EncryptionProtocolFlags encryptionProtocol)
    {
        TestArgs args = TestArgs.GetInstance();

        args.RunTime = TimeSpan.FromSeconds(30);
        args.GuaranteedOutputBuffers = 700;
        args.GlobalLocking = true;
        args.WriteLocking = true;
        args.Blocking = false;
        args.CompressionType = CompressionType.ZLIB;
        args.CompressionLevel = 6;
        args.Encrypted = true;
        args.EncryptionProtocol = encryptionProtocol;

        int[] sizes = { 50000 };
        args.MessageSizes = sizes;
        args.PrintReceivedData = false;
        args.MessageContent = MessageContentType.MIXED_UNIFORM_START;

        TestRunner("test6z_encrypted: mixed data uniform start", args);
    }

    // Alternate fragments with Random and Uniform data.
    // Starting with Uniform forces compression fragmentation on the second fragment
    [Theory, Category("Unit")]
    [InlineData(EncryptionProtocolFlags.ENC_TLSV1_2)]
#if TEST_TLSV1_3
    [InlineData(EncryptionProtocolFlags.ENC_TLSV1_3)]
#endif
    public void test7_encrypted(EncryptionProtocolFlags encryptionProtocol)
    {
        TestArgs args = TestArgs.GetInstance();

        args.RunTime = TimeSpan.FromSeconds(30);
        args.GuaranteedOutputBuffers = 700;
        args.GlobalLocking = true;
        args.WriteLocking = true;
        args.Blocking = false;
        args.CompressionType = CompressionType.LZ4;
        args.CompressionLevel = 6;
        args.Encrypted = true;
        args.EncryptionProtocol = encryptionProtocol;
        args.SysBufSize = 65535 * 4;

        int[] sizes = { 500000 };
        args.MessageSizes = sizes;
        args.PrintReceivedData = false;
        args.MessageContent = MessageContentType.MIXED_UNIFORM_START;

        TestRunner("test7_encrypted: mixed data large message", args);
    }

    [Theory, Category("Unit")]
    [InlineData(EncryptionProtocolFlags.ENC_TLSV1_2)]
#if TEST_TLSV1_3
    [InlineData(EncryptionProtocolFlags.ENC_TLSV1_3)]
#endif
    public void test8_encrypted(EncryptionProtocolFlags encryptionProtocol)
    {
        TestArgs args = TestArgs.GetInstance();

        args.RunTime = TimeSpan.FromSeconds(30);
        args.GuaranteedOutputBuffers = 700;
        args.GlobalLocking = true;
        args.WriteLocking = true;
        args.Blocking = false;
        args.CompressionType = CompressionType.NONE;
        args.CompressionLevel = 0;
        args.Encrypted = true;
        args.EncryptionProtocol = encryptionProtocol;

        int[] sizes = { 7000 };
        args.MessageSizes = sizes;
        args.PrintReceivedData = false;
        args.MessageContent = MessageContentType.RANDOM;

        // frag 1: 6147
        // frag 2:  869
        args.ExpectedTotalBytes = 7016;

        TestRunner("test8_encrypted: ", args);
    }

    // fragment + compressed frag testing writeArgs bytes written
    [Theory, Category("Unit")]
    [InlineData(EncryptionProtocolFlags.ENC_TLSV1_2)]
#if TEST_TLSV1_3
    [InlineData(EncryptionProtocolFlags.ENC_TLSV1_3)]
#endif
    public void test8lz4_encrypted(EncryptionProtocolFlags encryptionProtocol)
    {
        TestArgs args = TestArgs.GetInstance();

        args.RunTime = TimeSpan.FromSeconds(30);
        args.GuaranteedOutputBuffers = 700;
        args.GlobalLocking = true;
        args.WriteLocking = true;
        args.Blocking = false;
        args.CompressionType = CompressionType.LZ4;
        args.CompressionLevel = 0;
        args.Encrypted = true;
        args.EncryptionProtocol = encryptionProtocol;

        int[] sizes = { 7000 };
        args.MessageSizes = sizes;
        args.PrintReceivedData = false;
        args.MessageContent = MessageContentType.RANDOM;

        // frag 1: 6147
        // frag 1b: 29
        // frag 2: 874
        args.ExpectedTotalBytes = 7050;

        // frag 1: 10
        // frag 1b: 3
        // frag 2: 6
        // data: 7000
        args.ExpectedUncompressedBytes = 7019;

        TestRunner("test8lz4_encrypted: fragment with compFragment testing write args bytes written", args);
    }

    // fragment + compressed frag testing writeArgs bytes written
    [Theory, Category("Unit")]
    [InlineData(EncryptionProtocolFlags.ENC_TLSV1_2)]
#if TEST_TLSV1_3
    [InlineData(EncryptionProtocolFlags.ENC_TLSV1_3)]
#endif
    public void test8z_encrypted(EncryptionProtocolFlags encryptionProtocol)
    {
        TestArgs args = TestArgs.GetInstance();

        args.RunTime = TimeSpan.FromSeconds(30);
        args.GuaranteedOutputBuffers = 700;
        args.GlobalLocking = true;
        args.WriteLocking = true;
        args.Blocking = false;
        args.CompressionType = CompressionType.ZLIB;
        args.CompressionLevel = 0;
        args.Debug = false;
        args.Encrypted = true;
        args.EncryptionProtocol = encryptionProtocol;

        int[] sizes = { 7000 };
        args.MessageSizes = sizes;
        args.PrintReceivedData = false;
        args.MessageContent = MessageContentType.RANDOM;

        // frag 1: 6147
        // frag 1b: 15
        // frag 2: 879
        args.ExpectedTotalBytes = 7041;

        // frag 1: 10
        // frag 1b: 3
        // frag 2: 6
        // data: 7000
        args.ExpectedUncompressedBytes = 7019;

        TestRunner("test8z_encrypted: zlib fragment with compFragment testing write args bytes written", args);
    }

    // compressed frag normal: testing writeArgs bytes written
    [Theory, Category("Unit")]
    [InlineData(EncryptionProtocolFlags.ENC_TLSV1_2)]
#if TEST_TLSV1_3
    [InlineData(EncryptionProtocolFlags.ENC_TLSV1_3)]
#endif
    public void test9lz4_encrypted(EncryptionProtocolFlags encryptionProtocol)
    {
        TestArgs args = TestArgs.GetInstance();

        args.RunTime = TimeSpan.FromSeconds(30);
        args.GuaranteedOutputBuffers = 700;
        args.GlobalLocking = true;
        args.WriteLocking = true;
        args.Blocking = false;
        args.CompressionType = CompressionType.LZ4;
        args.CompressionLevel = 0;

        int[] sizes = { 6140 };
        args.MessageSizes = sizes;
        args.PrintReceivedData = false;
        args.MessageContent = MessageContentType.RANDOM;
        args.Encrypted = true;
        args.EncryptionProtocol = encryptionProtocol;

        // 1: 6147
        // 1b: 25
        args.ExpectedTotalBytes = 6172;

        // 1: 3
        // 1b: 3
        // data: 6140
        args.ExpectedUncompressedBytes = 6146;

        TestRunner("test9lz4_encrypted: compFragment normal testing write args bytes written", args);
    }

    [Theory, Category("Unit")]
    [InlineData(EncryptionProtocolFlags.ENC_TLSV1_2)]
#if TEST_TLSV1_3
    [InlineData(EncryptionProtocolFlags.ENC_TLSV1_3)]
#endif
    public void test10_encrypted(EncryptionProtocolFlags encryptionProtocol)
    {
        TestArgs args = TestArgs.GetInstance();

        args.RunTime = TimeSpan.FromSeconds(30);
        args.GuaranteedOutputBuffers = 700;
        args.GlobalLocking = true;
        args.WriteLocking = true;
        args.Blocking = false;
        args.CompressionType = CompressionType.NONE;
        args.CompressionLevel = 0;
        args.Encrypted = true;
        args.EncryptionProtocol = encryptionProtocol;

        int[] sizes = { 12300 };
        args.MessageSizes = sizes;
        args.PrintReceivedData = false;
        args.MessageContent = MessageContentType.RANDOM;

        TestRunner("test10_encrypted: ", args);
    }

    // Designed so that the last fragment size is below the compression threshold
    [Theory, Category("Unit")]
    [InlineData(EncryptionProtocolFlags.ENC_TLSV1_2)]
#if TEST_TLSV1_3
    [InlineData(EncryptionProtocolFlags.ENC_TLSV1_3)]
#endif
    public void test10lz4_encrypted(EncryptionProtocolFlags encryptionProtocol)
    {
        TestArgs args = TestArgs.GetInstance();

        args.RunTime = TimeSpan.FromSeconds(30);
        args.GuaranteedOutputBuffers = 700;
        args.GlobalLocking = true;
        args.WriteLocking = true;
        args.Blocking = false;
        args.CompressionType = CompressionType.LZ4;
        args.CompressionLevel = 0;
        args.Encrypted = true;
        args.EncryptionProtocol = encryptionProtocol;

        int[] sizes = { 12300 };
        args.MessageSizes = sizes;
        args.PrintReceivedData = false;
        args.MessageContent = MessageContentType.RANDOM;

        TestRunner("test10lz4_encrypted: last fragment not compressed", args);
    }

    // Designed so that the last fragment size is below the compression threshold
    [Theory, Category("Unit")]
    [InlineData(EncryptionProtocolFlags.ENC_TLSV1_2)]
#if TEST_TLSV1_3
    [InlineData(EncryptionProtocolFlags.ENC_TLSV1_3)]
#endif
    public void test10z_encrypted(EncryptionProtocolFlags encryptionProtocol)
    {
        TestArgs args = TestArgs.GetInstance();

        args.RunTime = TimeSpan.FromSeconds(30);
        args.GuaranteedOutputBuffers = 700;
        args.GlobalLocking = true;
        args.WriteLocking = true;
        args.Blocking = false;
        args.CompressionType = CompressionType.ZLIB;
        args.CompressionLevel = 0;
        args.Encrypted = true;
        args.EncryptionProtocol = encryptionProtocol;

        int[] sizes = { 12300 };
        args.MessageSizes = sizes;
        args.PrintReceivedData = false;
        args.MessageContent = MessageContentType.RANDOM;

        TestRunner("test10z_encrypted: last fragment not compressed", args);
    }

    [Theory, Category("Unit")]
    [InlineData(EncryptionProtocolFlags.ENC_TLSV1_2)]
#if TEST_TLSV1_3
    [InlineData(EncryptionProtocolFlags.ENC_TLSV1_3)]
#endif
    public void test11z_encrypted(EncryptionProtocolFlags encryptionProtocol)
    {
        TestArgs args = TestArgs.GetInstance();

        args.RunTime = TimeSpan.FromSeconds(30);
        args.GuaranteedOutputBuffers = 700;
        args.GlobalLocking = true;
        args.WriteLocking = true;
        args.Blocking = false;
        args.CompressionType = CompressionType.ZLIB;
        args.CompressionLevel = 0;
        args.Encrypted = true;
        args.EncryptionProtocol = encryptionProtocol;

        int[] sizes = { 6128, 6129 };
        args.MessageSizes = sizes;
        args.PrintReceivedData = false;
        args.MessageContent = MessageContentType.RANDOM;

        TestRunner("test11z_encrypted: ", args);
    }

    private void TestRunner(string testName, TestArgs args)
    {
        output.WriteLine("--------------------------------------------------------------------------------");
        output.WriteLine("Test: " + testName);
        output.WriteLine(args.ToString());
        output.WriteLine("--------------------------------------------------------------------------------");
        DateTime endTime = DateTime.Now + args.RunTime;

        Assert.True((args.Blocking && args.GlobalLocking) == false,
                    "Cannot have client Blocking and GlobalLocking=true with server using same CLR, GlobalLocking will deadlock.");

        // BindOptions
        BindOptions bindOptions = args.Encrypted
            ? EncryptedBindOptions(args.PORT_NUMBER, args.EncryptionProtocol)
            : DefaultBindOptions(args.PORT_NUMBER);

        bindOptions.CompressionType = args.CompressionType;
        if (args.CompressionType > CompressionType.NONE)
            bindOptions.CompressionLevel = args.CompressionLevel;

        // AcceptOptions
        AcceptOptions acceptOptions = DefaultAcceptOptions();

        // create the server thread and start it
        EtaNetServer server = new EtaNetServer(bindOptions, acceptOptions, args.GlobalLocking, args, output);
        Thread serverThread = new Thread(server.Run);
        Thread clientThread = null;
        EtaNetClient etaNetClient = null;
        try
        {
            serverThread.Start();

            output.WriteLine("Server started");

            if (!WaitForStateRunning(server))
            {
                Assert.Fail("server terminated while waiting for RUNNING state, error="
                            + server.ErrorMsg);
                server.Terminate();
                serverThread.Join();
                return;
            }

            output.WriteLine("Server bound");
            // start the channels that represent client session
            IChannel clientChannel = StartClientChannel(args.GuaranteedOutputBuffers,
                                                        args.Blocking,
                                                        args.WriteLocking,
                                                        args.CompressionType,
                                                        args.PORT_NUMBER,
                                                        args.SysBufSize,
                                                        args.EncryptionProtocol);
            // "StartClientChannel failed, check output"
            Assert.NotNull(clientChannel);

            etaNetClient = new EtaNetClient(1, // etaNetClientCount
                                                         0, // priority
                                                         clientChannel,
                                                         args.GlobalLocking,
                                                         output);
            clientThread = new Thread(etaNetClient.Run);
            clientThread.Start();

            int messageCount = 0;
            bool testFailed = false;

            foreach (int testMessageSize in args.MessageSizes)
            {
                ++messageCount;
                if (server.State != RunningState.RUNNING)
                {
                    output.WriteLine("Terminating message loop (Server !RUNNING): at message "
                                     + messageCount + " of " + args.MessageSizes.Length);
                    break;
                }

                ByteBuffer msgData = new ByteBuffer(testMessageSize);
                etaNetClient.PopulateMessage(msgData, msgData.Capacity, args.MessageContent);
                args.SetExpectedMessage(msgData);

                TransportReturnCode writeReturn = etaNetClient.WriteMessage(msgData);
                if (writeReturn < TransportReturnCode.SUCCESS)
                {
                    output.WriteLine("Terminating message loop (WriteMessage return code "
                                     + writeReturn + "): at message "
                                     + messageCount + " of " + args.MessageSizes.Length);
                    testFailed = true;
                    break;
                }
                if (args.Debug)
                    output.WriteLine("writeArgs.BytesWritten=" + etaNetClient.WriteArgs.BytesWritten
                                     + " writeArgs.UncompressedBytesWritten=" + etaNetClient.WriteArgs.UncompressedBytesWritten);

                if (args.ExpectedTotalBytes != -1)
                {
                    Assert.True(args.ExpectedTotalBytes == etaNetClient.WriteArgs.BytesWritten);
                }
                if (args.ExpectedUncompressedBytes != -1)
                {
                    Assert.True(args.ExpectedUncompressedBytes == etaNetClient.WriteArgs.UncompressedBytesWritten);
                }

                try
                {
                    // wait for message to be received to compare with message sent
                    bool messageTestComplete = false;
                    while (!messageTestComplete
                           && server.State == RunningState.RUNNING
                           && DateTime.Now < endTime)
                    {
                        Thread.Sleep(100);

                        if (messageCount == args.ReceivedCount)
                        {
                            List<bool> results = args.ComparisonResults;

                            if (results.Count == messageCount)
                                output.WriteLine(messageCount + " of " + args.MessageSizes.Length
                                                 + ": message comparison "
                                                 + (results[messageCount - 1] ? "Success" : "Failed"));
                            else
                            {
                                Assert.Fail(
                                            $"Unexpected results count: {results.Count} while {messageCount} is expected");
                                return;
                            }

                            testFailed = !results[messageCount - 1];

                            messageTestComplete = true; // next
                        }
                    }
                }
                catch (ThreadInterruptedException e)
                {
                    output.WriteLine(e.Message);
                }
            }

            // verify test made it through all messages before exit
            Assert.True(messageCount == args.MessageSizes.Length);
            Assert.False(testFailed);

            TerminateServerAndClients(serverThread, server, clientThread, etaNetClient, clientChannel);

            // If a server failure occurred, fail the test.
            if (server.ErrorMsg != null)
                Assert.Fail(server.ErrorMsg);

        }
        catch (Exception)
        {
            // gracefully terminate threads if something goes wrong (like assertion failure)
            TerminateServerAndClients(serverThread, server, clientThread, etaNetClient, null);
            throw;
        }
        output.WriteLine("TestRunner finished");
    }

    private void TestRunnerPacked(string testName, PackedTestArgs args, ITestOutputHelper output)
    {
        output.WriteLine("--------------------------------------------------------------------------------");
        output.WriteLine("Packed Test: " + testName);
        output.WriteLine(args.ToString());
        output.WriteLine("--------------------------------------------------------------------------------");
        DateTime endTime = DateTime.Now + args.RunTime;

        Assert.True((args.Blocking && args.GlobalLocking) == false,
                    "Cannot have client Blocking and GlobalLocking=true with server using same JVM, GlobalLocking will deadlock.");

        // BindOptions
        BindOptions bindOptions = DefaultBindOptions(args.PORT_NUMBER);
        bindOptions.CompressionType = args.CompressionType;
        if (args.CompressionType > CompressionType.NONE)
            bindOptions.CompressionLevel = args.CompressionLevel;

        // AcceptOptions
        AcceptOptions acceptOptions = DefaultAcceptOptions();

        // create the server thread and start it
        EtaNetServer server = new EtaNetServer(bindOptions, acceptOptions, args.GlobalLocking, args, output);
        Thread serverThread = new Thread(server.Run);
        Thread clientThread = null;
        try
        {
            serverThread.Start();

            if (!WaitForStateRunning(server))
            {
                Assert.Fail(
                            "server terminated while waiting for RUNNING state, error=" + server.ErrorMsg);
                server.Terminate();
                serverThread.Join();
                return;
            }
            else
            {
                // start the channels that represent client session
                IChannel clientChannel = StartClientChannel(args.GuaranteedOutputBuffers,
                                                            args.Blocking, args.WriteLocking, args.CompressionType, args.PORT_NUMBER, null, null);
                // StartClientChannel failed, check output
                Assert.NotNull(clientChannel);

                EtaNetClient etaNetClient = new EtaNetClient(1,
                                                             WritePriorities.HIGH, clientChannel, args.GlobalLocking, output);
                clientThread = new Thread(etaNetClient.Run);
                clientThread.Start();

                if (server.State != RunningState.RUNNING)
                {
                    output.WriteLine("Terminating message loop (Server !RUNNING)");
                    return;
                }

                output.WriteLine("[TestRunnerPacked] sending " + args.PackedCount
                                 + " packed messages of size " + args.PackedMessageSize);

                TransportReturnCode writeReturn = etaNetClient.WritePackedMessages(args);
                if (writeReturn < TransportReturnCode.SUCCESS)
                {
                    output.WriteLine($"Terminating message loop writePackedMessages return code {writeReturn}");
                    return;
                }
                output.WriteLine("writeArgs.BytesWritten=" + etaNetClient.WriteArgs.BytesWritten
                                 + " writeArgs.UncompressedBytesWritten=" + etaNetClient.WriteArgs.UncompressedBytesWritten);

                try
                {
                    // wait for message to be received to compare with message sent
                    bool messageTestComplete = false;
                    while (!messageTestComplete
                           && server.State == RunningState.RUNNING
                           && DateTime.Now < endTime)
                    {
                        Thread.Sleep(100);

                        if (args.ReceivedCount == args.PackedCount)
                            messageTestComplete = true;
                    }
                }
                catch (ThreadInterruptedException e)
                {
                    output.WriteLine(e.Message);
                }

                Assert.True(args.PackedCount == args.ReceivedCount);

                List<bool> results = args.ComparisonResults;
                if (results != null)
                {
                    int count = 0;
                    foreach (var comparisonResult in results)
                    {
                        ++count;
                        if (!comparisonResult)
                            output.WriteLine("[TestRunnerPacked] message #" + count + " comparison failed");

                        Assert.True(comparisonResult);
                    }
                    output.WriteLine("[TestRunnerPacked] verified " + results.Count
                                     + " of " + args.PackedCount + " received messages");
                }

                TerminateServerAndClients(serverThread, server, clientThread, etaNetClient, clientChannel);
            }

            Assert.False(serverThread.IsAlive);

            // If a server failure occurred, fail the test.
            if (server.ErrorMsg != null)
                Assert.Fail(server.ErrorMsg);
        }
        catch (Exception)
        {
            // gracefully terminate threads if something goes wrong (like assertion failure)
            TerminateServerAndClients(serverThread, null, clientThread, null, null);
            throw;
        }

    }

    #endregion
}
