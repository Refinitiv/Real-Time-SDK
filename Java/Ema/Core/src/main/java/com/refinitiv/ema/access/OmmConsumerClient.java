///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;

/**
 * OmmConsumerClient class provides callback interfaces to pass received messages.
 * 
 * <p>Application needs to implement an application client class inheriting from OmmConsumerClient.<br>
 * In its own class, application needs to override callback methods it desires to use for item processing.</p>
 *
 * <p>Application may chose to implement specific callbacks (e.g. {@link #onUpdateMsg(UpdateMsg, OmmConsumerEvent)})<br>
 * or a general callback (e.g. {@link #onAllMsg(Msg, OmmConsumerEvent)}).</p>
 * 
 * <p>Thread safety of all OmmConsumerClient methods depends on the user's implementation.</p>
 *
 *
 * The following code snippet shows basic usage of OmmConsumerClient class to print<br>
 * received refresh, update, and status messages to screen.
 * 
 * <pre>
 * class AppClient implements OmmConsumerClient
 * {
 *    public void onRefreshMsg(RefreshMsg refreshMsg, OmmConsumerEvent event)
 *    {
 *       System.out.println("Handle: " + event.handle() + " Closure: " + event.closure());
 *       System.out.println(refreshMsg);
 *    }
 * 	
 *    public void onUpdateMsg(UpdateMsg updateMsg, OmmConsumerEvent event) 
 *    {
 *       System.out.println("Handle: " + event.handle() + " Closure: " + event.closure());
 *       System.out.println(updateMsg);
 *    }
 * 
 *    public void onStatusMsg(StatusMsg statusMsg, OmmConsumerEvent event) 
 *    {
 *       System.out.println("Handle: " + event.handle() + " Closure: " + event.closure());
 *       System.out.println(statusMsg);
 *    }
 * 
 *    public void onGenericMsg(GenericMsg genericMsg, OmmConsumerEvent consumerEvent){}
 *    public void onAckMsg(AckMsg ackMsg, OmmConsumerEvent consumerEvent){}
 *    public void onAllMsg(Msg msg, OmmConsumerEvent consumerEvent){}
 * }
 * </pre>
 * 
 * 
 * @see OmmConsumer
 * @see Msg
 * @see AckMsg
 * @see GenericMsg
 * @see PostMsg
 * @see RefreshMsg
 * @see StatusMsg
 * @see UpdateMsg
 */
public interface OmmConsumerClient
{
	/**
	 * This callback is invoked upon receiving a refresh message.
	 * Refresh message may be a start, interim or final part.
	 * 
	 * @param refreshMsg received RefreshMsg ({@link com.refinitiv.ema.access.RefreshMsg})
	 * @param consumerEvent identifies open item for which this message is received
	 */
	public void onRefreshMsg(RefreshMsg refreshMsg,	OmmConsumerEvent consumerEvent);

	/**
	 * This callback is invoked upon receiving an update message.
	 * Update messages may be interlaced within a multiple part refresh message sequence.
	 * 
	 * @param updateMsg received UpdateMsg ({@link com.refinitiv.ema.access.UpdateMsg})
	 * @param consumerEvent identifies open item for which this message is received
	 */
	public void onUpdateMsg(UpdateMsg updateMsg, OmmConsumerEvent consumerEvent);

	/**
	 * This callback is invoked upon receiving a status message. 
	 * 
	 * @param statusMsg received StatusMsg
	 * @param consumerEvent identifies open item for which this message is received
	 */
	public void onStatusMsg(StatusMsg statusMsg, OmmConsumerEvent consumerEvent);

	/**
	 * This callback is invoked upon receiving any generic message.
	 * 
	 * @param genericMsg received GenericMsg ({@link com.refinitiv.ema.access.GenericMsg})
	 * @param consumerEvent identifies open item for which this message is received
	 */
	public void onGenericMsg(GenericMsg genericMsg,	OmmConsumerEvent consumerEvent);

	/**
	 * This callback is invoked upon receiving any ack message.
	 * 
	 * @param ackMsg received AckMsg ({@link com.refinitiv.ema.access.AckMsg})
	 * @param consumerEvent identifies open item for which this message is received
	 */
	public void onAckMsg(AckMsg ackMsg, OmmConsumerEvent consumerEvent);

	/**
	 * This callback is invoked upon receiving any message.
	 * 
	 * @param msg received message
	 * @param consumerEvent identifies open item for which this message is received
	 */
	public void onAllMsg(Msg msg, OmmConsumerEvent consumerEvent);
}