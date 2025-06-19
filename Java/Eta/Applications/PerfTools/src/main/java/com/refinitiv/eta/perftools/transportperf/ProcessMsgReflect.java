/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.perftools.transportperf;

import com.refinitiv.eta.perftools.common.ClientChannelInfo;
import com.refinitiv.eta.transport.Channel;
import com.refinitiv.eta.transport.ChannelState;
import com.refinitiv.eta.transport.Error;
import com.refinitiv.eta.transport.TransportBuffer;
import com.refinitiv.eta.transport.TransportFactory;
import com.refinitiv.eta.transport.TransportReturnCodes;
import com.refinitiv.eta.transport.WriteArgs;

/** Process messages as a message reflector. */
public class ProcessMsgReflect implements ProcessMsg
{
	private WriteArgs _writeArgs = TransportFactory.createWriteArgs();
	
	@SuppressWarnings("fallthrough")
	@Override
	public int processMsg(TransportChannelHandler channelHandler, ClientChannelInfo channelInfo, TransportBuffer msgBuffer,	Error error)
	{
		SessionHandler handler = (SessionHandler)channelHandler.userSpec();
		Channel chnl = channelInfo.channel;
		int ret;
		TransportBuffer outBuffer;

		handler.transportThread().msgsReceived().increment();
		handler.transportThread().bytesReceived().add(msgBuffer.length());

		if ((outBuffer = chnl.getBuffer(msgBuffer.length(), false, error)) == null)
		{
			ret = chnl.flush(error);
			if(ret < TransportReturnCodes.SUCCESS)
				return ret;

			if ((outBuffer = chnl.getBuffer(msgBuffer.length(), false, error)) == null)
			{
				System.out.printf("Channel.getbuffer failed after attempting to flush");
				return TransportReturnCodes.NO_BUFFERS;
			}
		}

		if (outBuffer.length() < msgBuffer.length())
			return TransportReturnCodes.FAILURE;
		
		outBuffer.data().put(msgBuffer.data());
		
		ret = chnl.write(outBuffer, _writeArgs, error);

		/* call flush and write again */
		while (ret == TransportReturnCodes.WRITE_CALL_AGAIN)
		{
			if ((ret = chnl.flush(error)) < TransportReturnCodes.SUCCESS)
			{
				System.out.printf("Channel.flush() failed with return code %d - <%s>\n", ret, error.text());
				return ret;
			}
			ret = chnl.write(outBuffer, _writeArgs, error);
		}

		if (ret >= TransportReturnCodes.SUCCESS)
		{
			handler.transportThread().bytesSent().add(_writeArgs.bytesWritten());
			handler.transportThread().msgsSent().increment();

			if(ret > 0)
				channelHandler.requestFlush(channelInfo);
			return ret;
		}

		switch(ret)
		{
			case TransportReturnCodes.WRITE_FLUSH_FAILED:
				if (chnl.state() == ChannelState.ACTIVE)
				{
					handler.transportThread().bytesSent().add(_writeArgs.bytesWritten());
					handler.transportThread().msgsSent().increment();
					channelHandler.requestFlush(channelInfo);
					return 1;
				}
				/* Otherwise treat as error, fall through to default. */
			default:
                if (ret != TransportReturnCodes.NO_BUFFERS)
                {
                    System.out.printf("Channel.write() failed: %d(%s)\n", ret, error.text());
                }
				return ret;
		}
	}
}
