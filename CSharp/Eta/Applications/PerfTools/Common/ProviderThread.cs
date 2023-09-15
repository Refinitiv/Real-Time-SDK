/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using System;
using System.IO;

using LSEG.Eta.Codec;
using LSEG.Eta.Common;
using LSEG.Eta.Transports;
using LSEG.Eta.ValueAdd.Reactor;

namespace LSEG.Eta.PerfTools.Common
{
    /// <summary>
    /// ProviderThreads are used to control individual threads.
    /// Each thread handles providing data to its open channels.
    /// </summary>
    public class ProviderThread
    {
        private const int LATENCY_RANDOM_ARRAY_SET_COUNT = 20;
        private const int ALWAYS_SEND_LATENCY_UPDATE = -1;
        private const int ALWAYS_SEND_LATENCY_GENMSG = -1;

        /// <summary>
        /// Index given to this provider thread
        /// </summary>
        public long ProviderIndex { get; set; }
        /// <summary>
        /// Counts refreshes sent
        /// </summary>
        public CountStat RefreshMsgCount { get; set; } = new CountStat();
        /// <summary>
        /// Counts updates sent
        /// </summary>
        public CountStat UpdateMsgCount { get; set; } = new CountStat();
        /// <summary>
        /// Counts requests received
        /// </summary>
        public CountStat ItemRequestCount { get; set; } = new CountStat();
        /// <summary>
        /// Counts closes received.
        /// </summary>
        public CountStat CloseMsgCount { get; set; } = new CountStat();
        /// <summary>
        /// Counts posts received
        /// </summary>
        public CountStat PostMsgCount { get; set; } = new CountStat();
        /// <summary>
        /// Counts of messages blocked due  not sent due to lack of output buffers
        /// </summary>
        public CountStat OutOfBuffersCount { get; set; } = new CountStat();
        /// <summary>
        /// Counts total messages sent
        /// </summary>
        public CountStat MsgSentCount { get; set; } = new CountStat();
        /// <summary>
        /// Counts total buffers sent (used with msgSentCount for packing statistics)
        /// </summary>
        public CountStat BufferSentCount { get; set; } = new CountStat();
        /// <summary>
        /// Thread information
        /// </summary>
        public ProviderThreadInfo? ProvThreadInfo { get; protected set; }
        /// <summary>
        /// Signals thread to shutdown
        /// </summary>
        public volatile bool Shutdown;
        /// <summary>
        /// Acknowledges thread is shutdown
        /// </summary>
        public volatile bool ShutdownAck;

        WriteArgs m_WriteArgs = new WriteArgs();

        protected XmlMsgData m_XmlMsgData;                          // Msgs from XML
        protected ItemEncoder m_ItemEncoder;                        // item encoder
        private LatencyRandomArray m_UpdateLatencyRandomArray;      // Updates random latency array
        private LatencyRandomArray m_GenMsgLatencyRandomArray;      // Generic Messages random latency array
        private LatencyRandomArrayOptions m_RandomArrayOpts;        // random array options

        protected Error? m_Error = new Error();                      // Error information

        private long m_CurrentTicks;                                // Current tick out of ticks per second.

        /// <summary>Nanoseconds per <see cref="ProviderPerfConfig.TicksPerSec">tick</see></summary>
        protected long m_nsecPerTick;
        /// <summary>Microseconds per <see cref="ProviderPerfConfig.TicksPerSec">tick</see></summary>
        protected long m_usecPerTick;
        /// <summary>Milliseconds per <see cref="ProviderPerfConfig.TicksPerSec">tick</see></summary>
        protected long m_millisPerTick;

        /// Use the VA Reactor instead of the ETA Channel for sending and receiving
        private ReactorSubmitOptions m_SubmitOptions;

        public ProviderThread(XmlMsgData xmlMsgData)
        {
            m_XmlMsgData = xmlMsgData;
            m_ItemEncoder = new ItemEncoder(xmlMsgData);
            m_UpdateLatencyRandomArray = new LatencyRandomArray();
            m_GenMsgLatencyRandomArray = new LatencyRandomArray();
            m_RandomArrayOpts = new LatencyRandomArrayOptions();
            ProvThreadInfo = new ProviderThreadInfo();

            m_usecPerTick = 1_000_000 / ProviderPerfConfig.TicksPerSec;
            m_nsecPerTick = 1_000_000_000 / ProviderPerfConfig.TicksPerSec;
            m_millisPerTick = 1000 / ProviderPerfConfig.TicksPerSec;

            m_SubmitOptions = new ReactorSubmitOptions();
            m_SubmitOptions.WriteArgs.Clear();
            m_SubmitOptions.WriteArgs.Priority = WritePriorities.HIGH;
            m_SubmitOptions.WriteArgs.Flags = ProviderPerfConfig.DirectWrite ? WriteFlags.DIRECT_SOCKET_WRITE : 0;
        }

        /// <summary>
        /// Initializes a ProviderThread
        /// </summary>
        /// <param name="providerIndex">the provider index</param>
        /// <param name="providerType">the provider type</param>
        public void Init(int providerIndex, ProviderType providerType)
        {
            RefreshMsgCount.Init();
            UpdateMsgCount.Init();
            ItemRequestCount.Init();
            CloseMsgCount.Init();
            PostMsgCount.Init();
            OutOfBuffersCount.Init();
            MsgSentCount.Init();
            BufferSentCount.Init();
            ProviderIndex = providerIndex;
            m_CurrentTicks = 0;

            if (ProviderPerfConfig.UpdatesPerSec != 0 && ProviderPerfConfig.LatencyUpdateRate > 0)
            {
                m_RandomArrayOpts.TotalMsgsPerSec = ProviderPerfConfig.UpdatesPerSec;
                m_RandomArrayOpts.LatencyMsgsPerSec = ProviderPerfConfig.LatencyUpdateRate;
                m_RandomArrayOpts.TicksPerSec = ProviderPerfConfig.TicksPerSec;
                m_RandomArrayOpts.ArrayCount = LATENCY_RANDOM_ARRAY_SET_COUNT;

                if (m_UpdateLatencyRandomArray.Create(m_RandomArrayOpts) != PerfToolsReturnCode.SUCCESS)
                {
                    Console.Error.WriteLine("Error initializing application: Failed to create updates latency random array");
                    Environment.Exit(-1);
                }
            }

            if (ProviderPerfConfig.GenMsgsPerSec != 0 && ProviderPerfConfig.LatencyGenMsgRate > 0)
            {
                m_RandomArrayOpts.TotalMsgsPerSec = ProviderPerfConfig.GenMsgsPerSec;
                m_RandomArrayOpts.LatencyMsgsPerSec = ProviderPerfConfig.LatencyGenMsgRate;
                m_RandomArrayOpts.TicksPerSec = ProviderPerfConfig.TicksPerSec;
                m_RandomArrayOpts.ArrayCount = LATENCY_RANDOM_ARRAY_SET_COUNT;

                if (m_GenMsgLatencyRandomArray.Create(m_RandomArrayOpts) != PerfToolsReturnCode.SUCCESS)
                {
                    Console.Error.WriteLine("Error initializing application: Failed to create generic messages latency random array");
                    Environment.Exit(-1);
                }
            }

            // Open stats file.
            ProvThreadInfo!.StatsFile = ProviderPerfConfig.StatsFilename + (ProviderIndex + 1) + ".csv";
            try
            {
                ProvThreadInfo.StatsFileWriter = new StreamWriter(ProvThreadInfo.StatsFile);
            }
            catch (Exception)
            {
                Console.Error.WriteLine($"Error initializing application:  Failed to open stats file '{ProvThreadInfo.StatsFile}'");
                Environment.Exit(-1);
            }

            if (providerType == ProviderType.PROVIDER_INTERACTIVE)
                ProvThreadInfo.StatsFileWriter.WriteLine("UTC, Requests received, Images sent, Updates sent, Posts reflected, GenMsgs sent, GenMsg Latencies sent, GenMsgs received, GenMsg Latencies received, GenMsg Latency avg (usec), GenMsg Latency std dev (usec), GenMsg Latency max (usec), GenMsg Latency min (usec), CPU usage (%%), Memory (MB)");
            else
                ProvThreadInfo.StatsFileWriter.WriteLine("UTC, Images sent, Updates sent, CPU usage (%), Memory (MB)");
            ProvThreadInfo.StatsFileWriter.Flush();

            // Open latency file if configured.
            if (ProviderPerfConfig.LogLatencyToFile)
            {
                ProvThreadInfo.LatencyLogFile = ProviderPerfConfig.LatencyFilename + (ProviderIndex + 1) + ".csv";
                try
                {
                    ProvThreadInfo.LatencyLogFileWriter = new StreamWriter(ProvThreadInfo.LatencyLogFile);
                }
                catch (Exception)
                {
                    Console.Error.WriteLine($"Error initializing application:  Failed to open latency file '{ProvThreadInfo.LatencyLogFile}'");
                    Environment.Exit(-1);
                }
                ProvThreadInfo.LatencyLogFileWriter.WriteLine("Message type, Send time, Receive time, Latency (usec)");
                ProvThreadInfo.LatencyLogFileWriter.Flush();
            }
        }

        public virtual void Run() { }

        /// <summary>
        /// Gets a TransportBuffer for encoding a message.
        /// This method handles packing of messages,
        /// if packing is configured -- it will pack as long as appropriate,
        /// stopping to write if the present buffer is too full to accommodate
        /// the requested length.
        /// </summary>
        /// <param name="session">Channel session to get transport buffer from</param>
        /// <param name="length">buffer size to get</param>
        /// <param name="error">error populated when get transport buffer fails</param>
        /// <returns><see cref="TransportReturnCode"/> value indicating the status of the operation</returns>
        public TransportReturnCode GetItemMsgBuffer(ProviderSession session, int length, out Error? error)
        {
            TransportReturnCode transportRetunCode = TransportReturnCode.SUCCESS;
            if (ProviderPerfConfig.TotalBuffersPerPack == 1)  // Not packing
            {
                transportRetunCode = GetNewBuffer(session, length, out error);
                if (transportRetunCode < TransportReturnCode.SUCCESS)
                    return transportRetunCode;
            }
            else
            {
                // We are packing, and may need to do something different based on the size of
                // the message and how much room is left in the present buffer.
                if (length > ProviderPerfConfig.PackingBufferLength)
                {
                    // Message too large for our packing buffer, write and get a bigger one.
                    if (session.WritingBuffer != null)
                    {
                        transportRetunCode = WriteCurrentBuffer(session, out error);
                        if (transportRetunCode < TransportReturnCode.SUCCESS)
                        {
                            return transportRetunCode;
                        }
                    }
                }
                else if (session.WritingBuffer == null)
                {
                    // Have no buffer currently, so get a new one.
                    transportRetunCode = GetNewBuffer(session, ProviderPerfConfig.PackingBufferLength, out error);
                    if (transportRetunCode < TransportReturnCode.SUCCESS)
                    {
                        return transportRetunCode;
                    }
                }
                else if (length > session.WritingBuffer.Data.Limit - session.WritingBuffer.Data.Position)
                {
                    //Out of room in current packing buffer. Write the current one and get a new one.
                    transportRetunCode = WriteCurrentBuffer(session, out error);
                    if (transportRetunCode < TransportReturnCode.SUCCESS)
                    {
                        return transportRetunCode;
                    }

                    TransportReturnCode transportRetunCode1 = GetNewBuffer(session, ProviderPerfConfig.PackingBufferLength, out error);
                    if (transportRetunCode1 < TransportReturnCode.SUCCESS)
                    {
                        return transportRetunCode1;
                    }
                }
                else
                {
                    //Enough room in current packing buffer, don't need a new one.
                    error = null;
                    return transportRetunCode;
                }
            }

            if (session.WritingBuffer != null)
            {
                error = null;
                return transportRetunCode;
            }
            else
            {
                ITransportBuffer msgBuffer = session.ClientChannelInfo!.Channel!.GetBuffer(length, ProviderPerfConfig.TotalBuffersPerPack > 1, out error);
                if (msgBuffer == null)
                    return TransportReturnCode.FAILURE;
                session.WritingBuffer = msgBuffer;

                return transportRetunCode;
            }
        }

        /// <summary>
        /// Sends a completed transport buffer. This method packs messages if packing is configured.
        /// The allowPack option may be used to prevent packing if needed
        /// (for example, we just encoded the last message of a burst so it is time to write to the transport).
        /// </summary>
        /// <param name="session">client channel session</param>
        /// <param name="allowPack">if false, write buffer without packing. if true, pack according to configuration</param>
        /// <param name="error">error information populated in case of failure</param>
        /// <returns><see cref="TransportReturnCode"/> value indicating the status of the operation</returns>
        public TransportReturnCode SendItemMsgBuffer(ProviderSession session, bool allowPack, out Error? error)
        {
            MsgSentCount.Increment();

            // Make sure we stop packing at the end of a burst of updates
            // in case the next burst is for a different channel.
            // (This will also prevent any latency updates from sitting in the pack for a tick).
            if (session.PackedBufferCount == (ProviderPerfConfig.TotalBuffersPerPack - 1) || !allowPack)
            {
                return WriteCurrentBuffer(session, out error);
            }
            else
            {
                //Pack the buffer and continue using it.
                session.PackedBufferCount = session.PackedBufferCount + 1;
                if (!NIProvPerfConfig.UseReactor && !ProviderPerfConfig.UseReactor) // use ETA Channel for sending and receiving
                {
                    IChannel channel = session.ClientChannelInfo!.Channel!;

                    int remainingLength;

                    if ((remainingLength = (int)channel.PackBuffer(session.WritingBuffer, out error)) < 0)
                    {
                        return TransportReturnCode.FAILURE;
                    }

                    session.RemaingPackedBufferLength = remainingLength;

                    error = null;
                }
                else // use ETA VA Reactor for sending and receiving
                {
                    if (session.ClientChannelInfo!.ReactorChannel!.PackBuffer(session.WritingBuffer!, out var errorInfo) < ReactorReturnCode.SUCCESS)
                    {
                        error = new Error()
                        {
                            ErrorId = errorInfo?.Error.ErrorId ?? TransportReturnCode.FAILURE,
                            Text = errorInfo?.Error.Text ?? "Failed to pack buffer"
                        };
                        return TransportReturnCode.FAILURE;
                    }
                    error = null;
                }

                return TransportReturnCode.SUCCESS;
            }
        }

        public virtual void Cleanup() {}

        /// <summary>
        /// Sends a burst of refreshes for items that currently need to send one
        /// </summary>
        /// <param name="providerSession">the provider session</param>
        /// <param name="error"><see cref="Error"/> instance that carries error details in case of failure</param>
        /// <returns><see cref="TransportReturnCode"/> instance indicating the status of the operation</returns>
        protected TransportReturnCode SendRefreshBurst(ProviderSession providerSession, out Error? error)
        {
            int refreshLeft = Math.Min(providerSession.RefreshItemList.Count(), ProviderPerfConfig.RefreshBurstSize);

            CodecReturnCode codecReturnCode;
            TransportReturnCode transportReturnCode = TransportReturnCode.SUCCESS;
            for (; refreshLeft > 0; --refreshLeft)
            {
                ItemInfo? itemInfo = providerSession.RefreshItemList.RemoveFront();

                transportReturnCode = GetItemMsgBuffer(providerSession, m_ItemEncoder.EstimateItemMsgBufferLength(itemInfo!, MsgClasses.REFRESH), out error);
                if (transportReturnCode < TransportReturnCode.SUCCESS)
                {
                    return transportReturnCode;
                }

                // Encode the message with data appropriate for the domain
                if (providerSession.TempBufferForPacking is not null)
                {
                    ByteBuffer byteBuffer = providerSession.TempBufferForPacking.Data();
                    byteBuffer.Clear();
                    providerSession.TempBufferForPacking.Data(byteBuffer);
                    codecReturnCode = m_ItemEncoder.EncodeRefresh(providerSession.ClientChannelInfo!.Channel!, itemInfo!, providerSession.TempBufferForPacking, null, 0, out error);
                }
                else
                {
                    codecReturnCode = m_ItemEncoder.EncodeRefresh(providerSession.ClientChannelInfo!.Channel!, itemInfo!, providerSession.WritingBuffer!, null, 0, out error);
                }

                if (codecReturnCode < CodecReturnCode.SUCCESS)
                {
                    return TransportReturnCode.FAILURE;
                }

                transportReturnCode = SendItemMsgBuffer(providerSession, refreshLeft > 1, out error);
                if (transportReturnCode < TransportReturnCode.SUCCESS)
                {
                    return transportReturnCode;
                }

                RefreshMsgCount.Increment();

                //If it's not a streaming request, don't add it to the update list.
                if (!((itemInfo!.ItemFlags & (int)ItemFlags.IS_STREAMING_REQ) > 0))
                {
                    continue;
                }

                providerSession.UpdateItemList.Add(itemInfo);
                providerSession.GenMsgItemList.Add(itemInfo);
            }

            error = null;
            return transportReturnCode;
        }

        /// <summary>
        /// Sends a burst of item updates
        /// </summary>
        /// <param name="providerSession">the provider session</param>
        /// <param name="error"><see cref="Error"/> instance that carries error details in case of failure</param>
        /// <returns><see cref="TransportReturnCode"/> instance indicating the status of the operation</returns>
        protected TransportReturnCode SendUpdateBurst(ProviderSession providerSession, out Error? error)
        {
            CodecReturnCode codecReturnCode;
            TransportReturnCode transportReturnCode = TransportReturnCode.SUCCESS;
            //Determine updates to send out. Spread the remainder out over the first ticks
            int updatesLeft = ProviderPerfConfig.UpdatesPerTick;
            int updatesPerTickRemainder = ProviderPerfConfig.UpdatesPerTickRemainder;
            if (updatesPerTickRemainder > m_CurrentTicks)
            {
                ++updatesLeft;
            }
            int latencyUpdateNumber = (ProviderPerfConfig.LatencyUpdateRate > 0) ? m_UpdateLatencyRandomArray.Next() : -1;

            if (providerSession.UpdateItemList.Count() == 0)
            {
                error = null;
                return TransportReturnCode.SUCCESS;
            }

            TransportReturnCode ret = TransportReturnCode.SUCCESS;
            for (; updatesLeft > 0; --updatesLeft)
            {
                double latencyStartTime;
                ItemInfo nextItem = providerSession.UpdateItemList.GetNext()!;

                // When appropriate, provide a latency timestamp for the updates.
                if (ProviderPerfConfig.LatencyUpdateRate == ALWAYS_SEND_LATENCY_UPDATE || latencyUpdateNumber == (updatesLeft - 1))
                {
                    latencyStartTime = GetTime.GetMicroseconds();
                }
                else
                {
                    latencyStartTime = 0;
                }

                // get a buffer for the response
                int val = m_ItemEncoder.EstimateItemMsgBufferLength(nextItem, MsgClasses.UPDATE);
                transportReturnCode = GetItemMsgBuffer(providerSession, m_ItemEncoder.EstimateItemMsgBufferLength(nextItem, MsgClasses.UPDATE), out error);
                if (transportReturnCode < TransportReturnCode.SUCCESS)
                {
                    if (transportReturnCode == TransportReturnCode.NO_BUFFERS)
                    {
                        OutOfBuffersCount.Add(updatesLeft);
                    }

                    return ret;
                }

                if (providerSession.TempBufferForPacking is not null)
                {
                    ByteBuffer byteBuffer = providerSession.TempBufferForPacking.Data();
                    byteBuffer.Clear();
                    providerSession.TempBufferForPacking.Data(byteBuffer);
                    codecReturnCode = m_ItemEncoder.EncodeUpdate(providerSession.ClientChannelInfo!.Channel!, nextItem, providerSession.TempBufferForPacking, null, (long)latencyStartTime, out error);
                }
                else
                {
                    codecReturnCode = m_ItemEncoder.EncodeUpdate(providerSession.ClientChannelInfo!.Channel!, nextItem, providerSession.WritingBuffer!, null, (long)latencyStartTime, out error);
                }

                if (codecReturnCode < CodecReturnCode.SUCCESS)
                    return TransportReturnCode.FAILURE;

                transportReturnCode = SendItemMsgBuffer(providerSession, updatesLeft > 1, out error);
                if (transportReturnCode < TransportReturnCode.SUCCESS)
                {
                    return transportReturnCode;
                }

                UpdateMsgCount.Increment();
            }

            if (++m_CurrentTicks == ProviderPerfConfig.TicksPerSec)
                m_CurrentTicks = 0;

            error = null;
            return transportReturnCode;
        }

        /// <summary>
        /// Sends a burst of item generic messages
        /// </summary>
        /// <param name="providerSession">the provider session</param>
        /// <param name="error"><see cref="Error"/> instance with error details in case of failure</param>
        /// <returns><see cref="TransportReturnCode"/> indicating the status of the operation</returns>
        protected TransportReturnCode SendGenMsgBurst(ProviderSession providerSession, out Error? error)
        {
            //Determine generic messages to send out. Spread the remainder out over the first ticks
            int genMsgsLeft = ProviderPerfConfig.GenMsgsPerTick;
            int genMsgsPerTickRemainder = ProviderPerfConfig.GenMsgsPerTickRemainder;
            if (genMsgsPerTickRemainder > m_CurrentTicks)
                ++genMsgsLeft;

            int latencyGenMsgNumber = (ProviderPerfConfig.LatencyGenMsgRate > 0) ? m_GenMsgLatencyRandomArray.Next() : -1;

            if (providerSession.GenMsgItemList.Count() == 0)
            {
                error = null;
                return TransportReturnCode.SUCCESS;
            }

            TransportReturnCode transportReturnCode = TransportReturnCode.SUCCESS;
            for (; genMsgsLeft > 0; --genMsgsLeft)
            {
                long latencyStartTime;
                ItemInfo nextItem = providerSession.GenMsgItemList.GetNext()!;

                // When appropriate, provide a latency timestamp for the generic messages.
                if (ProviderPerfConfig.LatencyGenMsgRate == ALWAYS_SEND_LATENCY_GENMSG || latencyGenMsgNumber == (genMsgsLeft - 1))
                {
                    ProvThreadInfo!.Stats.LatencyGenMsgSentCount.Increment();
                    latencyStartTime = (long)GetTime.GetMicroseconds();
                }
                else
                    latencyStartTime = 0;

                // get a buffer for the response
                transportReturnCode = GetItemMsgBuffer(providerSession, m_ItemEncoder.EstimateItemMsgBufferLength(nextItem, MsgClasses.GENERIC), out error);
                if (transportReturnCode < TransportReturnCode.SUCCESS)
                {
                    if (transportReturnCode == TransportReturnCode.NO_BUFFERS)
                        OutOfBuffersCount.Add(genMsgsLeft);

                    return transportReturnCode;
                }

                ProvThreadInfo!.Stats.GenMsgBufLenStats.Update(providerSession.WritingBuffer!.Length());

                CodecReturnCode codecReturnCode = m_ItemEncoder.EncodeItemGenMsg(providerSession.ClientChannelInfo!.Channel!, nextItem, providerSession.WritingBuffer, latencyStartTime);
                if (codecReturnCode < CodecReturnCode.SUCCESS)
                {
                    error = new Error()
                    {
                        Text = $"Failed encoding generic message, code {codecReturnCode.GetAsString()}"
                    };
                    return TransportReturnCode.FAILURE;
                }


                transportReturnCode = SendItemMsgBuffer(providerSession, genMsgsLeft > 1, out error);
                if (transportReturnCode < TransportReturnCode.SUCCESS)
                {
                    return transportReturnCode;
                }

                ProvThreadInfo.Stats.GenMsgSentCount.Increment();

                if (ProvThreadInfo.Stats.FirstGenMsgSentTime == 0)
                    ProvThreadInfo.Stats.FirstGenMsgSentTime = (long)GetTime.GetNanoseconds();
            }

            if (++m_CurrentTicks == ProviderPerfConfig.TicksPerSec)
                m_CurrentTicks = 0;

            error = null;
            return transportReturnCode;

        }

        private TransportReturnCode GetNewBuffer(ProviderSession session, int length, out Error error)
        {
            ITransportBuffer? msgBuf;

            if (!NIProvPerfConfig.UseReactor && !ProviderPerfConfig.UseReactor) // use ETA Channel for sending and receiving
            {
                bool packedBuffer = ProviderPerfConfig.TotalBuffersPerPack > 1 ? true : false;
                if ((msgBuf = session.ClientChannelInfo!.Channel!.GetBuffer(length, packedBuffer, out error)) == null)
                {
                    return TransportReturnCode.NO_BUFFERS;
                }
            }
            else // use ETA VA Reactor for sending and receiving
            {
                if (session.ClientChannelInfo!.ReactorChannel!.State == ReactorChannelState.READY)
                {
                    msgBuf = session.ClientChannelInfo.ReactorChannel.GetBuffer(length, ProviderPerfConfig.TotalBuffersPerPack > 1, out var errorInfo);
                    if (msgBuf == null)
                    {
                        error = new Error
                        {
                            Text = $"ReactorChannel.GetBuffer() failed with error: <{errorInfo}>",
                            ErrorId = errorInfo?.Error.ErrorId ?? TransportReturnCode.NO_BUFFERS
                        };
                        return TransportReturnCode.NO_BUFFERS;
                    }
                }
                else
                {
                    error = new Error
                    {
                        Text = $"ReactorChannel is not ready",
                        ErrorId = TransportReturnCode.NO_BUFFERS
                    };
                    return TransportReturnCode.NO_BUFFERS;
                }
                error = new Error();
            }

            session.WritingBuffer = msgBuf;

            return TransportReturnCode.SUCCESS;
        }

        private TransportReturnCode WriteCurrentBuffer(ProviderSession session, out Error error)
        {
            // Reset the session write buffer packed count,
            // so that packing can continue in the next buffer
            session.PackedBufferCount = 0;

            if (!NIProvPerfConfig.UseReactor && !ProviderPerfConfig.UseReactor) // use ETA Channel for sending and receiving
            {
                IChannel? channel = session.ClientChannelInfo!.Channel;
                TransportReturnCode ret;

                m_WriteArgs.Clear();
                m_WriteArgs.Priority = WritePriorities.HIGH;
                m_WriteArgs.Flags = ProviderPerfConfig.DirectWrite ? WriteFlags.DIRECT_SOCKET_WRITE : 0;

                ret = channel!.Write(session.WritingBuffer, m_WriteArgs, out error);

                // call flush and write again
                while (ret == TransportReturnCode.WRITE_CALL_AGAIN)
                {
                    ret = channel.Flush(out error);
                    if (ret < TransportReturnCode.SUCCESS)
                    {
                        return ret;
                    }
                    ret = channel.Write(session.WritingBuffer, m_WriteArgs, out error);
                }

                if (ret >= TransportReturnCode.SUCCESS)
                {
                    session.WritingBuffer = null;
                    BufferSentCount.Increment();
                    return ret;
                }

                switch (ret)
                {
                    case TransportReturnCode.WRITE_FLUSH_FAILED:
                        // If FLUSH_FAILED is received, check the channel state.
                        // if it is still active, it's okay, just need to flush.
                        if (channel.State == ChannelState.ACTIVE)
                        {
                            session.WritingBuffer = null;
                            BufferSentCount.Increment();
                            return TransportReturnCode.SUCCESS + 1;
                        }
                        else
                        {
                            error = new Error()
                            {
                                Text = $"IChannel.Write() failed with return code: {ret}",
                                ErrorId = ret
                            };
                            return TransportReturnCode.FAILURE;
                        }
                        // Otherwise treat as error, fall through to default.
                    default:
                        error = new Error()
                        {
                            Text = $"IChannel.Write() failed with return code: {ret}",
                            ErrorId = ret
                        };
                        return ret;
                }
            }
            else // use ETA VA Reactor for sending and receiving
            {
                ReactorChannel reactorChannel = session.ClientChannelInfo!.ReactorChannel!;

                ReactorReturnCode retval = reactorChannel.Submit(session.WritingBuffer!, m_SubmitOptions, out var submitErrorInfo);

                if (retval == ReactorReturnCode.WRITE_CALL_AGAIN)
                {
                    //call flush and write again until there is data in the queue
                    while (retval == ReactorReturnCode.WRITE_CALL_AGAIN)
                    {
                        retval = reactorChannel.Submit(session.WritingBuffer!, m_SubmitOptions, out _);
                    }
                }
                else if (retval < ReactorReturnCode.SUCCESS)
                {
                    // write failed, release buffer and shut down
                    if (reactorChannel.State != ReactorChannelState.CLOSED)
                    {
                        reactorChannel.ReleaseBuffer(session.WritingBuffer!, out _);
                    }
                    error = new Error()
                    {
                        Text = $"ReactorChannel.Submit() failed with return code: {retval} <{submitErrorInfo}>",
                        ErrorId = (TransportReturnCode)retval
                    };

                    return TransportReturnCode.FAILURE;
                }

                if(retval >= ReactorReturnCode.SUCCESS)
                {
                    session.WritingBuffer = null;
                    BufferSentCount.Increment();
                }
                error = new Error();
                return (TransportReturnCode)retval;
            }
        }

        protected void InitTimeFunctions()
        {
            if (ProviderPerfConfig.TicksPerSec <= 1000)
            {
                CurrentTime = () => GetTime.GetMilliseconds();
                InitNextTickTime = () => GetTime.GetMilliseconds() + m_millisPerTick;
                NextTickTime = nextTime => nextTime + m_millisPerTick;
                // select time should be in microseconds
                SelectTime = nextTickTime =>
                {
                    double res = (nextTickTime - GetTime.GetMilliseconds()) * 1000;
                    return res > 0 ? res : 0;
                };
            }
            else if (ProviderPerfConfig.TicksPerSec <= 1_000_000)
            {
                CurrentTime = () => GetTime.GetMicroseconds();
                InitNextTickTime = () => GetTime.GetMicroseconds() + m_usecPerTick;
                NextTickTime = nextTime => nextTime + m_usecPerTick;
                SelectTime = nextTickTime =>
                {
                    double res = nextTickTime - GetTime.GetMicroseconds();
                    return res > 0 ? res : 0;
                };
            }
            else
            {
                CurrentTime = () => GetTime.GetNanoseconds();
                InitNextTickTime = () => GetTime.GetNanoseconds() + m_nsecPerTick;
                NextTickTime = nextTime => nextTime + m_nsecPerTick;
                // select time should be in microseconds
                SelectTime = nextTickTime =>
                {
                    double res = nextTickTime / 1000 - GetTime.GetMicroseconds();
                    return res > 0 ? res : 0;
                };
            }
        }

        public Func<double>? CurrentTime;

        public Func<double>? InitNextTickTime;

        /// <summary>Given the moment of the previous tick returns the moment when the
        /// next tick should take place</summary>
        ///
        /// <remarks>Time unit depends on the <see cref="ProviderPerfConfig.TicksPerSec"/>
        /// value: it is milliseconds, microseconds, or nanoseconds depending whethere
        /// there are less than a thousand, a million or a billion ticks per
        /// second.</remarks>
        public Func<double, double>? NextTickTime;

        /// <summary>Returns number of microseconds left until next tick</summary>
        ///
        /// <remarks>Receives the moment when the next tick should take place and returns
        /// the number of microseconds left, or 0 if that moment has passed.</remarks>
        ///
        /// <returns>A non-negative number of microseconds left before next benchmark tick
        /// or 0 it has already passed.</returns>
        public Func<double, double>? SelectTime;
    }
}
