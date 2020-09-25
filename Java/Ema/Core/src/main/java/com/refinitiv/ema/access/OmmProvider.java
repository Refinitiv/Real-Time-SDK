///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;

/**
 * OmmProvider class encapsulates functionality of an OmmProvider application.
 * 
 * OmmProvider class provides interfaces for interactive and non interactive OmmProvider application use cases.
 * 
 * <p>The non interactive use case allows applications to publish items without any item request.
 * In this case OmmProvider establishes and maintains the configured connection to ADH. Right after the connection
 * is established, the application may start publishing item specific data while the app assigns unique handles 
 * or identifiers to each item.</p>
 *
 * <p>The interactive use case works based on the item requests received from client applications.
 * In this case OmmProvider establishes a server port to which clients connect. After clients login request
 * is accepted, the provider application may receive item requests from the connected client application.
 * The requested items are identified by the EMA with a unique handle or identifier. Provider application
 * uses this handle to respond to the item requests.</p>
 * 
 * <p>OmmProvider provides a default behavior / functionality.<br>
 * This may be tuned / modified by application when using OmmNiProviderConfig for non-interactive provider and 
 * OmmIProviderConfig for interactive provider.</p>
 * 
 * <p>Application interacts with ADH through the OmmProvider interface methods.<br>
 * The results of these interactions are communicated back to application through
 * OmmProviderClient and OmmProviderErrorClient.</p>
 * 
 * <p>An OmmProvider is created from EmaFactory<br>
 * (see {@link com.refinitiv.ema.access.EmaFactory#createOmmProvider(OmmProviderConfig)}
 *  or {@link com.refinitiv.ema.access.EmaFactory#createOmmProvider(OmmProviderConfig, OmmProviderErrorClient)}).</p>
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
 *    public void onPostMsg(PostMsg postMsg, OmmProviderEvent providerEvent) {}
 *	  public void onReqMsg(ReqMsg reqMsg, OmmProviderEvent providerEvent) {}
 *	  public void onReissue(ReqMsg reqMsg, OmmProviderEvent providerEvent) {}
 *    public void onClose(OmmProviderEvent providerEvent) {}
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
 * The following code snippet shows basic usage of OmmProvider interface in a simple interactive<br>
 * provider application.
 * 
 * <pre>
 * // create an implementation for OmmProviderClient to process request messages for login and market price domains
 * class AppClient implements OmmProviderClient
 * {
 *	public long itemHandle = 0;
 *	
 *	public void onReqMsg(ReqMsg reqMsg, OmmProviderEvent event)
 *	{
 *		switch (reqMsg.domainType())
 *		{
 *			case EmaRdm.MMT_LOGIN :
 *				processLoginRequest(reqMsg, event);
 *				break;
 *			case EmaRdm.MMT_MARKET_PRICE :
 *				processMarketPriceRequest(reqMsg, event);
 *				break;
 *		}
 *	}
 *	
 *	public void onRefreshMsg(RefreshMsg refreshMsg,	OmmProviderEvent event){}
 *	public void onStatusMsg(StatusMsg statusMsg, OmmProviderEvent event){}
 *	public void onGenericMsg(GenericMsg genericMsg, OmmProviderEvent event){}
 *	public void onPostMsg(PostMsg postMsg, OmmProviderEvent event){}
 *	public void onReissue(ReqMsg reqMsg, OmmProviderEvent event){}
 *	public void onClose(ReqMsg reqMsg, OmmProviderEvent event){}
 *	public void onAllMsg(Msg msg, OmmProviderEvent event){}
 *	
 *	void processLoginRequest(ReqMsg reqMsg, OmmProviderEvent event)
 *	{
 *		event.provider().submit( EmaFactory.createRefreshMsg().domainType(EmaRdm.MMT_LOGIN).name(reqMsg.name()).
 *				nameType(EmaRdm.USER_NAME).complete(true).solicited(true).
 *				state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "Login accepted"),
 *				event.handle() );
 *	}
 *	
 *	void processMarketPriceRequest(ReqMsg reqMsg, OmmProviderEvent event)
 *	{		
 *		FieldList fieldList = EmaFactory.createFieldList();
 *		fieldList.add( EmaFactory.createFieldEntry().real(22, 3990, OmmReal.MagnitudeType.EXPONENT_NEG_2));
 *		fieldList.add( EmaFactory.createFieldEntry().real(25, 3994, OmmReal.MagnitudeType.EXPONENT_NEG_2));
 *		fieldList.add( EmaFactory.createFieldEntry().real(30, 9,  OmmReal.MagnitudeType.EXPONENT_0));
 *		fieldList.add( EmaFactory.createFieldEntry().real(31, 19, OmmReal.MagnitudeType.EXPONENT_0));
 *		
 *		event.provider().submit( EmaFactory.createRefreshMsg().name(reqMsg.name()).serviceId(reqMsg.serviceId()).solicited(true).
 *				state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "Refresh Completed").
 *				payload(fieldList).complete(true),
 *				event.handle() );
 *
 *		itemHandle = event.handle();
 *	}
 *	
 *	public class IProvider
 *	{
 *		public static void main(String[] args)
 *		{
 *			OmmProvider provider = null;
 *			try
 *			{
 *				AppClient appClient = new AppClient();
 *				FieldList fieldList = EmaFactory.createFieldList();
 *	
 *				OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig();
 *				
 *				// instantiate OmmProvider object and bind a server port on 14002
 *				provider = EmaFactory.createOmmProvider(config.port("14002"), appClient);
 *				
 *				while( appClient.itemHandle == 0 ) Thread.sleep(1000);
 *					
 *				for( int i = 0; i &lt; 60; i++ )
 *				{
 *					fieldList.clear();
 *					fieldList.add(EmaFactory.createFieldEntry().real(22, 3991 + i, OmmReal.MagnitudeType.EXPONENT_NEG_2));
 *					fieldList.add(EmaFactory.createFieldEntry().real(30, 10 + i, OmmReal.MagnitudeType.EXPONENT_0));
 *					
 *					provider.submit( EmaFactory.createUpdateMsg().payload( fieldList ), appClient.itemHandle );
 *					
 *					Thread.sleep(1000);
 *				}
 *			} 
 *			catch (InterruptedException | OmmException excp)
 *			{
 *				System.out.println(excp.getMessage());
 *			}
 *			finally 
 *			{
 *				if (provider != null) provider.uninitialize();
 *			}
 *		}
 *	}
 * </pre>
 * 
 * @see OmmNiProviderConfig
 * @see OmmIProviderConfig
 * @see OmmProviderClient
 * @see OmmProviderErrorClient
 * @see	EmaFactory
 */
import java.util.List;

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
	 * Returns Provider's role
	 * 
	 * @return role of this OmmProvider instance
	 */
	public int providerRole();

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
	 * Changes the interest in an open item stream.
	 * The first formal parameter houses a ReqMsg.
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
	 * @throws OmmInvalidUsageException if failed to submit genericMsg
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
	 * @throws OmmInvalidUsageException if failed to submit refreshMsg
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
	 * @throws OmmInvalidUsageException if failed to submit updateMsg
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
	 * @throws OmmInvalidUsageException if failed to submit statusMsg
	 * @throws OmmInvalidHandleException if passed in handle does not refer to an open stream
	 * 
	 * @param statusMsg specifies StatusMsg to be sent on the open item stream
	 * @param handle identifies item stream on which to send the StatusMsg
	 */
	public void submit(StatusMsg statusMsg, long handle);

	/**
	 * Sends a AckMsg.
	 * <p>This method is ObjectLevelSafe.</p>
	 * 
	 * @throws OmmInvalidUsageException if failed to submit ackMsg
	 * @throws OmmInvalidHandleException if passed in handle does not refer to an open stream
	 * 
	 * @param ackMsg specifies AckMsg to be sent on the open item stream
	 * @param handle identifies item stream on which to send the AckMsg
	 */
	public void submit(AckMsg ackMsg, long handle);
	
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

	/**
	 * Returns a list of client ChannelInformation (e.g., the host name from which the client
	 * connected) for clients connected to an IProvider application.
	 * <p> This method is ObjectLevelSafe.</p>
	 *
	 * @throws OmmInvalidUsageException if is called by an NiProvider application.
	 *
	 * @param ci the ChannelInformation List
	 */
	public void connectedClientChannelInfo(List<ChannelInformation> ci);

	/**
	 * Returns the channel information for an NiProvider application. The channel would be
	 * the channel used to connect to the ADH, for example.
	 * <p> This method is ObjectLevelSafe.</p>
	 * 
	 * @throws OmmInvalidUsageException if is called by an IProvider applications.
	 *
	 * @param ci the ChannelInformation 
	 */
	public void channelInformation(ChannelInformation ci);
	
	/**
	 * Allows modifying some I/O values programmatically for a channel to override the default values for Non-Interactive Providers applications.
	 * <p> This method is ObjectLevelSafe.</p>
	 * 
	 * @param code provides Code of I/O option defined in {@link IOCtlCode} to modify.
	 * @param value provides Value to modify I/O option to
	 * @throws OmmInvalidUsageException if failed to modify I/O option to
	 */
	public void modifyIOCtl(int code, int value);
	
	/**
	 * Allows modifying some I/O values programmatically for a channel to override the default values for Interactive Providers applications.
	 * <p> This method is ObjectLevelSafe.</p>
	 * 
	 * @param code provides Code of I/O option defined in {@link IOCtlCode} to modify.
	 * @param value provides Value to modify I/O option to
	 * @param handle identifies item or login stream 
	 */
	public void modifyIOCtl(int code, int value, long handle);
}
