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
using System.Text;
using System.Collections.Generic;
using System.Net.Sockets;

using Xunit;
using Xunit.Categories;

using LSEG.Eta.Common;
using LSEG.Eta.Transports;

namespace LSEG.Eta.Transports.Tests;

[Category("TransportLock")]
public class TransportLockTests
{
    #region Utilities

    const string DEFAULT_PORT_NUMBER = "15000";

    internal enum RunningState
    {
        INACTIVE, INITIALIZING, RUNNING, TERMINATED
    };


    /// This server will accept connects and process messages until instructed to
    /// terminate. It will keep track of the following statistics:
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

        BindOptions m_BindOptions;
        AcceptOptions m_AcceptOptions;
        IServer m_Server;
        bool m_GlobalLocking;

        // statistics
        long m_MessageCount = 0;

        volatile bool m_Running = true;

        Error _error = new Error();
        ReadArgs m_ReadArgs = new ReadArgs();
        InProgInfo m_InProgInfo = new InProgInfo();
        const int TIMEOUTMS = 1000; // 100 milliseconds

        // registered clients
        Dictionary<Socket, IChannel> m_SocketChannelMap = new Dictionary<Socket, IChannel>();

        /// <summary>
        /// A blocking EtaNetServer is not supported for junits.
        /// </summary>
        public EtaNetServer(BindOptions bindOptions, AcceptOptions acceptOptions,
                TestArgs args)
        {
            Assert.NotNull(bindOptions);
            Assert.NotNull(acceptOptions);

            m_BindOptions = bindOptions;
            m_AcceptOptions = acceptOptions;
            m_GlobalLocking = args.GlobalLocking;
        }

        /// <summary>
        /// Check if the server is running.
        /// </summary>
        ///
        /// <c>true</c> if it does.
        public RunningState State { get; private set; }

        /// <summary>
        /// Instructs the server to terminate.
        /// </summary>
        public void Terminate()
        {
            if (DEBUG)
                Console.WriteLine("EtaNetServer: Terminate() entered.");

            m_Running = false;
        }


        /// <summary>
        /// the number of messages received.
        /// </summary>
        public long MessageCount { get; private set; }

        /// <summary>
        /// the number of bytes read.
        /// </summary>
        public long BytesRead { get; private set; }

        /// <summary>
        /// the number of uncompressed bytes read.
        /// </summary>
        public long UncompressedBytesRead { get; private set; }

        /// <summary>
        /// The error message that caused the server to abort.
        /// </summary>
        public string ErrorMsg { get; private set; }

        /// <summary>
        /// Sets up the Server by calling bind, and if non-blocking registers for
        /// Accept.
        /// </summary>
        private bool SetupServer()
        {
            if (DEBUG)
                Console.WriteLine("EtaNetServer: SetupServer() entered");

            if (m_BindOptions.ChannelIsBlocking)
            {
                ErrorMsg = "Blocking EtaNetServer is not supported in junits.";
                return false;
            }

            if (DEBUG)
                Console.WriteLine("EtaNetServer: SetupServer() binding");

            m_Server = Transport.Bind(m_BindOptions, out var bindError);
            if (m_Server == null)
            {
                ErrorMsg = "errorCode=" + bindError.ErrorId + " errorText=" + bindError.Text;
                return false;
            }

            if (DEBUG)
                Console.WriteLine("EtaNetServer: SetupServer() completed successfully");

            return true;
        }

        private void CloseSockets()
        {
            if (DEBUG)
                Console.WriteLine("EtaNetServer: CloseSockets() entered");

            if (m_SocketChannelMap.Count == 0)
                return;

            if (m_Server != null)
            {
                m_Server.Close(out _);
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
            if (channel.Info(channelInfo, out var infoError) != TransportReturnCode.SUCCESS)
            {
                ErrorMsg = "VerifyChannelInfoCompressionType() channel.Info() failed. error="
                        + infoError.Text;
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
                Console.WriteLine("EtaNetServer: initializeChannel() entered");

            m_InProgInfo.Clear();
            TransportReturnCode ret = channel.Init(m_InProgInfo, out _);
            if (ret == TransportReturnCode.SUCCESS)
            {
                if (DEBUG)
                    Console.WriteLine("EtaNetServer: initializeChannel() SUCCESS - verifying CompressionType is "
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
                ErrorMsg = "initializeChannel failed, TransportReturnCode=" + ret;
                return false;
            }
        }


        /// <summary>
        /// Process the message and store the statistics.
        /// </summary>
        ///
        /// <param name="msgBuf"></param>
        /// <param name="compressedBytes">Number of bytes from the pre-processed
        ///           message. We don't know if the message was actually
        ///           compressed.</param>
        private void ProcessStatistics(ITransportBuffer msgBuf, ReadArgs readArgs)
        {
            int msgLen = msgBuf.Length();

            ++m_MessageCount;

            if (DEBUG)
                Console.WriteLine("EtaNetServer: ProcessStatistics() processed message byte="
                        + msgLen + " messageCount=" + m_MessageCount);
        }

        private bool ProcessRead(IChannel channel)
        {
            if (DEBUG)
                Console.WriteLine("EtaNetServer: ProcessRead() entered");

            int runningExpiredCount = 0;
            do
            {
                m_ReadArgs.Clear();
                ITransportBuffer msgBuf = channel.Read(m_ReadArgs, out _error);
                BytesRead += m_ReadArgs.BytesRead;
                UncompressedBytesRead += m_ReadArgs.UncompressedBytesRead;

                if (msgBuf != null)
                {
                    ProcessStatistics(msgBuf, m_ReadArgs);
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
                        return true;
                    }
                }
                if (!m_Running)
                {
                    Thread.Sleep(1);
                    if (++runningExpiredCount > 10000)
                    {
                        Console.WriteLine("EtaNetServer: ProcessRead() IChannel.Read() is returning more to read, but runtime has expired.");
                        break;
                    }
                    else
                    {
                        Console.WriteLine("EtaNetServer: ProcessRead() runtime expired, _messageCount="
                                + m_MessageCount);
                    }

                }
            }
            while (m_ReadArgs.ReadRetVal > TransportReturnCode.SUCCESS);

            if (m_ReadArgs.ReadRetVal != TransportReturnCode.SUCCESS && m_Running)
            {
                // ignore this error if we are terminating, which would mean
                // that the client terminated as expected.
                ErrorMsg = "processRead(): channel.Read() returned " + m_ReadArgs.ReadRetVal
                        + ", " + _error.Text;
                return false;
            }
            else
            {
                return true;
            }
        }

        private bool ProcessSelector()
        {
            if (DEBUG)
                Console.WriteLine("EtaNetServer: processSelector() entered");


            List<Socket> readSockets = new List<Socket>();
            readSockets.AddRange(m_SocketChannelMap.Keys);
            readSockets.Add(m_Server.Socket);

            Socket.Select(readSockets, null, null, TIMEOUTMS);

            foreach (var sock in readSockets)
            {
                if (sock == m_Server.Socket)
                {
                    if (DEBUG)
                        Console.WriteLine("EtaNetServer: ProcessSelector() accepting a connection");

                    IChannel channel = m_Server.Accept(m_AcceptOptions, out _error);
                    if (channel != null)
                    {
                        m_SocketChannelMap.Add(channel.Socket, channel);
                    }
                    else
                    {
                        ErrorMsg = "server.Accept() failed to return a valid Channel, error="
                                + _error.Text;
                        return false;
                    }
                }
                else
                {
                    IChannel channel = m_SocketChannelMap[sock];
                    if (DEBUG)
                        Console.WriteLine("EtaNetServer: ProcessSelector() channel is readable, channelState="
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
                        ErrorMsg = "channel state (" + channel.State
                                + ") is no longer ACTIVE, aborting.";
                        return false;
                    }

                }
            }

            return true;
        }

        public void Run()
        {
            State = RunningState.INITIALIZING;

            if (DEBUG)
                Console.WriteLine("EtaNetServer: Run() entered");

            // Initialize Transport
            InitArgs initArgs = new InitArgs();
            initArgs.GlobalLocking = m_GlobalLocking;
            if (Transport.Initialize(initArgs, out var initError) != TransportReturnCode.SUCCESS)
            {
                ErrorMsg = "errorCode=" + initError.ErrorId + " errorText=" + initError.Text;
                m_Running = false;
            }
            else if (!SetupServer())
            {
                m_Running = false;
            }
            else
            {
                State = RunningState.RUNNING;
            }

            while (m_Running)
            {
                if (!ProcessSelector())
                    m_Running = false;
            }

            CloseSockets();

            if (ErrorMsg != null)
                Console.WriteLine("EtaNetServer: Run(): error occurred, " + ErrorMsg);

            if (Transport.Uninitialize() != TransportReturnCode.SUCCESS && ErrorMsg == null)
            {
                ErrorMsg = "Transport.uninitialize() failed.";
                return;
            }

            State = RunningState.TERMINATED;
            if (DEBUG)
                Console.WriteLine("EtaNetServer: Run() completed");
        }

    } // end of EtaNetServer class


    /// <summary>
    /// This client takes a channel and sends messages until instructed to terminate.
    /// </summary>
    ///
    /// It will keep track of the following statistics:
    /// <ul>
    /// <li>writeCount per priority.
    /// <li>messageCount per priority (verify if this is different from
    /// writeCount).
    /// <li>bytesWritten per priority.
    /// <li>uncompressedBytesWritten per priority.
    /// </ul>
    internal class EtaNetClient
    {
        bool DEBUG = false;

        IChannel m_Channel;

        Error m_Error = new Error();
        int m_Id;
        WritePriorities m_Priority;
        int m_MessageSize;
        int m_VariablePackedMessageSize;
        int m_FlushInterval;
        int m_MaxMessageCount;
        bool m_GlobalLocking;
        bool m_Packing;
        bool m_CallWriteWithPackedMessage;

        // statistics
        long m_MessageCount = 0;
        long m_PackedMessageCount = 0;

        volatile bool m_Running = true;

        WriteArgs m_WriteArgs = new WriteArgs();

        public EtaNetClient(int id, WritePriorities priority, IChannel channel, TestArgs args)
        {
            Debug.Assert(channel != null);

            m_Id = id;

            if (priority <= WritePriorities.LOW)
                m_Priority = priority;
            else
                m_Priority = WritePriorities.LOW;

            m_MessageSize = args.MessageSize;

            // start _variablePackedMessageSize one greater since it's
            // immediately decremented in writePackedMessage().
            m_VariablePackedMessageSize = m_MessageSize + 1;

            m_FlushInterval = args.FlushInterval;
            m_Channel = channel;
            m_GlobalLocking = args.GlobalLocking;
            m_Packing = args.Packing;
            m_CallWriteWithPackedMessage = args.CallWriteWithPackedMessage;
            m_MaxMessageCount = args.MaxMessageCount;

            if (DEBUG)
                Console.WriteLine("EtaNetClient id=" + m_Id + ": created. priority=" + m_Priority);
        }

        /// <summary>
        /// Whether server is running.
        /// </summary>
        ///
        /// <c>true</c> if it does.
        public RunningState State { get; private set; }

        /// <summary>
        /// Instructs the server to terminate.
        /// </summary>
        public void Terminate()
        {
            if (DEBUG)
                Console.WriteLine("EtaNetClient id=" + m_Id + ": Terminate() entered.");

            m_Running = false;
        }

        public long MessageCount { get; private set; }

        public long BytesWritten { get; private set; }

        public long UncompressedBytesWritten { get; private set; }

        /// <summary>
        /// Error message that caused the server to abort.
        /// </summary>
        public string ErrorMsg { get; private set; }

        void PopulateBufferWithData(ByteBuffer buffer, int count)
        {
            // populate msgBuf with data.
            for (int idx = 0; idx < count; idx++)
            {
                buffer.Put(new byte[] { (byte)m_Priority });
            }
        }

        ITransportBuffer GetBuffer(int size, bool packed)
        {
            TransportReturnCode retVal;
            ITransportBuffer msgBuf = null;

            while (msgBuf == null)
            {
                msgBuf = m_Channel.GetBuffer(size, packed, out var bufferError);
                if (msgBuf == null)
                {
                    if (DEBUG)
                        Console.WriteLine("EtaNetClient id=" + m_Id
                                + ": GetBuffer(): no msgBufs available, errorId="
                                + bufferError.ErrorId + " error=" + bufferError.Text
                                + ", attempting to flush.");

                    retVal = m_Channel.Flush(out var flushError);
                    if (DEBUG)
                        Console.WriteLine("EtaNetClient id=" + m_Id
                                + ": GetBuffer(): flush() returned retVal=" + retVal);

                    if (retVal < TransportReturnCode.SUCCESS)
                    {
                        ErrorMsg = "GetBuffer(): no msgBufs available to write and flush failed, retVal="
                                + retVal + " error=" + flushError.Text;
                        m_Error.ErrorId = retVal;
                        m_Error.Text = ErrorMsg;
                        if (DEBUG)
                            Console.WriteLine("EtaNetClient id=" + m_Id
                                    + ": GetBuffer(): flush failed, error=" + ErrorMsg);
                        return null;
                    }

                    if (DEBUG)
                        Thread.Sleep(250);
                    else
                        Thread.Sleep(10);

                    if (!m_Running)
                    {
                        ErrorMsg = "GetBuffer(): no msgBufs available to write and Run time expired";
                        m_Error.ErrorId = TransportReturnCode.FAILURE;
                        m_Error.Text = ErrorMsg;
                        return null;
                    }
                }
            }
            return msgBuf;
        }

        // returns TransportReturnCode
        private TransportReturnCode WriteMessage()
        {
            if (DEBUG)
                Console.WriteLine("EtaNetClient id=" + m_Id + ": WriteMessage() entered");

            TransportReturnCode retVal;

            // get a buffer
            ITransportBuffer msgBuf = GetBuffer(m_MessageSize, false);
            if (msgBuf == null)
            {
                return m_Error.ErrorId;
            }

            // populate msgBuf with data.
            PopulateBufferWithData(msgBuf.Data, m_MessageSize);

            // write
            m_WriteArgs.Clear();
            m_WriteArgs.Priority = m_Priority;

            retVal = 0;
            DateTime timeout = DateTime.MinValue;
            do
            {
                retVal = m_Channel.Write(msgBuf, m_WriteArgs, out m_Error);
                if (!m_Running)
                {
                    Console.WriteLine("EtaNetClient id=" + m_Id
                            + ": WriteMessage(): write() FYI: retVal=" + retVal
                            + " while Run time expired.");
                    if (timeout == DateTime.MinValue)
                    {
                        timeout = DateTime.Now + TimeSpan.FromSeconds(20); // 20s
                    }
                    else if (timeout < DateTime.Now)
                    {
                        ErrorMsg = "WriteMessage(): Write() returning retVal=" + retVal
                                + " 20s after Run time expired, aborting";
                        Console.WriteLine("EtaNetClient id=" + m_Id + " error=" + ErrorMsg);
                        return TransportReturnCode.FAILURE;
                    }
                }

                if (DEBUG)
                    Thread.Sleep(250);
                else
                    Thread.Sleep(1);
            }
            while (retVal == TransportReturnCode.WRITE_CALL_AGAIN);

            if (retVal >= TransportReturnCode.SUCCESS)
            {
                ++m_MessageCount;
                BytesWritten += m_WriteArgs.BytesWritten;
                UncompressedBytesWritten += m_WriteArgs.UncompressedBytesWritten;

                if (DEBUG)
                    Console.WriteLine("EtaNetClient id=" + m_Id + ": WriteMessage(): _messageCount="
                            + m_MessageCount + " _flushInterval=" + m_FlushInterval
                            + " _messageCount % _flushInterval = "
                            + (m_MessageCount % m_FlushInterval));

                // flush on flushInterval
                if ((m_MessageCount % m_FlushInterval) == 0)
                {
                    if (DEBUG)
                        Console.WriteLine("EtaNetClient id=" + m_Id
                                + ": WriteMessage(): calling flush()");
                    retVal = m_Channel.Flush(out m_Error);
                    if (DEBUG)
                        Console.WriteLine("EtaNetClient id=" + m_Id
                                + ": WriteMessage(): flush() returned retVal=" + retVal);

                    if (retVal < TransportReturnCode.SUCCESS)
                    {
                        ErrorMsg = "WriteMessage(): flush failed, retVal=" + retVal + " error="
                                + m_Error.Text;
                        if (DEBUG)
                            Console.WriteLine("EtaNetClient id=" + m_Id + ": WriteMessage(): error="
                                    + ErrorMsg);
                        return retVal;
                    }
                }
            }
            else
            {
                ErrorMsg = "WriteMessage(): write failed, retVal=" + retVal + " error="
                        + m_Error.Text;
                if (DEBUG)
                    Console.WriteLine("EtaNetClient id=" + m_Id + ": WriteMessage(): error="
                            + ErrorMsg);
                return retVal;
            }
            if (DEBUG)
                Console.WriteLine("EtaNetClient id=" + m_Id + ": WriteMessage(): completed normally");

            return 0;
        }

        /// <summary>
        /// A packed message cannot exceed maxFragmentSize.
        /// </summary>
        private TransportReturnCode WritePackedMessage(int availableBytes, bool callWriteWithPackedMessage)
        {
            Error error = new Error();

            if (--m_VariablePackedMessageSize == 0)
                m_VariablePackedMessageSize = m_MessageSize;

            if (DEBUG)
                Console.WriteLine("EtaNetClient id=" + m_Id
                        + ": writePackedMessage() entered, _variablePackedMessageSize="
                        + m_VariablePackedMessageSize);

            TransportReturnCode retVal;

            // get a buffer
            ITransportBuffer msgBuf = GetBuffer(availableBytes, true);
            if (msgBuf == null)
            {
                return m_Error.ErrorId;
            }

            /*
             * if "callWriteWithPackedMessage" is true, we will call
             * channel.write() with the last packed message in the buffer.
             * Otherwise, all messages will be packed with channel.packBuffer()
             * and channel.write() will be called with no additional data
             * written to the buffer (i.e. an empty buffer).
             */
            int bufferSpaceNeeded = m_VariablePackedMessageSize;
            if (callWriteWithPackedMessage)
            {
                /*
                 * ensure there is enough space remaining for two messages. The
                 * first message will be packed with channel.packBuffer(). The
                 * second message will be packed with channel.write(). Since
                 * this is a junit, we know to leave space for the PACKED_HDR
                 * which is needed for the last message.
                 */
                bufferSpaceNeeded += m_VariablePackedMessageSize
                        + RipcDataMessage.HeaderSize;
            }

            // pack message
            while (availableBytes >= bufferSpaceNeeded)
            {
                // populate msgBuf with data.
                PopulateBufferWithData(msgBuf.Data, m_VariablePackedMessageSize);

                // pack message
                availableBytes = (int)m_Channel.PackBuffer(msgBuf, out _);
                ++m_MessageCount;
                if (DEBUG)
                    Console.WriteLine("EtaNetClient id=" + m_Id
                            + ": writePackedMessage(): _messageCount=" + m_MessageCount
                            + " _variablePackedMessageSize=" + m_VariablePackedMessageSize);

                if (m_MaxMessageCount > 0 && m_MessageCount >= m_MaxMessageCount)
                {
                    Console.WriteLine("EtaNetClient id=" + m_Id
                            + ": writePackedMessage(): _messageCount=" + m_MessageCount
                            + " has exceeded _maxMessageCount=" + m_MaxMessageCount);
                    break;
                }
            }

            if (callWriteWithPackedMessage)
            {
                // populate msgBuf with a message so that write will have to
                // pack the last message.
                for (int idx = 0; idx < m_VariablePackedMessageSize; idx++)
                {
                    if (msgBuf.Data.Position == msgBuf.Data.Limit)
                        Console.WriteLine("EtaNetClient.writePackedMessage(): position == limit!!!");
                    msgBuf.Data.Put(new byte[] { (byte)m_Priority });
                }
                ++m_MessageCount;
            }

            if (DEBUG)
                Console.WriteLine("EtaNetClient id=" + m_Id
                        + ": writePackedMessage(): _messageCount=" + m_MessageCount
                        + " _variablePackedMessageSize=" + m_VariablePackedMessageSize);

            // write the packed message
            m_WriteArgs.Clear();
            m_WriteArgs.Priority = m_Priority;
            retVal = m_Channel.Write(msgBuf, m_WriteArgs, out m_Error);
            if (retVal >= TransportReturnCode.SUCCESS)
            {
                ++m_PackedMessageCount;
                BytesWritten += m_WriteArgs.BytesWritten;
                UncompressedBytesWritten += m_WriteArgs.UncompressedBytesWritten;

                if (DEBUG)
                    Console.WriteLine("EtaNetClient id=" + m_Id
                            + ": writePackedMessage(): _packedMessageCount=" + m_PackedMessageCount
                            + " _flushInterval=" + m_FlushInterval
                            + " _packedMessageCount % _flushInterval = "
                            + (m_PackedMessageCount % m_FlushInterval));

                // flush on flushInterval
                if ((m_PackedMessageCount % m_FlushInterval) == 0 || m_MaxMessageCount > 0 && m_MessageCount >= m_MaxMessageCount)
                {
                    if (DEBUG)
                        Console.WriteLine("EtaNetClient id=" + m_Id
                                + ": WritePackedMessage(): calling Flush()");
                    retVal = m_Channel.Flush(out m_Error);
                    if (DEBUG)
                        Console.WriteLine("EtaNetClient id=" + m_Id
                                + ": WritePackedMessage(): Flush() returned retVal=" + retVal);

                    if (retVal < TransportReturnCode.SUCCESS)
                    {
                        ErrorMsg = "WritePackedMessage(): flush failed, retVal=" + retVal
                                + " error=" + m_Error.Text;
                        if (DEBUG)
                            Console.WriteLine("EtaNetClient id=" + m_Id
                                    + ": writePackedMessage(): error=" + ErrorMsg);
                        return retVal;
                    }
                }
            }
            else
            {
                ErrorMsg = "writePackedMessage(): write failed, retVal=" + retVal + " error="
                        + m_Error.Text;
                if (DEBUG)
                    Console.WriteLine("EtaNetClient id=" + m_Id + ": writePackedMessage(): error="
                            + ErrorMsg);
                return retVal;
            }
            if (DEBUG)
                Console.WriteLine("EtaNetClient id=" + m_Id
                        + ": writePackedMessage(): completed normally");

            return 0;
        }

        public void Run()
        {
            State = RunningState.INITIALIZING;

            if (DEBUG)
                Console.WriteLine("EtaNetClient id=" + m_Id + ": Run() running");

            // Initialize Transport
            InitArgs initArgs = new InitArgs();
            initArgs.GlobalLocking = m_GlobalLocking;
            if (Transport.Initialize(initArgs, out var initError) != TransportReturnCode.SUCCESS)
            {
                ErrorMsg = "Run(): errorCode=" + initError.ErrorId + " errorText=" + initError.Text;
                m_Running = false;
            }

            // get channel info for maxFragmentSize.
            ChannelInfo channelInfo = new ChannelInfo();
            if (m_Channel.Info(channelInfo, out m_Error) != TransportReturnCode.SUCCESS)
            {
                ErrorMsg = "Run(): channel.info() failed. errorCode=" + m_Error.ErrorId
                        + " errorText=" + m_Error.Text;
                m_Running = false;
            }

            State = RunningState.RUNNING;

            TransportReturnCode retVal;
            int sleepTime = 1;
            if (DEBUG)
                sleepTime = 250;

            while (m_Running)
            {
                if (!m_Packing)
                    retVal = WriteMessage();
                else
                    retVal = WritePackedMessage(channelInfo.MaxFragmentSize, m_CallWriteWithPackedMessage);

                if (retVal < 0)
                {
                    m_Running = false;
                }
                else if (m_MaxMessageCount > 0 && m_MessageCount >= m_MaxMessageCount)
                {
                    Console.WriteLine("EtaNetClient id=" + m_Id + ": Run(): _messageCount="
                            + m_MessageCount + " has exceeded _maxMessageCount=" + m_MaxMessageCount
                            + ", stopping");
                    m_Running = false;
                }

                // Go to sleep to let others Run once in a while.
                Thread.Sleep(sleepTime);
            }

            if (ErrorMsg != null)
                Console.WriteLine("EtaNetClient id=" + m_Id + ": Run(): error occurred, " + ErrorMsg);

            if (Transport.Uninitialize() != TransportReturnCode.SUCCESS && ErrorMsg == null)
            {
                ErrorMsg = "EtaNetClient id=" + m_Id + ": Transport.uninitialize() failed.";
            }

            State = RunningState.TERMINATED;
            // if (DEBUG)
            Console.WriteLine("EtaNetClient id=" + m_Id + ": Run() complete. messageCount="
                    + m_MessageCount);

        }
    } // end of EtaNetClient class

    public BindOptions DefaultBindOptions()
    {
        BindOptions bindOptions = new BindOptions();
        bindOptions.MajorVersion = Codec.Codec.MajorVersion();
        bindOptions.MinorVersion = Codec.Codec.MinorVersion();
        bindOptions.ProtocolType = (ProtocolType)Codec.Codec.ProtocolType();
        bindOptions.ConnectionType = ConnectionType.SOCKET;
        bindOptions.ServiceName = DEFAULT_PORT_NUMBER;
        bindOptions.ServerToClientPings = false;
        return bindOptions;
    }

    public AcceptOptions DefaultAcceptOptions()
    {
        AcceptOptions acceptOptions = new AcceptOptions();
        return acceptOptions;
    }

    public ConnectOptions DefaultConnectOptions()
    {
        ConnectOptions connectOptions = new ConnectOptions();
        connectOptions.MajorVersion = Codec.Codec.MajorVersion();
        connectOptions.MinorVersion = Codec.Codec.MinorVersion();
        connectOptions.ProtocolType = (ProtocolType)Codec.Codec.ProtocolType();
        connectOptions.ConnectionType = ConnectionType.SOCKET;
        connectOptions.UnifiedNetworkInfo.Address = "localhost";
        connectOptions.UnifiedNetworkInfo.ServiceName = DEFAULT_PORT_NUMBER;
        return connectOptions;
    }

    /// <summary>
    /// Start and initialize the client channels.
    /// </summary>
    public IChannel[] StartClientChannels(int channelCount, int threadsPerChannels,
            int guaranteedOutputBuffers, bool blocking, bool writeLocking, CompressionType compressionType)
    {
        Console.WriteLine("StartClientChannels(): entered, channelCount=" + channelCount);

        IChannel[] channels = new IChannel[channelCount];
        ConnectOptions connectOptions = DefaultConnectOptions();
        connectOptions.Blocking = blocking;
        connectOptions.CompressionType = (CompressionType)compressionType;
        connectOptions.ChannelWriteLocking = writeLocking;
        connectOptions.GuaranteedOutputBuffers = guaranteedOutputBuffers;

        InProgInfo inProgInfo = new InProgInfo();

        // make all of the connections
        for (int idx = 0; idx < channels.Length; idx++)
        {
            if ((channels[idx] = Transport.Connect(connectOptions, out var error)) == null)
            {
                Console.WriteLine("StartClientChannels(): Transport.connect() for channels[" + idx
                        + "] failed, errorId=" + error.ErrorId + " error=" + error.Text);
                return null;
            }
            Console.WriteLine("StartClientChannels(): Channel[" + idx + "] is INITIALIZING");
        }

        // loop until all connections are ACTIVE.
        DateTime timeout = DateTime.Now + TimeSpan.FromSeconds(20); // 20 second timeout
        bool initializing = false;
        bool[] channelActive = new bool[channels.Length];

        do
        {
            initializing = false;
            for (int idx = 0; idx < channels.Length; idx++)
            {
                TransportReturnCode retVal;
                if (channels[idx].State != ChannelState.INITIALIZING)
                {
                    continue;
                }

                if ((retVal = channels[idx].Init(inProgInfo, out var error)) < TransportReturnCode.SUCCESS)
                {
                    Console.WriteLine("StartClientChannels(): channel.Init() for channels[" + idx
                            + "] failed, errorId=" + error.ErrorId + " error=" + error.Text);
                    return null;
                }
                else if (retVal == TransportReturnCode.CHAN_INIT_IN_PROGRESS)
                {
                    initializing = true;
                }
                else if (!channelActive[idx])
                {
                    // print once for each channel
                    channelActive[idx] = true;
                    Console.WriteLine("StartClientChannels(): Channel[" + idx + "] is ACTIVE");
                }
            }
        }
        while (initializing && DateTime.Now < timeout);

        if (!initializing)
        {
            Console.WriteLine("StartClientChannels(): all (" + channelCount
                    + ") channels initiailized");
            return channels;
        }
        else
        {
            Console.WriteLine("StartClientChannels(): failed to initialize all channels, channelActive="
                            + channelActive.ToString());
            return null;
        }
    }

    /// <summary>
    /// Create EtaNetClients for the specified number of threads for the specified
    /// number of channels.
    /// </summary>
    ///
    /// <param name="threadsPerChannels"></param>
    /// <param name="channels">created from caller.</param>
    /// <param name="etaNetClients">populated with new clients</param>
    /// <param name="clientThreads">populated with the new clients' threads.</param>
    ///
    internal void StartClientThreads(TestArgs args, IChannel[] channels, EtaNetClient[] etaNetClients, Thread[] clientThreads)
    {
        int threadsPerChannels = args.ThreadsPerChannels;

        Assert.NotNull(channels);
        Assert.NotNull(etaNetClients);
        Assert.NotNull(clientThreads);

        int etaNetClientCount = -1;

        for (int channelIdx = 0; channelIdx < channels.Length; channelIdx++)
        {
            for (int threadIdx = 0; threadIdx < threadsPerChannels; threadIdx++)
            {
                ++etaNetClientCount;
                etaNetClients[etaNetClientCount] = new EtaNetClient(etaNetClientCount, (WritePriorities)threadIdx,
                        channels[channelIdx], args);
                clientThreads[etaNetClientCount] = new Thread(etaNetClients[etaNetClientCount].Run);
                clientThreads[etaNetClientCount].Start();
            }
        }
        return;
    }

    /// <summary>
    /// Wait for server state of RUNNING or TERMINATED.
    /// </summary>
    ///
    /// <param name="server"></param>
    /// <returns><c>true</c> if RunningState.RUNNING, <c>false</c> if RunningState.TERMINATED.</returns>
    ///
    internal bool WaitForStateRunning(EtaNetServer server)
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

    private void TerminateServerAndClients(Thread serverThread, EtaNetServer server,
            Thread[] clientThreads, EtaNetClient[] etaNetClients, IChannel[] channels)
    {
        Console.WriteLine("TerminateServerAndClients(): stopping clients");

        // instruct clients to stop
        foreach (EtaNetClient client in etaNetClients)
        {
            client.Terminate();
        }

        Console.WriteLine("TerminateServerAndClients(): waiting for clients to finish");

        // wait for all clients to finish
        DateTime timeout = DateTime.Now + TimeSpan.FromSeconds(120); // 30? second timeout
        bool stillRunning;
        do
        {
            stillRunning = false;
            foreach (EtaNetClient client in etaNetClients)
            {
                if (client.State == RunningState.RUNNING)
                    stillRunning = true;
            }

            Thread.Sleep(1000);

            if (DateTime.Now > timeout)
            {
                Console.WriteLine("TerminateServerAndClients(): failed to stop clients after 10 seconds.");
                break;
            }
        }
        while (stillRunning);

        if (!stillRunning)
        {
            Console.WriteLine("TerminateServerAndClients(): flushing client channels.");
            timeout = DateTime.Now + TimeSpan.FromSeconds(30); // 30 second timeout
            foreach (IChannel channel in channels)
            {
                Error error;
                TransportReturnCode retVal;
                do
                {
                    retVal = channel.Flush(out error);

                    Thread.Sleep(10);

                    if (DateTime.Now > timeout)
                    {
                        Console.WriteLine("TerminateServerAndClients(): failed to flush client channels after 10 seconds.");
                        break;
                    }
                }
                while (retVal > TransportReturnCode.SUCCESS);

                if (retVal != TransportReturnCode.SUCCESS)
                    Console.WriteLine("TerminateServerAndClients(): channel.Flush() failed. retVal="
                            + retVal + " errorId=" + error.ErrorId + " error="
                            + error.Text);
            }
        }
        else
        {
            Console.WriteLine("TerminateServerAndClients(): skipping the flushing client channels, due to non-responsive client threads.");
        }

        Console.WriteLine("TerminateServerAndClients(): terminating server");
        // allow extra time for server to receive client packets

        Thread.Sleep(3000);

        server.Terminate();

        Console.WriteLine("TerminateServerAndClients(): waiting for server to finish");
        // wait for server to terminate
        timeout = DateTime.Now + TimeSpan.FromSeconds(30); // 30 second timeout
        bool serverStillRunning = false;
        do
        {
            serverStillRunning = false;
            if (server.State == RunningState.RUNNING)
                serverStillRunning = true;

            Thread.Sleep(100);

            if (DateTime.Now > timeout)
            {
                Console.WriteLine("TerminateServerAndClients(): timeout waiting for server to finish.");
                break;
            }
        }
        while (serverStillRunning);

        // join all client threads
        if (!stillRunning)
        {
            Console.WriteLine("TerminateServerAndClients(): joining client threads");
            foreach (Thread clientThread in clientThreads)
            {
                try
                {
                    clientThread.Join();
                }
                catch (ThreadInterruptedException e)
                {
                    Console.Error.WriteLine("TerminateServerAndClients(): exception while waiting for the client thread: " + e.Message);
                }
            }
        }
        else
        {
            Console.WriteLine("TerminateServerAndClients(): skipping the joining of client threads, due to non-responsive client threads.");
        }

        try
        {
            serverThread.Join();
        }
        catch (ThreadInterruptedException e)
        {
            Console.Error.WriteLine("TerminateServerAndClients(): exception while waiting for the server thread: " + e.Message);
        }

        Console.WriteLine("TerminateServerAndClients(): closing channels");

        // close all channels
        if (!stillRunning)
        {
            foreach (IChannel channel in channels)
            {
                channel.Close(out _); // ignore return code since server will
                                      // close the channels as well, just prior
                                      // to this call.
            }
        }
        else
        {
            Console.WriteLine("TerminateServerAndClients(): skipping the closing of client channels, due to non-responsive client threads.");
        }

        Console.WriteLine("TerminateServerAndClients(): completed");
    }

    private string CompareStatistics(bool packing, int clientMessageSize, EtaNetServer server,
            EtaNetClient[] etaNetClients)
    {
        long clientMessageCount = 0;
        long clientBytesWritten = 0;
        long clientUncompressedBytesWritten = 0;
        long serverMessageCount = 0;
        long serverBytesRead = 0;
        long serverUncompressedBytesRead = 0;
        StringBuilder sb = new StringBuilder();

        // collect and combine all client stats
        foreach (EtaNetClient client in etaNetClients)
        {
            clientMessageCount += client.MessageCount;
            clientBytesWritten += client.BytesWritten;
            clientUncompressedBytesWritten += client.UncompressedBytesWritten;
        }

        // collect server stats
        serverMessageCount = server.MessageCount;
        serverBytesRead = server.BytesRead;
        serverUncompressedBytesRead = server.UncompressedBytesRead;

        // compare server vs client stats
        Console.WriteLine("MessageCount: server=" + serverMessageCount + " clients="
                + clientMessageCount + " diff=" + (serverMessageCount - clientMessageCount));
        Console.WriteLine("Bytes Read vs Written: server=" + serverBytesRead + " clients="
                + clientBytesWritten + " diff=" + (serverBytesRead - clientBytesWritten));
        Console.WriteLine("Uncompressed Bytes Read vs Written: server="
                + serverUncompressedBytesRead + " clients=" + clientUncompressedBytesWritten
                + " diff=" + (serverUncompressedBytesRead - clientUncompressedBytesWritten));

        if (serverMessageCount != clientMessageCount)
        {
            sb.Append("MessageCount mismatch: serverMessageCount=");
            sb.Append(serverMessageCount);
            sb.Append(" clientMessageCount=");
            sb.Append(clientMessageCount);
            sb.Append(". ");
        }

        if (serverBytesRead != clientBytesWritten)
        {
            sb.Append("ServerBytesRead mismatch ClientBytesWritten: serverBytesRead=");
            sb.Append(serverBytesRead);
            sb.Append(" clientBytesWritten=");
            sb.Append(clientBytesWritten);
            sb.Append(". ");
        }

        if (serverUncompressedBytesRead != clientUncompressedBytesWritten)
        {
            sb.Append("ServerUncompressedBytesRead mismatch ClientUncompressedBytesWritten: serverUncompressedBytesRead=");
            sb.Append(serverUncompressedBytesRead);
            sb.Append(" clientUncompressedBytesWritten=");
            sb.Append(clientUncompressedBytesWritten);
            sb.Append(". ");
        }

        if (sb.Length == 0)
            return null; // no errors
        else
            return sb.ToString();
    }

    public class TestArgs
    {
        // set default test args
        internal TimeSpan RunTime = TimeSpan.FromSeconds(30);
        internal int ClientSessionCount = 1;
        internal int ThreadsPerChannels = 1;
        internal int MessageSize = 75;
        internal int FlushInterval = 10;
        internal int GuaranteedOutputBuffers = 7000;
        internal bool GlobalLocking = true;
        internal bool WriteLocking = true;
        internal bool Blocking = false;
        internal bool Packing = false;
        internal CompressionType CompressionType = CompressionType.NONE;
        internal int CompressionLevel = 6;
        internal int RipcMaxUserMsgSize = 6144;
        internal int MaxMessageCount = 0;

        /// <summary>
        /// when enough messages are packed, write is called. The following
        /// options allows write to be called with data to be packed or without.
        /// </summary>
        internal bool CallWriteWithPackedMessage = false;

        public override string ToString()
        {
            return $"RunTime={RunTime}"
                + $"\t\tclientSessionCount={ClientSessionCount}\tthreadsPerChannels={ThreadsPerChannels}\n"
                + $"messageSize={MessageSize}\tflushInterval={FlushInterval}\tguaranteedOutputBuffers={GuaranteedOutputBuffers}\n"
                + $"GlobalLocking={GlobalLocking}\twriteLocking={WriteLocking}\tblocking={Blocking}\npacking={Packing}\t\tcallWriteWithPackedMessage={CallWriteWithPackedMessage}\n"
                + $"CompressionType={CompressionType}\tcompressionLevel={CompressionLevel}\tmaxMessageCount={MaxMessageCount}";
        }
    }

    #endregion

    #region Tests

    /// <summary>
    /// One session, one thread per channel, global lock false, non-blocking
    /// client.
    /// </summary>
    [Fact, Category("Unit")]
    public void LockTest1()
    {
        TestArgs args = new TestArgs();

        args.RunTime = TimeSpan.FromSeconds(5);
        args.ClientSessionCount = 1;
        args.ThreadsPerChannels = 1;
        args.MessageSize = 75;
        args.FlushInterval = 10;
        args.GuaranteedOutputBuffers = 7000;
        args.GlobalLocking = false;
        args.WriteLocking = false;
        args.Blocking = false;
        args.Packing = false;
        args.CallWriteWithPackedMessage = false;
        args.CompressionType = CompressionType.NONE;

        TestRunner("LockTest1", args);
    }


    /// One session, one thread per channel, global lock false, blocking client.
    [Fact, Category("Unit")]
    public void LockTest2()
    {
        TestArgs args = new TestArgs();

        args.RunTime = TimeSpan.FromSeconds(5);
        args.ClientSessionCount = 1;
        args.ThreadsPerChannels = 1;
        args.MessageSize = 75;
        args.FlushInterval = 10;
        args.GuaranteedOutputBuffers = 7000;
        args.GlobalLocking = false;
        args.WriteLocking = false;
        args.Blocking = true;
        args.Packing = false;
        args.CallWriteWithPackedMessage = false;
        args.CompressionType = CompressionType.NONE;

        TestRunner("LockTest2", args);
    }

    /// Two sessions, one thread per channel, global lock true, non-blocking clients.
    [Fact, Category("Unit")]
    public void LockTest3()
    {
        TestArgs args = new TestArgs();

        args.RunTime = TimeSpan.FromSeconds(5);
        args.ClientSessionCount = 2;
        args.ThreadsPerChannels = 1;
        args.MessageSize = 75;
        args.FlushInterval = 10;
        args.GuaranteedOutputBuffers = 7000;
        args.GlobalLocking = true;
        args.WriteLocking = false;
        args.Blocking = false;
        args.Packing = false;
        args.CallWriteWithPackedMessage = false;
        args.CompressionType = CompressionType.NONE;

        TestRunner("LockTest3", args);
    }

    /// Two sessions, one thread per channel, global lock false, blocking clients.
    [Fact, Category("Unit")]
    public void LockTest4()
    {
        TestArgs args = new TestArgs();

        args.RunTime = TimeSpan.FromSeconds(5);
        args.ClientSessionCount = 2;
        args.ThreadsPerChannels = 1;
        args.MessageSize = 75;
        args.FlushInterval = 10;
        args.GuaranteedOutputBuffers = 7000;
        args.GlobalLocking = false;
        args.WriteLocking = false;
        args.Blocking = true;
        args.Packing = false;
        args.CallWriteWithPackedMessage = false;
        args.CompressionType = CompressionType.NONE;

        TestRunner("LockTest4", args);
    }

    /// Three sessions, three thread per channel, global lock true, writeLock
    /// true, non-blocking clients.
    [Fact, Category("Unit")]
    public void LockTest5()
    {
        TestArgs args = new TestArgs();

        args.RunTime = TimeSpan.FromSeconds(5);
        args.ClientSessionCount = 3;
        args.ThreadsPerChannels = 3;
        args.MessageSize = 75;
        args.FlushInterval = 10;
        args.GuaranteedOutputBuffers = 7000;
        args.GlobalLocking = true;
        args.WriteLocking = true;
        args.Blocking = false;
        args.Packing = false;
        args.CallWriteWithPackedMessage = false;
        args.CompressionType = CompressionType.NONE;

        TestRunner("LockTest5", args);
    }

    /// Three sessions, three thread per channel, global lock false, writeLock
    /// true, blocking clients
    [Fact, Category("Unit")]
    public void LockTest6()
    {
        TestArgs args = new TestArgs();

        args.RunTime = TimeSpan.FromSeconds(5);
        args.ClientSessionCount = 3;
        args.ThreadsPerChannels = 3;
        args.MessageSize = 75;
        args.FlushInterval = 10;
        args.GuaranteedOutputBuffers = 7000;
        args.GlobalLocking = false;
        args.WriteLocking = true;
        args.Blocking = true;
        args.Packing = false;
        args.CallWriteWithPackedMessage = false;
        args.CompressionType = CompressionType.NONE;

        TestRunner("LockTest6", args);
    }

    /// Three sessions, three thread per channel, global lock false, writeLock
    /// true, blocking clients, packing (callWriteWithPackedMessage = false).
    [Fact, Category("Unit")]
    public void LockTest8()
    {
        TestArgs args = new TestArgs();

        args.RunTime = TimeSpan.FromSeconds(5);
        args.ClientSessionCount = 3;
        args.ThreadsPerChannels = 3;
        args.MessageSize = 75;
        args.FlushInterval = 10;
        args.GuaranteedOutputBuffers = 7000;
        args.GlobalLocking = false;
        args.WriteLocking = true;
        args.Blocking = true;
        args.Packing = true;
        args.CallWriteWithPackedMessage = false;
        args.CompressionType = CompressionType.NONE;

        TestRunner("LockTest8", args);
    }

    /// Three sessions, three thread per channel, global lock false, writeLock
    /// true, blocking clients, packing (callWriteWithPackedMessage = true).
    [Fact, Category("Unit")]
    public void LockTest10()
    {
        TestArgs args = new TestArgs();

        args.RunTime = TimeSpan.FromSeconds(5);
        args.ClientSessionCount = 3;
        args.ThreadsPerChannels = 3;
        args.MessageSize = 75;
        args.FlushInterval = 10;
        args.GuaranteedOutputBuffers = 7000;
        args.GlobalLocking = false;
        args.WriteLocking = true;
        args.Blocking = true;
        args.Packing = true;
        args.CallWriteWithPackedMessage = true;
        args.CompressionType = CompressionType.NONE;

        TestRunner("LockTest10", args);
    }

    /// Three sessions, three thread per channel, global lock true, writeLock
    /// true, non-blocking clients, ZLIB compression with compression level 6.
    [Fact, Category("Unit")]
    public void LockTest11()
    {
        TestArgs args = new TestArgs();

        args.RunTime = TimeSpan.FromSeconds(5);
        args.ClientSessionCount = 3;
        args.ThreadsPerChannels = 3;
        args.MessageSize = 75;
        args.FlushInterval = 10;
        args.GuaranteedOutputBuffers = 7000;
        args.GlobalLocking = true;
        args.WriteLocking = true;
        args.Blocking = false;
        args.Packing = false;
        args.CallWriteWithPackedMessage = false;
        args.CompressionType = CompressionType.ZLIB;
        args.CompressionLevel = 6;

        TestRunner("LockTest11", args);
    }

    /// Three sessions, three thread per channel, global lock false, writeLock
    /// true, blocking clients, ZLIB compression with compression level 6.
    [Fact, Category("Unit")]
    public void LockTest12()
    {
        TestArgs args = new TestArgs();

        args.RunTime = TimeSpan.FromSeconds(5);
        args.ClientSessionCount = 3;
        args.ThreadsPerChannels = 3;
        args.MessageSize = 75;
        args.FlushInterval = 10;
        args.GuaranteedOutputBuffers = 7000;
        args.GlobalLocking = false;
        args.WriteLocking = true;
        args.Blocking = true;
        args.Packing = false;
        args.CallWriteWithPackedMessage = false;
        args.CompressionType = CompressionType.ZLIB;
        args.CompressionLevel = 6;

        TestRunner("LockTest12", args);
    }

    /// Three sessions, three thread per channel, global lock false, writeLock
    /// true, blocking clients, ZLIB compression with compression level 6 and
    /// packing.
    [Fact, Category("Unit")]
    public void LockTest14()
    {
        TestArgs args = new TestArgs();

        args.RunTime = TimeSpan.FromSeconds(5);
        args.ClientSessionCount = 3;
        args.ThreadsPerChannels = 3;
        args.MessageSize = 75;
        args.FlushInterval = 10;
        args.GuaranteedOutputBuffers = 7000;
        args.GlobalLocking = false;
        args.WriteLocking = true;
        args.Blocking = true;
        args.Packing = true;
        args.CallWriteWithPackedMessage = false;
        args.CompressionType = CompressionType.ZLIB;
        args.CompressionLevel = 6;

        TestRunner("LockTest14", args);
    }

    /// Three sessions, three thread per channel, global lock true, writeLock
    /// true, non-blocking clients, LZ4 compression with compression level 6.
    [Fact, Category("Unit")]
    public void LockTest16()
    {
        TestArgs args = new TestArgs();

        args.RunTime = TimeSpan.FromSeconds(5);
        args.ClientSessionCount = 3;
        args.ThreadsPerChannels = 3;
        args.MessageSize = 75;
        args.FlushInterval = 10;
        args.GuaranteedOutputBuffers = 7000;
        args.GlobalLocking = true;
        args.WriteLocking = true;
        args.Blocking = false;
        args.Packing = false;
        args.CallWriteWithPackedMessage = false;
        args.CompressionType = CompressionType.LZ4;
        args.CompressionLevel = 6;

        TestRunner("LockTest16", args);
    }

    /// Three sessions, three thread per channel, global lock false, writeLock
    /// true, blocking clients, ZL4 compression with compression level 6.
    [Fact, Category("Unit")]
    public void LockTest17()
    {
        TestArgs args = new TestArgs();

        args.RunTime = TimeSpan.FromSeconds(5);
        args.ClientSessionCount = 3;
        args.ThreadsPerChannels = 3;
        args.MessageSize = 75;
        args.FlushInterval = 10;
        args.GuaranteedOutputBuffers = 7000;
        args.GlobalLocking = false;
        args.WriteLocking = true;
        args.Blocking = true;
        args.Packing = false;
        args.CallWriteWithPackedMessage = false;
        args.CompressionType = CompressionType.LZ4;
        args.CompressionLevel = 6;

        TestRunner("LockTest17", args);
    }

    /// Three sessions, three thread per channel, global lock false, writeLock
    /// true, blocking clients, LZ4 compression with compression level 6 and
    /// packing.
    [Fact, Category("Unit")]
    public void LockTest20()
    {
        TestArgs args = new TestArgs();

        args.RunTime = TimeSpan.FromSeconds(5);
        args.ClientSessionCount = 3;
        args.ThreadsPerChannels = 3;
        args.MessageSize = 75;
        args.FlushInterval = 10;
        args.GuaranteedOutputBuffers = 7000;
        args.GlobalLocking = false;
        args.WriteLocking = true;
        args.Blocking = true;
        args.Packing = true;
        args.CallWriteWithPackedMessage = false;
        args.CompressionType = CompressionType.LZ4;
        args.CompressionLevel = 6;

        TestRunner("LockTest20", args);
    }

    /// One session, one thread per channel, global lock false, non-blocking
    /// client. MessageSize of 6142 (Channel.info.maxFragmentSize)
    [Fact, Category("Unit")]
    public void LockTest21()
    {
        TestArgs args = new TestArgs();

        args.RunTime = TimeSpan.FromSeconds(5);
        args.ClientSessionCount = 1;
        args.ThreadsPerChannels = 1;
        args.MessageSize = 6142;
        args.FlushInterval = 10;
        args.GuaranteedOutputBuffers = 7000;
        args.GlobalLocking = false;
        args.WriteLocking = false;
        args.Blocking = false;
        args.Packing = false;
        args.CallWriteWithPackedMessage = false;
        args.CompressionType = CompressionType.NONE;

        TestRunner("LockTest21", args);
    }

    /// One session, one thread per channel, global lock false, non-blocking
    /// client. MessageSize of 6145 (three byte larger than
    /// Channel.info.maxFragmentSize + RIPC_HDR_SIZE), BigBuffers will need to be
    /// used.
    [Fact, Category("Unit")]
    public void LockTest22()
    {
        TestArgs args = new TestArgs();

        args.RunTime = TimeSpan.FromSeconds(5);
        args.ClientSessionCount = 1;
        args.ThreadsPerChannels = 1;
        args.MessageSize = 6145;
        args.FlushInterval = 10;
        args.GuaranteedOutputBuffers = 7000;
        args.GlobalLocking = false;
        args.WriteLocking = false;
        args.Blocking = false;
        args.Packing = false;
        args.CallWriteWithPackedMessage = false;
        args.CompressionType = CompressionType.NONE;

        TestRunner("LockTest22", args);
    }

    /// Test with non-default RIPC Max User Message Size, which the server will
    /// negotiate with the client(s). Select the max message size the client can
    /// use before fragmentation kicks in. That value is RIPC MaxUserMsgSize -
    /// PACKED_HDR (2).
    ///
    /// non-packing, non-compression, non-fragmented, single client scenario.
    [Fact, Category("Unit")]
    public void LockTest23()
    {
        TestArgs args = new TestArgs();

        args.RunTime = TimeSpan.FromSeconds(5);
        args.ClientSessionCount = 1;
        args.ThreadsPerChannels = 1;
        args.MessageSize = 1022;
        args.RipcMaxUserMsgSize = 1024;
        args.FlushInterval = 10;
        args.GuaranteedOutputBuffers = 7000;
        args.GlobalLocking = false;
        args.WriteLocking = false;
        args.Blocking = false;
        args.Packing = false;
        args.CallWriteWithPackedMessage = false;
        args.CompressionType = CompressionType.NONE;

        TestRunner("LockTest23", args);
    }

    /// Test with non-default RIPC Max User Message Size, which the server will
    /// negotiate with the client(s). Select the max message size the client can
    /// use before fragmentation kicks in. That value is RIPC MaxUserMsgSize -
    /// PACKED_HDR (2).
    ///
    /// non-packing, non-compression, non-fragmented, multi-client scenario.
    [Fact, Category("Unit")]
    public void LockTest24()
    {
        TestArgs args = new TestArgs();

        args.RunTime = TimeSpan.FromSeconds(5);
        args.ClientSessionCount = 3;
        args.ThreadsPerChannels = 3;
        args.MessageSize = 1022;
        args.RipcMaxUserMsgSize = 1024;
        args.FlushInterval = 10;
        args.GuaranteedOutputBuffers = 7000;
        args.GlobalLocking = true;
        args.WriteLocking = true;
        args.Blocking = false;
        args.Packing = false;
        args.CallWriteWithPackedMessage = false;
        args.CompressionType = CompressionType.NONE;

        TestRunner("LockTest24", args);
    }

    /// Test with non-default RIPC Max User Message Size, which the server will
    /// negotiate with the client(s). Select the max message size the client can
    /// use before fragmentation kicks in. That value is RIPC MaxUserMsgSize -
    /// PACKED_HDR (2).
    ///
    /// packing, non-compression, non-fragmented, multi-client scenario.
    [Fact, Category("Unit")]
    public void LockTest25()
    {
        TestArgs args = new TestArgs();

        args.RunTime = TimeSpan.FromSeconds(5);
        args.ClientSessionCount = 3;
        args.ThreadsPerChannels = 3;
        args.MessageSize = 1022;
        args.RipcMaxUserMsgSize = 1024;
        args.FlushInterval = 10;
        args.GuaranteedOutputBuffers = 7000;
        args.GlobalLocking = true;
        args.WriteLocking = true;
        args.Blocking = false;
        args.Packing = true;
        args.CallWriteWithPackedMessage = false;
        args.CompressionType = CompressionType.NONE;

        TestRunner("LockTest25", args);
    }

    /// Test with non-default RIPC Max User Message Size, which the server will
    /// negotiate with the client(s). Select the max message size the client can
    /// use before fragmentation kicks in. That value is RIPC MaxUserMsgSize -
    /// PACKED_HDR (2).
    ///
    /// packing (callWriteWithPackedMessage), non-compression, non-fragmented,
    /// multi-client scenario.
    [Fact, Category("Unit")]
    public void LockTest26()
    {
        TestArgs args = new TestArgs();

        args.RunTime = TimeSpan.FromSeconds(5);
        args.ClientSessionCount = 3;
        args.ThreadsPerChannels = 3;
        args.MessageSize = 1022;
        args.RipcMaxUserMsgSize = 1024;
        args.FlushInterval = 10;
        args.GuaranteedOutputBuffers = 7000;
        args.GlobalLocking = true;
        args.WriteLocking = true;
        args.Blocking = false;
        args.Packing = true;
        args.CallWriteWithPackedMessage = true;
        args.CompressionType = CompressionType.NONE;

        TestRunner("LockTest26", args);
    }

    /// Test with non-default RIPC Max User Message Size, which the server will
    /// negotiate with the client(s). Select the max message size the client can
    /// use before fragmentation kicks in. That value is RIPC MaxUserMsgSize -
    /// PACKED_HDR (2).
    ///
    /// Non-packing, compression (ZLIB), non-fragmented, multi-client scenario.
    [Fact, Category("Unit")]
    public void LockTest27()
    {
        TestArgs args = new TestArgs();

        args.RunTime = TimeSpan.FromSeconds(5);
        args.ClientSessionCount = 3;
        args.ThreadsPerChannels = 3;
        args.MessageSize = 1022;
        args.RipcMaxUserMsgSize = 1024;
        args.FlushInterval = 10;
        args.GuaranteedOutputBuffers = 7000;
        args.GlobalLocking = true;
        args.WriteLocking = true;
        args.Blocking = false;
        args.Packing = false;
        args.CallWriteWithPackedMessage = false;
        args.CompressionType = CompressionType.ZLIB;
        args.CompressionLevel = 6;

        TestRunner("LockTest27", args);
    }

    /// Test with non-default RIPC Max User Message Size, which the server will
    /// negotiate with the client(s). Select the max message size the client can
    /// use before fragmentation kicks in. That value is RIPC MaxUserMsgSize -
    /// PACKED_HDR (2).
    ///
    /// packing, compression (ZLIB), non-fragmented, multi-client scenario.
    [Fact, Category("Unit")]
    public void LockTest28()
    {
        TestArgs args = new TestArgs();

        args.RunTime = TimeSpan.FromSeconds(5);
        args.ClientSessionCount = 3;
        args.ThreadsPerChannels = 3;
        args.MessageSize = 1022;
        args.RipcMaxUserMsgSize = 1024;
        args.FlushInterval = 10;
        args.GuaranteedOutputBuffers = 7000;
        args.GlobalLocking = true;
        args.WriteLocking = true;
        args.Blocking = false;
        args.Packing = true;
        args.CallWriteWithPackedMessage = false;
        args.CompressionType = CompressionType.ZLIB;
        args.CompressionLevel = 6;

        TestRunner("LockTest28", args);
    }

    /// Test with non-default RIPC Max User Message Size, which the server will
    /// negotiate with the client(s). Select the max message size the client can
    /// use before fragmentation kicks in. That value is RIPC MaxUserMsgSize -
    /// PACKED_HDR (2).
    ///
    /// Non-packing, compression (LZ4), non-fragmented, multi-client scenario.
    [Fact, Category("Unit")]
    public void LockTest29()
    {
        TestArgs args = new TestArgs();

        args.RunTime = TimeSpan.FromSeconds(5);
        args.ClientSessionCount = 3;
        args.ThreadsPerChannels = 3;
        args.MessageSize = 1022;
        args.RipcMaxUserMsgSize = 1024;
        args.FlushInterval = 10;
        args.GuaranteedOutputBuffers = 7000;
        args.GlobalLocking = true;
        args.WriteLocking = true;
        args.Blocking = false;
        args.Packing = false;
        args.CallWriteWithPackedMessage = false;
        args.CompressionType = CompressionType.LZ4;
        args.CompressionLevel = 6;

        TestRunner("LockTest29", args);
    }

    /// Test with non-default RIPC Max User Message Size, which the server will
    /// negotiate with the client(s). Select the max message size the client can
    /// use before fragmentation kicks in. That value is RIPC MaxUserMsgSize -
    /// PACKED_HDR (2).
    ///
    /// Non-packing, non-compression, fragmented, multi-client scenario.
    [Fact, Category("Unit")]
    public void LockTest30()
    {
        TestArgs args = new TestArgs();

        args.RunTime = TimeSpan.FromSeconds(5);
        args.ClientSessionCount = 3;
        args.ThreadsPerChannels = 3;
        args.MessageSize = 3000;
        args.RipcMaxUserMsgSize = 1024;
        args.FlushInterval = 10;
        args.GuaranteedOutputBuffers = 7000;
        args.GlobalLocking = true;
        args.WriteLocking = true;
        args.Blocking = false;
        args.Packing = false;
        args.CallWriteWithPackedMessage = false;
        args.CompressionType = CompressionType.NONE;

        TestRunner("LockTest30", args);
    }

    /// Three sessions, three threads per channel, global lock true, write
    /// locking true, ZLIB compression with compression level 0, non-packing,
    /// non-blocking client. MessageSize of 6142 (Channel.info.maxFragmentSize)
    [Fact, Category("Unit")]
    public void LockTest31()
    {
        TestArgs args = new TestArgs();

        args.RunTime = TimeSpan.FromSeconds(5);
        args.ClientSessionCount = 3;
        args.ThreadsPerChannels = 3;
        args.MessageSize = 6142;
        args.FlushInterval = 10;
        args.GuaranteedOutputBuffers = 7000;
        args.GlobalLocking = true;
        args.WriteLocking = true;
        args.Blocking = false;
        args.Packing = false;
        args.CallWriteWithPackedMessage = false;
        args.CompressionType = CompressionType.ZLIB;
        args.CompressionLevel = 0;

        TestRunner("LockTest31", args);
    }

    /// Three sessions, three threads per channel, global lock true, write
    /// locking true, ZLIB compression with compression level 9, non-packing,
    /// non-blocking client. MessageSize of 6142 (Channel.info.maxFragmentSize)
    [Fact, Category("Unit")]
    public void LockTest33()
    {
        TestArgs args = new TestArgs();

        args.RunTime = TimeSpan.FromSeconds(5);
        args.ClientSessionCount = 3;
        args.ThreadsPerChannels = 3;
        args.MessageSize = 6142;
        args.FlushInterval = 10;
        args.GuaranteedOutputBuffers = 7000;
        args.GlobalLocking = true;
        args.WriteLocking = true;
        args.Blocking = false;
        args.Packing = false;
        args.CallWriteWithPackedMessage = false;
        args.CompressionType = CompressionType.ZLIB;
        args.CompressionLevel = 9;

        TestRunner("LockTest33", args);
    }

    /// Three sessions, three threads per channel, global lock true, write
    /// locking true, ZLIB compression with compression level 9, packing,
    /// non-blocking client. MessageSize of 6142 (Channel.info.maxFragmentSize)
    [Fact, Category("Unit")]
    public void LockTest34()
    {
        TestArgs args = new TestArgs();

        args.RunTime = TimeSpan.FromSeconds(5);
        args.ClientSessionCount = 3;
        args.ThreadsPerChannels = 3;
        args.MessageSize = 6142;
        args.FlushInterval = 10;
        args.GuaranteedOutputBuffers = 7000;
        args.GlobalLocking = true;
        args.WriteLocking = true;
        args.Blocking = false;
        args.Packing = true;
        args.CallWriteWithPackedMessage = false;
        args.CompressionType = CompressionType.ZLIB;
        args.CompressionLevel = 9;

        TestRunner("LockTest34", args);
    }

    /// Three sessions, three threads per channel, global lock true, write
    /// locking true, LZ4 compression with compression level 0, non-packing,
    /// non-blocking client. MessageSize of 6142 (Channel.info.maxFragmentSize)
    [Fact, Category("Unit")]
    public void LockTest35()
    {
        TestArgs args = new TestArgs();

        args.RunTime = TimeSpan.FromSeconds(5);
        args.ClientSessionCount = 3;
        args.ThreadsPerChannels = 3;
        args.MessageSize = 6142;
        args.FlushInterval = 10;
        args.GuaranteedOutputBuffers = 7000;
        args.GlobalLocking = true;
        args.WriteLocking = true;
        args.Blocking = false;
        args.Packing = false;
        args.CallWriteWithPackedMessage = false;
        args.CompressionType = CompressionType.LZ4;
        args.CompressionLevel = 0;

        TestRunner("LockTest35", args);
    }

    /// Three sessions, three threads per channel, global lock true, write
    /// locking true, LZ4 compression with compression level 0, packing,
    /// non-blocking client. MessageSize of 6142 (Channel.info.maxFragmentSize)
    [Fact, Category("Unit")]
    public void LockTest36()
    {
        TestArgs args = new TestArgs();

        args.RunTime = TimeSpan.FromSeconds(5);
        args.ClientSessionCount = 3;
        args.ThreadsPerChannels = 3;
        args.MessageSize = 6142;
        args.FlushInterval = 10;
        args.GuaranteedOutputBuffers = 7000;
        args.GlobalLocking = true;
        args.WriteLocking = true;
        args.Blocking = false;
        args.Packing = true;
        args.CallWriteWithPackedMessage = false;
        args.CompressionType = CompressionType.LZ4;
        args.CompressionLevel = 0;

        TestRunner("LockTest36", args);
    }

    /// Three sessions, three threads per channel, global lock true, write
    /// locking true, LZ4 compression with compression level 9, non-packing,
    /// non-blocking client. MessageSize of 6142 (Channel.info.maxFragmentSize)
    [Fact, Category("Unit")]
    public void LockTest37()
    {
        TestArgs args = new TestArgs();

        args.RunTime = TimeSpan.FromSeconds(5);
        args.ClientSessionCount = 3;
        args.ThreadsPerChannels = 3;
        args.MessageSize = 6142;
        args.FlushInterval = 10;
        args.GuaranteedOutputBuffers = 7000;
        args.GlobalLocking = true;
        args.WriteLocking = true;
        args.Blocking = false;
        args.Packing = false;
        args.CallWriteWithPackedMessage = false;
        args.CompressionType = CompressionType.LZ4;
        args.CompressionLevel = 9;

        TestRunner("LockTest37", args);
    }

    /// Three sessions, three threads per channel, global lock true, write
    /// locking true, LZ4 compression with compression level 9, packing,
    /// non-blocking client. MessageSize of 6142 (Channel.info.maxFragmentSize)
    [Fact, Category("Unit")]
    public void LockTest38()
    {
        TestArgs args = new TestArgs();

        args.RunTime = TimeSpan.FromSeconds(5);
        args.ClientSessionCount = 3;
        args.ThreadsPerChannels = 3;
        args.MessageSize = 6142;
        args.FlushInterval = 10;
        args.GuaranteedOutputBuffers = 7000;
        args.GlobalLocking = true;
        args.WriteLocking = true;
        args.Blocking = false;
        args.Packing = true;
        args.CallWriteWithPackedMessage = false;
        args.CompressionType = CompressionType.LZ4;
        args.CompressionLevel = 9;

        TestRunner("LockTest38", args);
    }

    private void TestRunner(String testName, TestArgs args)
    {
        Console.WriteLine("--------------------------------------------------------------------------------");
        Console.WriteLine("Test: " + testName);
        Console.WriteLine(args.ToString());
        Console.WriteLine("--------------------------------------------------------------------------------");
        DateTime endTime = DateTime.Now + args.RunTime;

        Assert.True((args.Blocking && args.GlobalLocking) == false,
            "Cannot have client Blocking and GlobalLocking=true with server using same JVM, GlobalLocking will deadlock.");

        if (args.Packing)
            Assert.True(args.MessageSize <= 6142,
                "Cannot perform packing on message sizes greater than 6142.");

        // BindOptions
        BindOptions bindOptions = DefaultBindOptions();
        bindOptions.MaxFragmentSize = args.RipcMaxUserMsgSize;
        bindOptions.CompressionType = (CompressionType)args.CompressionType;
        if (args.CompressionType > CompressionType.NONE)
            bindOptions.CompressionLevel = args.CompressionLevel;

        // AcceptOptions
        AcceptOptions acceptOptions = DefaultAcceptOptions();

        // Client sessions and threads
        EtaNetClient[] etaNetClients = new EtaNetClient[args.ClientSessionCount * args.ThreadsPerChannels];
        Thread[] clientThreads = new Thread[args.ClientSessionCount * args.ThreadsPerChannels];

        // create the server thread and start it
        EtaNetServer server = new EtaNetServer(bindOptions, acceptOptions, args);
        Thread serverThread = new Thread(server.Run);
        serverThread.Start();

        if (!WaitForStateRunning(server))
        {
            Assert.Fail("server terminated while waiting for RUNNING state, error="
                               + server.ErrorMsg);
        }
        else
        {
            // start the channels that represent client sessions
            IChannel[] channels = StartClientChannels(args.ClientSessionCount,
                                                      args.ThreadsPerChannels,
                                                      args.GuaranteedOutputBuffers, args.Blocking,
                                                      args.WriteLocking, args.CompressionType);
            Assert.NotNull(channels);

            // start the client threads.
            StartClientThreads(args, channels, etaNetClients, clientThreads);

            // Run until timeout or server.state() is RunningState.TERMINATED
            while (server.State == RunningState.RUNNING
                    && DateTime.Now < endTime)
                Thread.Sleep(100);

            TerminateServerAndClients(serverThread, server, clientThreads, etaNetClients, channels);

            // Server errors will take precedence over the statistics
            // comparison. If both the statistics comparison and the server
            // failed, fail the test based on the server failure. Otherwise,
            // fail the test on the statistics comparison failure.
            String result = CompareStatistics(args.Packing, args.MessageSize, server, etaNetClients);
            if (result != null)
            {
                if (server.ErrorMsg == null)
                    Assert.Fail(result);
                else
                    Console.WriteLine("server failed (errorMsg to follow) and comparison of stats failed with: "
                                        + result);
            }
        }

        // If a server failure occurred, fail the test.
        if (server.ErrorMsg != null)
            Assert.Fail(server.ErrorMsg);
    }

    #endregion
}
