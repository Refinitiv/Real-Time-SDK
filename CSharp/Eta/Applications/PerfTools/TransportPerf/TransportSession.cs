/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using System;
using Refinitiv.Common.Interfaces;
using Refinitiv.Eta.Transports.Interfaces;
using Refinitiv.Eta.PerfTools.Common;
using Refinitiv.Eta.Transports;
using System.Net;

namespace Refinitiv.Eta.PerfTools.TransportPerf
{
    /// <summary>
    /// Stores information about an open session on a channel.
    /// Handles sending messages for a transport thread.
    /// </summary>
    public class TransportSession
    {
        private const int SEQNUM_TIMESTAMP_LEN = 16; /* length of sequence number and timestamp */
        private const int MSG_SIZE_LEN = 4;
        private int m_MaxMsgBufSize;
        private ITransportBuffer? m_WritingBuffer;
        private int m_PackedBufferCount;
        private WriteArgs m_WriteArgs = new WriteArgs();

        private long m_SendSequenceNumber;    /* Next sequence number to send. */

        /// <summary>
        /// Gets or sets channel associated with this session.
        /// </summary>
        public ClientChannelInfo? ChannelInfo { get; private set; }

        /// <summary>
        /// Gets or sets next sequence number that should be received.
        /// </summary>
        public long RecvSequenceNumber { get; set; }

        /// <summary>
        /// Gets or sets whether a sequence number has been received yet.
        /// </summary>
        public bool ReceivedFirstSequenceNumber { get; set; }

        /// <summary>
        /// Gets or sets time at which this channel was fully setup.
        /// </summary>
        public long TimeActivated { get; set; }

        public TransportSession(ClientChannelInfo channelInfo)
        {
            m_MaxMsgBufSize = TransportThreadConfig.TotalBuffersPerPack * TransportThreadConfig.MsgSize;

            /* If the buffer is to be packed, add some additional bytes for each message. */
            if (TransportThreadConfig.TotalBuffersPerPack > 1)
                m_MaxMsgBufSize += (TransportThreadConfig.TotalBuffersPerPack * 8);

            ChannelInfo = channelInfo;
            ChannelInfo.UserSpec = this;

            m_WriteArgs.Flags = TransportThreadConfig.WriteFlags;
        }

        /// <summary>
        /// Sends a burst of messages for one tick.
        /// </summary>
        /// <param name="handler">Transport thread sending messages.</param>
        /// <param name="error">Gives detailed information about error if any occurred during socket operations.</param>
        /// <returns><see cref="TransportReturnCode"/></returns>
        public TransportReturnCode SendMsgBurst(TransportThread handler, out Error? error)
        {
            error = null;
            int msgsLeft;
            int latencyUpdateNumber;
            TransportReturnCode ret = TransportReturnCode.SUCCESS;

            /* Determine msgs to send out. Spread the remainder out over the first ticks */
            msgsLeft = TransportThreadConfig.MsgsPerTick;
            if (TransportThreadConfig.MsgsPerTickRemainder > handler.CurrentTicks)
                ++msgsLeft;

            latencyUpdateNumber = (TransportThreadConfig.LatencyMsgsPerSec > 0) ?
                    handler.LatencyRandomArray.Next() : -1;

            for (; msgsLeft > 0; --msgsLeft)
            {
                /* Send the item. */
                ret = SendMsg(handler, msgsLeft,
                        (msgsLeft - 1) == latencyUpdateNumber || TransportThreadConfig.LatencyMsgsPerSec == TransportThreadConfig.ALWAYS_SEND_LATENCY_MSG,
                        out error);

                if (ret < TransportReturnCode.SUCCESS)
                {
                    if (ret == TransportReturnCode.NO_BUFFERS)
                        handler.OutOfBuffersCount.Add(msgsLeft);

                    return ret;
                }
            }

            return ret;
        }

        private TransportReturnCode SendMsg(TransportThread handler, int msgsLeft, bool sendLatency, out Error? error)
        {
            TransportReturnCode ret;
            long currentTime;
            int msgPrefix = SEQNUM_TIMESTAMP_LEN;

            /* Add latency timestamp, if appropriate. */
            if (sendLatency)
                currentTime = (long)GetTime.GetNanoseconds();
            else
                currentTime = 0;

            if ((ret = GetMsgBuffer(out error)) < TransportReturnCode.SUCCESS)
                return ret;

            int bufferLength = m_WritingBuffer!.Length;

            if (bufferLength < TransportThreadConfig.MsgSize)
            {
                Console.WriteLine("Error: TransportSession.SendMsg(): Buffer length {0} is too small to write next message.\n", m_WritingBuffer.Length);
                Environment.Exit(-1);
            }

            /* Add sequence number */
            m_WritingBuffer.Data.Write(IPAddress.NetworkToHostOrder(m_SendSequenceNumber));

            /* Add currentTime */
            m_WritingBuffer.Data.Write(IPAddress.NetworkToHostOrder(currentTime));

            /* Zero out remainder of message */

            for (int i = 0; i < TransportThreadConfig.MsgSize - msgPrefix; i++)
            {
                m_WritingBuffer.Data.Write((byte)0);
            }

            if ((ret = WriteMsgBuffer(handler, msgsLeft > 1, out error)) >= TransportReturnCode.SUCCESS)
            {
                /* update sequence number for next time */
                ++m_SendSequenceNumber;
            }

            return ret;
        }

        private TransportReturnCode GetMsgBuffer(out Error? error)
        {
            error = null;
            IChannel? chnl = ChannelInfo!.Channel;

            if (m_WritingBuffer != null)
                return TransportReturnCode.SUCCESS;
            else
            {
                m_WritingBuffer = chnl!.GetBuffer(m_MaxMsgBufSize, (TransportThreadConfig.TotalBuffersPerPack > 1) ? true : false, out error);
                if (m_WritingBuffer != null)
                {
                    return TransportReturnCode.SUCCESS;
                }
                else if (error.ErrorId == TransportReturnCode.FAILURE)
                {
                    return TransportReturnCode.FAILURE;
                }
                else
                {
                    return TransportReturnCode.NO_BUFFERS;
                }
            }
        }

        private TransportReturnCode WriteMsgBuffer(TransportThread handler, bool allowPack, out Error? error)
        {
            error = null;
            TransportReturnCode ret;
            IChannel? chnl = ChannelInfo!.Channel;

            /* Make sure we stop packing at the end of a burst of msgs
             *   in case the next burst is for a different channel. 
             *   (This will also prevent any latency msgs from sitting in the pack for a tick). */
            if (m_PackedBufferCount == (TransportThreadConfig.TotalBuffersPerPack - 1) || !allowPack)
            {
                /* Send the completed buffer(or if there is no packing being done, send as normal) */
                m_PackedBufferCount = 0;

                ret = chnl!.Write(m_WritingBuffer, m_WriteArgs, out error);

                /* call flush and write again */
                while (ret == TransportReturnCode.WRITE_CALL_AGAIN)
                {
                    if ((ret = chnl.Flush(out error)) < TransportReturnCode.SUCCESS)
                    {
                        Console.WriteLine("Channel.Flush() failed with return code {0} - <{1}>\n", ret, error.Text);
                        return ret;
                    }
                    ret = chnl.Write(m_WritingBuffer, m_WriteArgs, out error);
                }

                m_WritingBuffer = null;

                if (ret >= TransportReturnCode.SUCCESS)
                {
                    handler.BytesSent.Add(m_WriteArgs.BytesWritten);
                    handler.MsgsSent.Increment();
                    return ret;
                }

                switch (ret)
                {
                    case TransportReturnCode.WRITE_FLUSH_FAILED:
                        if (chnl.State == ChannelState.ACTIVE)
                        {
                            handler.BytesSent.Add(m_WriteArgs.BytesWritten);
                            handler.MsgsSent.Increment();
                            return (TransportReturnCode)1;
                        }
                        break;
                    /* Otherwise treat as error, fall through to default. */
                    default:
                        if (ret != TransportReturnCode.NO_BUFFERS)
                        {
                            Console.WriteLine("Channel.Write() failed: {0}({1})\n", ret, error?.Text);
                        }
                        return ret;
                }
            }
            else
            {
                /* Pack the buffer and continue using it. */
                ++m_PackedBufferCount;
                ret = chnl!.PackBuffer(m_WritingBuffer, out error);
                if (ret < TransportReturnCode.SUCCESS)
                {
                    Console.WriteLine("Channel.PackBuffer() failed: {0} <{1}>", error.ErrorId, error.Text);
                    return TransportReturnCode.FAILURE;
                }
                handler.MsgsSent.Increment();
                return TransportReturnCode.SUCCESS;
            }

            return TransportReturnCode.SUCCESS;
        }
    }
}
