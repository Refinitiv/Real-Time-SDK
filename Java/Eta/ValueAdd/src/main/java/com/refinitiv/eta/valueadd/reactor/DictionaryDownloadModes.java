/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

/**
 * Available methods for automatically retrieving dictionary messages
 * from a provider.
 * 
 * @see ConsumerRole
 */
public class DictionaryDownloadModes
{
	/** Do not automatically request dictionary messages. */
    public static final int NONE = 0;
    /** Reactor searches DirectoryMsgs for the RWFFld and RWFEnum dictionaries.
	 * Once found, it will request the dictionaries and close their streams once all
	 * necessary data is retrieved. This option is for use with an ADS. */
    public static final int FIRST_AVAILABLE = 1;
}
