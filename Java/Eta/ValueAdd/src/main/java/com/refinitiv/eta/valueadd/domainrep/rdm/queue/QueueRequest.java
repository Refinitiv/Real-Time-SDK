/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.domainrep.rdm.queue;

import com.refinitiv.eta.codec.Buffer;

/** The queue request message. Used by an OMM Consumer to connect to a QueueProvider. */
public interface QueueRequest extends QueueMsg
{
    
    /**
     * Returns the source name of the queue request message. Use to
     * retrieve and set the source name of the queue request message.
     *
     * @return the buffer
     */
    public Buffer sourceName();
    
    /**
     * Sets the source name of the queue request message. Use to
     *
     * @param sourceName the source name
     */   
    public void sourceName(Buffer sourceName);	
	    
}
