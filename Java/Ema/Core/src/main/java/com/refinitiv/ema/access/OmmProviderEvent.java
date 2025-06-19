/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.access;

/**
 * OmmProviderEvent encapsulates item identifiers.
 * 
 * <p>OmmProviderEvent is used to convey item identifiers and OmmProvider to application.<br>
 * OmmProviderEvent is returned through OmmProviderClient callback methods.</p>
 * 
 * <p>OmmProviderEvent is read only interface. It is used for item identification only.</p>
 * 
 * @see OmmProvider
 * @see OmmProviderClient
 */

public interface OmmProviderEvent 
{
	/**
	 * Returns a unique item identifier (a.k.a., item handle) associated by EMA with an open item stream.<br>
	 * Item identifier is returned from all OmmProvider registerClient() methods.
	 * 
	 * @return item identifier or handle
	 */
	public long handle();

	/**
	 * Returns an identifier (a.k.a., closure) associated with an open stream by OmmProvider application.<br>
	 * Application associates the closure with an open item stream on<br>
	 * OmmProvider registerClient(... , ... , Object closure, ...) methods
	 * 
	 * @return closure value
	 */
	public Object closure();
	
	/**
	 * Returns OmmProvider instance for this event.
	 * 
	 * @return reference to OmmProvider
	 */
	public OmmProvider provider();
	
	/**
	 * Returns a unique client identifier (a.k.a., client handle) associated by EMA with a connected client.
	 * 
	 * @return client identifier or handle
	 */
	public long clientHandle();

	/**
	 * Returns the channel information for the channel associated with the event
	 *
	 * @return the channel information associated with the event
	 */
	public ChannelInformation channelInformation();
}
