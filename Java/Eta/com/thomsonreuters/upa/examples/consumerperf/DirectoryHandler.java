package com.thomsonreuters.upa.examples.consumerperf;

import java.util.ArrayList;

import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.Msg;
import com.thomsonreuters.upa.codec.MsgClasses;
import com.thomsonreuters.upa.codec.RefreshMsg;
import com.thomsonreuters.upa.codec.State;
import com.thomsonreuters.upa.codec.StatusMsg;
import com.thomsonreuters.upa.transport.Channel;
import com.thomsonreuters.upa.transport.Error;
import com.thomsonreuters.upa.transport.TransportBuffer;
import com.thomsonreuters.upa.transport.TransportFactory;

/*
 * This is the source directory handler for the Consumer application.
 * It provides methods for sending the source directory request to a
 * provider and processing the response.  Methods for setting the service
 * name, getting the service id, and closing a source directory stream are
 * also provided.
 */
public class DirectoryHandler
{
	public static final int SRCDIR_STREAM_ID = 2;
	
	/* source directory response information */
	ArrayList<SourceDirectoryResponseInfo> sourceDirectoryResponseInfo = new ArrayList<SourceDirectoryResponseInfo>();
	/* service name requested by application */
	Buffer serviceName = CodecFactory.createBuffer();
	/* service id associated with the service name requested by application */
	int serviceId;
	/* service name found flag */
	boolean serviceNameFound;

	/*
	 * Sets the service name requested by the application.
	 * servicename - The service name requested by the application
	 */
	public void serviceName(String servicename)
	{
		serviceName.data(servicename);
	}

	/*
	 * Returns the service id associated with service name
	 * requested by the application
	 */
	public int serviceId()
	{
		return serviceId;
	}

	public SourceDirectoryResponseInfo responseInfo()
	{
		return sourceDirectoryResponseInfo.get(0);
	}

	/*
	 * Sends a source directory request to a channel.  This consists
	 * of getting a message buffer, encoding the source directory request,
	 * and sending the source directory request to the server.
	 * chnl - The channel to send a source directory request to
	 */
	public int sendRequest(Channel chnl)
	{
		Error error = TransportFactory.createError();
		TransportBuffer msgBuf = null;
		int ret;

		/* get a buffer for the source directory request */
		msgBuf = chnl.getBuffer(SendMsgUtils.MAX_MSG_SIZE, false, error);


		if (msgBuf != null)
		{
			/* encode source directory request */
			if (DirectoryEncoderDecoder.encodeRequest(chnl, msgBuf, SRCDIR_STREAM_ID) != CodecReturnCodes.SUCCESS)
			{
				System.out.println("\nencodeSourceDirectoryRequest() failed");
				return CodecReturnCodes.FAILURE;
			}

			/* send source directory request */
			ret = SendMsgUtils.sendMessage(chnl, msgBuf);
		}
		else
		{
			System.out.println("getBuffer(): Failed <" + error.text() + ">");
			return CodecReturnCodes.FAILURE;
		}
		
		return ret;
	}

	/*
	 * Processes a source directory response.  This consists of calling
	 * decodeSourceDirectoryResponse() to decode the response and calling
	 * sendDictionaryRequest() to send the dictionary request or calling
	 * sendItemRequest() to send the item request if the service name
	 * requested by application is found.
	 * chnl - The channel of the response
	 * msg - The partially decoded message
	 * dIter - The decode iterator
	 */
	public int processResponse(Channel chnl, Msg msg, DecodeIterator dIter)
	{
		State state = null;
		int i;

		switch(msg.msgClass())
		{
		case MsgClasses.REFRESH:
			RefreshMsg refreshMsg = (RefreshMsg)msg;
			/* decode source directory response */
			if (DirectoryEncoderDecoder.decodeResponse(sourceDirectoryResponseInfo, dIter) != CodecReturnCodes.SUCCESS)
			{
				System.out.println("\ndecodeSourceDirectoryResponse() failed");
				return CodecReturnCodes.FAILURE;
			}

			System.out.println("\nReceived Source Directory Response");

			state = refreshMsg.state();
			System.out.println("	" + state);

			/* check if service name received in response matches that entered by user */
			for (i = 0; i < sourceDirectoryResponseInfo.size() - 1; i++)
			{
				System.out.println("Received serviceName: " + sourceDirectoryResponseInfo.get(i).serviceGeneralInfo.serviceName.toString());
				/* check if name matches service name entered by user */
				/* if it does, store the service id */
				if (sourceDirectoryResponseInfo.get(i).serviceGeneralInfo.serviceName.equals(serviceName))
				{
					serviceId = (int) sourceDirectoryResponseInfo.get(i).serviceId.toLong();
					serviceNameFound = true;
				}
				System.out.println("");
			}

			/* exit if service name entered by user cannot be found */
			if (!serviceNameFound)
			{
				System.out.println("\nSource directory response does not contain service name: " + serviceName.toString());
				return CodecReturnCodes.FAILURE;
			}
			
			for (i = 0; i < sourceDirectoryResponseInfo.size() - 1; i++)
			{
				System.out.println(sourceDirectoryResponseInfo.get(i));
			}

			break;

		case MsgClasses.UPDATE:
			System.out.println("\nReceived Source Directory Update");
			break;

		case MsgClasses.CLOSE:
			System.out.println("\nReceived Source Directory Close");
			break;

		case MsgClasses.STATUS:
			StatusMsg statusMsg = (StatusMsg)msg;
			System.out.println("\nReceived Source Directory StatusMsg");
			if (statusMsg.checkHasState())
	    	{
				// get state information
				state = statusMsg.state();
				System.out.println("	" + state + "\n");
			}
			break;

		default:
			System.out.println("\nReceived Unhandled Source Directory Msg Class: " + msg.msgClass());
			return CodecReturnCodes.FAILURE;
		}
		
		return CodecReturnCodes.SUCCESS;
	}

	/*
	 * Close the source directory stream.
	 * chnl - The channel to send a source directory close to
	 */
	public int closeStream(Channel chnl)
	{
		int ret;
		Error error = TransportFactory.createError();
		TransportBuffer msgBuf = null;

		/* get a buffer for the source directory close */
		msgBuf = chnl.getBuffer(SendMsgUtils.MAX_MSG_SIZE, false, error);

		if (msgBuf != null)
		{
			/* encode source directory close */
			if ((ret = DirectoryEncoderDecoder.encodeClose(chnl, msgBuf, SRCDIR_STREAM_ID)) != CodecReturnCodes.SUCCESS)
			{
				System.out.println("\nencodeSourceDirectoryClose() failed with return code: " + ret);
				return ret;
			}

			/* send close */
			ret = SendMsgUtils.sendMessage(chnl, msgBuf);
		}
		else
		{
			System.out.println("getBuffer(): Failed <" + error.text() + ">");
			return CodecReturnCodes.FAILURE;
		}
		
		return ret;

	}

}
