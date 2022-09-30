/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using Refinitiv.Common.Interfaces;
using Refinitiv.Eta.Codec;
using Refinitiv.Eta.Example.Common;
using Refinitiv.Eta.PerfTools.Common;
using Refinitiv.Eta.Rdm;
using Refinitiv.Eta.Transports;
using Refinitiv.Eta.Transports.Interfaces;
using Buffer = Refinitiv.Eta.Codec.Buffer;
using ProviderSession = Refinitiv.Eta.PerfTools.Common.ProviderSession;

namespace Refinitiv.Eta.PerfTools.ProvPerf
{
    /// <summary>
    /// Interactive provider implementation of the provider thread. 
    /// Handles accepting of new channels, processing of incoming messages 
    /// and sending of message bursts.
    /// </summary>
    public class IProviderThread : ProviderThread
    {
        private const string applicationName = "ProvPerf";
        private const string applicationId = "256";

        private IProvDirectoryProvider m_DirectoryProvider;     // Source directory requests handler
        private LoginProvider m_LoginProvider;                  // Login requests handler
        private DictionaryProvider m_DictionaryProvider;        // Dictionary requests handler
        private ItemRequestHandler m_ItemRequestHandler;        // Iterm requests handler
        private DecodeIterator m_DecodeIter;                    // Decode iterator

        private ChannelInfo m_ChannelInfo;                      // Active channel information
        private ChannelHandler m_ChannelHandler;                // Channel handler.

        private Msg m_tmpMsg;

        public volatile int ConnectionCount;                    //Number of client sessions currently connected.

        //private long m_divisor = 1; /* This is used when using ETA VA Reactor */

        /// <summary>
        /// Instantiates a new i provider thread.
        /// </summary>
        /// <param name="xmlMsgData">the xml msg data</param>
        public IProviderThread(XmlMsgData xmlMsgData) : base(xmlMsgData) 
        {
            m_DecodeIter = new DecodeIterator();
            m_tmpMsg = new Msg();
            m_LoginProvider = new LoginProvider();
            m_DirectoryProvider = new IProvDirectoryProvider();
            m_DictionaryProvider = new DictionaryProvider();
            m_ChannelInfo = new ChannelInfo();
            m_ItemRequestHandler = new ItemRequestHandler();
            m_ChannelHandler = new ChannelHandler(this);

            if (ProviderPerfConfig.TicksPerSec > 1000)
            {
                //m_divisor = 1000000; /* This is used when using ETA VA Reactor */
            }

            InitTimeFunctions();
        }

        /// <summary>
        /// Handles newly accepted client channel.
        /// </summary>
        /// <param name="channel">the accepted channel</param>
        public void AcceptNewChannel(IChannel channel)
        {
            ProviderSession provSession = new ProviderSession(m_XmlMsgData, m_ItemEncoder);
            ++ConnectionCount;
            m_ChannelHandler.AddChannel(channel, provSession, true);
        }

        /// <summary>
        /// Process the channel active event for the interactive provider application. 
        /// This causes some additional channel configuration and a print out of the channel configuration.
        /// </summary>
        /// <param name="clientChannelInfo">the client channel info</param>
        /// <param name="error"><see cref="Error"/> instance that carries error details in case of failure</param>
        /// <returns><see cref="PerfToolsReturnCode"/> indicating the status of the operation</returns>
        public PerfToolsReturnCode ProcessActiveChannel(ClientChannelInfo clientChannelInfo, out Error? error)
        {
            TransportReturnCode transportReturnCode;
            if (ProviderPerfConfig.HighWaterMark > 0)
            {
                transportReturnCode = clientChannelInfo.Channel!.IOCtl(IOCtlCode.HIGH_WATER_MARK, ProviderPerfConfig.HighWaterMark, out error);
                if (transportReturnCode != TransportReturnCode.SUCCESS)
                    return PerfToolsReturnCode.FAILURE;
            }

            transportReturnCode = clientChannelInfo.Channel!.Info(m_ChannelInfo, out error);
            if (transportReturnCode != TransportReturnCode.SUCCESS)
                return PerfToolsReturnCode.FAILURE;

            Console.WriteLine($"Client channel active. Channel Info: {m_ChannelInfo.ToString()}\n");

            int count = m_ChannelInfo.ComponentInfoList == null ? 0 : m_ChannelInfo.ComponentInfoList.Count;
            if (count == 0)
                Console.WriteLine("(No component info)");
            else
            {
                for (int i = 0; i < count; ++i)
                {
                    Console.WriteLine(m_ChannelInfo.ComponentInfoList![i].ComponentVersion);
                    if (i < count - 1)
                        Console.Write(", ");
                }
            }

            Console.WriteLine("\n\n");

            // Check that we can successfully pack, if packing messages
            if (ProviderPerfConfig.TotalBuffersPerPack > 1 && ProviderPerfConfig.PackingBufferLength > m_ChannelInfo.MaxFragmentSize)
            {
                Console.Write($"Error (Channel {clientChannelInfo.Channel}): MaxFragmentSize {m_ChannelInfo.MaxFragmentSize} is too small for packing buffer size {ProviderPerfConfig.PackingBufferLength}\n");
                error = new Error()
                {
                    Text = $"Error (Channel {clientChannelInfo.Channel}): MaxFragmentSize {m_ChannelInfo.MaxFragmentSize} is too small for packing buffer size {ProviderPerfConfig.PackingBufferLength}\n"
                };
                return PerfToolsReturnCode.FAILURE;
            }

            ProviderSession provSession = (ProviderSession)clientChannelInfo.UserSpec!;
            PerfToolsReturnCode perfToolsReturnCode = provSession.PrintEstimatedMsgSizes(out error);
            if (perfToolsReturnCode != PerfToolsReturnCode.SUCCESS)
                return perfToolsReturnCode;

            provSession.TimeActivated = (long)GetTime.GetMicroseconds();

            error = null;
            return PerfToolsReturnCode.SUCCESS;
        }

        /// <summary>
        /// Method called by ChannelHandler when a channel is closed
        /// </summary>
        /// <param name="channelHandler">the channel handler</param>
        /// <param name="clientChannelInfo">the client channel info</param>
        /// <param name="error">error information</param>
        /// <returns><see cref="PerfToolsReturnCode"/> value indicating the status of the operation</returns>
        public PerfToolsReturnCode ProcessInactiveChannel(ChannelHandler channelHandler, ClientChannelInfo clientChannelInfo, Error error)
        {
            long inactiveTime = (long)GetTime.GetMicroseconds();
            channelHandler.ProviderThread.ProvThreadInfo!.Stats.InactiveTime = inactiveTime;
            Console.WriteLine($"processInactiveChannel ({inactiveTime})");

            m_ChannelHandler.ChannelLock.EnterWriteLock();
            --ConnectionCount;
            m_ChannelHandler.ChannelLock.ExitWriteLock();

            Console.WriteLine($"Channel Closed: {error!.Text}");

            ProviderSession? provSession = (ProviderSession?)clientChannelInfo.UserSpec;
            if (provSession != null)
            {
                provSession.Cleanup();
            }
            
            return PerfToolsReturnCode.SUCCESS;
        }

        /// <summary>
        /// Method called by ChannelHandler when ETA read() returns a buffer
        /// </summary>
        /// <param name="channelHandler">the channel handler</param>
        /// <param name="clientChannelInfo">the client channel info</param>
        /// <param name="msgBuf">the msg buffer</param>
        /// <param name="error"><see cref="Error"/> instance with error details in case of failure</param>
        /// <returns><see cref="PerfToolsReturnCode"/> value indicating the status of the operation</returns>
        public PerfToolsReturnCode ProcessMsg(ChannelHandler channelHandler, ClientChannelInfo clientChannelInfo, ITransportBuffer msgBuf, out Error? error)
        {
            ProviderThread providerThread = channelHandler.ProviderThread;
            IChannel channel = clientChannelInfo.Channel!;
            CodecReturnCode codecReturnCode;

            // clear decode iterator
            m_DecodeIter.Clear();
            m_tmpMsg.Clear();

            codecReturnCode = m_DecodeIter.SetBufferAndRWFVersion(msgBuf, channel.MajorVersion, channel.MinorVersion);

            if (codecReturnCode != CodecReturnCode.SUCCESS)
            {
                Console.Error.WriteLine($"DecodeIterator.SetBufferAndRWFVersion() failed with return code:  {codecReturnCode.GetAsString()}");
                error = new Error()
                {
                    Text = $"DecodeIterator.SetBufferAndRWFVersion() failed with return code:  {codecReturnCode.GetAsString()}"
                };
                return PerfToolsReturnCode.FAILURE;
            }

            codecReturnCode = m_tmpMsg.Decode(m_DecodeIter);
            if (codecReturnCode != CodecReturnCode.SUCCESS)
            {
                Console.Error.WriteLine($"Msg.Decode() failed with return code:  {codecReturnCode.GetAsString()}");
                error = new Error()
                {
                    Text = $"Msg.Decode() failed with return code: {codecReturnCode.GetAsString()}"
                };
                return PerfToolsReturnCode.FAILURE;
            }

            TransportReturnCode transportReturnCode;
            switch (m_tmpMsg.DomainType)
            {
                case (int)DomainType.LOGIN:
                    transportReturnCode = m_LoginProvider.ProcessMsg(channelHandler, clientChannelInfo, m_tmpMsg, m_DecodeIter, out error);
                    break;
                case (int)DomainType.SOURCE:
                    transportReturnCode = m_DirectoryProvider.ProcessMsg(channelHandler, clientChannelInfo, m_tmpMsg, m_DecodeIter, out error);
                    break;
                case (int)DomainType.DICTIONARY:
                    transportReturnCode = m_DictionaryProvider.ProcessMsg(channelHandler, clientChannelInfo, m_tmpMsg, m_DecodeIter, out error);
                    break;
                case (int)DomainType.MARKET_PRICE:
                    if (m_XmlMsgData.UpdateCount > 0)
                        transportReturnCode = m_ItemRequestHandler.ProcessMsg(providerThread, (ProviderSession)clientChannelInfo.UserSpec!, m_tmpMsg, m_DirectoryProvider.OpenLimit, m_DirectoryProvider.ServiceId, m_DirectoryProvider.Qos, m_DecodeIter, out error);
                    else
                        transportReturnCode = m_ItemRequestHandler.SendRequestReject(providerThread, (ProviderSession)clientChannelInfo.UserSpec!, m_tmpMsg, ItemRejectReason.DOMAIN_NOT_SUPPORTED, out error);
                    break;
                default:
                    transportReturnCode = m_ItemRequestHandler.SendRequestReject(providerThread, (ProviderSession)clientChannelInfo.UserSpec!, m_tmpMsg, ItemRejectReason.DOMAIN_NOT_SUPPORTED, out error);
                    break;
            }

            if (transportReturnCode > TransportReturnCode.SUCCESS)
            {
                // The method sent a message and indicated that we need to flush.
                m_ChannelHandler.RequestFlush(clientChannelInfo);
            }

            return transportReturnCode < TransportReturnCode.SUCCESS ? PerfToolsReturnCode.FAILURE : PerfToolsReturnCode.SUCCESS;
        }

        /// <summary>
        /// Interactive provider thread's run method
        /// </summary>
        public override void Run()
        {
            m_DirectoryProvider.ServiceName = ProviderPerfConfig.ServiceName;
            m_DirectoryProvider.ServiceId = ProviderPerfConfig.ServiceId;
            m_DirectoryProvider.OpenLimit = ProviderPerfConfig.OpenLimit;
            m_DirectoryProvider.InitService(m_XmlMsgData);

            m_LoginProvider.InitDefaultPosition();
            m_LoginProvider.ApplicationId = applicationId;
            m_LoginProvider.ApplicationName = applicationName;

            if (!m_DictionaryProvider.LoadDictionary(out m_Error!))
            {
                Console.WriteLine($"Error loading dictionary: {m_Error!.Text}");
                Environment.Exit((int)PerfToolsReturnCode.FAILURE);
            }

            ProvThreadInfo!.Dictionary = m_DictionaryProvider.Dictionary;

            // Determine update rates on per-tick basis
            double nextTickTime = InitNextTickTime!();

            // this is the main loop
            while (!Shutdown)
            {
                if (nextTickTime <= CurrentTime!())
                {
                    nextTickTime = NextTickTime!(nextTickTime);
                    SendMsgBurst(nextTickTime);
                    m_ChannelHandler.ProcessNewChannels();
                }
                m_ChannelHandler.ReadChannels(nextTickTime, out m_Error!);
                if (m_Error is not null && !string.IsNullOrEmpty(m_Error.Text))
                {
                    Console.WriteLine($"Error while handling channels: {m_Error.Text}");
                }
                m_ChannelHandler.CheckPings();
            }

            ShutdownAck = true;
        }

        /// <summary>
        /// Cleans provider thread
        /// </summary>
        public override void Cleanup()
        {
            m_ChannelHandler.Cleanup();
        }

        /// <summary>
        /// Send refreshes and updates to open channels.
        /// If an operation on channel returns unrecoverable error, the channel is closed.
        /// </summary>
        /// <param name="stopTime">the time until which the messages are being sent</param>
        private void SendMsgBurst(double stopTime)
        {
            foreach (ClientChannelInfo clientChannelInfo in m_ChannelHandler.ActiveChannelList)
            {
                ProviderSession providerSession = (ProviderSession)clientChannelInfo.UserSpec!;


                // The application corrects for ticks that don't finish before the time 
                // that the next update burst should start. But don't do this correction 
                // for new channels.
                if (providerSession.TimeActivated == 0)
                {
                    continue;
                }

                TransportReturnCode ret = TransportReturnCode.SUCCESS;

                // Send burst of updates
                if (ProviderPerfConfig.UpdatesPerSec != 0 && providerSession.UpdateItemList.Count() != 0)
                {
                    ret = SendUpdateBurst(providerSession, out m_Error!);
                    if (ret > TransportReturnCode.SUCCESS)
                    {
                        // Need to flush
                        m_ChannelHandler.RequestFlush(clientChannelInfo);
                    }
                }

                // Send burst of generic messages
                if (ProviderPerfConfig.GenMsgsPerSec != 0 && providerSession.GenMsgItemList.Count() != 0)
                {
                    ret = SendGenMsgBurst(providerSession, out m_Error!);
                    if (ret > TransportReturnCode.SUCCESS)
                    {
                        // Need to flush
                        m_ChannelHandler.RequestFlush(clientChannelInfo);
                    }
                }

                // Use remaining time in the tick to send refreshes.
                while (ret >= TransportReturnCode.SUCCESS && providerSession.RefreshItemList.Count() != 0 && CurrentTime!() < stopTime) 
                {
                    ret = SendRefreshBurst(providerSession, out m_Error!);
                }

                if (ret < TransportReturnCode.SUCCESS)
                {
                    switch (ret)
                    {
                        case TransportReturnCode.NO_BUFFERS:
                            m_ChannelHandler.RequestFlush(clientChannelInfo);
                            break;
                        default:
                            if (Thread.CurrentThread.IsAlive)
                            {
                                Console.WriteLine($"Failure while writing message bursts: {m_Error!.Text} ({ret})\n");
                                m_ChannelHandler.CloseChannel(clientChannelInfo, m_Error); //Failed to send an update. Remove this client
                            }
                            break;
                    }
                }
                else if (ret > TransportReturnCode.SUCCESS)
                {
                    // need to flush
                    m_ChannelHandler.RequestFlush(clientChannelInfo);
                }
            }
        }

        public ReaderWriterLockSlim HandlerLock()
        {
            return m_ChannelHandler.ChannelLock;
        }
    }
}
