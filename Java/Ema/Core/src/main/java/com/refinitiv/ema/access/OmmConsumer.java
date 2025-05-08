///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2019-2022, 2024-2025 LSEG. All rights reserved.
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;

import java.util.List;

/**
 * OmmConsumer class encapsulates functionality of an Omm consuming type application.
 * 
 * <p>OmmConsumer provides interfaces to open, modify and close items.<br>
 * It establishes and maintains connection to server, maintains open item watch list,
 * performs connection and item recovery, etc.</p>
 * 
 * <p>OmmConsumer provides a default behaviour / functionality.<br>
 * This may be tuned / modified by application when using OmmConsumerConfig.</p>
 * 
 * <p>Application interacts with server through the OmmConsumer interface methods.<br>
 * The results of these interactions are communicated back to application through
 * OmmConsumerClient and OmmConsumerErrorClient.</p>
 * 
 * <p>An OmmConsumer is created from EmaFactory<br>
 * (see {@link com.refinitiv.ema.access.EmaFactory#createOmmConsumer(OmmConsumerConfig)}
 *  or {@link com.refinitiv.ema.access.EmaFactory#createOmmConsumer(OmmConsumerConfig, OmmConsumerErrorClient)}).</p>
 * 
 * 
 * The following consumer example shows basic usage of OmmConsumer class in a simple consumer type app.<br>
 * This application opens a regular streaming item named RTR from a service RDF from the 1.1.1.1 server
 * on port 14002.
 *
 * <pre>
 * // create an implementation for OmmConsumerClient to process received item messages
 * class AppClient implements OmmConsumerClient
 * {
 *    public void onRefreshMsg(RefreshMsg refreshMsg, OmmConsumerEvent event)
 *    {
 *       System.out.println(refreshMsg);
 *    }
 * 	
 *    public void onUpdateMsg(UpdateMsg updateMsg, OmmConsumerEvent event) 
 *    {
 *       System.out.println(updateMsg);
 *    }
 * 
 *    public void onStatusMsg(StatusMsg statusMsg, OmmConsumerEvent event) 
 *    {
 *       System.out.println(statusMsg);
 *    }
 * 
 *    public void onGenericMsg(GenericMsg genericMsg, OmmConsumerEvent consumerEvent){}
 *    public void onAckMsg(AckMsg ackMsg, OmmConsumerEvent consumerEvent){}
 *    public void onAllMsg(Msg msg, OmmConsumerEvent consumerEvent){}
 * }
 * 
 * public class Consumer 
 * {
 *    public static void main(String[] args)
 *    {
 *       OmmConsumer consumer = null;
 *       try
 *       {
 *          AppClient appClient = new AppClient();			
 *          OmmConsumerConfig config = EmaFactory.createOmmConsumerConfig();
 * 			
 *          // instantiate OmmConsumer object and connect it to a server
 *          consumer  = EmaFactory.createOmmConsumer(config.host("1.1.1.1:14002"));
 *
 *          // open an item of interest
 *          ReqMsg reqMsg = EmaFactory.createReqMsg();
 *          consumer.registerClient(reqMsg.serviceName("RDF").name("RTR"), appClient);
 * 			
 *          Thread.sleep(60000);    // API calls onRefreshMsg(), onUpdateMsg() and onStatusMsg()
 *       } 
 *       catch (InterruptedException | OmmException excp)
 *       {
 *          System.out.println(excp.getMessage());
 *       }
 *       finally 
 *       {
 *          if (consumer != null) consumer.uninitialize();
 *       }
 *    }
 * }
 * </pre>
 * 
 * @see OmmConsumerConfig
 * @see OmmConsumerClient
 * @see OmmConsumerErrorClient
 * @see	EmaFactory
 */
public interface OmmConsumer
{
	
	/**
	 * The Class DispatchTimeout.
	 */
	public static class DispatchTimeout
	{
		
		/** dispatch blocks till a message arrives. */
		public static final int INFINITE_WAIT = 0;
		
		/** dispatch exits immediately even if there is no message. */
		public static final int NO_WAIT = 1; 
	}

	/**
	 * The Class DispatchReturn.
	 */
	public static class DispatchReturn
	{
		
		/** dispatch exits immediately even if there is no message. */
		public static final int TIMEOUT = 0;
		
		/** a message was dispatched on this dispatch call. */
		public static final int DISPATCHED = 1; 
	}

	/**
	 * Retrieve internally generated consumer instance name.
	 * 
	 * @return name of this OmmConsumer instance
	 */
	public String consumerName();

	/**
	 *  
	 * Opens an item stream.
	 * 
	 * <p>This method is ObjectLevelSafe if OmmConsumerErrorClient is used and an error condition is encountered,
	 * then null handle is returned.</p>
	 *
	 * @param reqMsg specifies item and its unique attributes
	 * @param client specifies OmmConsumerClient instance receiving notifications about this item
	 * @return item identifier (a.k.a. handle)
	 * @throws OmmInvalidUsageException if application passes invalid ReqMsg
	 * @throws OmmInvalidHandleException if application passes invalid parent item handle
	 */
	public long registerClient(ReqMsg reqMsg, OmmConsumerClient client);
	
	/**
	 *  
	 * Opens an item stream.
	 * 
	 * <p>This method is ObjectLevelSafe if OmmConsumerErrorClient is used and an error condition is encountered,
	 * then null handle is returned.</p>
	 *
	 * @param reqMsg specifies item and its unique attributes
	 * @param client specifies OmmConsumerClient instance receiving notifications about this item
	 * @param closure specifies application defined item identification
	 * @return item identifier (a.k.a. handle)
	 * @throws OmmInvalidUsageException if application passes invalid ReqMsg
	 * @throws OmmInvalidHandleException if application passes invalid parent item handle
	 */
	public long registerClient(ReqMsg reqMsg, OmmConsumerClient client, Object closure);
	
	/**
	 *  
	 * Opens an item stream.
	 * 
	 * <p>This method is ObjectLevelSafe if OmmConsumerErrorClient is used and an error condition is encountered,
	 * then null handle is returned.</p>
	 *
	 * @param reqMsg specifies item and its unique attributes
	 * @param client specifies OmmConsumerClient instance receiving notifications about this item
	 * @param closure specifies application defined item identification
	 * @param parentHandle specifies handle of tunnel stream over which this substream is open (required for substreams)
	 * @return item identifier (a.k.a. handle)
	 * @throws OmmInvalidUsageException if application passes invalid ReqMsg
	 * @throws OmmInvalidHandleException if application passes invalid parent item handle
	 */
	public long registerClient(ReqMsg reqMsg, OmmConsumerClient client, Object closure, long parentHandle);
	
	/**
	 * Opens a tunnel stream.
	 * 
	 * <p>This method is ObjectLevelSafe if OmmConsumerErrorClient is used and an error condition is encountered,
	 * then null handle is returned.</p>
	 *
	 * @param tunnelStreamRequest specifies tunnel stream attributes
	 * @param client specifies OmmConsumerClient instance receiving notifications about this item
	 * @return tunnel stream handle (a.k.a. parentHandle)
	 * @throws OmmInvalidUsageException if application passes invalid TunnelStreamRequest
	 */
	public long registerClient(TunnelStreamRequest tunnelStreamRequest, OmmConsumerClient client);
	
	/**
	 * Opens a tunnel stream.
	 * 
	 * <p>This method is ObjectLevelSafe if OmmConsumerErrorClient is used and an error condition is encountered,
	 * then null handle is returned.</p>
	 *
	 * @param tunnelStreamRequest specifies tunnel stream attributes
	 * @param client specifies OmmConsumerClient instance receiving notifications about this item
	 * @param closure specifies application defined item identification
	 * @return tunnel stream handle (a.k.a. parentHandle)
	 * @throws OmmInvalidUsageException if application passes invalid TunnelStreamRequest
	 */
	public long registerClient(TunnelStreamRequest tunnelStreamRequest, OmmConsumerClient client, Object closure);
	
	/**
	 *  
	 * Changes the interest in an open item stream.
	 * The first formal parameter houses a ReqMsg.
	 * ReqMsg attributes that may change are Priority(), InitialImage(), InterestAfterRefresh(),
	 * Pause() and Payload ViewData().
	 * The second formal parameter is a handle that identifies the open stream to be modified.
	 * This method is ObjectLevelSafe.
	 *
	 * @param reqMsg specifies modifications to the open item stream
	 * @param handle identifies item to be modified
	 * @throws OmmInvalidHandleException if passed in handle does not refer to an open stream
	 * @throws OmmInvalidUsageException if passed in ReqMsg violates reissue rules
	 */
	public void reissue(ReqMsg reqMsg, long handle);
	
	/**
	 * Sends a GenericMsg.
	 * <p>This method is ObjectLevelSafe.</p>
	 *
	 * @param genericMsg specifies GenericMsg to be sent on the open item stream
	 * @param handle identifies item stream on which to send the GenericMsg
	 * @throws OmmInvalidHandleException if passed in handle does not refer to an open stream
	 */
	public void submit(GenericMsg genericMsg, long handle);
	
	/**
	 * Sends a PostMsg.
	 * Accepts a PostMsg and optionally a handle associated to an open item stream. 	
	 * Specifying an item handle is known as "on stream posting".
	 * Specifying a login handle is known as "off stream posting".
	 * <p> This method is ObjectLevelSafe.</p>
	 *
	 * @param postMsg specifies PostMsg to be sent on the open item stream
	 * @param handle identifies item stream on which to send the PostMsg
	 * @throws OmmInvalidHandleException if passed in handle does not refer to an open stream
	 */
	void submit(PostMsg postMsg, long handle);
	
	/**
	 * Relinquishes application thread of control to receive callbacks via OmmConsumerClient descendant.
	 * Requires OperationalModel to be set to {@link OmmConsumerConfig.OperationModel#USER_DISPATCH}.
	 * <p> This method is ObjectLevelSafe.</p>
	 *
	 * @return {@link DispatchReturn#TIMEOUT} if nothing was dispatched; {@link DispatchReturn#DISPATCHED} otherwise
	 * @throws OmmInvalidUsageException if OperationalModel is not set to {@link OmmConsumerConfig.OperationModel#USER_DISPATCH}
	 */
	public long dispatch();
	
	/**
	 * Relinquishes application thread of control to receive callbacks via OmmConsumerClient descendant.
	 * Requires OperationalModel to be set to {@link OmmConsumerConfig.OperationModel#USER_DISPATCH}.
	 * <p> This method is ObjectLevelSafe.</p>
	 *
	 * @param timeOut specifies time in microseconds to wait in dispatch() for a message to dispatch
	 * @return {@link DispatchReturn#TIMEOUT} if nothing was dispatched; {@link DispatchReturn#DISPATCHED} otherwise
	 * @throws OmmInvalidUsageException if OperationalModel is not set to {@link OmmConsumerConfig.OperationModel#USER_DISPATCH}
	 */
	public long dispatch(long timeOut);
	
	/**
	 * Relinquishes interest in an open item stream.
	 * <p> This method is ObjectLevelSafe.</p>
	 * 
	 * @param handle identifies item to close
	*/
	public void unregister(long handle);
	
	/**
	 * Uninitializes the OmmConsumer object.
	 * <p> This method is ObjectLevelSafe.</p>
	 */
	public void uninitialize();

	/**
	 * Retrieves channel information on the OmmConsumer object.
	 * 
	 * @throws OmmInvalidUsageException if the request routing feature is enabled on the OmmConsumer object.
	 *
	 * @param ci the ChannelInformation
	 */
	public void channelInformation(ChannelInformation ci);
	
	/**
	 * Allows modifying some I/O values programmatically for a channel to override the default values.
	 * <p> This method is ObjectLevelSafe.</p>
	 * 
	 * @param code provides Code of I/O option defined in {@link IOCtlCode} to modify.
	 * @param value provides Value to modify I/O option to
	 * @throws OmmInvalidUsageException if failed to modify I/O option to
	 */
	public void modifyIOCtl(int code, int value);
	
	
	/** Provide updated OAuth2 credentials when the callback OmmOAuth2ConsumerClient::onCredentialRenewal is called.
	 *  This method allows the application to use a secure credential storage when using LDP functionality such as
	 *  the LDP token service or LDP service discovery.
	 *	This function can only be called within the onCredentialRenewal callback.  It will throw an 
	 *  OmmInvalidUsageException if not called in the callback
	 *	@param credentials OAuth2CredentialRenewal object that contains the credentials.
	 *
	 *	@throws OmmInvalidUsageException if the credential update fails or if this method is called outside of an onCredentialRenewal callback.
	 */
	public void renewOAuthCredentials(OAuth2CredentialRenewal credentials);
	
	/**
	 * Returns a list of channel information for session channels associated with the OmmConsumer object.
	 * 
	 * <p>This function returns an empty list if this event does not have any session channels.</p>
	 * 
	 * @param sessionChannelInfo the ChannelInformation List
	 */
	public void sessionChannelInfo(List<ChannelInformation> sessionChannelInfo);
}

