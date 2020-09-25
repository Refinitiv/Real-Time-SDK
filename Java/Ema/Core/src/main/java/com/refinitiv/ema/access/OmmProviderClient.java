///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;

/**
 * OmmProviderClient interface provides callback methods to pass received messages.
 * 
 * <p>Application needs to implement an application client class inheriting from OmmProviderClient.<br>
 * In its own class, application needs to override callback methods it desires to use for item processing.</p>
 *
 * <p>Application may chose to implement specific callbacks (e.g. {@link #onRefreshMsg(RefreshMsg, OmmProviderEvent)})<br>
 * or a general callback (e.g. {@link #onAllMsg(Msg, OmmProviderEvent)}).</p>
 * 
 * <p>Thread safety of all OmmProviderClient methods depends on the user's implementation.</p>
 *
 *
 * The following code snippet shows basic usage of OmmProviderClient class to print<br>
 * received refresh, and status messages to screen.
 * 
 * <pre>
 * class AppClient implements OmmProviderClient
 * {
 *    public void onRefreshMsg(RefreshMsg refreshMsg, OmmProviderEvent event)
 *    {
 *       System.out.println("Handle: " + event.handle() + " Closure: " + event.closure());
 *       System.out.println(refreshMsg);
 *    }
 * 
 *    public void onStatusMsg(StatusMsg statusMsg, OmmProviderEvent event) 
 *    {
 *       System.out.println("Handle: " + event.handle() + " Closure: " + event.closure());
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
 * </pre>
 * 
 * 
 * @see OmmProvider
 * @see OmmProviderEvent
 * @see Msg
 * @see GenericMsg
 * @see RefreshMsg
 * @see PostMsg
 * @see ReqMsg 
 * 
 */

public interface OmmProviderClient {
	
	/**
	 * This callback is invoked upon receiving a refresh message.
	 * Refresh message may be a start, interim or final part.
	 * 
	 * @param refreshMsg received RefreshMsg ({@link com.refinitiv.ema.access.RefreshMsg})
	 * @param providerEvent identifies open item for which this message is received
	 */
	public void onRefreshMsg(RefreshMsg refreshMsg,	OmmProviderEvent providerEvent);

	/**
	 * This callback is invoked upon receiving a status message. 
	 * 
	 * @param statusMsg received StatusMsg
	 * @param providerEvent identifies open item for which this message is received
	 */
	public void onStatusMsg(StatusMsg statusMsg, OmmProviderEvent providerEvent);

	/**
	 * This callback is invoked upon receiving any generic message.
	 * 
	 * @param genericMsg received GenericMsg ({@link com.refinitiv.ema.access.GenericMsg})
	 * @param providerEvent identifies open item for which this message is received
	 */
	public void onGenericMsg(GenericMsg genericMsg,	OmmProviderEvent providerEvent);
	
	/**
	 * This callback is invoked upon receiving any post message.<br>
	 * 
	 * @param postMsg received PostMsg ({@link com.refinitiv.ema.access.PostMsg})
	 * @param providerEvent identifies open item for which this message is received
	 */
	public void onPostMsg(PostMsg postMsg, OmmProviderEvent providerEvent);
	
	/**
	 * This callback is invoked upon receiving an initial item request message.<br>
	 * 
	 * @param reqMsg received ReqMsg ({@link com.refinitiv.ema.access.ReqMsg})
	 * @param providerEvent identifies open item for which this message is received
	 */
	public void onReqMsg(ReqMsg reqMsg, OmmProviderEvent providerEvent);
	
	/**
	 * This callback is invoked upon receiving a reissue request.<br>
	 * 
	 * @param reqMsg received ReqMsg ({@link com.refinitiv.ema.access.ReqMsg})
	 * @param providerEvent identifies open item for which this message is received
	 */
	public void onReissue(ReqMsg reqMsg, OmmProviderEvent providerEvent);
	
	/**
	 * This callback is invoked upon receiving a close request message.<br>
	 *
	 * @param reqMsg received ReqMsg ({@link com.refinitiv.ema.access.ReqMsg})
	 * @param providerEvent identifies open item for which this message is received
	 */
	public void onClose(ReqMsg reqMsg, OmmProviderEvent providerEvent );

	/**
	 * This callback is invoked upon receiving any message.
	 * 
	 * @param msg received message
	 * @param providerEvent identifies open item for which this message is received
	 */
	public void onAllMsg(Msg msg, OmmProviderEvent providerEvent);

}
