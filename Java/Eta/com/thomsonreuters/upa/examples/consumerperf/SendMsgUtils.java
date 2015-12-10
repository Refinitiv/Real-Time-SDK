package com.thomsonreuters.upa.examples.consumerperf;

import com.thomsonreuters.upa.codec.Msg;
import com.thomsonreuters.upa.transport.Channel;
import com.thomsonreuters.upa.transport.Error;
import com.thomsonreuters.upa.transport.TransportBuffer;
import com.thomsonreuters.upa.transport.TransportFactory;
import com.thomsonreuters.upa.transport.TransportReturnCodes;
import com.thomsonreuters.upa.transport.WriteArgs;
import com.thomsonreuters.upa.transport.WriteFlags;
import com.thomsonreuters.upa.transport.WritePriorities;

public class SendMsgUtils
{
	public static final int MAX_MSG_SIZE = 1024;
	public static final int NUM_CLIENT_SESSIONS = 5;

	public static int sendMessage(Channel chnl, TransportBuffer msgBuf)
	{
		Error error =  TransportFactory.createError();
		int	retval = 0;
		WriteArgs writeArgs = TransportFactory.createWriteArgs();

		/* send the request */
        writeArgs.priority(WritePriorities.HIGH);
        writeArgs.flags(WriteFlags.DIRECT_SOCKET_WRITE);
		retval = chnl.write(msgBuf, writeArgs, error);
		if (retval == TransportReturnCodes.WRITE_CALL_AGAIN)
		{
			/* call flush and write again */
			while (retval == TransportReturnCodes.WRITE_CALL_AGAIN)
			{
				if ((retval = chnl.flush(error)) < TransportReturnCodes.SUCCESS)
				{
					System.out.println("Flush() failed with return code " + retval + " - <" + error.text() + ">");
				}
		        writeArgs.priority(WritePriorities.HIGH);
		        writeArgs.flags(WriteFlags.DIRECT_SOCKET_WRITE);
				retval = chnl.write(msgBuf, writeArgs, error);
			}
		}
		else if (retval <= TransportReturnCodes.FAILURE)
		{
			/* Write failed, release buffer */
			System.out.println("Write() failed with return code " + retval + " - <" + error.text() + ">");
			chnl.releaseBuffer(msgBuf, error);
		}
		
		return retval;
	}
	
	public static int sendPing(Channel chnl)
	{
		Error error =  TransportFactory.createError();
		int ret = 0;

		if ((ret = chnl.ping(error)) < TransportReturnCodes.SUCCESS)
		{
			System.out.println("\nPing(): Failed on channel " + chnl.selectableChannel() + " with code: " + ret);
			return ret;
		}

		return TransportReturnCodes.SUCCESS;
	}
	
	public static int sendNotSupportedStatus(Channel chnl, Msg requestMsg)
	{
		return TransportReturnCodes.SUCCESS;
	}
	
	public static int encodeNotSupportedStatus(Channel chnl, Msg requestMsg, TransportBuffer msgBuf)
	{
		return TransportReturnCodes.SUCCESS;
	}
}
