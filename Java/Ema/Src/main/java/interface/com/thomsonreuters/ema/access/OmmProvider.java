///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2016. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

/**
 * OmmProvider class encapsulates functionality of an OmmProvider application.
 * 
 * <p>OmmProvider provides interfaces to publish items.<br>
 * It establishes and maintains connection to ADH.
 * 
 * <p>OmmProvider provides a default behavior / functionality.<br>
 * This may be tuned / modified by application when using OmmNiProviderConfig for non-interactive provider.</p>
 * 
 * <p>Application interacts with ADH through the OmmProvider interface methods.<br>
 * The results of these interactions are communicated back to application through
 * OmmProviderClient and OmmProviderErrorClient.</p>
 * 
 * <p>An OmmProvider is created from EmaFactory<br>
 * (see {@link com.thomsonreuters.ema.access.EmaFactory#createOmmProvider(OmmProviderConfig)}
 *  or {@link com.thomsonreuters.ema.access.EmaFactory#createOmmProvider(OmmProviderConfig, OmmProviderErrorClient)}).</p>
 * 
 * 
 * The following code snippet shows basic usage of OmmProvider interface in a simple non-interactive<br>
 * provider application.
 *
 * <pre>
 * // create an implementation for OmmProviderClient to process received Login and Dictionary messages
 * class AppClient implements OmmProviderClient
 * {
 *    public void onRefreshMsg(RefreshMsg refreshMsg, OmmProviderEvent event)
 *    {
 *       System.out.println(refreshMsg);
 *    }
 * 
 *    public void onStatusMsg(StatusMsg statusMsg, OmmProviderEvent event) 
 *    {
 *       System.out.println(statusMsg);
 *    }
 * 
 *    public void onGenericMsg(GenericMsg genericMsg, OmmProviderEvent consumerEvent){}
 *    public void onAllMsg(Msg msg, OmmProviderEvent consumerEvent){}
 * }
 * 
 * public class NIProvider 
 * {
 *    public static void main(String[] args)
 *    {
 *       OmmProvider provider = null;
 *       try
 *       {
 *          AppClient appClient = new AppClient();			
 *          OmmNiProviderConfig config = EmaFactory.createOmmNiProviderConfig();
 * 			
 *          // instantiate OmmProvider object and connect it to an ADH
 *          provider  = EmaFactory.createOmmProvider(config.host("localhost:14003").username("user"));
 *          
 *          // indicate that application is providing ...
 * 			
 *          Thread.sleep(60000);    // API calls onRefreshMsg(), and onStatusMsg()
 *       } 
 *       catch (InterruptedException | OmmException excp)
 *       {
 *          System.out.println(excp.getMessage());
 *       }
 *       finally 
 *       {
 *          if (provider != null) provider.uninitialize();
 *       }
 *    }
 * }
 * </pre>
 * 
 * @see OmmNiProviderConfig
 * @see OmmProviderClient
 * @see OmmProviderErrorClient
 * @see	EmaFactory
 */

public interface OmmProvider 
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
	 * Retrieve internally generated provider instance name.
	 * 
	 * @return name of this OmmProvider instance
	 */
	public String providerName();

	/** 
	 * Opens an item stream.
	 * 
	 * <p>This method is ObjectLevelSafe if OmmProviderErrorClient is used and an error condition is encountered,
	 * then null handle is returned.</p>
	 * 
	 * @throws OmmInvalidUsageException if application passes invalid ReqMsg
	 * 
	 * @param reqMsg specifies item and its unique attributes
	 * @param client specifies OmmProviderClient instance receiving notifications about this item
	 * @return item identifier (a.k.a. handle)
	 */
	public long registerClient(ReqMsg reqMsg, OmmProviderClient client);
	
	/** 
	 * Opens an item stream.
	 * 
	 * <p>This method is ObjectLevelSafe if OmmProviderErrorClient is used and an error condition is encountered,
	 * then null handle is returned.</p>
	 * 
	 * @throws OmmInvalidUsageException if application passes invalid ReqMsg
	 * 
	 * @param reqMsg specifies item and its unique attributes
	 * @param client specifies OmmProviderClient instance receiving notifications about this item
	 * @param closure specifies application defined item identification
	 * @return item identifier (a.k.a. handle)
	 */
	public long registerClient(ReqMsg reqMsg, OmmProviderClient client, Object closure);
	
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
	 * Sends a RefreshMsg.
	 * <p>This method is ObjectLevelSafe.</p>
	 * 
	 * @throws OmmInvalidHandleException if passed in handle does not refer to an open stream
	 * 
	 * @param refreshMsg specifies RefreshMsg to be sent on the open item stream
	 * @param handle identifies item stream on which to send the RefreshMsg
	 */
	public void submit(RefreshMsg refreshMsg, long handle);
	
	/**
	 * Sends a UpdateMsg.
	 * <p>This method is ObjectLevelSafe.</p>
	 * 
	 * @throws OmmInvalidHandleException if passed in handle does not refer to an open stream
	 * 
	 * @param updateMsg specifies UpdateMsg to be sent on the open item stream
	 * @param handle identifies item stream on which to send the UpdateMsg
	 */
	public void submit(UpdateMsg updateMsg, long handle);
	
	/**
	 * Sends a StatusMsg.
	 * <p>This method is ObjectLevelSafe.</p>
	 * 
	 * @throws OmmInvalidHandleException if passed in handle does not refer to an open stream
	 * 
	 * @param statusMsg specifies StatusMsg to be sent on the open item stream
	 * @param handle identifies item stream on which to send the StatusMsg
	 */
	public void submit(StatusMsg statusMsg, long handle);
	
	/**
	 * Relinquishes application thread of control to receive callbacks via OmmProviderClient descendant.
	 * Requires OperationalModel to be set to {@link OmmNiProviderConfig.OperationModel#USER_DISPATCH}.
	 * <p> This method is ObjectLevelSafe.</p>
	 * 
	 * @throws OmmInvalidUsageException if OperationalModel is not set to {@link OmmNiProviderConfig.OperationModel#USER_DISPATCH}
	 * 
	 * @return {@link DispatchReturn#TIMEOUT} if nothing was dispatched; {@link DispatchReturn#DISPATCHED} otherwise
	 */
	public long dispatch();
	
	/**
	 * Relinquishes application thread of control to receive callbacks via OmmProviderClient descendant.
	 * Requires OperationalModel to be set to {@link OmmNiProviderConfig.OperationModel#USER_DISPATCH}.
	 * <p> This method is ObjectLevelSafe.</p>
	 * 
	 * @throws OmmInvalidUsageException if OperationalModel is not set to {@link OmmNiProviderConfig.OperationModel#USER_DISPATCH}
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
	 * Uninitializes the OmmProvider object.
	 * <p> This method is ObjectLevelSafe.</p>
	 */
	public void uninitialize();

}
