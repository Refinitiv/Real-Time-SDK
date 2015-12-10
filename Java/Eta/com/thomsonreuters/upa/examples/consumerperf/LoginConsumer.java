package com.thomsonreuters.upa.examples.consumerperf;

import java.net.InetAddress;

import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DataStates;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.Msg;
import com.thomsonreuters.upa.codec.MsgClasses;
import com.thomsonreuters.upa.codec.RefreshMsg;
import com.thomsonreuters.upa.codec.State;
import com.thomsonreuters.upa.codec.StatusMsg;
import com.thomsonreuters.upa.codec.StreamStates;
import com.thomsonreuters.upa.transport.Channel;
import com.thomsonreuters.upa.transport.Error;
import com.thomsonreuters.upa.transport.Transport;
import com.thomsonreuters.upa.transport.TransportBuffer;
import com.thomsonreuters.upa.transport.TransportFactory;

public class LoginConsumer
{
	/*
	 * This is the login consumer for both the Consumer and
	 * NIProvider applications.  It only supports a single
	 * login instance for an application.  It provides methods
	 * for sending a login request and processing the response.
	 * A methods for closing the login stream is also provided.
	 */

	/* LoginConsumer only supports a single login instance for an application */

	public static final int CONSUMER = 0;
	public static final int PROVIDER = 1;
	public static final int LOGIN_STREAM_ID = 1;

	/* tracks state returned by status and refresh messages */
	static boolean isClosed = false;
	static boolean isClosedRecoverable = false;
	static boolean isSuspect = true;

	/* login response information */
	LoginResponseInfo loginResponseInfo;

	/* default user name */
	String defaultUsername = "user";
	/* application id */
	String applicationId = "256";
	/* password */
	String pword = "mypassword";
	/* instance id */
	String instanceId = "instance1";

	/* user name requested by application */
	String cmdLineUsername;

	/* login success callback method pointer */
	LoginSuccessCallback loginSuccessCallback;
	
	{
		loginResponseInfo = new LoginResponseInfo();
	}

	/* returns whether login stream is closed */
	public boolean isStreamClosed()
	{
		return isClosed;
	}

	/* returns whether login stream is closed recoverable */
	public boolean isStreamClosedRecoverable()
	{
		return isClosedRecoverable;
	}

	/* returns whether login stream is suspect */
	public boolean isStreamSuspect()
	{
		return isSuspect;
	}

	/*
	 * Sets the user name requested by the application.
	 * username - The user name requested by the application
	 */
	public void username(String username)
	{
		cmdLineUsername = username;
	}

	/*
	 * Returns the login response information structure
	 */
	public LoginResponseInfo responseInfo()
	{
		return loginResponseInfo;
	}

	/*
	 * Sends a login request to a channel.  This consists of getting
	 * a message buffer, setting the login request information, encoding
	 * the login request, and sending the login request to the server.
	 * Returns success if send login request succeeds or failure if it fails.
	 * chnl - The channel to send a login request to
	 * appName - The application name of the login request
	 * role - The role of the login request
	 * loginSuccessCB - The login success callback method pointer
	 */
	public int sendRequest(Channel chnl, String appName, int role, LoginSuccessCallback loginSuccessCB)
	{
		int ret = 0;
		Error error = TransportFactory.createError();
		TransportBuffer msgBuf;
		LoginRequestInfo loginReqInfo = new LoginRequestInfo();
		String userName, positionStr = null;

		/* initialize login request info to default values */
		loginReqInfo.init();

		/* set login success callback method */
		loginSuccessCallback = loginSuccessCB;

		/* get a buffer for the login request */
		msgBuf = chnl.getBuffer(SendMsgUtils.MAX_MSG_SIZE, false, error);

		if (msgBuf != null)
		{
			/* provide login request information */
			/* StreamId */
			loginReqInfo.streamId = LOGIN_STREAM_ID;
			/* Username */
			if (cmdLineUsername.isEmpty()) /* no command line username */
			{
				userName = Transport.userName();
				if (userName != null)
				{
					loginReqInfo.username = userName;
				}
				else
				{
					loginReqInfo.username = defaultUsername;
				}
			}
			else /* use command line username */
			{
				loginReqInfo.username = cmdLineUsername;
			}
			/* ApplicationId */
			loginReqInfo.applicationId = applicationId;
			/* ApplicationName */
			loginReqInfo.applicationName = appName;
			/* Position */
			try
			{
				positionStr = InetAddress.getLocalHost().getHostAddress();
			}
			catch(Exception e)
			{
				System.out.println("InetAddress.getLocalHost().getHostAddress exception: " + e.getLocalizedMessage());
				System.exit(-1);
			}
			if (positionStr == null)
			{
				positionStr = "localhost";
			}
			else
			{
				positionStr += "/net";
			}
			loginReqInfo.position = positionStr;
			/* Password */
			loginReqInfo.password = pword;
			/* InstanceId */
			loginReqInfo.instanceId = instanceId;
			/* if provider, change role and single open from default values */
			if (role == PROVIDER)
			{
				/* this provider does not support SingleOpen behavior */
				loginReqInfo.singleOpen = 0;
				/* provider role */
				loginReqInfo.role = PROVIDER; 
			}
			/* keep default values for all others */

			/* encode login request */
			if ((ret = LoginEncoderDecoder.encodeRequest(chnl, loginReqInfo, msgBuf)) != CodecReturnCodes.SUCCESS)
			{
				System.out.println("\nencodeLoginRequest() failed with return code: " + ret);
				return ret;
			}

			/* send login request */
			ret = SendMsgUtils.sendMessage(chnl, msgBuf);
		}
		else
		{
			System.out.println("getBuffer(): Failed <" + error.text() + ">");
		}

		return ret;
	}

	/*
	 * Processes a login response.  This consists of calling
	 * decodeLoginResponse() to decode the response and calling
	 * loginSuccessCallback() if login is successful.  Returns
	 * success if process login response succeeds or failure
	 * if it fails.
	 * chnl - The channel of the response
	 * msg - The partially decoded message
	 * dIter - The decode iterator
	 */
	public int processResponse(Channel chnl, Msg msg, DecodeIterator dIter)
	{
		int ret;
		State state = null;

		switch(msg.msgClass())
		{
		case MsgClasses.REFRESH:
			RefreshMsg refreshMsg = (RefreshMsg)msg;
			// first clear response info structure
			loginResponseInfo.clear();
			// decode login response
			if ((ret = LoginEncoderDecoder.decodeResponse(loginResponseInfo, msg, dIter)) != CodecReturnCodes.SUCCESS)
			{
				System.out.println("\ndecodeLoginResponse() failed with return code: " + ret);
				return ret;
			}

			System.out.println("\nReceived Login Response for Username: " + loginResponseInfo.username.toString());

			// get state information
			state = refreshMsg.state();
			System.out.println("	" + state);
			
			System.out.println(loginResponseInfo);

			// call login success callback if login okay and is solicited
			if (state.streamState() == StreamStates.OPEN && state.dataState() == DataStates.OK)
			{
				isClosed = false;
				isClosedRecoverable = false;
				isSuspect = false;
				if (loginResponseInfo.isSolicited)
				{
					loginSuccessCallback.loginSucceeded(chnl);
				}
			}
			else /* handle error cases */
			{
				if (state.streamState() == StreamStates.CLOSED_RECOVER)
				{
					System.out.println("\nLogin stream is closed recover");
					isClosedRecoverable = true;
					return CodecReturnCodes.FAILURE;
				}
				else if (state.streamState() == StreamStates.CLOSED)
				{
					System.out.println("\nLogin attempt failed (stream closed)");
					isClosed = true;
					return CodecReturnCodes.FAILURE;
				}
				else if (state.streamState() == StreamStates.OPEN && state.dataState() == DataStates.SUSPECT)
				{
					System.out.println("\nLogin stream is suspect");
					isSuspect = true;
					return CodecReturnCodes.FAILURE;
				}
			}
			break;

		case MsgClasses.STATUS:
			StatusMsg statusMsg = (StatusMsg)msg;
			System.out.println("\nReceived Login StatusMsg\n");
			if (statusMsg.checkHasState())
	    	{
				// get state information
				state = statusMsg.state();
				System.out.println("	" + state + "\n");
				/* handle error cases */
				if (state.streamState() == StreamStates.CLOSED_RECOVER)
				{
					System.out.println("\nLogin stream is closed recover");
					isClosedRecoverable = true;
					return CodecReturnCodes.FAILURE;
				}
				else if (state.streamState() == StreamStates.CLOSED)
				{
					System.out.println("\nLogin attempt failed (stream closed)");
					isClosed = true;
					return CodecReturnCodes.FAILURE;
				}
				else if (state.streamState() == StreamStates.OPEN && state.dataState() == DataStates.SUSPECT)
				{
					System.out.println("\nLogin stream is suspect");
					isSuspect = true;
					return CodecReturnCodes.FAILURE;
				}
			}
			break;

		case MsgClasses.UPDATE:
			System.out.println("\nReceived Login Update\n");
			break;

		case MsgClasses.CLOSE:
			System.out.println("\nReceived Login Close\n");
			isClosed = true;
			return CodecReturnCodes.FAILURE;

		default:
			System.out.println("\nReceived Unhandled Login Msg Class: " + msg.msgClass());
			return CodecReturnCodes.FAILURE;
		}

		return CodecReturnCodes.SUCCESS;
	}

	/*
	 * Close the login stream.  Note that closing login stream
	 * will automatically close all other streams at the provider.
	 * Returns success if close login stream succeeds or failure
	 * if it fails.
	 * chnl - The channel to send a login close to
	 */
	public int closeStream(Channel chnl)
	{
		int ret;
		Error error = TransportFactory.createError();
		TransportBuffer msgBuf = null;

		// get a buffer for the login close
		msgBuf = chnl.getBuffer(SendMsgUtils.MAX_MSG_SIZE, false, error);

		if (msgBuf != null)
		{
			// encode login close
			if ((ret = LoginEncoderDecoder.encodeClose(chnl, msgBuf, LOGIN_STREAM_ID)) != CodecReturnCodes.SUCCESS)
			{
				System.out.println("\nencodeLoginClose() failed with return code: " + ret);
				return ret;
			}

			// send close
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
