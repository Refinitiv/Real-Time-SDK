///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;

/**
 * IOCtlCode class provides I/O codes for modifying I/O values programmatically using the modifyIOCtl() method of provided by OmmProvider and OmmConsumer classes.
 * 
 * <p>IOCtlCode defines numeric representation of I/O codes to modify option for a particular channel or server.</p>
 * 
 * Code snippet:
 * <pre>
 * OmmProvider provider; // This provider variable is created for Interactive Provider applications.
 * 
 * provider.modifyIOCtl(IOCtlCode.NUM_GUARANTEED_BUFFERS, 500, event.handle()); // Modifies the number of guaranteed buffers for the underlying channel.
 * </pre> 
 * 
 */

public final class IOCtlCode
{
	private IOCtlCode()
	{
		throw new AssertionError();
	}
	
	/**
	 * Used for changing the max number of buffers. This option is used for IProvider applications only.
	 */
	public final static int MAX_NUM_BUFFERS = 1;
	
	/**
	 * Used for changing the number of guaranteed buffers.
	 */
	public static final int NUM_GUARANTEED_BUFFERS = 2;
	
	/**
	 * Used to set the upper buffer usage threshold.
	 */
	public static final int HIGH_WATER_MARK = 3;

	/**
	 * Allows to change the TCP receive buffer size
	 * associated with the connection. Value is an int.
	 * <br>
	 * Please note that if the value is larger than 64K, the value needs to
	 * be specified before the socket is connected to the remote peer.
	 */
	public static final int SYSTEM_READ_BUFFERS = 4;

	/**
	 * Allows to change the TCP send buffer size associated
	 * with the connection. Value is an int.
	 */
	public static final int SYSTEM_WRITE_BUFFERS = 5;
	
	/**
	 * Used to increase or decrease the number of server shared pool buffers. This option is used for IProvider applications only. 
	 */
	public static final int SERVER_NUM_POOL_BUFFERS = 8;
	
	/**
	 * When compression is on, this value is the smallest size packet that will be compressed.
	 */
	public static final int COMPRESSION_THRESHOLD = 9;
}
