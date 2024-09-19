/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Common;
using LSEG.Eta.PerfTools.Common;
using LSEG.Eta.Transports;
using System.Net;

namespace LSEG.Eta.PerfTools.TransportPerf
{
    /// <summary>
    /// Process messages as a reader/writer.
    /// </summary>
    public class ProcessMsgReadWrite : IProcessMsg
    {
        public PerfToolsReturnCode ProcessMsg(TransportChannelHandler channelHandler, ClientChannelInfo channelInfo, ITransportBuffer msgBuffer, out Error? error)
        {
            error = null;
            int minMsgSize = sizeof(long) * 2;
            SessionHandler handler = (SessionHandler)channelHandler.UserSpec!;
            TransportSession session = (TransportSession)channelInfo.UserSpec!;
            long timeTracker;

            handler.TransportThread!.MsgsReceived.Increment();
            handler.TransportThread.BytesReceived.Add(msgBuffer.Length());

            if (msgBuffer.Length() < minMsgSize)
            {
                error = new Error
                {
                    ErrorId = TransportReturnCode.FAILURE,
                    Text = $"Error: Message was too small to be valid(length {msgBuffer.Length})"
                };

                return (PerfToolsReturnCode)TransportReturnCode.FAILURE;
            }

            ByteBuffer byteBuffer = msgBuffer.Data;
            if (session.ReceivedFirstSequenceNumber)
            {
                long recvSequenceNumber;

                recvSequenceNumber = IPAddress.NetworkToHostOrder(msgBuffer.Data.ReadLong());

                if (session.RecvSequenceNumber != recvSequenceNumber)
                {
                    error = new Error
                    {
                        ErrorId = TransportReturnCode.FAILURE,
                        Text = $"Error: Received out-of-order sequence number({recvSequenceNumber} instead of {session.RecvSequenceNumber})."
                    };

                    return (PerfToolsReturnCode)TransportReturnCode.FAILURE;
                }

                session.RecvSequenceNumber += 1;

                timeTracker = IPAddress.NetworkToHostOrder(msgBuffer.Data.ReadLong());

                if (timeTracker > 0)
                {
                    TransportThread.TimeRecordSubmit(handler.LatencyRecords, timeTracker, (long)GetTime.GetNanoseconds(), 1000);
                }
            }
            else
            {
                session.RecvSequenceNumber = IPAddress.NetworkToHostOrder(byteBuffer.ReadLong());
                session.ReceivedFirstSequenceNumber = true;
                session.RecvSequenceNumber += 1;
            }

            return (PerfToolsReturnCode)TransportReturnCode.SUCCESS;
        }
    }
}
