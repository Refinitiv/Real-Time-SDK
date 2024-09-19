/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.PerfTools.Common;
using LSEG.Eta.Transports;
using LSEG.Eta.Common;
using System;

namespace LSEG.Eta.PerfTools.TransportPerf
{
    /// <summary>
    /// Process messages as a message reflector.
    /// </summary>
    public class ProcessMsgReflect : IProcessMsg
    {
        private WriteArgs m_WriteArgs = new WriteArgs();

        public PerfToolsReturnCode ProcessMsg(TransportChannelHandler channelHandler, ClientChannelInfo channelInfo, ITransportBuffer msgBuffer, out Error? error)
        {
            SessionHandler handler = (SessionHandler)channelHandler.UserSpec!;
			IChannel chnl = channelInfo.Channel!;
			TransportReturnCode ret;
			ITransportBuffer outBuffer;

			handler.TransportThread!.MsgsReceived.Increment();
			handler.TransportThread.BytesReceived.Add(msgBuffer.Length());

			if ((outBuffer = chnl.GetBuffer(msgBuffer.Length(), false, out error)) == null)
			{
				ret = chnl.Flush(out error);
				if (ret < TransportReturnCode.SUCCESS)
					return (PerfToolsReturnCode)ret;

				if ((outBuffer = chnl.GetBuffer(msgBuffer.Length(), false, out error)) == null)
				{
					Console.WriteLine("Channel.Getbuffer() failed after attempting to flush");
					return (PerfToolsReturnCode)TransportReturnCode.NO_BUFFERS;
				}
			}

			if (outBuffer.Length() < msgBuffer.Length())
				return (PerfToolsReturnCode)TransportReturnCode.FAILURE;

			outBuffer.Data.Put(msgBuffer.Data);

			ret = chnl.Write(outBuffer, m_WriteArgs, out error);

			/* call flush and write again */
			while (ret == TransportReturnCode.WRITE_CALL_AGAIN)
			{
				if ((ret = chnl.Flush(out error)) < TransportReturnCode.SUCCESS)
				{
					Console.WriteLine($"Channel.Flush() failed with return code {ret} - <{error.Text}>");
					return (PerfToolsReturnCode)ret;
				}
				ret = chnl.Write(outBuffer, m_WriteArgs, out error);
			}

			if (ret >= TransportReturnCode.SUCCESS)
			{
				handler.TransportThread.BytesSent.Add(m_WriteArgs.BytesWritten);
				handler.TransportThread.MsgsSent.Increment();

				if (ret > 0)
					TransportChannelHandler.RequestFlush(channelInfo);
				return (PerfToolsReturnCode)ret;
			}

			switch (ret)
			{
				case TransportReturnCode.WRITE_FLUSH_FAILED:
					if (chnl.State == ChannelState.ACTIVE)
					{
						handler.TransportThread.BytesSent.Add(m_WriteArgs.BytesWritten);
						handler.TransportThread.MsgsSent.Increment();
						TransportChannelHandler.RequestFlush(channelInfo);
						return (PerfToolsReturnCode)1;
					}

					/* Otherwise treat as error, fall through to default. */
					goto default;

				default:
				if (ret != TransportReturnCode.NO_BUFFERS)
				{
					Console.WriteLine($"Channel.Write() failed: {ret}({error.Text})");
				}
				return (PerfToolsReturnCode)ret;
			};
        }
    }
}
