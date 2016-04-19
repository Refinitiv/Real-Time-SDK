///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

/**
 * OmmConsumer class encapsulates functionality of an Omm consuming type application.
 * 
 * <p>OmmConsumer provides interfaces to open, modify and close items. It establishes and maintains
 * connection to server, maintains open item watch list, performs connection and item recovery, etc.</p>
 * 
 * <p>OmmConsumer provides a default behaviour / functionality. This may be tuned / modified by
 * application when using OmmConsumerConfig.</p>
 * 
 * <p>Application interacts with server through the OmmConsumer interface methods. The results of
 * these interactions are communicated back to application through OmmConsumerClient and
 * OmmConsumerErrorClient.</p>
 * 
 * <p>An OmmConsumer is created from EmaFactory
 * (see {@link com.thomsonreuters.ema.access.EmaFactory#createOmmConsumer(OmmConsumerConfig)}
 *  or {@link com.thomsonreuters.ema.access.EmaFactory#createOmmConsumer(OmmConsumerConfig, OmmConsumerErrorClient)}).</p>
 * 
 * @see OmmConsumerConfig
 * @see OmmConsumerClient
 * @see OmmConsumerErrorClient
 * @see	EmaFactory
 */
public interface OmmConsumer
{
	public static class DispatchTimeout
	{
		/**
		 * dispatch blocks till a message arrives
		 */
		public static final int INFINITE_WAIT = 0;
		
		/**
		 * dispatch exits immediately even if there is no message
		 */
		public static final int NO_WAIT = 1; 
	}

	public static class DispatchReturn
	{
		/**
		 * dispatch exits immediately even if there is no message
		 */
		public static final int TIMEOUT = 0;
		
		/**
		 * a message was dispatched on this dispatch call
		 */
		public static final int DISPATCHED = 1; 
	}

	/**
	 * Retrieve internally generated consumer instance name.
	 * 
	 * @return name of this OmmConsumer instance
	 */
	public String consumerName();

	/** 
	 * Opens an item stream.
	 * 
	 * <p>This method is ObjectLevelSafe if OmmConsumerErrorClient is used and an error condition is encountered,
	 * then null handle is returned.</p>
	 * 
	 * @throws OmmInvalidUsageException if application passes invalid ReqMsg
	 * @throws OmmInvalidHandleException if application passes invalid parent item handle
	 * 
	 * @param reqMsg specifies item and its unique attributes
	 * @param client specifies OmmConsumerClient instance receiving notifications about this item
	 * @return item identifier (a.k.a. handle)
	 */
	public long registerClient(ReqMsg reqMsg, OmmConsumerClient client);
	
	/** 
	 * Opens an item stream.
	 * 
	 * <p>This method is ObjectLevelSafe if OmmConsumerErrorClient is used and an error condition is encountered,
	 * then null handle is returned.</p>
	 * 
	 * @throws OmmInvalidUsageException if application passes invalid ReqMsg
	 * @throws OmmInvalidHandleException if application passes invalid parent item handle
	 * 
	 * @param reqMsg specifies item and its unique attributes
	 * @param client specifies OmmConsumerClient instance receiving notifications about this item
	 * @param closure specifies application defined item identification
	 * @return item identifier (a.k.a. handle)
	 */
	public long registerClient(ReqMsg reqMsg, OmmConsumerClient client, Object closure);
	
	/** 
	 * Opens an item stream.
	 * 
	 * <p>This method is ObjectLevelSafe if OmmConsumerErrorClient is used and an error condition is encountered,
	 * then null handle is returned.</p>
	 * 
	 * @throws OmmInvalidUsageException if application passes invalid ReqMsg
	 * @throws OmmInvalidHandleException if application passes invalid parent item handle
	 * 
	 * @param reqMsg specifies item and its unique attributes
	 * @param client specifies OmmConsumerClient instance receiving notifications about this item
	 * @param closure specifies application defined item identification
	 * @param parentHandle specifies a handle of stream over which this stream is open
	 * @return item identifier (a.k.a. handle)
	 */
	public long registerClient(ReqMsg reqMsg, OmmConsumerClient client, Object closure, long parentHandle);
	
	/**
	 * Opens a tunnel stream.
	 * 
	 * <p>This method is ObjectLevelSafe if OmmConsumerErrorClient is used and an error condition is encountered,
	 * then null handle is returned.</p>
	 * 
	 * @throws OmmInvalidUsageException if application passes invalid TunnelStreamRequest
	 * 
	 * @param tunnelStreamRequest specifies tunnel stream attributes
	 * @param client specifies OmmConsumerClient instance receiving notifications about this item
	 * 
	 * @return tunnel stream handle (a.k.a. parentHandle)
	 */
	public long registerClient(TunnelStreamRequest tunnelStreamRequest, OmmConsumerClient client);
	
	/**
	 * Opens a tunnel stream.
	 * 
	 * <p>This method is ObjectLevelSafe if OmmConsumerErrorClient is used and an error condition is encountered,
	 * then null handle is returned.</p>
	 * 
	 * @throws OmmInvalidUsageException if application passes invalid TunnelStreamRequest
	 * 
	 * @param tunnelStreamRequest specifies tunnel stream attributes
	 * @param client specifies OmmConsumerClient instance receiving notifications about this item
	 * @param closure specifies application defined item identification
	 * 
	 * @return tunnel stream handle (a.k.a. parentHandle)
	 */
	public long registerClient(TunnelStreamRequest tunnelStreamRequest, OmmConsumerClient client, Object closure);
	
	/** 
	 * Changes the interest in an open item stream.
	 * The first formal parameter houses a ReqMsg.
	 * ReqMsg attributes that may change are Priority(), InitialImage(), InterestAfterRefresh(),
	 * Pause() and Payload ViewData().
	 * The second formal parameter is a handle that identifies the open stream to be modified.
	 * This method is ObjectLevelSafe.
	 * 
	 * @throws OmmInvalidHandleException if passed in handle does not refer to an open stream
	 * @throws OmmInvalidUsageException if passed in ReqMsg violates reissue rules
	 * 
	 * @param reqMsg specifies modifications to the open item stream
	 * @param handle identifies item to be modified
	 */
	public void reissue(ReqMsg reqMsg, long handle);
	
	/**
	 * Sends a GenericMsg.
	 * <p>This method is ObjectLevelSafe.</p>
	 * 
	 * @throws OmmInvalidHandleException if passed in handle does not refer to an open stream
	 * 
	 * @param genericMsg specifies GenericMsg to be sent on the open item stream
	 * @param handle identifies item stream on which to send the GenericMsg
	 */
	public void submit(GenericMsg genericMsg, long handle);
	
	/**
	 * Sends a PostMsg.
	 * Accepts a PostMsg and optionally a handle associated to an open item stream. 	
	 * Specifying an item handle is known as "on stream posting".
	 * Specifying a login handle is known as "off stream posting".
	 * <p> This method is ObjectLevelSafe.</p>
	 * 
	 * @throws OmmInvalidHandleException if passed in handle does not refer to an open stream
	 * 
	 * @param postMsg specifies PostMsg to be sent on the open item stream
	 * @param handle identifies item stream on which to send the PostMsg
	 */
	void submit(PostMsg postMsg, long handle);
	
	/**
	 * Relinquishes application thread of control to receive callbacks via OmmConsumerClient descendant.
	 * Requires OperationalModel to be set to {@link OmmConsumerConfig.OperationModel#USER_DISPATCH}.
	 * <p> This method is ObjectLevelSafe.</p>
	 * 
	 * @throws OmmInvalidUsageException if OperationalModel is not set to {@link OmmConsumerConfig.OperationModel#USER_DISPATCH}
	 * 
	 * @return {@link DispatchReturn#TIMEOUT} if nothing was dispatched; {@link DispatchReturn#DISPATCHED} otherwise
	 */
	public long dispatch();
	
	/**
	 * Relinquishes application thread of control to receive callbacks via OmmConsumerClient descendant.
	 * Requires OperationalModel to be set to {@link OmmConsumerConfig.OperationModel#USER_DISPATCH}.
	 * <p> This method is ObjectLevelSafe.</p>
	 * 
	 * @throws OmmInvalidUsageException if OperationalModel is not set to {@link OmmConsumerConfig.OperationModel#USER_DISPATCH}
	 * 
	 * @param timeOut specifies time in microseconds to wait in dispatch() for a message to dispatch
	 * @return {@link DispatchReturn#TIMEOUT} if nothing was dispatched; {@link DispatchReturn#DISPATCHED} otherwise
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

}