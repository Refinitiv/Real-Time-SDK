/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

using Refinitiv.Common.Interfaces;
using Refinitiv.Eta.Codec;
using Refinitiv.Eta.Common;
using Refinitiv.Eta.Example.Common;
using Refinitiv.Eta.PerfTools.Common;
using Refinitiv.Eta.Rdm;
using Refinitiv.Eta.Transports;
using Refinitiv.Eta.Transports.Interfaces;
using Refinitiv.Eta.ValueAdd.Rdm;
using System.Net.Sockets;
using Buffer = Refinitiv.Eta.Codec.Buffer;
using DictionaryHandler = Refinitiv.Eta.PerfTools.Common.DictionaryHandler;
using DirectoryHandler = Refinitiv.Eta.PerfTools.Common.DirectoryHandler;
using ItemInfo = Refinitiv.Eta.PerfTools.Common.ItemInfo;
using LoginHandler = Refinitiv.Eta.PerfTools.Common.LoginHandler;
using MarketPriceItem = Refinitiv.Eta.PerfTools.Common.MarketPriceItem;

namespace Refinitiv.Eta.PerfTools.ConsPerf
{
    /// <summary>
    /// Provides the logic that consumer connections use in ConsPerf for connecting to a provider, 
    /// requesting items, and processing the received refreshes and updates.
    /// </summary>
    internal class ConsumerThread
    {
        private const int CONNECTION_RETRY_TIME = 1;                // in seconds
        private const int ITEM_STREAM_ID_START = 6;
        private const int LATENCY_RANDOM_ARRAY_SET_COUNT = 20;
        private const int REQUEST_MSG_BUF_SIZE = 512;

        private ConsumerThreadInfo m_ConsThreadInfo;                // thread information
        private ConsPerfConfig m_ConsPerfConfig;                    // configuration information
        
        private EncodeIterator m_EncIter = new EncodeIterator();    // encode iterator
        private DecodeIterator m_DecIter = new DecodeIterator();    // decode iterator
        
        private WriteArgs m_WriteArgs = new WriteArgs();            // WriteArgs instance used for Write operations
        private ReadArgs m_ReadArgs = new ReadArgs();               // ReasArgs instance used for Read operations
        
        private Msg? m_ResponseMsg;                                  // response message
        
        private LoginHandler? m_LoginHandler;                        // login handler
        private DirectoryHandler? m_SrcDirHandler;                   // source directory handler
        private DictionaryHandler? m_DictionaryHandler;              // dictionary handler
        private PingHandler? m_PingHandler;                          // ping handler
        
        private MarketPriceDecoder m_MarketPriceDecoder;            // market price decoder
        private InProgInfo? m_InProg;                                // connection in progress information
        private Error? m_Error;                                      // error structure
        
        private XmlItemInfoList m_ItemInfoList;                     // item information list from XML file
        private XmlMsgData m_MsgData;                               // message data information from XML file
        
        private ItemRequest[] m_ItemRequestList;                    // item request list
        private int m_PostItemCount;                                // number of items in _itemRequestList that are posting items
        private int m_GenMsgItemCount;                              // number of items in _itemRequestList that are for sending generic msgs on items
        private IRequestMsg? m_RequestMsg;                           // request message
        private PostUserInfo m_PostUserInfo;                        // post user information
        private bool m_RequestsSent;                                // indicates if requested service is up 
        private long m_MicrosPerTick;                               // microseconds per tick
        private int m_RequestListSize;                              // request list size
        private int m_RequestListIndex;                             // current request list index
        private IShutdownCallback m_ShutdownCallback;               // shutdown callback to main application
        private ConnectOptions? m_ConnectOpts;                       // connection options
        private ChannelInfo? m_ChnlInfo;                             // channel information
        private bool m_HaveMarketPricePostItems;                    // indicates there are post items in the item list
        private long m_PostsPerTick;                                // posts per tick
        private long m_PostsPerTickRemainder;                       // posts per tick remainder
        private bool m_HaveMarketPriceGenMsgItems;                  // indicates there are generic msg items in the item list
        private long m_GenMsgsPerTick;                              // gen msgs per tick
        private long m_GenMsgsPerTickRemainder;                     // gen msgs per tick remainder
        private LatencyRandomArrayOptions? m_RandomArrayOpts;        // random array options
        private LatencyRandomArray? m_PostLatencyRandomArray;        // post random latency array
        private int m_PostItemIndex;                                // current post item index
        private LatencyRandomArray? m_GenMsgLatencyRandomArray;      // generic msg random latency array
        private int m_GenMsgItemIndex;                              // current generic msg item index
        private ItemEncoder m_ItemEncoder;                          // item encoder
        private MarketPriceItem? m_MpItem;                           // market price item
        private IMsgKey? m_MsgKey;                                   // message key
        private ItemInfo? m_ItemInfo;                                // item information
        private int m_JITWarmupRefreshCount;                        // used to determine when JIT warmup is complete
        private IChannel? m_Channel;                                 // ETA Channel
 
        private Buffer m_PostBuffer;
        private Buffer m_GenericBuffer;
        private Buffer m_FieldDictionaryName = new Buffer(); 
        private Buffer m_EnumTypeDictionaryName = new Buffer();

        private volatile bool shouldWrite = false;

        private List<Socket> m_ReadList = new List<Socket>();
        private List<Socket> m_WriteList = new List<Socket>();

        public ConsumerThread(ConsumerThreadInfo consInfo, ConsPerfConfig consConfig, XmlItemInfoList itemList, XmlMsgData msgData, PostUserInfo postUserInfo, IShutdownCallback shutdownCallback)
        {
            InitFields();
            m_ConsThreadInfo = consInfo;
            m_ConsPerfConfig = consConfig;
            m_RequestListSize = m_ConsThreadInfo.ItemListCount + ITEM_STREAM_ID_START;
            m_ItemInfoList = itemList;
            m_ItemRequestList = new ItemRequest[m_RequestListSize];
            m_PostUserInfo = postUserInfo;
            for (int i = 0; i < m_RequestListSize; ++i)
            {
                m_ItemRequestList[i] = new ItemRequest();
            }
            m_RequestListIndex = ITEM_STREAM_ID_START;
            m_PostItemIndex = ITEM_STREAM_ID_START;
            m_GenMsgItemIndex = ITEM_STREAM_ID_START;
            m_MsgData = msgData;
            m_ItemEncoder = new ItemEncoder(msgData);
            m_MarketPriceDecoder = new MarketPriceDecoder(m_PostUserInfo);

            m_PostBuffer = new Buffer();
            m_PostBuffer.Data(new ByteBuffer(512));
            m_GenericBuffer = new Buffer();
            m_GenericBuffer.Data(new ByteBuffer(512));
            m_FieldDictionaryName.Data("RWFFld");
            m_EnumTypeDictionaryName.Data("RWFEnum");
            m_ShutdownCallback = shutdownCallback;
        }

        /// <summary>
        /// Run the consumer thread
        /// </summary>
        public void Run()
        {
            // initialize the test data from configuration and xml files
            Initialize();

            if (!m_ConsThreadInfo.Shutdown)
            {
                // Check if the test is configured for the correct buffer size to fit post / generic messages
                PrintEstimatedMsgSizes(m_Channel!, MsgClasses.POST);
                PrintEstimatedMsgSizes(m_Channel!, MsgClasses.GENERIC);

                // set service name in directory handler
                m_SrcDirHandler!.ServiceName.Data(m_ConsPerfConfig.ServiceName);

                // get and print the channel info
                if (m_Channel!.Info(m_ChnlInfo, out m_Error) != TransportReturnCode.SUCCESS)
                {
                    CloseChannelAndShutDown("Channel.Info() failed");
                    return;
                }
                Console.WriteLine("Channel active. \n" + m_ChnlInfo!.ToString());

                // set login parameters
                m_LoginHandler!.ApplicationName = "ConsPerf";
                m_LoginHandler.UserName = m_ConsPerfConfig.Username;
                m_LoginHandler.Role = Login.RoleTypes.CONS;

                // Send login request message
                ITransportBuffer? msg = m_LoginHandler.GetRequest(m_Channel, out m_Error);
                if (msg != null)
                {
                    Write(msg);
                }
                else
                {
                    CloseChannelAndShutDown("Sending login request failed");
                    return;
                }

                // Initialize ping handler
                m_PingHandler!.InitPingHandler(m_Channel.PingTimeOut);
            }

            TransportReturnCode transportReturnCode;
            int currentTicks = 0;
            double nextTickTime;
            double selectTime;
            double currentTime;

            nextTickTime = InitNextTickTime();
            while (!m_ConsThreadInfo.Shutdown)
            {
                // read until no more to read and then write leftover from previous burst
                selectTime = SelectTime(nextTickTime);             

                if (!m_ConsPerfConfig.BusyRead)
                {
                    SelectReadAndWrite(selectTime);
                }
                else
                {
                    BusyReadAndWrite(selectTime);
                }

                // Handle pings
                if (m_PingHandler!.HandlePings(m_Channel!, out m_Error) != TransportReturnCode.SUCCESS)
                {
                    CloseChannelAndShutDown(string.Format($"Error handling pings: {(m_Error != null ? m_Error.Text : "")}"));
                    return;
                }

                currentTime = CurrentTime();
                if (currentTime >= nextTickTime)
                {
                    nextTickTime = NextTickTime(nextTickTime);
                   
                    // only send bursts on tick boundary
                    if (m_RequestsSent)
                    {
                        Service service;

                        service = m_SrcDirHandler!.ServiceInfo();
                        if ((transportReturnCode = SendBursts(currentTicks, service)) < TransportReturnCode.SUCCESS)
                        {
                            if (transportReturnCode != TransportReturnCode.NO_BUFFERS)
                            {
                                continue;
                            }
                            // not successful cases were handled in SendBursts method
                        }
                    }

                    if (++currentTicks == m_ConsPerfConfig.TicksPerSec)
                    {
                        currentTicks = 0;
                    }
                }
            }   // end of run loop

            m_ConsThreadInfo.ShutdownAck = true;
            CloseChannel();
            Console.WriteLine($"\nConsumerThread {m_ConsThreadInfo.ThreadId} exiting...");
        }

        private void InitFields()
        {
            m_DecIter = new DecodeIterator();
            m_ResponseMsg = new Msg();
            m_LoginHandler = new LoginHandler();
            m_SrcDirHandler = new DirectoryHandler();
            m_DictionaryHandler = new DictionaryHandler();
            m_PingHandler = new PingHandler();
            m_InProg = new InProgInfo();
            m_Error = new Error();
            m_ConnectOpts = new ConnectOptions();
            m_ChnlInfo = new ChannelInfo();
            m_RequestMsg = new Msg();
            m_RandomArrayOpts = new LatencyRandomArrayOptions();
            m_PostLatencyRandomArray = new LatencyRandomArray();
            m_GenMsgLatencyRandomArray = new LatencyRandomArray();
            m_MpItem = new MarketPriceItem();
            m_MsgKey = new MsgKey();
            m_ItemInfo = new ItemInfo();
        }

        /// <summary>
        /// Connect and wait until connection is active
        /// </summary>
        private void Connect()
        {
            // set connect options
            m_ConnectOpts!.MajorVersion = Codec.Codec.MajorVersion();
            m_ConnectOpts.MinorVersion = Codec.Codec.MinorVersion();
            m_ConnectOpts.ConnectionType = m_ConsPerfConfig.ConnectionType;
            m_ConnectOpts.GuaranteedOutputBuffers = m_ConsPerfConfig.GuaranteedOutputBuffers;
            m_ConnectOpts.NumInputBuffers = m_ConsPerfConfig.NumInputBuffers;
            if (m_ConsPerfConfig.SendBufSize > 0)
            {
                m_ConnectOpts.SysSendBufSize = m_ConsPerfConfig.SendBufSize;
            }
            if (m_ConsPerfConfig.RecvBufSize > 0)
            {
                m_ConnectOpts.SysRecvBufSize = m_ConsPerfConfig.RecvBufSize;
            }
            if (m_ConsPerfConfig.ConnectionType == ConnectionType.SOCKET)
            {
                m_ConnectOpts.TcpOpts.TcpNoDelay = m_ConsPerfConfig.TcpNoDelay;
            }
            // set the connection parameters on the connect options 
            m_ConnectOpts.UnifiedNetworkInfo.Address = m_ConsPerfConfig.HostName;
            m_ConnectOpts.UnifiedNetworkInfo.ServiceName = m_ConsPerfConfig.PortNo;
            m_ConnectOpts.UnifiedNetworkInfo.InterfaceName = m_ConsPerfConfig.InterfaceName;

            if (m_ConnectOpts.ConnectionType == ConnectionType.ENCRYPTED)
            {
                m_ConnectOpts.EncryptionOpts.EncryptedProtocol = ConnectionType.SOCKET;
            }

            // Initialize Transport
            InitArgs initArgs = new InitArgs();
            initArgs.GlobalLocking = m_ConsPerfConfig.ThreadCount > 1 ? true : false;
            if (Transport.Initialize(initArgs, out m_Error) != TransportReturnCode.SUCCESS)
            {
                Console.WriteLine("Transport.Initialize failed. {0}", m_Error?.Text);
                Environment.Exit((int)TransportReturnCode.FAILURE);
            }

            // Connection recovery loop. It will try to connect until successful
            Console.WriteLine("Starting connection...");
            TransportReturnCode handshake;
            bool initializedChannel;
            while (!m_ConsThreadInfo.Shutdown)
            {
                initializedChannel = false;
                while (!initializedChannel)
                {
                    m_Channel = Transport.Connect(m_ConnectOpts, out m_Error);
                    if (m_Channel == null)
                    {
                        Console.Error.WriteLine($"Error: Transport connect failure: {(m_Error != null ? m_Error.Text : "")}. Will retry shortly.");
                        try
                        {
                            Thread.Sleep(CONNECTION_RETRY_TIME * 1000);
                            continue;
                        }
                        catch (Exception e)
                        {
                            Console.WriteLine($"Thread.Sleep failed: {e.Message}");
                            Environment.Exit((int)TransportReturnCode.FAILURE);
                        }
                    }
                    initializedChannel = true;
                }

                while ((handshake = m_Channel!.Init(m_InProg, out m_Error)) != TransportReturnCode.SUCCESS)
                {
                    if (handshake == TransportReturnCode.FAILURE)
                        break;

                    try
                    {
                        Thread.Sleep(1000);
                    }
                    catch (Exception e)
                    {
                        Console.WriteLine($"Thread.Sleep failed: {e.Message}");
                        Environment.Exit((int)TransportReturnCode.FAILURE);
                    }
                }

                if (handshake == TransportReturnCode.SUCCESS)
                {
                    break;
                }

                Console.WriteLine($"Connection failure: {(m_Error != null ? m_Error.Text : "")}. Will retry shortly.");
                try
                {
                    Thread.Sleep(CONNECTION_RETRY_TIME * 1000);
                }
                catch (Exception e)
                {
                    Console.WriteLine($"Thread.Sleep failed: {e.Message}");
                    Environment.Exit((int)TransportReturnCode.FAILURE);
                }
            }
            if (!m_ConsThreadInfo.Shutdown)
            {
                m_ConsThreadInfo.Channel = m_Channel;
                Console.WriteLine("Connected.");

                // set the high water mark if configured
                if (m_ConsPerfConfig.HighWaterMark > 0)
                {
                    if (m_Channel!.IOCtl(IOCtlCode.HIGH_WATER_MARK, m_ConsPerfConfig.HighWaterMark, out m_Error) != TransportReturnCode.SUCCESS)
                    {
                        CloseChannelAndShutDown("Channel.IOCtl() failed");
                        return;
                    }
                }
            }
        }

        /// <summary>
        /// Initializes consumer thread
        /// </summary>
        private void Initialize()
        {
            // create latency log file writer for this thread 
            if (m_ConsPerfConfig.LogLatencyToFile)
            {
                string latencyLogPath = m_ConsPerfConfig.LatencyLogFilename + m_ConsThreadInfo.ThreadId + ".csv";
                try
                {
                    // Open latency log file. 
                    m_ConsThreadInfo.LatencyLogFile = latencyLogPath;
                    m_ConsThreadInfo.LatencyLogFileWriter = new StreamWriter(latencyLogPath);
                }
                catch (Exception e)
                {
                    Console.WriteLine($"Error: Failed to open latency log file {latencyLogPath}. Exception: {e.Message}\n");
                    Environment.Exit((int)TransportReturnCode.FAILURE);
                }
                m_ConsThreadInfo.LatencyLogFileWriter.WriteLine("Message type, Send Time, Receive Time, Latency (usec)\n");
            }

            // create stats file writer for this thread 

            string statsFilePath = m_ConsPerfConfig.StatsFilename + m_ConsThreadInfo.ThreadId + ".csv";
            try
            {
                m_ConsThreadInfo.StatsFile = statsFilePath;
                m_ConsThreadInfo.StatsFileWriter = new StreamWriter(statsFilePath);
            }
            catch (Exception e)
            {
                Console.WriteLine($"Error: Failed to open stats file {statsFilePath}. Exception: {e.Message}\n");
                Environment.Exit((int)TransportReturnCode.FAILURE);
            }
            m_ConsThreadInfo.StatsFileWriter.WriteLine("UTC, Latency updates, Latency avg (usec), Latency std dev (usec), Latency max (usec), Latency min (usec), Images, Update rate, Posting Latency updates, Posting Latency avg (usec), Posting Latency std dev (usec), Posting Latency max (usec), Posting Latency min (usec), GenMsgs sent, GenMsg Latencies sent, GenMsgs received, GenMsg Latencies received, GenMsg Latency avg (usec), GenMsg Latency std dev (usec), GenMsg Latency max (usec), GenMsg Latency min (usec), CPU usage (%), Memory(MB)");

            // Create latency random array for post messages. Latency random array is used
            // to randomly insert latency RIC fields into post messages while sending bursts. 
            if (m_ConsPerfConfig.LatencyPostsPerSec > 0)
            {
                m_RandomArrayOpts!.TotalMsgsPerSec = m_ConsPerfConfig.PostsPerSec;
                m_RandomArrayOpts.LatencyMsgsPerSec = m_ConsPerfConfig.LatencyPostsPerSec;
                m_RandomArrayOpts.TicksPerSec = m_ConsPerfConfig.TicksPerSec;
                m_RandomArrayOpts.ArrayCount = LATENCY_RANDOM_ARRAY_SET_COUNT;

                if (m_PostLatencyRandomArray!.Create(m_RandomArrayOpts) != PerfToolsReturnCode.SUCCESS)
                {
                    Console.WriteLine("Error: Incorrect Post LatencyRandomArrayOptions.\n");
                    Environment.Exit((int)TransportReturnCode.FAILURE);
                }
            }

            // Create latency random array for generic messages. Latency random array is used
            // to randomly insert latency RIC fields into generic messages while sending bursts.
            if (m_ConsPerfConfig.LatencyGenMsgsPerSec > 0)
            {
                m_RandomArrayOpts!.TotalMsgsPerSec = m_ConsPerfConfig.GenMsgsPerSec;
                m_RandomArrayOpts.LatencyMsgsPerSec = m_ConsPerfConfig.LatencyGenMsgsPerSec;
                m_RandomArrayOpts.TicksPerSec = m_ConsPerfConfig.TicksPerSec;
                m_RandomArrayOpts.ArrayCount = LATENCY_RANDOM_ARRAY_SET_COUNT;

                if (m_GenMsgLatencyRandomArray!.Create(m_RandomArrayOpts) != PerfToolsReturnCode.SUCCESS)
                {
                    Console.WriteLine("Error: Incorrect Generic Msg LatencyRandomArrayOptions.\n");
                    Environment.Exit((int)TransportReturnCode.FAILURE);
                }
            }

            // populate item information from the XML list. 
            PopulateItemInfo();

            // initialize time tracking parameters
            m_MicrosPerTick = 1000000 / m_ConsPerfConfig.TicksPerSec;
            m_PostsPerTick = m_ConsPerfConfig.PostsPerSec / m_ConsPerfConfig.TicksPerSec;
            m_PostsPerTickRemainder = m_ConsPerfConfig.PostsPerSec % m_ConsPerfConfig.TicksPerSec;
            m_GenMsgsPerTick = m_ConsPerfConfig.GenMsgsPerSec / m_ConsPerfConfig.TicksPerSec;
            m_GenMsgsPerTickRemainder = m_ConsPerfConfig.GenMsgsPerSec % m_ConsPerfConfig.TicksPerSec;

            // load dictionary
            CodecError codecError;
            m_DictionaryHandler!.LoadDictionary(out codecError);
            if (codecError != null)
            {
                Console.WriteLine($"Error while loading dictionary: {(codecError != null ? codecError.Text : "")}");
            }

            // connect to provider application
            Connect();
        }

        /// <summary>
        /// Populates item information from the XML list
        /// </summary>
        private void PopulateItemInfo()
        {
            int itemListIndex = 0;
            m_PostItemCount = 0;
            m_GenMsgItemCount = 0;

            for (int streamId = ITEM_STREAM_ID_START; streamId < m_RequestListSize; ++streamId)
            {
                // Once we have filled our list with the common items,
                // start using the range of items unique to this consumer thread.
                if (itemListIndex == m_ConsPerfConfig.CommonItemCount && itemListIndex < m_ConsThreadInfo.ItemListUniqueIndex)
                {
                    itemListIndex = m_ConsThreadInfo.ItemListUniqueIndex;
                }                  

                m_ItemRequestList[streamId].Clear();
                m_ItemRequestList[streamId].ItemInfo.StreamId = streamId;
                m_ItemRequestList[streamId].ItemInfo.Attributes.DomainType = m_ItemInfoList.ItemInfoList[itemListIndex].DomainType;

                m_ItemRequestList[streamId].ItemName = m_ItemInfoList.ItemInfoList[itemListIndex].Name;
                m_ItemRequestList[streamId].MsgKey.ApplyHasName();
                
                Buffer buffer = new Buffer();
                buffer.Data(m_ItemInfoList.ItemInfoList[itemListIndex].Name);
                m_ItemRequestList[streamId].MsgKey.Name = buffer;
                m_ItemRequestList[streamId].ItemInfo.Attributes.MsgKey = m_ItemRequestList[streamId].MsgKey;

                if (!m_ItemInfoList.ItemInfoList[itemListIndex].IsSnapshot)
                {
                    int flags = m_ItemRequestList[streamId].ItemInfo.ItemFlags | (int)ItemFlags.IS_STREAMING_REQ;
                    m_ItemRequestList[streamId].ItemInfo.ItemFlags = flags;
                }

                if (m_ItemInfoList.ItemInfoList[itemListIndex].IsPost && m_ConsPerfConfig.PostsPerSec > 0)
                {
                    object? itemData;

                    ++m_PostItemCount;

                    int flags = m_ItemRequestList[streamId].ItemInfo.ItemFlags | (int)ItemFlags.IS_POST;
                    m_ItemRequestList[streamId].ItemInfo.ItemFlags = flags;

                    //get item info data from appropriate pool
                    switch (m_ItemRequestList[streamId].ItemInfo.Attributes.DomainType)
                    {
                        case (int)DomainType.MARKET_PRICE:
                            itemData = new MarketPriceItem();
                            m_HaveMarketPricePostItems = true;
                            break;
                        default:
                            itemData = null;
                            break;
                    }

                    if (itemData == null)
                    {
                        Console.WriteLine("\nFailed to get storage for ItemInfo data.\n");
                        Environment.Exit((int)TransportReturnCode.FAILURE);
                    }

                    m_ItemRequestList[streamId].ItemInfo.ItemData = itemData;
                }

                if (m_ConsPerfConfig.PostsPerSec > 0)
                {
                    if (m_HaveMarketPricePostItems && m_MsgData.PostCount == 0)
                    {
                        Console.WriteLine("Error: No MarketPrice posting data in file: {0}.", m_ConsPerfConfig.MsgFilename);
                        Environment.Exit((int)TransportReturnCode.FAILURE);
                    }
                }

                if (m_ItemInfoList.ItemInfoList[itemListIndex].IsGenMsg && m_ConsPerfConfig.GenMsgsPerSec > 0)
                {
                    object? itemData;

                    ++m_GenMsgItemCount;

                    int flags = m_ItemRequestList[streamId].ItemInfo.ItemFlags | (int)ItemFlags.IS_GEN_MSG;
                    m_ItemRequestList[streamId].ItemInfo.ItemFlags = flags;

                    // get item info data from appropriate pool
                    switch (m_ItemRequestList[streamId].ItemInfo.Attributes.DomainType)
                    {
                        case (int)DomainType.MARKET_PRICE:
                            itemData = new MarketPriceItem();
                            m_HaveMarketPriceGenMsgItems = true;
                            break;
                        default:
                            itemData = null;
                            break;
                    }

                    if (itemData == null)
                    {
                        Console.WriteLine("\nFailed to get storage for ItemInfo data.");
                        Environment.Exit((int)TransportReturnCode.FAILURE);
                    }

                    m_ItemRequestList[streamId].ItemInfo.ItemData = itemData;
                }

                if (m_ConsPerfConfig.GenMsgsPerSec > 0)
                {
                    if (m_HaveMarketPriceGenMsgItems && m_MsgData.GenMsgCount == 0)
                    {
                        Console.WriteLine("Error: No MarketPrice generic msg data in file: {0}.", m_ConsPerfConfig.MsgFilename);
                        Environment.Exit((int)TransportReturnCode.FAILURE);
                    }
                }

                ++itemListIndex;
            }
        }

        /// <summary>
        /// Gets the buffer of the necessary length
        /// </summary>
        /// <param name="length">the length of the buffer</param>
        /// <param name="packedBuffer">determines whether the buffer is packed</param>
        /// <returns><see cref="ITransportBuffer"/> instance</returns>
        private ITransportBuffer GetBuffer(int length, bool packedBuffer)
        {
            return m_Channel!.GetBuffer(length, packedBuffer, out m_Error);
        }

        /// <summary>
        /// Writes the content of the ITransportBuffer to the ETA channel
        /// </summary>
        /// <param name="msgBuf"><see cref="ITransportBuffer"/> with message data</param>
        private void Write(ITransportBuffer msgBuf)
        {
            // write data to the channel
            m_WriteArgs.Clear();
            m_WriteArgs.Priority = WritePriorities.HIGH;
            if (m_ConsPerfConfig.HighWaterMark <= 0)
            {
                m_WriteArgs.Flags = WriteFlags.DIRECT_SOCKET_WRITE;
            }

            TransportReturnCode retval = m_Channel!.Write(msgBuf, m_WriteArgs, out m_Error);

            if (retval > TransportReturnCode.SUCCESS)
            {
                // register for write if there's still data queued
                shouldWrite = true;
            }
            else if (retval == TransportReturnCode.SUCCESS)
            {
                //nothing to do, data was successfully written
            }
            else
            {
                switch (retval)
                {
                    case TransportReturnCode.WRITE_CALL_AGAIN:
                        while (retval == TransportReturnCode.WRITE_CALL_AGAIN)
                        {
                            retval = m_Channel.Flush(out m_Error);
                            if (retval < TransportReturnCode.SUCCESS)
                            {
                                Console.WriteLine("Channel flush failed with returned code: " + retval.ToString() + " - " + m_Error.Text);
                            }
                            retval = m_Channel.Write(msgBuf, m_WriteArgs, out m_Error);
                        }

                        if (retval > TransportReturnCode.SUCCESS || (retval == TransportReturnCode.WRITE_FLUSH_FAILED && m_Channel.State != ChannelState.CLOSED))
                        {
                            shouldWrite = true;
                        }
                        break;
                    case TransportReturnCode.WRITE_FLUSH_FAILED:
                        if (m_Channel.State != ChannelState.CLOSED)
                        {
                            shouldWrite = true;
                        }
                        break;
                    default:
                        m_Channel.ReleaseBuffer(msgBuf, out m_Error);
                        CloseChannelAndShutDown("Failed to write to channel: " + retval);
                        break;
                }
            }
        }

        /// <summary>
        /// Performs read / write operations
        /// </summary>
        /// <param name="selectTime">the time during which the thread is waiting before assessing the socket read / write status</param>
        private void SelectReadAndWrite(double selectTime)
        {
            try
            {
                if (m_Channel!.Socket != null && m_Channel.Socket.Connected)
                {
                    m_ReadList.Add(m_Channel.Socket);
                    m_WriteList.Add(m_Channel.Socket);
                    try
                    {
                        Socket.Select(m_ReadList, m_WriteList, null, selectTime > 0 ? (int)selectTime : 0);

                        if (m_ReadList.Count > 0)
                        {
                            Read();
                        }

                        if (shouldWrite && m_WriteList.Count > 0)
                        {
                            if (m_Channel.Flush(out m_Error) == TransportReturnCode.SUCCESS)
                            {
                                shouldWrite = false;
                            }
                        }
                    }
                    finally
                    {
                        m_WriteList.Clear();
                        m_ReadList.Clear();
                    }                
                }
            }
            catch (Exception e1)
            {
                CloseChannelAndShutDown(e1.Message);
                return;
            }                    
        }

        /// <summary>
        /// Reads from a channel without calling Socket.Poll method
        /// </summary>
        /// <param name="selectTime">time during which the method will try to perform read operation</param>
        private void BusyReadAndWrite(double selectTime)
        {
            double pollTime; 
            double currentTime = CurrentTime();

            if (selectTime < 0)
            {
                selectTime = 0;
            }

            pollTime = currentTime + selectTime;

            // call flush once
            m_Channel!.Flush(out m_Error);

            while (currentTime <= pollTime)
            {
                Read();
                currentTime = CurrentTime();
            }

        }

        /// <summary>
        /// Reads data from the channel
        /// </summary>
        private void Read()
        {
            ITransportBuffer msgBuf;
            do // read until no more to read
            {
                msgBuf = m_Channel!.Read(m_ReadArgs, out m_Error);
                if (msgBuf != null)
                {
                    ProcessResponse(msgBuf);
                    //set flag for server message received
                    m_PingHandler!.ReceivedRemoteMsg = true;
                }
                else
                {
                    if (m_ReadArgs.ReadRetVal == TransportReturnCode.READ_PING)
                    {
                        //set flag for server message received
                        m_PingHandler!.ReceivedRemoteMsg = true;
                    }
                }
            }
            while (m_ReadArgs.ReadRetVal > TransportReturnCode.SUCCESS);
        }

        /// <summary>
        /// Process transport response
        /// </summary>
        /// <param name="buffer"><see cref="ITransportBuffer"/> instance that carries data</param>
        public void ProcessResponse(ITransportBuffer buffer)
        {
            CodecReturnCode codecReturnCode;
            
            m_DecIter.Clear();
            m_ResponseMsg!.Clear();

            codecReturnCode = m_DecIter.SetBufferAndRWFVersion(buffer, m_Channel!.MajorVersion, m_Channel.MinorVersion);

            if (codecReturnCode != CodecReturnCode.SUCCESS)
            {
                CloseChannelAndShutDown($"DecodeIterator.SetBufferAndRWFVersion() failed with return code:  {codecReturnCode.GetAsString()}");
                return;
            }

            codecReturnCode = m_ResponseMsg.Decode(m_DecIter);

            if (codecReturnCode != CodecReturnCode.SUCCESS)
            {
                CloseChannelAndShutDown($"DecodeMsg() failed: {codecReturnCode.GetAsString()}");
                return;
            }

            switch (m_ResponseMsg.DomainType)
            {
                case (int)DomainType.LOGIN:
                    ProcessLoginResponse();
                    break;
                case (int)DomainType.SOURCE:
                    ProcessSourceDirectoryResponse();
                    break;
                case (int)DomainType.DICTIONARY:
                    ProcessDictionaryResponse(m_ResponseMsg, m_DecIter);
                    break;
                case (int)DomainType.MARKET_PRICE:
                    ProcessMarketPriceResponse(m_ResponseMsg, m_DecIter);
                    break;
                default:
                    Console.WriteLine($"Unhandled Domain Type: {m_ResponseMsg.DomainType}");
                    break;
            }
        }

        /// <summary>
        /// Process login response
        /// </summary>
        private void ProcessLoginResponse()
        {
            TransportReturnCode ret = m_LoginHandler!.ProcessResponse(m_ResponseMsg!, m_DecIter, out m_Error);
            if (ret != TransportReturnCode.SUCCESS)
            {
                CloseChannelAndShutDown(m_Error!.Text);
                return;
            }

            if (m_ResponseMsg!.MsgClass == MsgClasses.REFRESH)
            {
                if (m_ConsPerfConfig.PostsPerSec > 0 
                    && (!m_LoginHandler.LoginRefresh.HasFeatures 
                        || !m_LoginHandler.LoginRefresh.SupportedFeatures.HasSupportPost 
                        || m_LoginHandler.LoginRefresh.SupportedFeatures.SupportOMMPost == 0))
                {
                    CloseChannelAndShutDown("Provider for this connection does not support posting.");
                    return;
                }
            }

            //Handle login states
            ConsumerLoginState loginState = m_LoginHandler.LoginState;
            if (loginState == ConsumerLoginState.OK_SOLICITED)
            {
                ITransportBuffer? buffer = m_SrcDirHandler!.GetRequest(m_Channel!, out m_Error);
                if (buffer != null)
                {
                    Write(buffer);
                }
                else
                {
                    CloseChannelAndShutDown($"Error sending directory request: {(m_Error != null ? m_Error.Text : "")}");
                    return;
                }
            }
            else
            {
                CloseChannelAndShutDown($"Invalid login state : {loginState}");
                return;
            }
        }

        /// <summary>
        /// Process source directory response
        /// </summary>
        private void ProcessSourceDirectoryResponse()
        {
            TransportReturnCode ret = m_SrcDirHandler!.ProcessResponse(m_ResponseMsg!, m_DecIter, out m_Error);
            if (ret != TransportReturnCode.SUCCESS)
            {
                CloseChannelAndShutDown(m_Error!.Text);
                return;
            }

            if (m_SrcDirHandler.IsRequestedServiceUp())
            {
                if (DictionariesLoaded())
                {
                    m_ConsThreadInfo.Dictionary = m_DictionaryHandler!.DataDictionary;
                    Console.WriteLine("Dictionary ready, requesting item(s)...\n");

                    m_RequestsSent = true;
                }
                else // dictionaries not loaded yet
                {
                    SendDictionaryRequests(m_Channel!, m_SrcDirHandler.ServiceInfo());
                }
            }
            else
            {
                // service not up or
                // previously up service went down
                m_RequestsSent = false;

                Console.WriteLine($"Requested service '{m_ConsPerfConfig.ServiceName}' not up. Waiting for service to be up...");
            }
        }

        /// <summary>
        /// Sends dictionry requests to the specified service
        /// </summary>
        /// <param name="channel">the channel to send requests to</param>
        /// <param name="service">the service from which dictionaries are requested</param>
        private void SendDictionaryRequests(IChannel channel, Service service)
        {
            if (m_RequestsSent)
                return;

            // dictionaries were loaded at initialization. send item requests and post
            // messages only after dictionaries are loaded.
            if (DictionariesLoaded())
            {
                m_ConsThreadInfo.Dictionary = m_DictionaryHandler!.DataDictionary;
                Console.WriteLine("Dictionary ready, requesting item(s)...\n");
            }
            else
            {
                m_DictionaryHandler!.ServiceId = service.ServiceId;
                ITransportBuffer? buffer = m_DictionaryHandler.GetRequest(channel, DictionaryHandler.FIELD_DICTIONARY_TYPE, out m_Error);
                if (buffer != null)
                {
                    Write(buffer);
                }
                else
                {
                    CloseChannelAndShutDown("Sending dictionary request failed");
                    return;
                }

                buffer = m_DictionaryHandler.GetRequest(channel, DictionaryHandler.ENUM_DICTIONARY_TYPE, out m_Error);
                if (buffer != null)
                {
                    Write(buffer);
                }
                else
                {
                    CloseChannelAndShutDown("Sending dictionary request failed");
                    return;
                }
            }
        }

        /// <summary>
        /// Process dictionary response
        /// </summary>
        /// <param name="responseMsg">the partially decoded response message</param>
        /// <param name="dIter">The DecodeIterator instance</param>
        private void ProcessDictionaryResponse(Msg responseMsg, DecodeIterator dIter)
        {
            if (m_DictionaryHandler!.ProcessResponse(m_Channel!, responseMsg, dIter, out m_Error) != TransportReturnCode.SUCCESS)
            {
                CloseChannelAndShutDown("Processing dictionary response failed");
                return;
            }

            if (m_DictionaryHandler.FieldDictionaryLoaded && m_DictionaryHandler.EnumTypeDictionaryLoaded)
            {
                m_ConsThreadInfo.Dictionary = m_DictionaryHandler.DataDictionary;
                Console.WriteLine("Dictionary ready, requesting item(s)...\n");

                m_RequestsSent = true;
            }
        }

        /// <summary>
        /// Process market price response
        /// </summary>
        /// <param name="responseMsg">The partially decoded response messageparam>
        /// <param name="dIter">The DecodeIterator instance</param>
        private void ProcessMarketPriceResponse(Msg responseMsg, DecodeIterator dIter)
        {
            CodecReturnCode ret;
            int msgClass = responseMsg.MsgClass;

            switch (msgClass)
            {
                case MsgClasses.REFRESH:
                    m_ConsThreadInfo.Stats.RefreshCount.Increment();

                    //If we are still retrieving images, check if this item still needs one.
                    if ((ret = m_MarketPriceDecoder.DecodeUpdate(dIter, responseMsg, m_ConsThreadInfo)) != CodecReturnCode.SUCCESS)
                    {
                        CloseChannelAndShutDown($"Decoding failure: {ret.GetAsString()}");
                        return;
                    }
                    if (m_ConsThreadInfo.Stats.ImageRetrievalEndTime == 0)
                    {
                        if (responseMsg.State.IsFinal())
                        {
                            CloseChannelAndShutDown($"Received unexpected final state {StreamStates.ToString(responseMsg.State.StreamState())} in refresh for item {responseMsg.MsgKey.Name.ToString()}");
                            return;
                        }

                        if (responseMsg.CheckRefreshComplete())
                        {
                            if (m_ConsPerfConfig.PrimeJIT == false)
                            {
                                if (responseMsg.State.DataState() == DataStates.OK)
                                {
                                    m_ItemRequestList[responseMsg.StreamId].RequestState = ItemRequestState.HAS_REFRESH;
                                    m_ConsThreadInfo.Stats.RefreshCompleteCount.Increment();
                                    if (m_ConsThreadInfo.Stats.RefreshCompleteCount.GetTotal() == (m_RequestListSize - ITEM_STREAM_ID_START))
                                    {
                                        m_ConsThreadInfo.Stats.ImageRetrievalEndTime = (long)GetTime.GetNanoseconds();
                                        m_ConsThreadInfo.Stats.SteadyStateLatencyTime = m_ConsThreadInfo.Stats.ImageRetrievalEndTime + m_ConsPerfConfig.DelaySteadyStateCalc * 1000000L;
                                    }
                                }
                            }
                            else // JIT priming enabled
                            {
                                // Count snapshot images used for priming, ignoring state
                                if (m_JITWarmupRefreshCount < (m_RequestListSize - ITEM_STREAM_ID_START))
                                {
                                    m_JITWarmupRefreshCount++;
                                    // reset request state so items can be re-requested
                                    m_ItemRequestList[responseMsg.StreamId].RequestState = ItemRequestState.NOT_REQUESTED;
                                    if (m_JITWarmupRefreshCount == (m_RequestListSize - ITEM_STREAM_ID_START))
                                    {
                                        // reset request count and _requestListIndex so items can be re-requested
                                        //set the image retrieval start time
                                        m_ConsThreadInfo.Stats.RequestCount.Init();
                                        m_ConsThreadInfo.Stats.RefreshCompleteCount.Init();
                                        m_ConsThreadInfo.Stats.RefreshCount.Init();
                                        m_RequestListIndex = ITEM_STREAM_ID_START;
                                    }
                                }
                                else // the streaming image responses after priming
                                {
                                    if (responseMsg.State.DataState() == DataStates.OK)
                                    {
                                        m_ItemRequestList[responseMsg.StreamId].RequestState = ItemRequestState.HAS_REFRESH;
                                        m_ConsThreadInfo.Stats.RefreshCompleteCount.Increment();
                                        if (m_ConsThreadInfo.Stats.RefreshCompleteCount.GetTotal() == (m_RequestListSize - ITEM_STREAM_ID_START))
                                        {
                                            m_ConsThreadInfo.Stats.ImageRetrievalEndTime = (long)GetTime.GetNanoseconds();
                                            m_ConsThreadInfo.Stats.SteadyStateLatencyTime = m_ConsThreadInfo.Stats.ImageRetrievalEndTime + m_ConsPerfConfig.DelaySteadyStateCalc * 1000000L;
                                        }
                                    }
                                }
                            }
                        }
                    }
                    break;
                case MsgClasses.UPDATE:
                    if (m_ConsThreadInfo.Stats.ImageRetrievalEndTime > 0)
                    {
                        m_ConsThreadInfo.Stats.SteadyStateUpdateCount.Increment();
                    }
                    else
                    {
                        m_ConsThreadInfo.Stats.StartupUpdateCount.Increment();
                    }
                    if (m_ConsThreadInfo.Stats.FirstUpdateTime == 0)
                        m_ConsThreadInfo.Stats.FirstUpdateTime = (long)GetTime.GetNanoseconds();
                    if ((ret = m_MarketPriceDecoder.DecodeUpdate(dIter, responseMsg, m_ConsThreadInfo)) != CodecReturnCode.SUCCESS)
                    {
                        CloseChannelAndShutDown($"Decoding failure: {ret.GetAsString()}");
                        return;
                    }
                    break;
                case MsgClasses.GENERIC:
                    m_ConsThreadInfo.Stats.GenMsgRecvCount.Increment();
                    if ((ret = m_MarketPriceDecoder.DecodeUpdate(dIter, responseMsg, m_ConsThreadInfo)) != CodecReturnCode.SUCCESS)
                    {
                        CloseChannelAndShutDown($"Decoding failure: {ret.GetAsString()}");
                        return;
                    }
                    break;
                case MsgClasses.STATUS:
                    m_ConsThreadInfo.Stats.StatusCount.Increment();

                    if (responseMsg.CheckHasState() && responseMsg.State.IsFinal())
                    {
                        CloseChannelAndShutDown($"Received unexpected final state {StreamStates.ToString(responseMsg.State.StreamState())} for item {m_ItemRequestList[responseMsg.StreamId].ItemName}");
                        return;
                    }
                    break;
                default:
                    break;
            }
        }

        /// <summary>
        /// Sends a burst of item requests
        /// </summary>
        /// <param name="itemBurstCount">the number of requests to send</param>
        /// <param name="service">the service requested by the consumer</param>
        /// <returns><see cref="TransportReturnCode"/> indicating the status of the operation</returns>
        private TransportReturnCode SendItemRequestBurst(int itemBurstCount, Service service)
        {
            TransportReturnCode ret;
            Qos? qos = null;

            //Use a QoS from the service, if one is given.
            if (service.HasInfo && service.Info.HasQos && service.Info.QosList.Count > 0)
            {
                qos = service.Info.QosList[0];
            }

            for (int i = 0; i < itemBurstCount; ++i)
            {
                ItemRequest itemRequest;

                if (m_RequestListIndex == m_RequestListSize)
                {
                    return TransportReturnCode.SUCCESS;
                }                   

                itemRequest = m_ItemRequestList[m_RequestListIndex];

                //Encode request msg.
                m_RequestMsg!.MsgClass = MsgClasses.REQUEST;

                //don't apply streaming for JIT Compiler priming and snapshot
                if (!m_ConsPerfConfig.RequestSnapshots 
                    && (itemRequest.ItemInfo.ItemFlags & (int)ItemFlags.IS_STREAMING_REQ) > 0 
                    && (!m_ConsPerfConfig.PrimeJIT || (m_ConsPerfConfig.PrimeJIT && m_JITWarmupRefreshCount == (m_RequestListSize - ITEM_STREAM_ID_START))))
                {
                    m_RequestMsg.ApplyStreaming();
                }

                if (qos != null)
                {
                    m_RequestMsg.ApplyHasQos();
                    m_RequestMsg.Qos.IsDynamic = qos.IsDynamic;
                    m_RequestMsg.Qos.Rate(qos.Rate());
                    m_RequestMsg.Qos.RateInfo(qos.RateInfo());
                    m_RequestMsg.Qos.TimeInfo(qos.TimeInfo());
                    m_RequestMsg.Qos.Timeliness(qos.Timeliness());
                }

                m_RequestMsg.StreamId = itemRequest.ItemInfo.StreamId;
                m_RequestMsg.DomainType = itemRequest.ItemInfo.Attributes.DomainType;
                m_RequestMsg.ContainerType = DataTypes.NO_DATA;

                m_RequestMsg.MsgKey.Flags = itemRequest.MsgKey.Flags;
                m_RequestMsg.MsgKey.Name = itemRequest.MsgKey.Name;
                m_RequestMsg.MsgKey.ApplyHasServiceId();
                m_RequestMsg.MsgKey.ServiceId = service.ServiceId;

                if ((ret = WriteRequestMsg()) != TransportReturnCode.SUCCESS)
                {
                    return ret;
                }

                //request has been made.
                itemRequest.RequestState = ItemRequestState.WAITING_FOR_REFRESH;

                m_RequestListIndex++;
                m_ConsThreadInfo.Stats.RequestCount.Increment();
            }

            return TransportReturnCode.SUCCESS;
        }

        /// <summary>
        /// Sends a burst of messages
        /// </summary>
        /// <param name="itemBurstCount">the number of messages to be sent</param>
        /// <returns><see cref="TransportReturnCode"/> value indicating the status of the operation</returns>
        private TransportReturnCode SendItemMsgBurst(int itemBurstCount, int msgClass)
        {
            CodecReturnCode codecReturnCode;
            long encodeStartTime;
            int latencyUpdateNumber = 0;
            Func<IChannel, ItemInfo, ITransportBuffer, PostUserInfo, long, CodecReturnCode> encodeMsg;

            switch (msgClass)
            {
                case MsgClasses.GENERIC:
                    latencyUpdateNumber = (m_ConsPerfConfig.LatencyGenMsgsPerSec > 0) ? m_GenMsgLatencyRandomArray!.Next() : -1;
                    encodeMsg = (IChannel chnl, ItemInfo info, ITransportBuffer buf, PostUserInfo userInfo, long time) => m_ItemEncoder.EncodeItemGenMsg(chnl, info, buf, time);
                    break;
                case MsgClasses.POST:
                    latencyUpdateNumber = (m_ConsPerfConfig.LatencyPostsPerSec > 0) ? m_PostLatencyRandomArray!.Next() : -1;
                    encodeMsg = (IChannel chnl, ItemInfo info, ITransportBuffer buf, PostUserInfo userInfo, long time) => m_ItemEncoder.EncodeItemPost(chnl, info, buf, userInfo, time);
                    break;
                default:
                    encodeMsg = (IChannel chnl, ItemInfo info, ITransportBuffer buf, PostUserInfo userInfo, long time) => CodecReturnCode.FAILURE;
                    break;
            }

            for (int i = 0; i < itemBurstCount; ++i)
            {
                ItemRequest? item = NextMsgItem(msgClass);
                if (latencyUpdateNumber == i)
                {
                    encodeStartTime = (long)GetTime.GetMicroseconds();
                }                   
                else
                {
                    encodeStartTime = 0;
                }

                int bufLen = m_ItemEncoder.EstimateItemMsgBufferLength(item!.ItemInfo, msgClass);
                ITransportBuffer msgBuf;
                if ((msgBuf = GetBuffer(bufLen, false)) == null)
                {
                    return TransportReturnCode.NO_BUFFERS;
                }
                if ((codecReturnCode = encodeMsg(m_Channel!, item.ItemInfo, msgBuf, m_PostUserInfo, encodeStartTime)) != CodecReturnCode.SUCCESS)
                {
                    Console.WriteLine($"Encode Item for MsgClass {MsgClasses.ToString(msgClass)} failed: {codecReturnCode.GetAsString()}.\n");
                    return TransportReturnCode.FAILURE;
                }

                Write(msgBuf);

                if (msgClass == MsgClasses.POST)
                {
                    m_ConsThreadInfo.Stats.PostSentCount.Increment();
                }
                else if (msgClass == MsgClasses.GENERIC)
                {
                    m_ConsThreadInfo.Stats.GenMsgSentCount.Increment();
                    if (latencyUpdateNumber == i)
                    {
                        m_ConsThreadInfo.Stats.LatencyGenMsgSentCount.Increment();
                    }
                }
            }

            return TransportReturnCode.SUCCESS;
        }

        /// <summary>
        /// Send item requests, post bursts and or generic msg bursts
        /// </summary>
        /// <param name="currentTicks">the current tick</param>
        /// <param name="service">the service ot request items from</param>
        /// <returns><see cref="TransportReturnCode"/> value indicating the status of the operation</returns>
        private TransportReturnCode SendBursts(int currentTicks, Service service)
        {
            TransportReturnCode ret;

            //send item requests until all sent
            if (m_ConsThreadInfo.Stats.RequestCount.GetTotal() < (m_RequestListSize - ITEM_STREAM_ID_START))
            {
                int requestBurstCount;

                requestBurstCount = m_ConsPerfConfig.RequestsPerTick;
                if (currentTicks > m_ConsPerfConfig.RequestsPerTickRemainder)
                {
                    ++requestBurstCount;
                } 
                if (m_ConsThreadInfo.Stats.ImageRetrievalStartTime == 0)
                {
                    if (!m_ConsPerfConfig.PrimeJIT || m_JITWarmupRefreshCount == (m_RequestListSize - ITEM_STREAM_ID_START))
                    {
                        m_ConsThreadInfo.Stats.ImageRetrievalStartTime = (long)GetTime.GetNanoseconds();
                    }
                }
                if ((ret = SendItemRequestBurst(requestBurstCount, service)) < TransportReturnCode.SUCCESS)
                {
                    if (ret != TransportReturnCode.NO_BUFFERS)
                    {
                        m_ShutdownCallback.Shutdown();
                    }
                    return ret;
                }
            }

            // send bursts of posts and or generic msgs
            if (m_ConsThreadInfo.Stats.ImageRetrievalEndTime > 0)
            {
                if (m_ConsPerfConfig.PostsPerSec > 0 && m_PostItemCount > 0)
                {
                    if ((ret = SendItemMsgBurst((int)(m_PostsPerTick + ((currentTicks < m_PostsPerTickRemainder) ? 1 : 0)), MsgClasses.POST)) < TransportReturnCode.SUCCESS)
                    {
                        if (ret != TransportReturnCode.NO_BUFFERS)
                        {
                            m_ShutdownCallback.Shutdown();
                        }
                        return ret;
                    }
                }
                if (m_ConsPerfConfig.GenMsgsPerSec > 0 && m_GenMsgItemCount > 0)
                {
                    if ((ret = SendItemMsgBurst((int)(m_GenMsgsPerTick + ((currentTicks < m_GenMsgsPerTickRemainder) ? 1 : 0)), MsgClasses.GENERIC)) < TransportReturnCode.SUCCESS)
                    {
                        if (ret != TransportReturnCode.NO_BUFFERS)
                        {
                            m_ShutdownCallback.Shutdown();
                        }
                        return ret;
                    }
                }
            }

            return TransportReturnCode.SUCCESS;
        }

        /// <summary>
        /// Writes the request message to the ETA channel
        /// </summary>
        /// <returns><see cref="TransportReturnCode"/> value indicating the status of the operation</returns>
        private TransportReturnCode WriteRequestMsg()
        {
            ITransportBuffer msgBuf;

            if ((msgBuf = GetBuffer(REQUEST_MSG_BUF_SIZE, false)) == null)
            {
                return TransportReturnCode.NO_BUFFERS;
            }

            m_EncIter.Clear();
            CodecReturnCode codecReturnCode = m_EncIter.SetBufferAndRWFVersion(msgBuf, MajorVersion(), MinorVersion());
            if (codecReturnCode != CodecReturnCode.SUCCESS)
            {
                Console.WriteLine($"SetBufferAndRWFVersion() failed: {codecReturnCode.GetAsString()}\n");
                return TransportReturnCode.FAILURE;
            }

            if ((codecReturnCode = m_RequestMsg!.Encode(m_EncIter)) != CodecReturnCode.SUCCESS)
            {
                Console.WriteLine($"EncodeMsg() failed: {codecReturnCode.GetAsString()}.\n");
                return TransportReturnCode.FAILURE;
            }

            Write(msgBuf);
            return TransportReturnCode.SUCCESS;
        }

        /// <summary>
        /// Returns the major version
        /// </summary>
        /// <returns>int value corresponding to the major version</returns>
        private int MajorVersion()
        {
            return m_Channel!.MajorVersion;
        }

        /// <summary>
        /// Returns minor version
        /// </summary>
        /// <returns>int value corresponding to the minor version</returns>
        private int MinorVersion()
        {
            return m_Channel!.MinorVersion;
        }

        private ItemRequest? NextMsgItem(int msgClass)
        {
            switch (msgClass)
            {
                case MsgClasses.GENERIC:
                    return NextGenMsgItem();
                case MsgClasses.POST:
                    return NextPostMsgItem();
                default:
                    return null;
            }
        }

        private ItemRequest? NextPostMsgItem()
        {
            ItemRequest? itemRequest = null;            
            do
            {
                if (m_PostItemIndex == m_RequestListSize)
                {
                    m_PostItemIndex = ITEM_STREAM_ID_START;
                }
                if ((m_ItemRequestList[m_PostItemIndex].ItemInfo.ItemFlags & (int)ItemFlags.IS_POST) > 0)
                {
                    itemRequest = m_ItemRequestList[m_PostItemIndex];
                }
                m_PostItemIndex++;
            } while (itemRequest == null);

            return itemRequest;
        }

        private ItemRequest? NextGenMsgItem()
        {
            ItemRequest? itemRequest = null;
            do
            {
                if (m_GenMsgItemIndex == m_RequestListSize)
                {
                    m_GenMsgItemIndex = ITEM_STREAM_ID_START;
                }
                if ((m_ItemRequestList[m_GenMsgItemIndex].ItemInfo.ItemFlags & (int)ItemFlags.IS_GEN_MSG) > 0)
                {
                    itemRequest = m_ItemRequestList[m_GenMsgItemIndex];
                }
                m_GenMsgItemIndex++;
            } while (itemRequest == null);

            return itemRequest;
        }

        /// <summary>
        /// Closes the connection
        /// </summary>
        /// <param name="text">Shutdown text</param>
        private void CloseChannelAndShutDown(string text)
        {
            Console.WriteLine(text);
            m_ShutdownCallback.Shutdown();
            m_ConsThreadInfo.ShutdownAck = true;
            CloseChannel();
        }

        /// <summary>
        /// Close channel used by this thread
        /// </summary>
        private void CloseChannel()
        {
            m_Channel!.Close(out m_Error);
            Transport.Uninitialize();
        }

        /// <summary>
        /// Indicates if dictionaries have been loaded
        /// </summary>
        /// <returns>true if dictionaries were loaded, false otherwise</returns>
        private bool DictionariesLoaded()
        {
            return m_DictionaryHandler!.FieldDictionaryLoaded && m_DictionaryHandler.EnumTypeDictionaryLoaded;
        }

        /// <summary>
        /// Print estimated message sizes
        /// </summary>
        /// <param name="channel"><see cref="IChannel"/> instance</param>
        /// <param name="msgClass">The class of the message</param>
        private void PrintEstimatedMsgSizes(IChannel channel, int msgClass)
        {
            CodecReturnCode ret;
            ITransportBuffer testBuffer;

            m_MsgKey!.Clear();
            m_MsgKey.ApplyHasNameType();
            m_MsgKey.ApplyHasServiceId();
            m_MsgKey.NameType = InstrumentNameTypes.RIC;
            m_MsgKey.Name.Data("RDT0");
            m_MsgKey.ServiceId = 0;

            int count = 0;
            bool printEstimatedLength = false;
            Func<IChannel, ItemInfo, ITransportBuffer, PostUserInfo, long, CodecReturnCode> encodeMsg;

            switch (msgClass)
            {
                case MsgClasses.GENERIC:
                    count = m_MsgData.GenMsgCount;
                    printEstimatedLength = m_ConsPerfConfig.GenMsgsPerSec > 0;
                    encodeMsg = (IChannel chnl, ItemInfo info, ITransportBuffer buf, PostUserInfo userInfo, long time) => m_ItemEncoder.EncodeItemGenMsg(chnl, info, buf, 0);
                    break;
                case MsgClasses.POST:
                    count = m_MsgData.PostCount;
                    printEstimatedLength = m_ConsPerfConfig.PostsPerSec > 0;
                    encodeMsg = (IChannel chnl, ItemInfo info, ITransportBuffer buf, PostUserInfo userInfo, long time) => m_ItemEncoder.EncodeItemPost(chnl, info, buf, userInfo, 0);
                    break;
                default:
                    encodeMsg = (IChannel chnl, ItemInfo info, ITransportBuffer buf, PostUserInfo userInfo, long time) => CodecReturnCode.FAILURE;
                    break;
            }           

            //Market Price
            if (count > 0)
            {
                m_ItemInfo!.Clear();
                m_ItemInfo.Attributes.MsgKey = m_MsgKey;
                m_ItemInfo.Attributes.DomainType = (int)DomainType.MARKET_PRICE;
                m_ItemInfo.ItemData = m_MpItem;

                if (printEstimatedLength)
                {
                    Console.Write("Approximate message sizes:\n");

                    for (int i = 0; i < count; ++i)
                    {
                        int bufLen = m_ItemEncoder.EstimateItemMsgBufferLength(m_ItemInfo, msgClass);
                        testBuffer = channel.GetBuffer(bufLen, false, out m_Error);
                        if (testBuffer == null)
                        {
                            CloseChannelAndShutDown(string.Format("Print Estimated Msg Sizes for MsgClass {0}: GetBuffer() failed", MsgClasses.ToString(msgClass)));
                            return;
                        }
                        if ((ret = encodeMsg(m_ConsThreadInfo.Channel!, m_ItemInfo, testBuffer, m_PostUserInfo, 0)) != CodecReturnCode.SUCCESS)
                        {
                            CloseChannelAndShutDown(string.Format("Print Estimated Msg Sizes for MsgClass {0} failed: {1}", 
                                MsgClasses.ToString(msgClass), ret.GetAsString()));
                            return;
                        }
                        Console.Write("  MarketPrice Msg {0} of class {1}: \n", i + 1, MsgClasses.ToString(msgClass));
                        Console.Write("         estimated length: {0} bytes\n", bufLen);
                        Console.Write("    approx encoded length: {0} bytes\n", testBuffer.Length);
                        channel.ReleaseBuffer(testBuffer, out m_Error);
                    }
                }
            }
            Console.Write("\n");
        }

        private double InitNextTickTime()
        {
            return GetTime.GetMicroseconds() + m_MicrosPerTick; ;
        }
        
        private double NextTickTime(double nextTickTime)
        {
            return nextTickTime + m_MicrosPerTick;
        }

        private double SelectTime(double nextTickTime)
        {
            return nextTickTime - GetTime.GetMicroseconds();
        }

        private double CurrentTime()
        {
            return GetTime.GetMicroseconds();
        }
    }
}
