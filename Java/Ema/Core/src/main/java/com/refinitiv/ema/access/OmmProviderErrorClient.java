/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.access;

/**
 * OmmProviderErrorclient class provides callback mechanism used in place of exceptions.
 * 
 * <p>By default OmmProvider class throws exceptions if a usage error occurs.
 * <br>Specifying OmmProviderErrorClient on the constructor of OmmProvider overwrites this behavior.
 * <br>Instead of throwing exceptions, respective callback method on OmmProviderErrorClient will be invoked.</p>
 * 
 * @see OmmProvider
 * @see OmmException
 * @see OmmInvalidUsageException
 * @see OmmInvalidHandleException
 */

public interface OmmProviderErrorClient
{
	/**
	 * Invoked upon receiving an invalid handle.
	 * <br>Requires OmmProvider constructor to have an OmmProviderErrorClient.
	 * 
	 * @param handle value of the handle that is invalid
	 * @param text specifies associated error text
	 */
	public void onInvalidHandle(long handle, String text);

	/**
	 * Invoked in the case of invalid usage.
	 * <br>Requires OmmProvider constructor to have an OmmProviderErrorClient.
	 * 
	 * @param text specifies associated error text
	 */ 
	public default void onInvalidUsage(String text) {}

	/**
	 * Invoked in the case of invalid usage.
	 * <br>Requires OmmProvider constructor to have an OmmProviderErrorClient.
	 * <p>This method provides an additional error code for applications to check and handle the error appropriately.
	 * <br>The applications should override only one of the onInvalidUsage() method to avoid receiving two callback calls for an invalid usage error.</p>
	 * 
	 * @param text specifies associated error text
	 * @param errorCode specifies associated error code
	 */ 
	public default void onInvalidUsage(String text, int errorCode) {}

	/**
	 * In
	 * <br>Requires OmmProvider constructor to have an OmmProviderErrorClient.
	 * @param providerSessionInfo specifies associated info about converter session of provider.
	 * @param errorCode specifies associated error code.
	 * @param text specifies associated error text.
	 */
	public default void onJsonConverterError(ProviderSessionInfo providerSessionInfo, int errorCode, String text) {
	}
}
