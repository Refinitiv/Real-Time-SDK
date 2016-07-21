/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2016. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

package com.thomsonreuters.ema.access;

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
	public void onInvalidUsage(String text);

}
