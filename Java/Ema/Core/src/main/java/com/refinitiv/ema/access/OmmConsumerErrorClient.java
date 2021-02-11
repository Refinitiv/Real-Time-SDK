///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;

/**
 * OmmConsumerErrorclient class provides callback mechanism used in place of exceptions.
 * 
 * <p>By default OmmConsumer class throws exceptions if a usage error occurs.
 * <br>Specifying OmmConsumerErrorClient on the constructor of OmmConsumer overwrites this behaviour.
 * <br>Instead of throwing exceptions, respective callback method on OmmConsumerErrorClient will be invoked.</p>
 * 
 * @see OmmConsumer
 * @see OmmException
 * @see OmmInvalidUsageException
 * @see OmmInvalidHandleException
 */
public interface OmmConsumerErrorClient
{
	/**
	 * Invoked upon receiving an invalid handle.
	 * <br>Requires OmmConsumer constructor to have an OmmConsumerErrorClient.
	 * 
	 * @param handle value of the handle that is invalid
	 * @param text specifies associated error text
	 */
	public void onInvalidHandle(long handle, String text);

	/**
	 * Invoked in the case of invalid usage.
	 * <br>Requires OmmConsumer constructor to have an OmmConsumerErrorClient.
	 * 
	 * @param text specifies associated error text
	 */ 
	public default void onInvalidUsage(String text) {}
	
	/**
	 * Invoked in the case of invalid usage.
	 * <br>Requires OmmConsumer constructor to have an OmmConsumerErrorClient.
	 * <p>This method provides an additional error code for applications to check and handle the error appropriately.
	 * <br>The applications should override only one of the onInvalidUsage() method to avoid receiving two callback calls for an invalid usage error.</p>
	 * 
	 * @param text specifies associated error text
	 * @param errorCode specifies associated error code
	 */ 
	public default void onInvalidUsage(String text, int errorCode) {}

	/**
	 * Invoked in case of error during RWF/JSON conversion.
	 * @param consumerSessionInfo specifies associated info about converter session of consumer.
	 * @param errorCode specifies associated error code.
	 * @param text specifies associated error text.
	 */
	default void onJsonConverterError(ConsumerSessionInfo consumerSessionInfo, int errorCode, String text) {
	}
}