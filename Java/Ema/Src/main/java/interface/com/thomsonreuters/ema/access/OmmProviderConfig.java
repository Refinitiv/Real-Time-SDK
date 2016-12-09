///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2016. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

/**
 * OmmProviderConfig is a base interface for the OmmNiProviderConfig and OmmIProviderConfig.
 * 
 * @see OmmProvider
 * @see OmmNiProviderConfig
 * @see OmmIProviderConfig
 */

public interface OmmProviderConfig 
{
	public static class ProviderRole
	{
		/**
		 * indicates a non interactive provider configuration
		 */
		public static final int NON_INTERACTIVE = 0;
		
		/**
		 * indicates interactive provider configuration
		 */
		public static final int INTERACTIVE = 1;
	}
	
	/**
	 * Returns Provider's role
	 * 
	 * @return role of this OmmProviderConfig instance
	 */
	public abstract int providerRole();
}
