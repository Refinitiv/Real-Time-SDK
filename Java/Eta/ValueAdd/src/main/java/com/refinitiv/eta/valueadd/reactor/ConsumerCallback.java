/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

/**
 * Callback used for processing all consumer events and messages.
 * 
 * @see ReactorChannelEventCallback
 * @see DefaultMsgCallback
 * @see RDMLoginMsgCallback
 * @see RDMDirectoryMsgCallback
 * @see RDMDictionaryMsgCallback
 */
public interface ConsumerCallback extends ReactorChannelEventCallback, DefaultMsgCallback,
        RDMLoginMsgCallback, RDMDirectoryMsgCallback, RDMDictionaryMsgCallback
{

}
