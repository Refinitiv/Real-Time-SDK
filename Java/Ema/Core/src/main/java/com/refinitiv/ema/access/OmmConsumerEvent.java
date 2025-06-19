/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.access;

import java.util.List;

/**
 * OmmConsumerEvent encapsulates item identifiers.
 * 
 * <p>OmmConsumerEvent is used to convey item identifiers to application.<br>
 * OmmConsumerEvent is returned through OmmConsumerClient callback methods.</p>
 * 
 * <p>OmmConsumerEvent is read only. It is used for item identification only.</p>
 * 
 * @see OmmConsumer
 * @see OmmConsumerClient
 */
public interface OmmConsumerEvent
{
	/**
	 * Returns a unique item identifier (a.k.a., item handle) associated by EMA with an open item stream.<br>
	 * Item identifier is returned from all OmmConsumer registerClient() methods. For tunnel stream onStatusMsg()<br>
	 * call backs this is the parent handle returned by the OmmConsumer registerClient() method. For tunnel stream<br>
	 * sub-stream call backs this is the handle of the sub-stream itself. 
	 * 
	 * @return item identifier or handle
	 */
	public long handle();

	/**
	 * Returns an identifier (a.k.a., closure) associated with an open stream by consumer application.<br>
	 * Application associates the closure with an open item stream on<br>
	 * OmmConsumer registerClient(... , ... , Object closure, ...) methods
	 * 
	 * @return closure value
	 */
	public Object closure();

	/**
	 * Returns current item's parent item identifier (a.k.a. parent item handle).<br>
	 * Application specifies parent item identifier on<br>
	 * OmmConsumer registerClient(... , ... , ... , long parentHandle) method.<br>
	 * For tunnel stream sub-stream call backs this is the handle of parent tunnel<br>
	 * stream. For batch request items this is the item identifier of the top level<br>
	 * batch request.
	 * 
	 * @return parent item identifier or parent handle
	*/
	public long parentHandle();

	/**
	 * Returns the channel information associated with the event.
	 * 
	 * @return the channel information associated with the event
	 */
	public ChannelInformation channelInformation();
	
	/**
	 * Returns a list of channel information for session channels associated with the event.
	 * 
	 * <p>This function returns an empty list if this event does not have any session channels.</p>
	 * 
	 * @param sessionChannelInfo the ChannelInformation List
	 */
	public void sessionChannelInfo(List<ChannelInformation> sessionChannelInfo);
}

