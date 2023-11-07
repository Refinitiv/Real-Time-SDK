/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Collections.Generic;
using System.Net.Sockets;
using System.Threading;

using LSEG.Eta.Codec;
using LSEG.Eta.Example.Common;
using LSEG.Eta.PerfTools.Common;
using LSEG.Eta.Rdm;
using LSEG.Eta.Common;
using LSEG.Eta.Transports;
using LSEG.Eta.ValueAdd.Rdm;
using LSEG.Eta.ValueAdd.Reactor;

using ItemInfo = LSEG.Eta.PerfTools.Common.ItemInfo;
using LoginHandler = LSEG.Eta.PerfTools.Common.LoginHandler;
using ProtocolType = LSEG.Eta.Transports.ProtocolType;
using ProviderSession = LSEG.Eta.PerfTools.Common.ProviderSession;

namespace LSEG.Eta.Perftools.NIProvPerf
{

    /// Non-interactive provider implementation of the provider thread.
    /// Handles connecting to channel and processing of login response,
    /// source directory refresh, and market data refresh/update messages.
    public class NIProviderThread : ProviderThread, IReactorChannelEventCallback, IDefaultMsgCallback, IRDMLoginMsgCallback
    {
        private static readonly int CONNECTION_RETRY_TIME = 1;          // in seconds
        private const int DIRECTORY_REFRESH_STREAM_ID = -1;

        private DecodeIterator m_DecodeIter = new();                    // decode iterator
        private Msg m_Msg = new();                                      // response message
        private LoginHandler m_LoginHandler = new();                    // login handler
        private NIDirectoryProvider m_DirectoryProvider = new();        // directory provider
        private ChannelInfo _m_ChannelInfo = new();                     // channel information
        private ConnectOptions? m_ConnectOpts;                           // connect options for non-interactive provider
        private IChannel? m_Channel;
        private ProviderSession? m_ProvSession;
        private InProgInfo m_InProg = new();                            // connection in progress information
        private WriteArgs m_WriteArgs = new();
        private ReadArgs m_ReadArgs = new();
        private PingHandler m_PingHandler = new();                      // ping handler
        private Msg m_ResponseMsg = new();                              // response message

        private Reactor? m_Reactor; // Use the VA Reactor instead of the ETA Channel for sending and receiving
        private ReactorOptions m_ReactorOptions; // Use the VA Reactor instead of the ETA Channel for sending and receiving
        private NIProviderRole m_Role; // Use the VA Reactor instead of the ETA Channel for sending and receiving

        private ReactorConnectOptions m_ConnectOptions; // Use the VA Reactor instead of the ETA Channel for sending and receiving
        private ReactorDispatchOptions m_DispatchOptions; // Use the VA Reactor instead of the ETA Channel for sending and receiving
        private ReactorConnectInfo m_ConnectInfo; // Use the VA Reactor instead of the ETA Channel for sending and receiving
        private ReactorChannelInfo m_ReactorChannnelInfo; // Use the VA Reactor instead of the ETA Channel for sending and receiving
        private ReactorChannel? m_ReactorChannel; // Use the VA Reactor instead of the ETA Channel for sending and receiving

        private bool m_ShouldWrite;

        List<Socket> m_ReadSocketList = new List<Socket>();
        List<Socket> m_WriteSocketList = new List<Socket>();

        public NIProviderThread(XmlMsgData xmlMsgData) : base(xmlMsgData)
        {
            m_Error = new();
            m_ReactorOptions = new ReactorOptions();
            m_Role = new NIProviderRole();
            m_ConnectOptions = new ReactorConnectOptions();
            m_DispatchOptions = new ReactorDispatchOptions();
            m_ConnectInfo = new ReactorConnectInfo();
            m_ReactorChannnelInfo = new ReactorChannelInfo();

            InitTimeFunctions();
        }

        /// <summary>
        /// Initialize the NIProvider thread for ETA Channel usage
        /// </summary>
        private void InitializeChannel()
        {
            // set-up directory provider
            m_DirectoryProvider.ServiceName = NIProvPerfConfig.ServiceName;
            m_DirectoryProvider.ServiceId = NIProvPerfConfig.ServiceId;
            m_DirectoryProvider.OpenLimit = NIProvPerfConfig.OpenLimit;
            m_DirectoryProvider.InitService(m_XmlMsgData);

            // Configure connection options.
            m_ConnectOpts = new();
            m_ConnectOpts.GuaranteedOutputBuffers = NIProvPerfConfig.GuaranteedOutputBuffers;
            m_ConnectOpts.MajorVersion = Codec.Codec.MajorVersion();
            m_ConnectOpts.MinorVersion = Codec.Codec.MinorVersion();
            m_ConnectOpts.ProtocolType = (ProtocolType)Codec.Codec.ProtocolType();
            if (NIProvPerfConfig.SendBufSize > 0)
            {
                m_ConnectOpts.SysSendBufSize = NIProvPerfConfig.SendBufSize;
            }
            if (NIProvPerfConfig.RecvBufSize > 0)
            {
                m_ConnectOpts.SysRecvBufSize = NIProvPerfConfig.RecvBufSize;
            }
            if (NIProvPerfConfig.SendTimeout > 0)
            {
                m_ConnectOpts.SendTimeout = NIProvPerfConfig.SendTimeout;
            }
            if (NIProvPerfConfig.RecvTimeout > 0)
            {
                m_ConnectOpts.ReceiveTimeout = NIProvPerfConfig.RecvTimeout;
            }
            m_ConnectOpts.ConnectionType = NIProvPerfConfig.ConnectionType;
            m_ConnectOpts.UnifiedNetworkInfo.Address = NIProvPerfConfig.HostName;
            m_ConnectOpts.UnifiedNetworkInfo.ServiceName = NIProvPerfConfig.PortNo;
            m_ConnectOpts.UnifiedNetworkInfo.InterfaceName = NIProvPerfConfig.InterfaceName;

            m_ConnectOpts.TcpOpts.TcpNoDelay = NIProvPerfConfig.TcpNoDelay;

            // Setup connection.
            SetupChannelConnection();

            if (m_ProvSession!.PrintEstimatedMsgSizes(out m_Error) != PerfToolsReturnCode.SUCCESS)
            {
                CloseChannelAndShutDown(m_Error!.Text);
            }

            // set login parameters
            m_LoginHandler.ApplicationName = "NIProvPerf";
            m_LoginHandler.UserName = NIProvPerfConfig.Username;
            m_LoginHandler.Role = Login.RoleTypes.PROV;

            // Send login request message
            ITransportBuffer? msg = m_LoginHandler.GetRequest(m_Channel!, out m_Error);
            if (msg != null)
            {
                Write(msg);
            }
            else
            {
                CloseChannelAndShutDown($"Sending login request failed: {m_Error?.Text}");
            }

            // Initialize ping handler
            m_PingHandler.InitPingHandler(m_Channel!.PingTimeOut);

            m_ProvSession.TimeActivated = (long)GetTime.GetMicroseconds();
        }

        /// <summary>
        /// Initialize the NIProvider thread for ETA VA Reactor usage.
        /// </summary>
        private void InitializeReactor()
        {
            // initialize Reactor
            m_Reactor = Reactor.CreateReactor(m_ReactorOptions, out var errorInfo);
            if (m_Reactor == null && errorInfo?.Code != ReactorReturnCode.SUCCESS)
            {
                Console.Error.WriteLine($"CreateReactor() failed ({errorInfo?.Code}): {errorInfo}");
                Environment.Exit((int)ReactorReturnCode.FAILURE);
            }

            /* Configure connection options. */
            m_ConnectInfo.ConnectOptions.GuaranteedOutputBuffers = NIProvPerfConfig.GuaranteedOutputBuffers;
            m_ConnectInfo.ConnectOptions.MajorVersion = Codec.Codec.MajorVersion();
            m_ConnectInfo.ConnectOptions.MinorVersion = Codec.Codec.MinorVersion();
            m_ConnectInfo.ConnectOptions.ProtocolType = (ProtocolType)Codec.Codec.ProtocolType();
            if (NIProvPerfConfig.SendBufSize > 0)
            {
                m_ConnectInfo.ConnectOptions.SysSendBufSize = NIProvPerfConfig.SendBufSize;
            }
            if (NIProvPerfConfig.RecvBufSize > 0)
            {
                m_ConnectInfo.ConnectOptions.SysRecvBufSize = NIProvPerfConfig.RecvBufSize;
            }
            if (NIProvPerfConfig.SendTimeout > 0)
            {
                m_ConnectInfo.ConnectOptions.SendTimeout = NIProvPerfConfig.SendTimeout;
            }
            if (NIProvPerfConfig.RecvTimeout > 0)
            {
                m_ConnectInfo.ConnectOptions.ReceiveTimeout = NIProvPerfConfig.RecvTimeout;
            }

            m_ConnectInfo.ConnectOptions.ConnectionType = NIProvPerfConfig.ConnectionType;
            if (m_ConnectInfo.ConnectOptions.ConnectionType == ConnectionType.SOCKET
                || m_ConnectInfo.ConnectOptions.ConnectionType == ConnectionType.ENCRYPTED)
            {
                m_ConnectInfo.ConnectOptions.UnifiedNetworkInfo.Address = NIProvPerfConfig.HostName;
                m_ConnectInfo.ConnectOptions.UnifiedNetworkInfo.ServiceName = NIProvPerfConfig.PortNo;
                m_ConnectInfo.ConnectOptions.UnifiedNetworkInfo.InterfaceName = NIProvPerfConfig.InterfaceName;
            }

            if (m_ConnectInfo.ConnectOptions.ConnectionType == ConnectionType.ENCRYPTED)
            {
                m_ConnectInfo.ConnectOptions.EncryptionOpts.EncryptedProtocol = NIProvPerfConfig.EncryptedConnectionType;

                try
                {
                    m_ConnectInfo.ConnectOptions.EncryptionOpts.EncryptionProtocolFlags = NIProvPerfConfig.EncryptionProtocol.Value;
                }
                catch (Exception ex)
                {
                    Console.Error.WriteLine($"Error: {ex.Message}.");
                    System.Environment.Exit((int)TransportReturnCode.FAILURE);
                }
            }
            m_ConnectInfo.ConnectOptions.TcpOpts.TcpNoDelay = NIProvPerfConfig.TcpNoDelay;

            m_ConnectOptions.ConnectionList.Add(m_ConnectInfo);

            // set consumer role information
            m_Role.ChannelEventCallback = this;
            m_Role.DefaultMsgCallback = this;
            m_Role.LoginMsgCallback = this;
            m_Role.InitDefaultRDMLoginRequest();
            // set login parameters
            m_Role.RdmLoginRequest!.HasAttrib = true;
            m_Role.RdmLoginRequest.LoginAttrib.HasApplicationName = true;
            m_Role.RdmLoginRequest.LoginAttrib.ApplicationName.Data("NIProvPerf");
            if (!String.IsNullOrEmpty(NIProvPerfConfig.Username))
            {
                m_Role.RdmLoginRequest.UserName.Data(NIProvPerfConfig.Username);
            }
            // set-up directory provider and initialize directory refresh
            m_DirectoryProvider.ServiceName = NIProvPerfConfig.ServiceName;
            m_DirectoryProvider.ServiceId = NIProvPerfConfig.ServiceId;
            m_DirectoryProvider.OpenLimit = NIProvPerfConfig.OpenLimit;
            m_DirectoryProvider.InitService(m_XmlMsgData);
            m_DirectoryProvider.InitRefresh(-1);
            m_Role.RdmDirectoryRefresh = m_DirectoryProvider.DirectoryRefresh;

            // Setup connection.
            SetupReactorConnection();
        }

        /// <summary>
        /// Set-up the ETA Channel connection for the NIProvider thread
        /// </summary>
        private void SetupChannelConnection()
        {
            TransportReturnCode handshake;
            while (true)
            {
                m_Channel = Transport.Connect(m_ConnectOpts, out m_Error);
                if (m_Channel == null)
                {
                    Console.Error.WriteLine($"Error: Transport connect failure: {m_Error?.Text}");
                    Environment.Exit(-1);
                }

                while ((handshake = m_Channel.Init(m_InProg, out m_Error)) != TransportReturnCode.SUCCESS)
                {
                    if (handshake == TransportReturnCode.FAILURE)
                        break;

                    try
                    {
                        Thread.Sleep(1000);
                    }
                    catch (Exception e)
                    {
                        Console.Error.WriteLine($"Thread.Sleep failed: {e.Message}");
                        Environment.Exit(-1);
                    }
                }
                if (handshake == TransportReturnCode.SUCCESS)
                    break;

                Console.WriteLine($"Connection failure, error: {m_Error?.Text}. Will retry shortly.");
                try
                {
                    Thread.Sleep(CONNECTION_RETRY_TIME * 1000);
                }
                catch (Exception e)
                {
                    Console.WriteLine($"Thread.Sleep failed: {e.Message}");
                    Environment.Exit(-1);
                }
            }
            Console.WriteLine("Connected.");

            m_ProvSession = new ProviderSession(m_XmlMsgData, m_ItemEncoder);
            ClientChannelInfo clientChannelInfo = new ClientChannelInfo();
            clientChannelInfo.Channel = m_Channel;
            m_ProvSession.Init(clientChannelInfo);

            // set the high water mark if configured
            if (ProviderPerfConfig.HighWaterMark > 0)
            {
                if (m_Channel.IOCtl(IOCtlCode.HIGH_WATER_MARK, ProviderPerfConfig.HighWaterMark, out m_Error) != TransportReturnCode.SUCCESS)
                {
                    CloseChannelAndShutDown($"Channel.IOCtl() failed, error: {m_Error?.Text}");
                }
            }

            // retrieve and print out channel information
            if (m_Channel.Info(_m_ChannelInfo, out m_Error) != TransportReturnCode.SUCCESS)
            {
                CloseChannelAndShutDown($"Channel.Info() failed: {m_Error?.Text}");
            }
            Console.WriteLine($"Channel active. {_m_ChannelInfo.ToString()}");

            // Check that we can successfully pack, if packing messages.
            if (NIProvPerfConfig.TotalBuffersPerPack > 1 && NIProvPerfConfig.PackingBufferLength > _m_ChannelInfo.MaxFragmentSize)
            {
                Console.Error.WriteLine($"Error(Channel {m_Channel.Socket}): MaxFragmentSize {_m_ChannelInfo.MaxFragmentSize} " +
                    $"is too small for packing buffer size {NIProvPerfConfig.PackingBufferLength}");
                Environment.Exit((int)PerfToolsReturnCode.FAILURE);
            }
        }

        /// <summary>
        /// Set-up the ETA VA Reactor connection for the NIProvider thread.
        /// </summary>
        private void SetupReactorConnection()
        {
            // connect via Reactor
            ReactorReturnCode ret;
            if ((ret = m_Reactor!.Connect(m_ConnectOptions, m_Role, out var errorInfo)) < ReactorReturnCode.SUCCESS)
            {
                Console.Error.WriteLine("Reactor.connect failed with return code: " + ret + " error = " + errorInfo?.Error.Text);
                Environment.Exit((int)ReactorReturnCode.FAILURE);
            }
        }

        /// <summary>
        /// Run the non-interactive provider thread. Sets up the directory provider and connection.
        /// Then enters loop that continually reads from and sends refresh/update messages to the connected channel
        /// </summary>
        override public void Run()
        {
            if (!NIProvPerfConfig.UseReactor) // use ETA Channel for sending and receiving
            {
                InitializeChannel();
            }
            else // use ETA VA Reactor for sending and receiving
            {
                InitializeReactor();
            }

            // Determine update rates on per-tick basis
            double nextTickTime = InitNextTickTime!();

            //the main loop
            while (!Shutdown)
            {
                // read until no more to read and then write leftover from previous burst
                double selectTime = SelectTime!(nextTickTime);
                SocketSelect(selectTime);

                // Handle pings
                if (!NIProvPerfConfig.UseReactor) // use ETA Channel for sending and receiving
                {
                    if (m_PingHandler.HandlePings(m_Channel!, out m_Error) != TransportReturnCode.SUCCESS)
                    {
                        CloseChannelAndShutDown($"Error handling pings: {m_Error?.Text}");
                    }
                }

                if (nextTickTime <= CurrentTime!())
                {
                    nextTickTime = NextTickTime!(nextTickTime);
                    SendMsgBurst();
                }
            }

            ShutdownAck = true;
        }

        /// <summary>
        /// Reads from a channel or flushses the channel if necessary
        /// </summary>
        /// <param name="selectTime">time the socket will wait in Select method</param>
        private void SocketSelect(double selectTime)
        {
            try
            {
                m_ReadSocketList.Clear();
                m_WriteSocketList.Clear();
                if (!NIProvPerfConfig.UseReactor
                    && m_Channel!.Socket != null
                    && m_Channel.Socket.Connected)
                {
                    m_ReadSocketList.Add(m_Channel.Socket);
                    if (m_ShouldWrite)
                    {
                        m_WriteSocketList.Add(m_Channel.Socket);
                    }
                }
                else if (NIProvPerfConfig.UseReactor)
                {
                    m_ReadSocketList.Add(m_Reactor!.EventSocket!);
                    if (m_ReactorChannel != null)
                    {
                        m_ReadSocketList.Add(m_ReactorChannel.Socket!);
                        if (m_ShouldWrite)
                            m_WriteSocketList.Add(m_ReactorChannel.Socket!);
                    }
                }

                Socket.Select(m_ReadSocketList, m_ShouldWrite ? m_WriteSocketList : null, null, (int)selectTime);

                if (m_ReadSocketList.Count > 0)
                {
                    if (!NIProvPerfConfig.UseReactor) // use ETA Channel for sending and receiving
                    {

                        ITransportBuffer msgBuf;
                        do // read until no more to read
                        {
                            msgBuf = m_Channel!.Read(m_ReadArgs, out m_Error);
                            if (msgBuf != null)
                            {
                                ProcessResponse(msgBuf);

                                //set flag for server message received
                                m_PingHandler.ReceivedRemoteMsg = true;
                            }
                            else
                            {
                                if (m_ReadArgs.ReadRetVal == TransportReturnCode.READ_PING)
                                {
                                    //set flag for server message received
                                    m_PingHandler.ReceivedRemoteMsg = true;
                                }
                            }
                        }
                        while (m_ReadArgs.ReadRetVal > TransportReturnCode.SUCCESS);
                    }
                    else // use ETA VA Reactor for sending and receiving
                    {
                        ReactorReturnCode ret;
                        ReactorErrorInfo? errorInfo;

                        /* read until no more to read */
                        while ((ret = m_Reactor!.Dispatch(m_DispatchOptions, out errorInfo)) > 0) {}

                        if (ret == ReactorReturnCode.FAILURE)
                        {
                            if (m_ReactorChannel?.State != ReactorChannelState.CLOSED
                                && m_ReactorChannel?.State != ReactorChannelState.DOWN_RECONNECTING)
                            {
                                CloseChannelAndShutDown("ReactorChannel dispatch failed: " + ret + "(" + errorInfo?.Error.Text + ")");
                                Environment.Exit((int)ReactorReturnCode.FAILURE);
                            }
                        }
                    }
                }

                if (!NIProvPerfConfig.UseReactor
                    && m_ShouldWrite
                    && m_WriteSocketList.Count > 0)
                {
                    TransportReturnCode ret;
                    if ((ret = m_Channel!.Flush(out m_Error)) == TransportReturnCode.SUCCESS)
                    {
                        m_ShouldWrite = false;
                    }
                    else if (ret < TransportReturnCode.SUCCESS)
                    {
                        CloseChannelAndShutDown($"Failed to write data to channel, return code: {ret}");
                    }
                }
            }
            catch (Exception e)
            {
                CloseChannelAndShutDown(e.Message);
            }
        }

        /// <summary>
        /// Process transport response
        /// </summary>
        /// <param name="buffer"><see cref="ITransportBuffer"/> instance containing the response message</param>
        private void ProcessResponse(ITransportBuffer buffer)
        {
            m_DecodeIter.Clear();
            m_DecodeIter.SetBufferAndRWFVersion(buffer, m_Channel!.MajorVersion, m_Channel.MinorVersion);

            CodecReturnCode codecReturnCode = m_ResponseMsg.Decode(m_DecodeIter);
            if (codecReturnCode != CodecReturnCode.SUCCESS)
            {
                CloseChannelAndShutDown($"DecodeMsg() Error: {codecReturnCode.GetAsString()}");
            }

            if (m_ResponseMsg.DomainType == (int)DomainType.LOGIN)
            {
                TransportReturnCode transportReturnCode = m_LoginHandler.ProcessResponse(m_ResponseMsg, m_DecodeIter, out m_Error);
                if (transportReturnCode != TransportReturnCode.SUCCESS)
                {
                    if (transportReturnCode != TransportReturnCode.SUCCESS)
                    {
                        CloseChannelAndShutDown(m_Error != null ? m_Error.Text : "");
                    }
                }

                if (m_LoginHandler.LoginState == ConsumerLoginState.OK_SOLICITED)
                {
                    // Send source directory refresh message
                    ITransportBuffer? sourceDir = m_DirectoryProvider!.EncodeRefresh(m_Channel, DIRECTORY_REFRESH_STREAM_ID, out m_Error);
                    if (sourceDir != null)
                    {
                        Write(sourceDir);
                    }
                    else
                    {
                        CloseChannelAndShutDown($"PublishDirectoryRefresh() failed: {m_Error?.Text}");
                    }

                    // create item list to publish
                    CreateItemList();
                }
                else
                {
                    Console.WriteLine("Login stream closed.\n");
                }
            }
            else
            {
                Console.Error.WriteLine($"Received message with unhandled domain: {m_Msg.DomainType}");
            }
        }

        void CreateItemList()
        {
            if (NIProvPerfConfig.ItemPublishCount > 0)
            {
                int itemListUniqueIndex;
                int itemListCount;
                int itemListCountRemainder;

                // Figure out which items this thread should publish.

                // Calculate unique index for each thread. Each thread publishes a common
                // and unique set of items. Unique index is so each thread has a unique
                // index into the shared item list. Unique items for this provider are after
                // the items assigned to providers with a lower index.
                itemListUniqueIndex = NIProvPerfConfig.CommonItemCount;
                itemListUniqueIndex += (NIProvPerfConfig.ItemPublishCount - NIProvPerfConfig.CommonItemCount) / NIProvPerfConfig.ThreadCount * (int)ProviderIndex;

                itemListCount = NIProvPerfConfig.ItemPublishCount / NIProvPerfConfig.ThreadCount;
                itemListCountRemainder = NIProvPerfConfig.ItemPublishCount % NIProvPerfConfig.ThreadCount;

                if (ProviderIndex < itemListCountRemainder)
                {
                    // This provider publishes an extra item
                    itemListCount += 1;

                    // Shift index by one for each provider before this one, since they publish extra items too.
                    itemListUniqueIndex += (int)ProviderIndex;
                }
                else
                {
                    // Shift index by one for each provider that publishes an extra item.
                    itemListUniqueIndex += itemListCountRemainder;
                }

                if (AddPublishingItems(NIProvPerfConfig.ItemFilename, m_ProvSession!, NIProvPerfConfig.CommonItemCount, itemListUniqueIndex, itemListCount - NIProvPerfConfig.CommonItemCount, NIProvPerfConfig.ServiceId)
                    != PerfToolsReturnCode.SUCCESS)
                {
                    CloseChannelAndShutDown("AddPublishingItems() failed\n");
                }
                else
                {
                    Console.WriteLine("Created publishing list.\n");
                }
            }
        }

        /// <summary>
        /// Loads xml file containing item messages
        /// to send and add item information to be published to item watch list
        /// </summary>
        /// <param name="xmlItemInfoFile">file with xml ItemInfo data</param>
        /// <param name="providerSession">the provider session</param>
        /// <param name="commonItemCount">the common item count</param>
        /// <param name="itemListUniqueIndex">item index</param>
        /// <param name="uniqueItemCount">unique item count</param>
        /// <param name="serviceId">the id of the current service</param>
        /// <returns><see cref="PerfToolsReturnCode"/> indicating the status of the operation</returns>
        private PerfToolsReturnCode AddPublishingItems(string xmlItemInfoFile, ProviderSession providerSession, int commonItemCount, int itemListUniqueIndex, int uniqueItemCount, int serviceId)
        {
            XmlItemInfoList _xmlItemInfoList = new XmlItemInfoList(itemListUniqueIndex + uniqueItemCount);
            if (_xmlItemInfoList.ParseFile(xmlItemInfoFile) == PerfToolsReturnCode.FAILURE)
            {
                Console.Error.WriteLine($"Failed to load item list from file '{xmlItemInfoFile}'.");
                return PerfToolsReturnCode.FAILURE;
            }

            int itemListIndex = 0;
            for (int i = 0; i < commonItemCount + uniqueItemCount; ++i)
            {
                if (itemListIndex == commonItemCount && itemListIndex < itemListUniqueIndex)
                {
                    itemListIndex = itemListUniqueIndex;
                }
                MsgKey msgKey = new MsgKey();
                msgKey.Flags = MsgKeyFlags.HAS_NAME_TYPE | MsgKeyFlags.HAS_NAME | MsgKeyFlags.HAS_SERVICE_ID;
                msgKey.NameType = InstrumentNameTypes.RIC;
                msgKey.ServiceId = serviceId;
                msgKey.Name.Data(_xmlItemInfoList.ItemInfoList[itemListIndex].Name);
                ItemAttributes attributes = new ItemAttributes();
                attributes.DomainType = _xmlItemInfoList.ItemInfoList[itemListIndex].DomainType;
                attributes.MsgKey = msgKey;

                ItemInfo? itemInfo = providerSession!.CreateItemInfo(attributes, - i - 6, out m_Error);
                if (itemInfo == null)
                {
                    Console.WriteLine($"Failed creating ItemInfo: {m_Error?.Text}");
                    return PerfToolsReturnCode.FAILURE;
                }
                itemInfo.ItemFlags = (int)ItemFlags.IS_STREAMING_REQ;
                providerSession.ItemAttributesTable.Add(itemInfo.Attributes, itemInfo);
                providerSession.ItemStreamIdTable.Add(itemInfo.StreamId, itemInfo);

                ++itemListIndex;
            }

            return PerfToolsReturnCode.SUCCESS;
        }

        /// <summary>
        /// Clean up provider threads.
        /// </summary>
        public override void Cleanup()
        {
            base.Cleanup();
        }

        /// <summary>
        /// Send refreshes & updates to channels open
        /// </summary>
        private void SendMsgBurst()
        {
            if (m_ProvSession == null)
            {
                return;
            }

            // The application corrects for ticks that don't finish before the time
            // that the next update burst should start.  But don't do this correction
            // for new channels.
            if (m_ProvSession.TimeActivated == 0)
            {
                return;
            }

            TransportReturnCode ret = TransportReturnCode.SUCCESS;

            // Send burst of updates
            if (ProviderPerfConfig.UpdatesPerSec != 0 && m_ProvSession.UpdateItemList.Count() != 0)
            {
                ret = SendUpdateBurst(m_ProvSession, out m_Error);
                if (ret > TransportReturnCode.SUCCESS)
                {
                    m_ShouldWrite = true;
                }
            }
            // Use remaining time in the tick to send refreshes.
            while (ret >= TransportReturnCode.SUCCESS && m_ProvSession.RefreshItemList.Count() != 0)
            {
                ret = SendRefreshBurst(m_ProvSession, out m_Error);
            }

            if (ret < TransportReturnCode.SUCCESS)
            {
                switch (ret)
                {
                    case TransportReturnCode.NO_BUFFERS:
                        {
                            m_ShouldWrite = true;
                        }
                        break;
                    default:
                        if (Thread.CurrentThread.ThreadState != ThreadState.Aborted)
                        {
                            CloseChannelAndShutDown($"Failure while writing message bursts, error: {m_Error?.Text}");
                        }
                        break;
                }
            }
            else if (ret > TransportReturnCode.SUCCESS)
            {
                m_ShouldWrite = true;
            }
        }

        /// <summary>
        /// Writes the content of the TransportBuffer to the ETA channel
        /// </summary>
        /// <param name="msgBuf"> the buffer to be written</param>
        private void Write(ITransportBuffer msgBuf)
        {
            // write data to the channel
            m_WriteArgs.Clear();
            m_WriteArgs.Priority = WritePriorities.HIGH;
            m_WriteArgs.Flags = WriteFlags.DIRECT_SOCKET_WRITE;
            TransportReturnCode ret = m_Channel!.Write(msgBuf, m_WriteArgs, out m_Error);

            while (ret == TransportReturnCode.WRITE_CALL_AGAIN)
            {
                ret = m_Channel.Flush(out m_Error);
                if (ret < TransportReturnCode.SUCCESS)
                {
                    break;
                }
                ret = m_Channel.Write(msgBuf, m_WriteArgs, out m_Error);
            }

            if (ret > TransportReturnCode.SUCCESS)
            {
                m_ShouldWrite = true;
            }
            else if (ret < TransportReturnCode.SUCCESS)
            {
                switch (ret)
                {
                    case TransportReturnCode.WRITE_FLUSH_FAILED:
                        if (m_Channel.State == ChannelState.ACTIVE)
                        {
                            m_ShouldWrite = true;
                        }
                        break;
                    default:
                        if (Thread.CurrentThread.IsAlive)
                        {
                            CloseChannelAndShutDown($"Failure while writing message bursts, error: {m_Error?.Text}");
                        }
                        break;
                }
            }
        }

        private void CloseChannelAndShutDown(string text)
        {
            Console.WriteLine(text);
            if (!NIProvPerfConfig.UseReactor) // use ETA Channel for sending and receiving
            {
                m_Channel!.Close(out m_Error);
            }
            else
            {
                m_ReactorChannel?.Close(out _);
            }
            Shutdown = true;
            ShutdownAck = true;
            Environment.Exit(-1);
        }

        #region Reactor Callbacks
        public ReactorCallbackReturnCode ReactorChannelEventCallback(ReactorChannelEvent evt)
        {
            switch (evt.EventType)
            {
                case ReactorChannelEventType.CHANNEL_UP:
                    {
                        Console.WriteLine("Connected ");

                        m_ReactorChannel = evt.ReactorChannel!;

                        // set the high water mark if configured
                        if (ProviderPerfConfig.HighWaterMark > 0)
                        {
                            if (m_ReactorChannel.IOCtl(IOCtlCode.HIGH_WATER_MARK, ProviderPerfConfig.HighWaterMark, out var ioErrorInfo) != ReactorReturnCode.SUCCESS)
                            {
                                CloseChannelAndShutDown($"ReactorChannel.IOCtl() failed: {ioErrorInfo}");
                            }
                        }

                        // retrieve and print out channel information
                        if (m_ReactorChannel.Info(m_ReactorChannnelInfo, out var errorInfo) != ReactorReturnCode.SUCCESS)
                        {
                            CloseChannelAndShutDown($"ReactorChannel.Info() failed {errorInfo}");
                        }
                        Console.WriteLine("Channel active. " + m_ReactorChannnelInfo.ChannelInfo.ToString() + "\n");

                        // Check that we can successfully pack, if packing messages.
                        if (NIProvPerfConfig.TotalBuffersPerPack > 1
                            && NIProvPerfConfig.PackingBufferLength > m_ReactorChannnelInfo.ChannelInfo.MaxFragmentSize)
                        {
                            Console.Error.WriteLine("Error (Channel {0}): MaxFragmentSize {1} is too small for packing buffer size {2}",
                                              m_ReactorChannel.Channel, m_ReactorChannnelInfo.ChannelInfo.MaxFragmentSize,
                                              NIProvPerfConfig.PackingBufferLength);
                            Environment.Exit(-1);
                        }

                        m_ProvSession = new ProviderSession(m_XmlMsgData, m_ItemEncoder);
                        ClientChannelInfo ccInfo = new ClientChannelInfo();
                        ccInfo.ReactorChannel = m_ReactorChannel;
                        ccInfo.Channel = m_ReactorChannel.Channel;
                        m_ProvSession.Init(ccInfo);

                        m_ProvSession.TimeActivated = (long)GetTime.GetMicroseconds();

                        break;
                    }
                case ReactorChannelEventType.CHANNEL_READY:
                    {
                        if (m_ProvSession!.PrintEstimatedMsgSizes(out var error) != PerfToolsReturnCode.SUCCESS)
                        {
                            CloseChannelAndShutDown(error!.Text);
                        }

                        break;
                    }
                case ReactorChannelEventType.FD_CHANGE:
                    {
                        Console.WriteLine("Channel Change - Old Channel: "
                                           + evt.ReactorChannel!.OldSocket + " New Channel: "
                                           + evt.ReactorChannel.Socket);
                        break;
                    }
                case ReactorChannelEventType.CHANNEL_DOWN_RECONNECTING:
                    {
                        if (evt.ReactorChannel!.Socket != null)
                            Console.Write("\nConnection down reconnecting: Channel " + evt.ReactorChannel.Socket);
                        else
                            Console.Write("\nConnection down reconnecting");

                        Console.WriteLine(" , Text: " + evt.ReactorErrorInfo.Error.Text);

                        // allow Reactor to perform connection recovery

                        break;
                    }
                case ReactorChannelEventType.CHANNEL_DOWN:
                    {
                        if (evt.ReactorChannel!.Socket != null)
                            Console.Write("\nConnection down: Channel " + evt.ReactorChannel.Socket);
                        else
                            Console.Write("\nConnection down");

                        Console.WriteLine(" , Text: " + evt.ReactorErrorInfo.Error.Text);

                        // close ReactorChannel
                        if (m_ReactorChannel != null)
                        {
                            m_ReactorChannel.Close(out _);
                        }
                        break;
                    }
                case ReactorChannelEventType.WARNING:
                    Console.WriteLine("Received ReactorChannel WARNING event");
                    break;
                default:
                    {
                        Console.WriteLine("Unknown channel event!\n");
                        return ReactorCallbackReturnCode.SUCCESS;
                    }
            }

            return ReactorCallbackReturnCode.SUCCESS;
        }

        public ReactorCallbackReturnCode DefaultMsgCallback(ReactorMsgEvent evt)
        {
            Console.WriteLine("Received message with unhandled domain: " + evt.Msg!.DomainType);

            return ReactorCallbackReturnCode.SUCCESS;
        }

        public ReactorCallbackReturnCode RdmLoginMsgCallback(RDMLoginMsgEvent evt)
        {
            LoginMsg loginMsg = evt.LoginMsg!;

            switch (loginMsg.LoginMsgType)
            {
                case LoginMsgType.REFRESH:
                    LoginRefresh loginRefresh = loginMsg.LoginRefresh!;
                    Console.WriteLine("Received Login Response for Username: " + loginRefresh.UserName);
                    Console.WriteLine(loginRefresh.ToString());

                    if (loginRefresh.State.StreamState() == StreamStates.OPEN
                        && loginRefresh.State.DataState() == DataStates.OK)
                    {
                        /* create item list to publish */
                        CreateItemList();
                    }
                    else
                    {
                        CloseChannelAndShutDown("Login stream closed");
                    }
                    break;
                default:
                    break;
            }

            return ReactorCallbackReturnCode.SUCCESS;
        }

        #endregion
    }
}
